#pragma once

#include <lvgl.h>

// Full automatic calibration sequence:
//   1. Auto Zero Voltage
//   2. Auto Zero Current
//   3. Internal Resistance measurement (A range)
//   4. Internal Resistance measurement (mA range)
//   5. ADC INL Voltage calibration
void full_auto_calibration_cb(lv_event_t *e);
