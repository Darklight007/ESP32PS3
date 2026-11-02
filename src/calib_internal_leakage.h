#pragma once

#include <lvgl.h>

// Internal leakage resistance calibration module
// Measures internal leakage resistance and zero current offset

// Start zero + leakage measurement: current at 1..32 V, least-squares line
// I = I0 + V/R sets both the zero code (iCal code_1) and internalLeakage[]
void start_leakage_resistance_measurement(lv_event_t *e);

// Start zero current offset calibration procedure
// Measures ADC code at 0A and updates calibration offset
void start_current_zero_calibration(lv_event_t *e);
// Whether the current auto-zero's first step sets V to 1 V (call before starting it)
void current_zero_set_voltage(bool set, const char *step_label);
void start_voltage_zero_calibration(lv_event_t *e);

// Open internal leakage calibration window
void internal_leakage_calibration_cb(lv_event_t *e);
