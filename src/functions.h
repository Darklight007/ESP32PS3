#pragma once

#include <lvgl.h>
#include <functional>
#include "globals.h"
#include "waveform_generator.h"

#define PI 3.14159265358979323846

// Display and initialization functions
void init_display(void);
void init_touch(void);

// Chart and graph functions
void ChartUpdate();
void HistPush();
void GraphPush();
void StatusBar();

// Scheduling and timing functions
void schedule(std::function<void(void)> func, unsigned long &&interval, unsigned long &startTime);
void keyCheckLoop();
void KeyCheckInterval(unsigned long interval);
void DACInterval(unsigned long interval);
void trackLoopExecution(const char *);

// UI event handlers
void handleHistogramPage(int32_t &encoder1_last_value, int32_t &encoder2_last_value);
void Utility_tabview(lv_obj_t *parent);
lv_obj_t *find_btn_by_tag(lv_obj_t *parent, uint32_t tag);

// Function generator
bool functionGenerator();
void functionGenerator_demo(void);

// Calibration and settings
void btn_calibration_ADC_voltage_event_cb(lv_event_t *e);
void getSettingEncoder(lv_indev_drv_t *drv, lv_indev_data_t *data);

// Static event callbacks (need to stay in header for inline/static linkage)
static void draw_event_cb(lv_event_t *e);
static void draw_event_cb2(lv_event_t *e);
