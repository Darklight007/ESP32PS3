// SetupHandlers.h
#ifndef SETUP_HANDLERS_H
#define SETUP_HANDLERS_H

#include <Arduino.h>
#include <Wire.h>
#include "TFT_eSPI.h"
#include "lvgl.h"

#include "Preferences.h"

#include "DispObject.h"
#include "device.h"

// #include "config.hpp"
// #include "input_device.h"
#include <Keypad_MC17.h> 
// #include "buzzer.h"
// #include "setting_menu.h"
// #include "globalFunctions.h"


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_heap_caps.h"
#include "esp_log.h"

#define DMA true
#define COLOR_DEPTH 32
#define IWIDTH 320
#define IHEIGHT 240

extern Keypad_MC17 kpd;

extern Device PowerSupply; // Assuming PowerSupply is a class or struct

extern TFT_eSPI tft; // Global TFT instance
extern TFT_eSprite spr[2];
extern Preferences preferences;

 
extern lv_disp_draw_buf_t draw_buf;

extern  uint16_t screenWidth;
extern  uint16_t screenHeight;

extern lv_color_t *buf[2];
extern void btn_event_cb(lv_event_t *e);

extern void IRAM_ATTR VoltageEnc(void *arg);
extern void IRAM_ATTR CurrentEnc(void *arg);
extern void IRAM_ATTR ADCPinISR(void *arg);
extern void GraphChart(lv_obj_t *parent, lv_coord_t x, lv_coord_t y);
extern void StatsChart(lv_obj_t *parent, lv_coord_t x, lv_coord_t y);
extern void stat_measure(lv_obj_t *parent, lv_coord_t x, lv_coord_t y);
extern void textarea(lv_obj_t *parent);
extern void Utility_tabview(lv_obj_t *parent);
extern void SettingMenu(lv_obj_t *parent);
extern void voltage_current_calibration(void);
extern void btn_calibration_ADC_voltage_event_cb(lv_event_t *e);
extern void btn_calibration_ADC_current_event_cb(lv_event_t *e); 
extern void open_dac_calibration_cb(lv_event_t *e);

// extern void btn_calibration_ADC_voltage_event_cb(lv_event_t *e);
// extern void btn_calibration_ADC_current_event_cb(lv_event_t *e);
extern void updateObjectPos_cb(lv_event_t *e);

// extern lv_obj_t *menu;
// extern lv_obj_t *voltageCurrentCalibration;

// extern lv_obj_t *win_ADC_voltage_calibration;
// extern lv_obj_t *win_ADC_current_calibration;
// extern lv_obj_t *win_DAC_calibration;


extern TaskHandle_t Task_adc; 
extern TaskHandle_t Task1; 
extern TaskHandle_t Task_dac; 

// extern lv_obj_t *dd_calibration;
// extern lv_obj_t *lbl_voltageCalib_m;
// extern lv_obj_t *lbl_voltageCalib_b;
// extern lv_obj_t *lbl_rawCode;
// extern lv_obj_t *lbl_calibratedValue;
// extern lv_obj_t *lbl_rawAVG_;
// extern lv_obj_t *lbl_calibValueAVG_;
// extern lv_obj_t *lbl_ER_;
extern int heepMemory;

extern void Task_BarGraph(void *pvParameters);
extern void Task_ADC(void *pvParameters);
extern void  init_touch();


// Declarations of setup and utility functions
void initializeSerial();
void initialMemory();
void initializeI2C();
void initializeDisplay();
void setupPowerSupply();
void initializeTouch();
void setupLVGL();
void setupDMA();
void setupTasks();
void setupPreferences();
void setupADC();
void setupDAC();
void createTasks();
void setupCalibPage();
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);

void init_display();
// Add other necessary function declarations here

bool scanI2CBus(TwoWire &i2cBus, uint8_t address);

#endif // SETUP_HANDLERS_H