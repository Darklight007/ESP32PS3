#pragma once
#include <lvgl.h>

// Li-ion battery CC/CV charger + tester ("Batt" tab on Utility page).
// State machine runs ONLY on Core 1 (main loop) via BatteryChargerInterval().

void createBatteryTab(lv_obj_t *parent);           // Build UI (call from Utility_tabview)
void BatteryChargerInterval(unsigned long interval); // Call from main loop (Core 1)
bool batteryChargerActive();                        // true while charging/testing
