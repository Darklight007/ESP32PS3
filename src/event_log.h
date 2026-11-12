#pragma once
#include <stdint.h>

// Fault/event history kept across restarts (NVS, last 40 events), read with
// SYST:LOG?. Call from Core 1 only. Each event is one small NVS write (a few
// ms with both cores stalled), so log rare things only - never per sample.
enum class Event : uint8_t
{
    BOOT = 1,      // arg = esp_reset_reason()
    OUTPUT_LOST_POWER,   // value = 1 min average current, A
    OUTPUT_LOST_THERMAL, // value = 1 min average current, A
    OVP,           // value = measured V
    OCP,           // value = measured A
};

void eventLogBegin();                                        // load + log this boot
void eventLog(Event type, float value = 0, uint8_t arg = 0);
void eventLogPrint();                                        // one line per event, oldest first
void eventLogClear();
