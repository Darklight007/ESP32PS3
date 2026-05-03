#pragma once

#include <lvgl.h>

// INL Zero-Current calibration module
// Same sweep as regular INL but with current setpoint forced to 0,
// capturing ADC voltage nonlinearity under no-load conditions.

class MonotoneCubicCalibrator;
extern MonotoneCubicCalibrator g_voltINL_ZC;
extern bool g_voltINL_ZC_ready;

void ADC_INL_ZC_calibration_cb(lv_event_t *e);
void rebuildINLZCFromCalibration();
