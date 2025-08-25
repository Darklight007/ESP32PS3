#pragma once
#include <lvgl.h>
#include <stdint.h>

// public "builder variant" enum used by create_text(...)
using lv_menu_builder_variant_t = uint8_t;
enum : lv_menu_builder_variant_t
{
    LV_MENU_ITEM_BUILDER_VARIANT_1 = 0,
    LV_MENU_ITEM_BUILDER_VARIANT_2 = 1
};

// ---- forward declarations (avoid heavy headers here) ----
struct Device;
struct DAC_codes;
class TFT_eSPI;

// ---- singletons provided elsewhere ----
extern Device PowerSupply;
extern bool buzzerSound;
extern TFT_eSPI tft;

// ---- GUI bundle shared with other files ----
struct setting_
{
    lv_obj_t *vin_1{};
    lv_obj_t *vin_2{};
    lv_obj_t *code_1{};
    lv_obj_t *code_2{};

    lv_obj_t *lbl_calib_m{};
    lv_obj_t *lbl_calib_b{};

    lv_obj_t *lbl_rawCode{};
    lv_obj_t *lbl_calibratedValue{};
    lv_obj_t *lbl_rawAVG_{};
    lv_obj_t *lbl_calibValueAVG_{};
    lv_obj_t *lbl_ER{};
};


struct setting_GUI
{
    setting_ Voltage;
    setting_ Current;
    
    lv_obj_t *internalResistor{};
};


extern setting_GUI Calib_GUI;

// ---- public API ----
void SettingMenu(lv_obj_t *parent);

// helpers used elsewhere
double get_m(double code1, double code2, double vin1, double vin2);
double get_b(double code, double m, double vin);

// callbacks referenced by other TUs (e.g. SetupHandlers.cpp)
void btn_calibration_ADC_voltage_event_cb(lv_event_t *e);
void btn_calibration_ADC_current_event_cb(lv_event_t *e);
void internal_current_calibration_cb(lv_event_t *e);
void ADC_INL_Voltage_calibration_cb(lv_event_t *e);

void open_dac_calibration_cb(lv_event_t *e);
