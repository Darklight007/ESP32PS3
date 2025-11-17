#pragma once

#include <lvgl.h>

// Internal current calibration procedures
// Measures internal resistance and zero current offset

// Start total resistance measurement procedure (measures internal leakage)
// Runs auto-sequence: 0V measure → 32V measure → calculate R
void start_current_totalR();

// Start zero current calibration procedure
// Measures ADC code at 0A and updates calibration
void start_current_zeros(lv_event_t *e);
