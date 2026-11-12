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
