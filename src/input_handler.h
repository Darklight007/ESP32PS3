#pragma once

#include <lvgl.h>
#include <TFT_eSPI.h>
#include "setting_menu.h"
#include "ui_creation.h"
#include "memory.h"

// Touch/calibration functions
void touch_calibrate();
void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data);
void init_touch();

// Encoder functions
void getSettingEncoder(lv_indev_drv_t *drv, lv_indev_data_t *data);

// Encoder page handler functions
void handleCalibrationPage(int32_t encoder1_last_value, int32_t encoder2_last_value);
void handleGraphPage(int32_t &encoder1_last_value, int32_t &encoder2_last_value);
void handleHistogramPage(int32_t &encoder1_last_value, int32_t &encoder2_last_value);
void handleUtilityPage(int32_t encoder1_last_value, int32_t encoder2_last_value);
void handleUtility_function_Page(int32_t encoder1_last_value, int32_t encoder2_last_value);
void managePageEncoderInteraction();

// Keyboard input handler
void keyCheckLoop();

// Deferred mA toggle UI update (must be called from Core 1 main loop)
void processDeferredMaToggle();

// Deferred power on/off toggle from 'O' key (must be called from Core 1 main loop)
void drainPendingPowerToggle();
extern volatile bool g_powerTogglePending;

// Deferred keypad-triggered LVGL actions (Home, numeric entry, V/v/A/a
// readback, Z-key avg slider, memory load/save, chart/graph toggles, rotary
// step buttons, V/A series toggle) - must be called from Core 1 main loop.
void drainPendingKeyEvents();

// Waveform selection helper
void updateDutySpinboxState();
