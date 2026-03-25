#pragma once

#include <lvgl.h>
#include "setting_menu.h"
#include "device.hpp"
// #include "calibration_gui.h"

// ADC calibration window builder and callbacks
// Provides voltage and current ADC calibration UI

// Build/show ADC calibration window (generic for voltage or current)
// autozero_cb: Callback for "Auto zeros" button (nullptr to hide button)
void build_adc_calibration_window(lv_obj_t **win_holder,
                                   const char *title,
                                   AdcCalibrationControls &gui,
                                   const CalPrefill &pf,
                                   lv_event_cb_t autozero_cb = nullptr);

// Button callbacks (declared in setting_menu.h, implemented here for proximity)
void btn_calibration_ADC_voltage_event_cb(lv_event_t *e);
void btn_calibration_ADC_current_event_cb(lv_event_t *e);

