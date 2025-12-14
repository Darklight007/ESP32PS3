#pragma once

#include <lvgl.h>

// DAC calibration window and handlers
// Allows calibration of voltage and current DAC zero/max values

// Open the DAC calibration window (creates if needed)
void open_dac_calibration_cb(lv_event_t *e);

// Refresh DAC spinboxes from saved data (called by Load button)
void refreshDacCalibSpinboxes();
