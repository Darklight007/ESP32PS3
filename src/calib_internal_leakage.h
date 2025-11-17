#pragma once

#include <lvgl.h>

// Internal leakage resistance calibration module
// Measures internal leakage resistance (0V→32V) and zero current offset

// Start total leakage resistance measurement procedure
// Runs auto-sequence: 0V measure → 32V measure → calculate R = 32V / (i1 - i0)
void start_leakage_resistance_measurement(lv_event_t *e);

// Start zero current offset calibration procedure
// Measures ADC code at 0A and updates calibration offset
void start_current_zero_calibration(lv_event_t *e);

// Open internal leakage calibration window
void internal_leakage_calibration_cb(lv_event_t *e);
