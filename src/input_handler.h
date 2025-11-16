#pragma once

#include <lvgl.h>
#include <TFT_eSPI.h>

// Touch/calibration functions
void touch_calibrate();
void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data);
void init_touch();

// Encoder page handler functions
void handleCalibrationPage(int32_t encoder1_last_value, int32_t encoder2_last_value);
void handleGraphPage(int32_t &encoder1_last_value, int32_t &encoder2_last_value);
void handleHistogramPage(int32_t &encoder1_last_value, int32_t &encoder2_last_value);
void handleUtilityPage(int32_t encoder1_last_value, int32_t encoder2_last_value);
void handleUtility_function_Page(int32_t encoder1_last_value, int32_t encoder2_last_value);
void managePageEncoderInteraction();
