#pragma once

#include <lvgl.h>
#include <Arduino.h>

// Speed Match: each ADC speed (Slow/Normal/Fast/Turbo) reads a slightly
// different voltage than Quiet (rate 0) - up to ~1.4 mV at 29 V for Turbo.
// With no load, the sequence measures every speed at 2 V and 29 V and fits a
// gain and offset per speed so all of them match Quiet.

// Voltage = raw * cal * g_rateGain[rate] + g_rateOff[rate]. Read on Core 0;
// each entry is a 32-bit float, so a write never tears.
extern volatile float g_rateGain[5];
extern volatile float g_rateOff[5];

void rateMatchLoad();  // setup: stored trim, else the built-in defaults
void rateMatchClear(); // back to the built-in defaults, forget the stored trim
String rateMatchString(); // "gain,offset_mV;..." for rates 0-4 (CAL:RATE?)

// Start (output must be ON, no load). false = refused. Core 1; caller opens
// the log window and sets g_calibration_in_progress.
bool start_rate_match_calibration();

// Settings menu button
void rate_match_calibration_cb(lv_event_t *e);
