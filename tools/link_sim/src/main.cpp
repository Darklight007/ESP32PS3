// Fake PSU for the PSU Link (same packets as src/psu_link.cpp), on a bare
// ESP32-S3. Answers scans, accepts link requests (auto or asked), follows as
// slave with a simulated load, or acts as master. Commands on the serial
// port (native USB and UART0), one per line:
//   status | scan | list | req <n> | auto 0|1 | y | n | unlink
//   set <V> <A>  (master setpoints) | on | off      (master output)
//   load <ohm>   (slave load, 0 = open) | trip (slave output off) | mute 0|1 (stop answering)
#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_now.h>

namespace
{
    constexpr uint32_t kMagic = 0x4C555350;
    constexpr uint8_t kChannel = 1;
    enum : uint8_t { PAIR_REQ = 1, PAIR_ACK, STATE, STATUS, UNPAIR, SCAN, HELLO, PAIR_NAK };
    struct __attribute__((packed)) Pkt
    {
        uint32_t magic;
        uint8_t type, seq, on, flags;
        float v, i;
    };
    const uint8_t kBcast[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

    enum Role { READY, MASTER, SLAVE } role = READY;
    uint8_t peer[6], selfMac[6];
    bool hasPeer = false, autoAccept = true, mute = false;
    // slave
    float sV = 0, sI = 0, loadOhm = 10;
    bool sOn = false, sLastOn = false;
    // master
    float mV = 5, mI = 1;
    bool mOn = false;
    Pkt lastStatus = {};
    uint32_t lastRx = 0, rxCount = 0;
    // scan / requests
    uint8_t found[6][6];
    int nFound = 0;
    uint32_t scanUntil = 0;
    uint8_t inMac[6];
    bool inPending = false;
    uint8_t askMac[6];

    QueueHandle_t q;
    struct Rx { Pkt p; uint8_t mac[6]; };

    void out(const char *fmt, ...)
    {
        char b[200];
        va_list a;
        va_start(a, fmt);
        vsnprintf(b, sizeof(b), fmt, a);
        va_end(a);
        Serial.print(b);
        Serial0.print(b);
    }
    const char *name(const uint8_t *m)
    {
        static char b[2][12];
        static int k = 0;
        k ^= 1;
        snprintf(b[k], 12, "PSU-%02X%02X%02X", m[3], m[4], m[5]);
        return b[k];
    }
    void addPeer(const uint8_t *m)
    {
        if (esp_now_is_peer_exist(m))
            return;
        esp_now_peer_info_t pi = {};
        memcpy(pi.peer_addr, m, 6);
        pi.channel = kChannel;
        pi.ifidx = WIFI_IF_STA;
        esp_now_add_peer(&pi);
    }
    void send(const uint8_t *m, uint8_t type, uint8_t on = 0, float v = 0, float i = 0)
    {
        static uint8_t seq;
        addPeer(m);
        Pkt p = {kMagic, type, seq++, on, 0, v, i};
        esp_now_send(m, (uint8_t *)&p, sizeof(p));
    }
    void onRecv(const uint8_t *mac, const uint8_t *d, int len)
    {
        if (len != sizeof(Pkt))
            return;
        Rx r;
        memcpy(&r.p, d, sizeof(Pkt));
        memcpy(r.mac, mac, 6);
        if (r.p.magic == kMagic)
            xQueueSend(q, &r, 0);
    }
    void link(const uint8_t *m, Role r)
    {
        memcpy(peer, m, 6);
        hasPeer = true;
        role = r;
        addPeer(m);
        out("LINKED to %s as %s\n", name(m), r == MASTER ? "MASTER" : "SLAVE");
    }

    // simulated slave output: CV at sV, or CC when the load wants more than sI
    void slaveOutput(float &v, float &i)
    {
        if (!sOn) { v = 0; i = 0; return; }
        if (loadOhm <= 0) { v = sV; i = 0; return; }
        i = sV / loadOhm;
        v = sV;
        if (i > sI) { i = sI; v = sI * loadOhm; }
    }

    void handle(const Rx &r)
    {
        const Pkt &p = r.p;
        if (mute)
            return;
        switch (p.type)
        {
        case SCAN: send(r.mac, HELLO); return;
        case HELLO:
            if (millis() < scanUntil && nFound < 6)
            {
                for (int k = 0; k < nFound; k++)
                    if (!memcmp(found[k], r.mac, 6)) return;
                memcpy(found[nFound], r.mac, 6);
                out("found %d: %s\n", nFound, name(r.mac));
                nFound++;
            }
            return;
        case PAIR_REQ:
            out("%s asks to be followed\n", name(r.mac));
            if (autoAccept) { send(r.mac, PAIR_ACK); link(r.mac, SLAVE); }
            else { memcpy(inMac, r.mac, 6); inPending = true; out("answer: y / n\n"); }
            return;
        case PAIR_ACK: if (!memcmp(r.mac, askMac, 6)) link(r.mac, MASTER); return;
        case PAIR_NAK: out("%s refused\n", name(r.mac)); return;
        }
        if (!hasPeer || memcmp(r.mac, peer, 6))
            return;
        lastRx = millis();
        rxCount++;
        if (p.type == UNPAIR) { hasPeer = false; role = READY; out("unlinked by partner\n"); return; }
        if (role == SLAVE && p.type == STATE)
        {
            if (p.v != sV || p.i != sI || (p.on != 0) != sLastOn)
                out("STATE V=%.4f I=%.4f out=%s\n", p.v, p.i, p.on ? "ON" : "OFF");
            sV = p.v;
            sI = p.i;
            if ((p.on != 0) != sLastOn) { sLastOn = p.on; sOn = sLastOn; } // edges, like the PSU
            float v, i;
            slaveOutput(v, i);
            send(peer, STATUS, sOn, v, i);
        }
        if (role == MASTER && p.type == STATUS)
        {
            if (p.on != lastStatus.on)
                out("slave output %s\n", p.on ? "ON" : "OFF");
            lastStatus = p;
        }
    }

    void command(String c)
    {
        c.trim();
        float a = 0, b = 0;
        if (c == "status")
        {
            out("%s role=%s peer=%s connected=%d rx=%lu auto=%d mute=%d\n", name(selfMac),
                role == MASTER ? "MASTER" : role == SLAVE ? "SLAVE" : "READY", hasPeer ? name(peer) : "-",
                hasPeer && millis() - lastRx < 300, (unsigned long)rxCount, autoAccept, mute);
            if (role == SLAVE) { float v, i; slaveOutput(v, i); out("slave set %.4fV %.4fA out=%s load=%.1f ohm -> %.4fV %.4fA\n", sV, sI, sOn ? "ON" : "OFF", loadOhm, v, i); }
            if (role == MASTER) out("master set %.4fV %.4fA out=%s | slave %.4fV %.4fA %s\n", mV, mI, mOn ? "ON" : "OFF", lastStatus.v, lastStatus.i, lastStatus.on ? "ON" : "OFF");
        }
        else if (c == "scan") { nFound = 0; scanUntil = millis() + 3000; out("scanning 3 s\n"); }
        else if (c == "list") for (int k = 0; k < nFound; k++) out("%d: %s\n", k, name(found[k]));
        else if (c.startsWith("req ")) { int k = c.substring(4).toInt(); if (k < nFound) { memcpy(askMac, found[k], 6); send(askMac, PAIR_REQ); out("asked %s, confirm on the PSU\n", name(askMac)); } }
        else if (c.startsWith("auto ")) autoAccept = c.substring(5).toInt();
        else if ((c == "y" || c == "n") && inPending) { inPending = false; send(inMac, c == "y" ? PAIR_ACK : PAIR_NAK); if (c == "y") link(inMac, SLAVE); }
        else if (c == "unlink") { if (hasPeer) send(peer, UNPAIR); hasPeer = false; role = READY; out("unlinked\n"); }
        else if (sscanf(c.c_str(), "set %f %f", &a, &b) == 2) { mV = a; mI = b; }
        else if (c == "on") mOn = true;
        else if (c == "off") mOn = false;
        else if (sscanf(c.c_str(), "load %f", &a) == 1) loadOhm = a;
        else if (c == "trip") { sOn = false; out("slave output tripped OFF\n"); }
        else if (c.startsWith("mute ")) mute = c.substring(5).toInt();
        else out("? status scan list req n auto y n unlink set V A on off load R trip mute\n");
    }
}

void setup()
{
    Serial.begin(115200);
    Serial0.begin(115200);
    q = xQueueCreate(16, sizeof(Rx));
    WiFi.mode(WIFI_STA);
    esp_wifi_set_ps(WIFI_PS_NONE);
    esp_wifi_set_channel(kChannel, WIFI_SECOND_CHAN_NONE);
    esp_now_init();
    esp_now_register_recv_cb(onRecv);
    addPeer(kBcast);
    esp_wifi_get_mac(WIFI_IF_STA, selfMac);
    out("\nLINK SIM %s ready (auto accept on). Type ? for commands.\n", name(selfMac));
}

void loop()
{
    Rx r;
    while (xQueueReceive(q, &r, 0))
        handle(r);
    static String line[2];
    Stream *ports[2] = {&Serial, &Serial0};
    for (int k = 0; k < 2; k++)
        while (ports[k]->available())
        {
            char ch = ports[k]->read();
            if (ch == '\n' || ch == '\r') { if (line[k].length()) command(line[k]); line[k] = ""; }
            else line[k] += ch;
        }
    static uint32_t lastSend, lastScan;
    const uint32_t now = millis();
    if (now < scanUntil && now - lastScan >= 500) { lastScan = now; send(kBcast, SCAN); }
    if (role == MASTER && hasPeer && !mute && now - lastSend >= 20) { lastSend = now; send(peer, STATE, mOn, mV, mI); }
    static bool lost = false;
    const bool isLost = hasPeer && lastRx && now - lastRx > 300;
    if (isLost && !lost) out("link lost\n");
    lost = isLost;
    delay(1);
}
