#include "event_log.h"
#include <Arduino.h>
#include <Preferences.h>
#include <esp_system.h>

namespace
{
    struct Entry
    {
        uint32_t uptimeS; // seconds since that boot (there is no clock)
        float value;
        uint16_t boot;    // boot counter, to tell sessions apart
        uint8_t type;
        uint8_t arg;
    };
    constexpr uint8_t kCap = 40;
    struct Store
    {
        uint16_t boots = 0;
        uint8_t head = 0;  // next slot to write
        uint8_t count = 0;
        Entry e[kCap] = {};
    } s_log;

    void save()
    {
        Preferences p;
        p.begin("evlog", false);
        p.putBytes("log", &s_log, sizeof(s_log));
        p.end();
    }

    const char *name(uint8_t t)
    {
        switch ((Event)t)
        {
        case Event::BOOT: return "BOOT";
        case Event::OUTPUT_LOST_POWER: return "OUTPUT_LOST_POWER";
        case Event::OUTPUT_LOST_THERMAL: return "OUTPUT_LOST_THERMAL";
        case Event::OVP: return "OVP";
        case Event::OCP: return "OCP";
        case Event::LINK_LOST: return "LINK_LOST";
        case Event::LINK_PEER_OFF: return "LINK_PEER_OFF";
        }
        return "?";
    }
}

void eventLogBegin()
{
    Preferences p;
    p.begin("evlog", true);
    Store tmp;
    if (p.getBytes("log", &tmp, sizeof(tmp)) == sizeof(tmp) && tmp.head < kCap && tmp.count <= kCap)
        s_log = tmp;
    p.end();
    s_log.boots++;
    eventLog(Event::BOOT, 0, (uint8_t)esp_reset_reason());
}

void eventLog(Event type, float value, uint8_t arg)
{
    s_log.e[s_log.head] = {(uint32_t)(millis() / 1000), value, s_log.boots, (uint8_t)type, arg};
    s_log.head = (s_log.head + 1) % kCap;
    if (s_log.count < kCap)
        s_log.count++;
    save();
}

void eventLogPrint()
{
    Serial.printf("%u events (boot,uptime_s,event,value,arg)\n", s_log.count);
    for (uint8_t i = 0; i < s_log.count; i++)
    {
        const Entry &e = s_log.e[(s_log.head + kCap - s_log.count + i) % kCap];
        Serial.printf("%u,%lu,%s,%.3f,%u\n", e.boot, (unsigned long)e.uptimeS, name(e.type), e.value, e.arg);
    }
}

void eventLogClear()
{
    const uint16_t boots = s_log.boots;
    s_log = Store{};
    s_log.boots = boots;
    save();
}

namespace
{
    const char *resetName(uint8_t r)
    {
        switch ((esp_reset_reason_t)r)
        {
        case ESP_RST_POWERON: return "power on";
        case ESP_RST_SW: return "restart";
        case ESP_RST_PANIC: return "CRASH";
        case ESP_RST_INT_WDT:
        case ESP_RST_TASK_WDT:
        case ESP_RST_WDT: return "WATCHDOG";
        case ESP_RST_BROWNOUT: return "BROWNOUT";
        case ESP_RST_DEEPSLEEP: return "wake";
        case ESP_RST_EXT: return "reset pin";
        default: return "reset";
        }
    }
}

void eventLogText(char *buf, size_t len)
{
    size_t n = 0;
    buf[0] = 0;
    if (s_log.count == 0)
    {
        snprintf(buf, len, "No events");
        return;
    }
    auto at = [](int i) -> const Entry & { return s_log.e[(s_log.head + kCap - s_log.count + i) % kCap]; };
    for (int i = s_log.count - 1; i >= 0 && n + 1 < len; i--)
    {
        const Entry &e = at(i);
        char line[72];
        if ((Event)e.type == Event::BOOT)
        {
            // a run of boots with the same reason -> one line
            int j = i;
            while (j > 0 && (Event)at(j - 1).type == Event::BOOT && at(j - 1).arg == e.arg)
                j--;
            if (j == i)
                snprintf(line, sizeof(line), "#%u  Boot (%s)", e.boot, resetName(e.arg));
            else
                snprintf(line, sizeof(line), "#%u-%u  %d boots (%s)", at(j).boot, e.boot, i - j + 1, resetName(e.arg));
            i = j;
        }
        else
        {
            const uint32_t t = e.uptimeS;
            char what[48];
            switch ((Event)e.type)
            {
            case Event::OUTPUT_LOST_POWER: snprintf(what, sizeof(what), "Output lost: power? %.2f A", e.value); break;
            case Event::OUTPUT_LOST_THERMAL: snprintf(what, sizeof(what), "Output lost: thermal? %.2f A", e.value); break;
            case Event::OVP: snprintf(what, sizeof(what), "OVP %.3f V", e.value); break;
            case Event::OCP: snprintf(what, sizeof(what), "OCP %.3f A", e.value); break;
            case Event::LINK_LOST: snprintf(what, sizeof(what), "Link lost, output off"); break;
            case Event::LINK_PEER_OFF: snprintf(what, sizeof(what), "Slave went off, output off"); break;
            default: snprintf(what, sizeof(what), "?"); break;
            }
            snprintf(line, sizeof(line), "#%u %lu:%02lu:%02lu  %s", e.boot, (unsigned long)(t / 3600),
                     (unsigned long)(t / 60 % 60), (unsigned long)(t % 60), what);
        }
        int w = snprintf(buf + n, len - n, "%s\n", line);
        if (w < 0)
            break;
        n += (size_t)w;
    }
    if (n >= len)
        n = len - 1;
    if (n > 0 && buf[n - 1] == '\n')
        buf[n - 1] = 0;
}
