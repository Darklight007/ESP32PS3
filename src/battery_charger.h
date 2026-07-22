#pragma once
#include <lvgl.h>

// Li-ion battery CC/CV charger + tester ("Batt" tab on Utility page).
// State machine runs ONLY on Core 1 (main loop) via BatteryChargerInterval().

void createBatteryTab(lv_obj_t *parent);           // Build UI (call from Utility_tabview)
void BatteryChargerInterval(unsigned long interval); // Call from main loop (Core 1)
bool batteryChargerActive();                        // true while charging/testing

// Encoder navigation for the battery-chemistry dropdown while its list is
// open (dropdowns have no built-in encoder support in this project — only
// touch). Call from the Utility-page encoder handler.
bool batteryChemDropdownOpen();
void batteryChemDropdownStep(int dir); // dir: +1 next item, -1 previous item

// Confirm/close via the Current-encoder push button ('>' key — same physical
// switch used elsewhere to lock/unlock Current). keyCheckLoop runs on Core 0
// (Task_ADC), so closing the dropdown (an LVGL widget op) can't happen there
// directly — request sets a flag only; the main loop (Core 1) drains it.
void requestBattChemDropdownClose(); // Core 0-safe: just sets a volatile flag
void drainBattChemDropdownClose();   // Core 1 only: call from main loop

// Lead-resistance calibration (Settings > Calibration menu entry, "Batt Lead R").
// Short the PS output leads together first — battLeadCalMenu_cb() shows the
// confirm dialog, startBattLeadCal() runs the actual sweep. The measured value
// is persisted and subtracted from every future battery IR reading.
void battLeadCalMenu_cb(lv_event_t *e); // wire this into the Calibration menu
void startBattLeadCal();                // begins the sweep (Core 1 only)
