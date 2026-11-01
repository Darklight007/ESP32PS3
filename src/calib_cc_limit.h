#pragma once

#include <lvgl.h>
#include <Arduino.h>

// Current-limit (CC) DAC calibration with the output shorted.
// The CC loop's real limit differs from the setpoint by a gain/offset that
// the measurement side doesn't see (+0.56% at 5 A measured 2026-11-01). With
// a thick wire across the output, the sequence steps the limit and records
// the current the A range actually measures; DACUpdate() then inverts that
// table so the delivered limit matches the setpoint.

// Setpoint in A -> current-limit command in A (identity if no table). Core 0.
double ccLimitCorrect(double amps);

// Load the stored table (setup) / forget it (and its saved copy). Core 1.
void ccLimitLoad();
void ccLimitClear();

// Points as "set,actual;..." in A, empty if none (CAL:CCLIM?)
String ccLimitTableString();

// Start the sequence (output must be ON and shorted). false = refused.
// Core 1; caller opens the log window and sets g_calibration_in_progress.
bool start_cc_limit_calibration();

// Settings menu button: explains the setup, then runs the sequence
void cc_limit_calibration_cb(lv_event_t *e);
