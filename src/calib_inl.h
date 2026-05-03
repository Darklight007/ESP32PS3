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
void start_inl_calibration();      // Start INL calibration programmatically (no prompt)
bool is_inl_calibration_running(); // Check if INL calibration FSM is active
void open_inl_window_no_load();    // Show INL window without calling LoadCalibrationData()

// Internal debugging (Serial only)
void INL_dbg(const char *fmt, ...);
