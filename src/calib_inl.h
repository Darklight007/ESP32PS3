#pragma once

#include <lvgl.h>

// INL (Integral Non-Linearity) voltage calibration module
// Provides automatic voltage sweep calibration with monotone cubic spline correction

// External dependencies
class MonotoneCubicCalibrator;
extern MonotoneCubicCalibrator g_voltINL;
extern bool g_voltINL_ready;

// Public API
void ADC_INL_Voltage_calibration_cb(lv_event_t *e);
void rebuildINLFromCalibration();  // Rebuild INL interpolator from saved data at startup

// Internal debugging (Serial only)
void INL_dbg(const char *fmt, ...);
