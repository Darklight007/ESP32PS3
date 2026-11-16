// PSU-to-PSU link over ESP-NOW. See psu_link.h.
//
// Radio: WiFi can't start at runtime with the stock buffer counts (internal
// RAM). Here it starts with 2 static RX buffers, dynamic TX and no
// aggregation (~9 KB internal), from a reserve kept aside at boot
// (radioReserve(), after Task_ADC). ESP-NOW only, no router, channel 1.
//
// Pairing: Scan broadcasts SCAN, every enabled unit answers HELLO. Picking
// one sends it PAIR_REQ; it asks its user, answers PAIR_ACK or PAIR_NAK. The
// asking unit becomes master, the other slave; both save the partner.
//
// Threads: the receive callback runs in the WiFi task and only queues the
// packet. Everything else runs in linkLoop() on Core 1.
#include "psu_link.h"
#include "device.hpp"
#include "event_log.h"
#include <Preferences.h>
#include <esp_wifi.h>
#include <esp_now.h>
#include <esp_event.h>
#include <esp_heap_caps.h>
#include <esp_mac.h>
#include <cmath>
#include <cstring>

extern Device PowerSupply;
void radioReserve();
static volatile bool s_logAllocFail = false; // print failed mallocs only while the radio starts

namespace
{
    constexpr uint32_t kMagic = 0x4C555350; // "PSUL"
    constexpr uint8_t kChannel = 1;
    constexpr uint32_t kSendMs = 20;      // master state rate, 50 Hz
    constexpr uint32_t kLostMs = 300;     // no packet this long = link lost
    constexpr uint32_t kScanMs = 3000;
    constexpr uint32_t kAskMs = 20000;    // time to answer a link request

    enum : uint8_t { PAIR_REQ = 1, PAIR_ACK, STATE, STATUS, UNPAIR, SCAN, HELLO, PAIR_NAK };

    struct __attribute__((packed)) Pkt
    {
        uint32_t magic;
        uint8_t type;
        uint8_t seq;
        uint8_t on;    // output on (STATE: master wants on; STATUS: slave is on)
        uint8_t flags; // STATUS: bit0 = slave mA range
        float v;       // STATE: slave V-Set; STATUS: measured V
        float i;       // STATE: slave I-Limit (A); STATUS: measured I (A)
    };

    const uint8_t kBcast[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

    LinkRole s_role = LinkRole::OFF;
    uint8_t s_peer[6] = {};
    bool s_hasPeer = false;
    float s_ratio = 100;
    bool s_radio = false;
    uint32_t s_saveAt = 0;
    bool s_ovr = false, s_ovrOn = false; // linkOverride()
    float s_ovrV = 0, s_ovrI = 0;
    char s_selfName[12] = "PSU-?";

    // scan
    struct Found { uint8_t mac[6]; };
    constexpr int kMaxFound = 6;
    Found s_found[kMaxFound];
    int s_nFound = 0;
    uint32_t s_foundVer = 0;
    uint32_t s_scanUntil = 0;
    // request this unit sent / received
    uint8_t s_askMac[6];
    uint32_t s_askUntil = 0;      // waiting for the other unit's answer
    uint8_t s_inMac[6];
    uint32_t s_inUntil = 0;       // other unit waiting for our answer
    char s_note[40] = "";         // last pairing result, for the status line

    // receive queue (WiFi task -> Core 1)
    struct Rx { Pkt p; uint8_t mac[6]; };
    constexpr int kQ = 8;
    Rx s_q[kQ];
    volatile uint8_t s_qHead = 0, s_qTail = 0;
    portMUX_TYPE s_mux = portMUX_INITIALIZER_UNLOCKED;

    uint32_t s_lastRx = 0;
    bool s_wasConnected = false;
    Pkt s_peerState = {};
    uint32_t s_rxCount = 0, s_txFail = 0, s_txOk = 0, s_rxAll = 0;
    esp_err_t s_lastSendErr = ESP_OK;

    void *s_reserve[8] = {}; // ~20 KB: the radio uses ~17 KB of internal RAM

    void macName(const uint8_t *m, char *out) { snprintf(out, 12, "PSU-%02X%02X%02X", m[3], m[4], m[5]); }

    void save()
    {
        Preferences p;
        p.begin("psulink", false);
        p.putUChar("role", (uint8_t)s_role);
        p.putBytes("peer", s_peer, 6);
        p.putBool("hasPeer", s_hasPeer);
        p.putFloat("ratio", s_ratio);
        p.end();
    }

    void onRecv(const uint8_t *srcMac, const uint8_t *data, int len)
    {
        if (len != sizeof(Pkt))
            return;
        Pkt p;
        memcpy(&p, data, sizeof(p));
        if (p.magic != kMagic)
            return;
        s_rxAll++;
        portENTER_CRITICAL(&s_mux);
        const uint8_t next = (s_qHead + 1) % kQ;
        if (next != s_qTail) // full: drop (STATE repeats every 20 ms)
        {
            s_q[s_qHead].p = p;
            memcpy(s_q[s_qHead].mac, srcMac, 6);
            s_qHead = next;
        }
        portEXIT_CRITICAL(&s_mux);
    }

    void onSent(const uint8_t *, esp_now_send_status_t st)
    {
        if (st != ESP_NOW_SEND_SUCCESS)
            s_txFail++;
        else
            s_txOk++;
    }

    void addPeer(const uint8_t *mac)
    {
        if (esp_now_is_peer_exist(mac))
            return;
        esp_now_peer_info_t pi = {};
        memcpy(pi.peer_addr, mac, 6);
        pi.channel = kChannel;
        pi.ifidx = WIFI_IF_STA;
        pi.encrypt = false;
        esp_now_add_peer(&pi);
    }

    bool radioStart()
    {
        if (s_radio)
            return true;
        for (auto &r : s_reserve)
        {
            heap_caps_free(r);
            r = nullptr;
        }
        esp_event_loop_create_default(); // may already exist
        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        cfg.static_rx_buf_num = 2;
        cfg.dynamic_rx_buf_num = 8;
        cfg.tx_buf_type = 1; // dynamic
        cfg.dynamic_tx_buf_num = 8;
        // cache_tx_buf_num stays default (PSRAM): at 0 every esp_now_send()
        // failed with ESP_ERR_ESPNOW_NO_MEM
        cfg.ampdu_rx_enable = 0;
        cfg.ampdu_tx_enable = 0;
        cfg.amsdu_tx_enable = 0;
        cfg.nvs_enable = 0;
        const char *stepName = "init";
        s_logAllocFail = true;
        esp_err_t e = esp_wifi_init(&cfg);
        if (e == ESP_OK)
        {
            esp_wifi_set_storage(WIFI_STORAGE_RAM);
            esp_wifi_set_mode(WIFI_MODE_STA);
            stepName = "start";
            e = esp_wifi_start();
        }
        if (e == ESP_OK)
        {
            esp_wifi_set_ps(WIFI_PS_NONE); // low latency
            esp_wifi_set_channel(kChannel, WIFI_SECOND_CHAN_NONE);
            stepName = "esp_now";
            e = esp_now_init();
        }
        s_logAllocFail = false;
        if (e != ESP_OK)
        {
            Serial.printf("[LINK] radio %s failed: %d (DMA free %u, largest %u)\n", stepName, e,
                          (unsigned)heap_caps_get_free_size(MALLOC_CAP_DMA),
                          (unsigned)heap_caps_get_largest_free_block(MALLOC_CAP_DMA));
            Serial.printf("[LINK] internal free %u, largest %u\n",
                          (unsigned)heap_caps_get_free_size(MALLOC_CAP_INTERNAL),
                          (unsigned)heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL));
            esp_wifi_stop();
            esp_wifi_deinit();
            radioReserve();
            return false;
        }
        esp_now_register_recv_cb(onRecv);
        esp_now_register_send_cb(onSent);
        addPeer(kBcast);
        if (s_hasPeer)
            addPeer(s_peer);
        s_radio = true;
        return true;
    }

    void radioStop()
    {
        if (!s_radio)
            return;
        esp_now_deinit();
        esp_wifi_stop();
        esp_wifi_deinit();
        s_radio = false;
        radioReserve();
    }

    void send(const uint8_t *mac, uint8_t type, uint8_t on = 0, uint8_t flags = 0, float v = 0, float i = 0)
    {
        static uint8_t seq = 0;
        if (!s_radio)
            return;
        addPeer(mac);
        Pkt p = {kMagic, type, seq++, on, flags, v, i};
        s_lastSendErr = esp_now_send(mac, (const uint8_t *)&p, sizeof(p));
    }

    double vSet() { auto &V = PowerSupply.Voltage; return (V.adjValue - V.adjOffset) / V.adjFactor; }
    double iSet() { auto &I = PowerSupply.Current; return (I.adjValue - I.adjOffset) / I.adjFactor; }
    bool outputOn() { return PowerSupply.getStatus() != DEVICE::OFF; }
    bool outputArmed() { return outputOn() || lv_obj_has_state(PowerSupply.powerSwitch.btn, LV_STATE_CHECKED); }

    void outputSet(bool on)
    {
        if (on && !outputArmed())
            PowerSupply.toggle(); // honours the Output Delay
        else if (!on && outputArmed())
            PowerSupply.turn(SWITCH::OFF);
    }

    void setPeer(const uint8_t *mac, LinkRole role)
    {
        if (s_hasPeer && memcmp(s_peer, mac, 6) != 0 && esp_now_is_peer_exist(s_peer))
            esp_now_del_peer(s_peer);
        memcpy(s_peer, mac, 6);
        s_hasPeer = true;
        s_role = role;
        s_lastRx = 0;
        addPeer(mac);
        save();
    }

    void dropPeer()
    {
        if (s_hasPeer && esp_now_is_peer_exist(s_peer))
            esp_now_del_peer(s_peer);
        s_hasPeer = false;
        s_lastRx = 0;
        if (s_role != LinkRole::OFF)
            s_role = LinkRole::READY;
        save();
    }

    // ---- slave: follow the master's STATE ----
    void slaveApply(const Pkt &p, bool first)
    {
        static float lastV = NAN, lastI = NAN;
        static bool lastOn = false;
        auto &V = PowerSupply.Voltage;
        auto &I = PowerSupply.Current;
        const double vMax = std::min((double)V.maxValue / V.adjFactor, (double)V.setCeiling);
        const double iMax = (double)I.maxValue / I.adjFactor;
        if (first || p.v != lastV)
        {
            lastV = p.v;
            V.SetUpdate(std::clamp((double)p.v, 0.0, vMax) * V.adjFactor + V.adjOffset);
        }
        if (first || p.i != lastI)
        {
            lastI = p.i;
            I.SetUpdate(std::clamp((double)p.i, 0.0, iMax) * I.adjFactor + I.adjOffset);
        }
        // output follows the master's switching (edges): a slave that tripped
        // stays off until the master switches again. On (re)connect it may
        // only turn off: a link that comes back must not switch the output on.
        if (first)
        {
            lastOn = p.on != 0;
            if (!lastOn)
                outputSet(false);
        }
        else if ((p.on != 0) != lastOn)
        {
            lastOn = p.on != 0;
            outputSet(lastOn);
        }
    }

    void handlePacket(const Pkt &p, const uint8_t *mac)
    {
        char nm[12];
        macName(mac, nm);
        switch (p.type)
        {
        case SCAN: // any enabled unit answers, linked or not
            send(mac, HELLO);
            return;
        case HELLO:
            if (millis() < s_scanUntil)
            {
                for (int k = 0; k < s_nFound; k++)
                    if (memcmp(s_found[k].mac, mac, 6) == 0)
                        return;
                if (s_nFound < kMaxFound)
                {
                    memcpy(s_found[s_nFound++].mac, mac, 6);
                    s_foundVer++;
                }
            }
            return;
        case PAIR_REQ: // someone wants this unit to follow it: ask the user
            memcpy(s_inMac, mac, 6);
            s_inUntil = millis() + kAskMs;
            return;
        case PAIR_ACK:
            if (s_askUntil && memcmp(mac, s_askMac, 6) == 0)
            {
                s_askUntil = 0;
                setPeer(mac, LinkRole::MASTER);
                s_note[0] = 0;
            }
            return;
        case PAIR_NAK:
            if (s_askUntil && memcmp(mac, s_askMac, 6) == 0)
            {
                s_askUntil = 0;
                snprintf(s_note, sizeof(s_note), "%s said no.", nm);
            }
            return;
        }

        if (!s_hasPeer || memcmp(mac, s_peer, 6) != 0)
            return;
        if (p.type == UNPAIR)
        {
            dropPeer();
            snprintf(s_note, sizeof(s_note), "%s unlinked.", nm);
            return;
        }
        const bool first = !linkConnected();
        s_lastRx = millis();
        s_rxCount++;
        s_peerState = p;
        if (s_role == LinkRole::SLAVE && p.type == STATE)
        {
            slaveApply(p, first);
            float iMeas = PowerSupply.Current.measured.value;
            if (PowerSupply.mA_Active)
                iMeas /= 1000.0f;
            send(s_peer, STATUS, outputOn(), PowerSupply.mA_Active ? 1 : 0,
                 PowerSupply.Voltage.measured.value, iMeas);
        }
    }

    void linkLost(bool log = true)
    {
        if (outputArmed())
        {
            outputSet(false);
            if (log)
                eventLog(Event::LINK_LOST, (float)vSet(), (uint8_t)s_role);
            Serial.println("[LINK] partner lost: output OFF");
        }
    }
}

static void allocFail(size_t size, uint32_t caps, const char *fn)
{
    if (s_logAllocFail)
        esp_rom_printf("[ALLOC FAIL] %u bytes caps 0x%x in %s, largest %u\n", (unsigned)size, (unsigned)caps, fn,
                   (unsigned)heap_caps_get_largest_free_block(caps));
}

void radioReserve()
{
    static bool hooked = false;
    if (!hooked)
        hooked = heap_caps_register_failed_alloc_callback(allocFail) == ESP_OK;
    // Free internal RAM is in pieces under ~8 KB by now; WiFi needs, among
    // others, 2216-byte RX buffers (DMA). Blocks of 2.5 KB fit the pieces
    // (4 KB ones mostly did not, so little was reserved) and each one freed
    // holds an RX buffer. ~20 KB in all.
    int n = 0;
    for (auto &r : s_reserve)
        if (!r && (r = heap_caps_malloc(2560, MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA)))
            n++;
    (void)n;
}

void linkBegin()
{
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    macName(mac, s_selfName);
    Preferences p;
    p.begin("psulink", true);
    s_role = (LinkRole)p.getUChar("role", 0);
    if (s_role > LinkRole::READY)
        s_role = LinkRole::OFF;
    s_hasPeer = p.getBool("hasPeer", false) && p.getBytes("peer", s_peer, 6) == 6;
    s_ratio = std::clamp(p.getFloat("ratio", 100), 1.0f, 100.0f);
    p.end();
    if ((s_role == LinkRole::MASTER || s_role == LinkRole::SLAVE) && !s_hasPeer)
        s_role = LinkRole::READY;
    if (s_role != LinkRole::OFF)
        radioStart();
}

void linkLoop()
{
    const uint32_t now = millis();
    if (s_saveAt && (int32_t)(now - s_saveAt) >= 0)
    {
        s_saveAt = 0;
        save();
    }
    if (!s_radio)
        return;
    while (s_qTail != s_qHead)
    {
        Rx r;
        portENTER_CRITICAL(&s_mux);
        r = s_q[s_qTail];
        s_qTail = (s_qTail + 1) % kQ;
        portEXIT_CRITICAL(&s_mux);
        handlePacket(r.p, r.mac);
    }
    if (s_askUntil && now > s_askUntil)
    {
        s_askUntil = 0;
        snprintf(s_note, sizeof(s_note), "No answer from the other PSU.");
    }
    if (s_inUntil && now > s_inUntil)
        s_inUntil = 0;

    static uint32_t lastSend = 0, lastScan = 0;
    if (now < s_scanUntil && now - lastScan >= 500)
    {
        lastScan = now;
        send(kBcast, SCAN);
    }
    if (now - lastSend >= kSendMs)
    {
        lastSend = now;
        if (s_role == LinkRole::MASTER && s_hasPeer)
        {
            if (s_ovr)
                send(s_peer, STATE, s_ovrOn, 0, s_ovrV, s_ovrI);
            else
                send(s_peer, STATE, outputArmed(), 0, (float)(vSet() * s_ratio / 100.0),
                     (float)(iSet() * s_ratio / 100.0));
        }
    }

    const bool conn = linkConnected();
    if (s_wasConnected && !conn)
        linkLost();
    s_wasConnected = conn;

    // master: the slave turned itself off (fault/trip) while tracking
    static uint32_t mismatchSince = 0;
    if (s_role == LinkRole::MASTER && conn && outputOn() && !s_peerState.on && !s_ovr)
    {
        const uint32_t grace = PowerSupply.settingParameters.outputDelayMs + 1500;
        if (!mismatchSince)
            mismatchSince = now;
        else if (now - mismatchSince > grace)
        {
            mismatchSince = 0;
            outputSet(false);
            eventLog(Event::LINK_PEER_OFF, s_peerState.v, 0);
            Serial.println("[LINK] slave output off: master OFF too");
        }
    }
    else
        mismatchSince = 0;
}

void linkEnable(bool on)
{
    if (on == linkEnabled())
        return;
    if (on)
    {
        s_role = s_hasPeer ? s_role : LinkRole::READY;
        if (s_role == LinkRole::OFF)
            s_role = LinkRole::READY;
        if (!radioStart())
            s_role = LinkRole::OFF;
    }
    else
    {
        if (linkConnected())
            linkLost();
        radioStop();
        s_role = LinkRole::OFF;
        s_wasConnected = false;
    }
    save();
}

bool linkEnabled() { return s_role != LinkRole::OFF; }
LinkRole linkRole() { return s_role; }

void linkScan()
{
    if (!s_radio)
        return;
    s_nFound = 0;
    s_foundVer++;
    s_note[0] = 0;
    s_scanUntil = millis() + kScanMs;
}

bool linkScanning() { return millis() < s_scanUntil; }

int linkFound(char names[][12], int max)
{
    const int n = std::min(s_nFound, max);
    for (int k = 0; k < n; k++)
        macName(s_found[k].mac, names[k]);
    return n;
}

uint32_t linkFoundVersion() { return s_foundVer; }

void linkRequest(int idx)
{
    if (idx < 0 || idx >= s_nFound)
        return;
    s_scanUntil = 0;
    memcpy(s_askMac, s_found[idx].mac, 6);
    s_askUntil = millis() + kAskMs;
    char nm[12];
    macName(s_askMac, nm);
    s_note[0] = 0;
    send(s_askMac, PAIR_REQ);
}

void linkTestIncoming() // SCPI LINK:TEST: fake request, to check the prompt
{
    const uint8_t fake[6] = {0x02, 0, 0, 0x12, 0x34, 0x56};
    memcpy(s_inMac, fake, 6);
    s_inUntil = millis() + kAskMs;
}

bool linkIncoming(char *name)
{
    if (!s_inUntil)
        return false;
    macName(s_inMac, name);
    return true;
}

void linkAnswer(bool accept)
{
    if (!s_inUntil)
        return;
    s_inUntil = 0;
    send(s_inMac, accept ? PAIR_ACK : PAIR_NAK);
    if (accept)
    {
        setPeer(s_inMac, LinkRole::SLAVE);
        s_note[0] = 0;
    }
}

void linkUnpair()
{
    if (s_hasPeer)
        send(s_peer, UNPAIR);
    if (linkConnected())
        linkLost(false); // asked for: output off, nothing to log
    s_wasConnected = false;
    dropPeer();
    s_note[0] = 0;
}

void linkSetRatio(float pct)
{
    s_ratio = std::clamp(pct, 1.0f, 100.0f);
    s_saveAt = millis() + 2000; // a slider sends every step: save once it settles
}

float linkRatio() { return s_ratio; }

bool linkConnected() { return s_radio && s_hasPeer && s_lastRx && millis() - s_lastRx < kLostMs; }

const char *linkSelfName() { return s_selfName; }

bool linkLinked(char *name)
{
    if (s_hasPeer && name)
        macName(s_peer, name);
    return s_hasPeer && s_role != LinkRole::OFF;
}

bool linkAsking(char *name)
{
    if (!s_askUntil)
        return false;
    macName(s_askMac, name);
    return true;
}

const char *linkNote() { return s_note; }

void linkOverride(bool active, float v, float i, bool on)
{
    s_ovr = active;
    s_ovrV = v;
    s_ovrI = i;
    s_ovrOn = active && on;
}

bool linkSlaveMilliamp() { return s_peerState.flags & 1; }

bool linkSlaveReading(float &v, float &i, bool &on)
{
    if (s_role != LinkRole::MASTER || !linkConnected())
        return false;
    v = s_peerState.v;
    i = s_peerState.i;
    on = s_peerState.on;
    return true;
}

String linkDiag()
{
    uint8_t ch = 0;
    wifi_second_chan_t sc;
    if (s_radio)
        esp_wifi_get_channel(&ch, &sc);
    char b[120];
    snprintf(b, sizeof(b), "radio %d,ch %u,tx ok %lu,tx fail %lu,send err %d,rx all %lu,rx peer %lu",
             s_radio, ch, (unsigned long)s_txOk, (unsigned long)s_txFail, s_lastSendErr,
             (unsigned long)s_rxAll, (unsigned long)s_rxCount);
    return String(b);
}

String linkStatus()
{
    static const char *roles[] = {"Off", "Master", "Slave", "Ready"};
    char b[160];
    char peer[12] = "";
    if (s_hasPeer)
        macName(s_peer, peer);
    const char *st = s_role == LinkRole::OFF ? "radio off"
                     : !s_radio              ? "radio failed"
                     : !s_hasPeer            ? "not linked"
                     : linkConnected()       ? "connected"
                                             : "waiting";
    int n = snprintf(b, sizeof(b), "%s,%s,%s,%.0f%%", roles[(int)s_role], st, s_hasPeer ? peer : "-", s_ratio);
    if (s_role == LinkRole::MASTER && linkConnected())
        n += snprintf(b + n, sizeof(b) - n, ",slave %.3fV %.3fA %s", s_peerState.v, s_peerState.i,
                      s_peerState.on ? "ON" : "OFF");
    if (s_note[0])
        n += snprintf(b + n, sizeof(b) - n, ",%s", s_note);
    return String(b);
}
