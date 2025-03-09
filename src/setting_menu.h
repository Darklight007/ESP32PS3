#ifndef SETTING_MENU_H
#define SETTING_MENU_H
#include "spinbox_pro.h"
#include "version.h"
/******************************************
 *
 * Menu example
 *
 ******************************************/
extern Device PowerSupply;
extern bool buzzerSound;
extern TFT_eSPI tft;
extern FunGen funGenMem;
extern DAC_codes dac_data_g;

lv_obj_t *menu;
lv_obj_t *voltageCurrentCalibration;
lv_obj_t *myTextBox;

lv_obj_t *win_DAC_calibration;
static bool win_dac_already_created = false;
bool isMyTextBoxHidden;
enum
{
    LV_MENU_ITEM_BUILDER_VARIANT_1,
    LV_MENU_ITEM_BUILDER_VARIANT_2
};
typedef uint8_t lv_menu_builder_variant_t;

static void switch_handler(lv_event_t *e);
lv_obj_t *root_page;

// static lv_obj_t *slider_label; unused variable
// static lv_obj_t *slider_label2; unused variable
// static lv_obj_t *sub_display_page;

static lv_obj_t *create_text(lv_obj_t *parent, const char *icon, const char *txt,
                             lv_menu_builder_variant_t builder_variant, lv_obj_t *obj);
// static lv_obj_t *create_slider(lv_obj_t *parent, const char *icon, const char *txt, int32_t min, int32_t max, int32_t val);
static lv_obj_t *create_slider(lv_obj_t *parent, const char *icon, const char *txt, int32_t min, int32_t max,
                               int32_t val, lv_event_cb_t event_cb, lv_event_code_t filter);
static lv_obj_t *create_switch(lv_obj_t *parent, const char *icon, const char *txt, bool chk);
static lv_obj_t *create_pushbutton(lv_obj_t *parent, const char *icon, lv_event_cb_t event_cb, const char *txt, const char *buttonTxt);

static void slider_backlight_event_cb(lv_event_t *e);
static void slider_adcRate_event_cb(lv_event_t *e);

static void slider_adcAVG_event_cb(lv_event_t *e);
static void slider_decimalPoints_event_cb(lv_event_t *e);

static void PRESSED_event_cb(lv_event_t *e);
static void RELEASED_event_cb(lv_event_t *e);
static void MainMenu_event_cb(lv_event_t *e);

static void switch_buzzer_event_cb(lv_event_t *e);
static void btn_calibration_ADC_event_cb(lv_event_t *e);
static void btn_calibration_DAC_event_cb(lv_event_t *e);
static void LCD_Calibration_cb(lv_event_t *e);

static void btn_close_hide_obj_cb(lv_event_t *e);

lv_obj_t *btn_load;

struct
{
    int current_index;
    int ids[10];
    int count;
    int digit_count[10];
    int id_index = 0;
    lv_obj_t *btn_plus[10];
    lv_obj_t *btn_minus[10];
} spinboxes;

// static std::map<int, const char *> ADC_SPS;

static void lv_spinbox_increment_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);
    if (code == LV_EVENT_SHORT_CLICKED || code == LV_EVENT_LONG_PRESSED_REPEAT)
    {
        lv_spinbox_increment(lv_obj_get_parent(btn));
    }
}

static void lv_spinbox_decrement_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);
    if (code == LV_EVENT_SHORT_CLICKED || code == LV_EVENT_LONG_PRESSED_REPEAT || code == LV_EVENT_PRESSED)
    {
        lv_spinbox_decrement(lv_obj_get_parent(btn));
    }
}

lv_obj_t *dd_calibration;
lv_obj_t *lbl_voltageCalib_m;
lv_obj_t *lbl_voltageCalib_b;
lv_obj_t *lbl_rawCode;
lv_obj_t *lbl_calibratedValue;
lv_obj_t *lbl_rawAVG_;
lv_obj_t *lbl_calibValueAVG_;
lv_obj_t *lbl_ER_;
// lv_obj_t *lbl_currentCalib_m;
// lv_obj_t *lbl_currentCalib_b;

double get_m(double code1, double code2, double vin1, double vin2)
{

    return (code2 - code1) / (vin2 - vin1);
}

double get_voltageCalib_m()
{

    lv_obj_t *vin1_ = lv_obj_get_child(voltageCurrentCalibration, spinboxes.ids[0]);
    lv_obj_t *code1_ = lv_obj_get_child(voltageCurrentCalibration, spinboxes.ids[1]);
    lv_obj_t *vin2_ = lv_obj_get_child(voltageCurrentCalibration, spinboxes.ids[2]);
    lv_obj_t *code2_ = lv_obj_get_child(voltageCurrentCalibration, spinboxes.ids[3]);

    int code1 = (lv_spinbox_get_value(code1_));
    int code2 = (lv_spinbox_get_value(code2_));
    double vin1 = double(lv_spinbox_get_value(vin1_)) / 10000.0;
    double vin2 = double(lv_spinbox_get_value(vin2_)) / 10000.0;

    return get_m(code1, code2, vin1, vin2);
}

double get_b(double code, double m, double vin)
{

    return code - m * vin;
}

static lv_style_t style_hide;
static lv_style_t style_unhide;
static lv_style_t style_border_def;
static lv_style_t style_border_select;
static void trackChild(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);

    spinboxes.current_index = lv_obj_get_index(obj);

    static bool flag = true;

    if (flag)
    {

        lv_style_init(&style_hide);
        lv_style_init(&style_unhide);
        lv_style_init(&style_border_def);
        lv_style_init(&style_border_select);
        flag = false;
    }

    lv_style_set_bg_opa(&style_unhide, LV_OPA_80);
    lv_style_set_text_opa(&style_unhide, LV_OPA_100);
    lv_style_set_border_color(&style_border_select, lv_color_hex(0xFF0000));

    lv_style_set_bg_opa(&style_hide, LV_OPA_TRANSP);
    lv_style_set_text_opa(&style_hide, LV_OPA_TRANSP);
    lv_style_set_border_color(&style_border_def, lv_color_hex(0x8f8f8f));

    lv_obj_remove_style(obj, &style_unhide, LV_PART_CURSOR);
    lv_obj_remove_style(obj, &style_border_select, LV_STATE_DEFAULT);

    lv_obj_add_style(obj, &style_unhide, LV_PART_CURSOR);
    lv_obj_add_style(obj, &style_border_select, LV_STATE_DEFAULT);

    for (int i = 0; i < spinboxes.count; i++)
    {
        if (spinboxes.ids[i] != spinboxes.current_index)
        {
            lv_obj_remove_style(lv_obj_get_child(lv_obj_get_parent(obj), spinboxes.ids[i]), &style_unhide, LV_PART_CURSOR);
            lv_obj_remove_style(lv_obj_get_child(lv_obj_get_parent(obj), spinboxes.ids[i]), &style_hide, LV_PART_CURSOR);
            lv_obj_remove_style(lv_obj_get_child(lv_obj_get_parent(obj), spinboxes.ids[i]), &style_border_def, LV_STATE_DEFAULT);

            lv_obj_add_style(lv_obj_get_child(lv_obj_get_parent(obj), spinboxes.ids[i]), &style_hide, LV_PART_CURSOR);
            lv_obj_add_style(lv_obj_get_child(lv_obj_get_parent(obj), spinboxes.ids[i]), &style_border_def, LV_STATE_DEFAULT);
        }
        else
            spinboxes.id_index = i;
    }
    // lv_label_set_text(lbl_voltageCalib_m, std::to_string(get_voltageCalib_m()).c_str());
    //  Serial.printf("\nPressed:%i", spinboxes.id_index);
}
static void set_spinbox_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    if (code == LV_EVENT_VALUE_CHANGED)
    {
        // char buf[32];
        // lv_dropdown_get_selected_str(obj, buf, sizeof(buf));
        // LV_LOG_USER("Option: %s", buf);
        lv_obj_t *vin1_ = lv_obj_get_child(voltageCurrentCalibration, spinboxes.ids[0]);
        lv_obj_t *code1_ = lv_obj_get_child(voltageCurrentCalibration, spinboxes.ids[1]);
        lv_obj_t *vin2_ = lv_obj_get_child(voltageCurrentCalibration, spinboxes.ids[2]);
        lv_obj_t *code2_ = lv_obj_get_child(voltageCurrentCalibration, spinboxes.ids[3]);

        if (lv_dropdown_get_selected(obj) == 0)
        {

            lv_spinbox_set_value(code1_, 1.0 * PowerSupply.CalBank[PowerSupply.bankCalibId].vCal.code_1);
            lv_spinbox_set_value(code2_, 1.0 * PowerSupply.CalBank[PowerSupply.bankCalibId].vCal.code_2);
            lv_spinbox_set_value(vin1_, 10000.0 * PowerSupply.CalBank[PowerSupply.bankCalibId].vCal.value_1);
            lv_spinbox_set_value(vin2_, 10000.0 * PowerSupply.CalBank[PowerSupply.bankCalibId].vCal.value_2);
        }
        else
        {

            lv_spinbox_set_value(code1_, 1.0 * PowerSupply.CalBank[PowerSupply.bankCalibId].iCal.code_1);
            lv_spinbox_set_value(code2_, 1.0 * PowerSupply.CalBank[PowerSupply.bankCalibId].iCal.code_2);
            lv_spinbox_set_value(vin1_, 10000.0 * PowerSupply.CalBank[PowerSupply.bankCalibId].iCal.value_1);
            lv_spinbox_set_value(vin2_, 10000.0 * PowerSupply.CalBank[PowerSupply.bankCalibId].iCal.value_2);
        }
    }
}

static void save_cb(lv_event_t *e)
{
    PowerSupply.SaveCalibrationData();
    PRESSED_event_cb(NULL);
}
static void load_cb(lv_event_t *e)
{
    PowerSupply.LoadCalibrationData();
    PRESSED_event_cb(NULL);

    lv_obj_t *vin1_ = lv_obj_get_child(voltageCurrentCalibration, spinboxes.ids[0]);
    lv_obj_t *code1_ = lv_obj_get_child(voltageCurrentCalibration, spinboxes.ids[1]);
    lv_obj_t *vin2_ = lv_obj_get_child(voltageCurrentCalibration, spinboxes.ids[2]);
    lv_obj_t *code2_ = lv_obj_get_child(voltageCurrentCalibration, spinboxes.ids[3]);

    if (lv_dropdown_get_selected(dd_calibration) == 0)
    {

        lv_spinbox_set_value(code1_, 1.0 * PowerSupply.CalBank[PowerSupply.bankCalibId].vCal.code_1);
        lv_spinbox_set_value(code2_, 1.0 * PowerSupply.CalBank[PowerSupply.bankCalibId].vCal.code_2);
        lv_spinbox_set_value(vin1_, 10000.0 * PowerSupply.CalBank[PowerSupply.bankCalibId].vCal.value_1);
        lv_spinbox_set_value(vin2_, 10000.0 * PowerSupply.CalBank[PowerSupply.bankCalibId].vCal.value_2);
    }
    else
    {

        lv_spinbox_set_value(code1_, 1.0 * PowerSupply.CalBank[PowerSupply.bankCalibId].iCal.code_1);
        lv_spinbox_set_value(code2_, 1.0 * PowerSupply.CalBank[PowerSupply.bankCalibId].iCal.code_2);
        lv_spinbox_set_value(vin1_, 10000.0 * PowerSupply.CalBank[PowerSupply.bankCalibId].iCal.value_1);
        lv_spinbox_set_value(vin2_, 10000.0 * PowerSupply.CalBank[PowerSupply.bankCalibId].iCal.value_2);
    }
}

void voltage_current_calibration(void)
{

    voltageCurrentCalibration = lv_obj_create(lv_scr_act());

    lv_obj_set_size(voltageCurrentCalibration, 320, 240);
    lv_obj_align(voltageCurrentCalibration, LV_ALIGN_CENTER, 0, 0);
    // static lv_style_t style_panel;
    // lv_style_init(&style_panel);
    // remove all padding
    // lv_style_set_pad_ver(&style_panel, 0);
    // lv_style_set_pad_hor(&style_panel, 0);
    // lv_obj_add_style(voltageCurrentCalibration, &style_panel, LV_STATE_DEFAULT);

    /********************
     * close button
     ********************/
    lv_obj_t *close_btn = lv_btn_create(voltageCurrentCalibration);
    lv_obj_t *close_btn_label = lv_label_create(close_btn);
    lv_obj_remove_style_all(close_btn_label);
    // lv_obj_remove_style_all(close_btn);

    lv_label_set_text(close_btn_label, " X ");
    lv_obj_set_pos(close_btn, 0, 0);
    lv_obj_add_event_cb(close_btn, MainMenu_event_cb, LV_EVENT_RELEASED, NULL);
    lv_obj_set_align(close_btn, LV_ALIGN_TOP_RIGHT);

    lv_obj_set_content_width(close_btn_label, 20);  // The actual width: padding left + 50 + padding right
    lv_obj_set_content_height(close_btn_label, 20); // The actual width: padding top + 30 + padding bottom

    /********************
     * drop down list
     ********************/
    //  lv_obj_remove_style_all(dd_calibration);
    dd_calibration = lv_dropdown_create(voltageCurrentCalibration);
    lv_dropdown_set_options(dd_calibration, "Voltage Calibration\n"
                                            "Current Calibration");

    lv_obj_align(dd_calibration, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_width(dd_calibration, 176);
    lv_obj_add_event_cb(dd_calibration, set_spinbox_cb, LV_EVENT_SHORT_CLICKED, NULL);

    static lv_style_t style_btn_loadSave;
    lv_style_init(&style_btn_loadSave);
    lv_style_set_bg_color(&style_btn_loadSave, lv_palette_main(LV_PALETTE_BLUE));
    lv_obj_add_style(dd_calibration, &style_btn_loadSave, LV_STATE_DEFAULT);

    static lv_style_t style_close_btn;
    lv_style_init(&style_close_btn);
    lv_style_set_text_font(&style_close_btn, &lv_font_montserrat_14);
    lv_style_set_pad_ver(&style_close_btn, 2);
    lv_style_set_pad_hor(&style_close_btn, 2);
    lv_obj_add_style(close_btn, &style_close_btn, LV_STATE_DEFAULT);

    /********************
     * Spinbox
     ********************/
    static lv_style_t style_spinboxLbl;
    lv_style_init(&style_spinboxLbl);

    static lv_style_t style_spinbox;
    lv_style_init(&style_spinbox);

    auto createSpinbox = [&](lv_obj_t *parent, const char *labelText, int32_t range_min, int32_t range_max,
                             uint8_t digit_count, uint8_t separator_position, lv_align_t align,
                             lv_coord_t x_ofs, lv_coord_t y_ofs)
    {
        lv_obj_t *spinbox = lv_spinbox_create(parent);
        lv_obj_t *_label = lv_label_create(spinbox);

        lv_label_set_recolor(_label, true); /*Enable re-coloring by commands in the text*/
        lv_label_set_text(_label, labelText);
        lv_obj_align(_label, LV_ALIGN_OUT_LEFT_TOP, -2, -15);
        // lv_obj_set_style_text_align(_label, LV_TEXT_ALIGN_LEFT, 0);

        lv_style_set_text_font(&style_spinboxLbl, &lv_font_montserrat_10); //    &Undertale_16b1); //
        lv_style_set_bg_color(&style_spinboxLbl, lv_color_hex(0xFF0000));

        lv_obj_remove_style(_label, &style_spinboxLbl, LV_STATE_DEFAULT);
        lv_obj_add_style(_label, &style_spinboxLbl, LV_STATE_DEFAULT);

        //*************************************************************************
        lv_spinbox_set_range(spinbox, range_min, range_max);
        lv_spinbox_set_digit_format(spinbox, digit_count, separator_position);
        lv_spinbox_step_prev(spinbox);
        lv_obj_set_width(spinbox, 118);
        // lv_obj_set_height(spinbox, 30);
        lv_obj_align_to(spinbox, parent, align, x_ofs, y_ofs);

        lv_style_set_text_font(&style_spinbox, &graph_R_16); // Undertale_16b1
        lv_style_set_pad_ver(&style_spinbox, 2);
        // lv_style_set_pad_hor(&style_spinbox, 3);
        lv_style_set_pad_left(&style_spinbox, 2);
        lv_style_set_pad_right(&style_spinbox, 0);
        lv_style_set_radius(&style_spinbox, 3);
        lv_style_set_border_width(&style_spinbox, 2);
        lv_style_set_border_color(&style_spinbox, lv_color_hex(0x8f8f8f));
        lv_style_set_text_color(&style_spinbox, lv_color_hex(0xFFFF00));

        lv_obj_add_style(spinbox, &style_spinbox, LV_STATE_DEFAULT);

        lv_obj_add_flag(spinbox, LV_OBJ_FLAG_OVERFLOW_VISIBLE);
        lv_obj_add_event_cb(spinbox, PRESSED_event_cb, LV_EVENT_SHORT_CLICKED, NULL);
        lv_spinbox_set_cursor_pos(spinbox, 0);

        spinboxes.digit_count[spinboxes.count] = digit_count;
        spinboxes.ids[spinboxes.count] = lv_obj_get_index(spinbox);

        lv_coord_t h = lv_obj_get_height(spinbox);

        // btn_plus = lv_btn_create(spinbox);
        spinboxes.btn_plus[spinboxes.count] = lv_btn_create(spinbox);

        lv_obj_set_size(spinboxes.btn_plus[spinboxes.count], h / 2, h / 2);
        lv_obj_align_to(spinboxes.btn_plus[spinboxes.count], spinbox, LV_ALIGN_OUT_RIGHT_MID, h / 2, 0);
        // lv_obj_set_style_bg_img_src(spinboxes.btn_plus[spinboxes.count], LV_SYMBOL_PLUS, 0);
        lv_obj_add_event_cb(spinboxes.btn_plus[spinboxes.count], lv_spinbox_increment_event_cb, LV_EVENT_SHORT_CLICKED, NULL);
        lv_obj_add_event_cb(spinboxes.btn_plus[spinboxes.count], PRESSED_event_cb, LV_EVENT_SHORT_CLICKED, NULL);
        lv_obj_add_event_cb(spinboxes.btn_plus[spinboxes.count], RELEASED_event_cb, LV_EVENT_CLICKED, NULL);

        lv_obj_add_flag(spinboxes.btn_plus[spinboxes.count], LV_OBJ_FLAG_HIDDEN);
        // Serial.printf("\n btn_plus:%i", lv_obj_get_index(spinboxes.btn_plus[spinboxes.count]));

        // btn_minus = lv_btn_create(spinbox);
        spinboxes.btn_minus[spinboxes.count] = lv_btn_create(spinbox);

        lv_obj_set_size(spinboxes.btn_minus[spinboxes.count], h / 2, h / 2);
        lv_obj_align_to(spinboxes.btn_minus[spinboxes.count], spinbox, LV_ALIGN_OUT_RIGHT_MID, 0, 0);
        // lv_obj_set_style_bg_img_src(spinboxes.btn_minus[spinboxes.count] , LV_SYMBOL_MINUS, 0);
        lv_obj_add_event_cb(spinboxes.btn_minus[spinboxes.count], lv_spinbox_decrement_event_cb, LV_EVENT_SHORT_CLICKED, NULL);
        lv_obj_add_event_cb(spinboxes.btn_minus[spinboxes.count], PRESSED_event_cb, LV_EVENT_SHORT_CLICKED, NULL);
        lv_obj_add_event_cb(spinboxes.btn_minus[spinboxes.count], RELEASED_event_cb, LV_EVENT_CLICKED, NULL);

        lv_obj_add_flag(spinboxes.btn_minus[spinboxes.count], LV_OBJ_FLAG_HIDDEN);
        // Serial.printf("\n btn_minus:%i", lv_obj_get_index(spinboxes.btn_minus[spinboxes.count] ));

        lv_obj_add_event_cb(spinbox, trackChild, LV_EVENT_PRESSED, NULL);
        // lv_obj_add_event_cb(spinbox, trackChild, LV_EVENT_VALUE_CHANGED, NULL);
        lv_obj_set_scrollbar_mode(spinbox, LV_SCROLLBAR_MODE_OFF);

        spinboxes.count++;
    };

    int verPad = 52;
    int verOffset = 52;
    int XOffset = 0;

    createSpinbox(voltageCurrentCalibration, "#FFFFF7 Vin1:#", -10000, 400000, 6, 2, LV_ALIGN_TOP_LEFT, XOffset, verOffset);
    createSpinbox(voltageCurrentCalibration, "#FFFFF7 Code1:#", -10000, 8388608, 7, 0, LV_ALIGN_TOP_LEFT, XOffset, verOffset + 38);
    createSpinbox(voltageCurrentCalibration, "#FFFFF7 Vin2:#", -10000, 400000, 6, 2, LV_ALIGN_TOP_LEFT, XOffset + 140, verOffset);
    createSpinbox(voltageCurrentCalibration, "#FFFFF7 Code2:#", -10000, 8388608, 7, 0, LV_ALIGN_TOP_LEFT, XOffset + 140, verOffset + 38);

    lv_event_send(lv_obj_get_child(voltageCurrentCalibration, spinboxes.ids[0]), LV_EVENT_PRESSED, NULL);

    lv_obj_t *vin1_ = lv_obj_get_child(voltageCurrentCalibration, spinboxes.ids[0]);
    lv_obj_t *code1_ = lv_obj_get_child(voltageCurrentCalibration, spinboxes.ids[1]);
    lv_obj_t *vin2_ = lv_obj_get_child(voltageCurrentCalibration, spinboxes.ids[2]);
    lv_obj_t *code2_ = lv_obj_get_child(voltageCurrentCalibration, spinboxes.ids[3]);

    lv_spinbox_set_value(code1_, 1.0 * PowerSupply.CalBank[PowerSupply.bankCalibId].vCal.code_1);
    lv_spinbox_set_value(code2_, 1.0 * PowerSupply.CalBank[PowerSupply.bankCalibId].vCal.code_2);
    lv_spinbox_set_value(vin1_, 10000.0 * PowerSupply.CalBank[PowerSupply.bankCalibId].vCal.value_1);
    lv_spinbox_set_value(vin2_, 10000.0 * PowerSupply.CalBank[PowerSupply.bankCalibId].vCal.value_2);

    lv_obj_t *lbl_raw = lv_label_create(voltageCurrentCalibration);
    lv_obj_t *lbl_calibValue = lv_label_create(voltageCurrentCalibration);

    lv_label_set_text(lbl_raw, "Raw Code:");
    lv_label_set_text(lbl_calibValue, "Calibrated Value:");

    lv_obj_set_pos(lbl_raw, XOffset, verOffset + 38 * 2 - 10);
    lv_obj_set_pos(lbl_calibValue, XOffset + 140, verOffset + 38 * 2 - 10);

    lv_obj_add_style(lbl_raw, &style_spinboxLbl, LV_STATE_DEFAULT);
    lv_obj_add_style(lbl_calibValue, &style_spinboxLbl, LV_STATE_DEFAULT);

    static lv_style_t style_;
    lv_style_init(&style_);
    lv_style_set_text_font(&style_, &graph_R_16); //&Undertale_16b1); //
    lv_style_set_text_color(&style_, lv_color_hex(0xFFFFFF));
    lv_style_set_bg_color(&style_, lv_color_hex(0xFFAAAA));

    lbl_rawCode = lv_label_create(voltageCurrentCalibration);
    lv_label_set_text(lbl_rawCode, "838860080");
    lv_obj_align(lbl_rawCode, LV_ALIGN_TOP_LEFT, XOffset, 131);
    lv_obj_add_style(lbl_rawCode, &style_, LV_STATE_DEFAULT);

    lbl_calibratedValue = lv_label_create(voltageCurrentCalibration);
    lv_label_set_text(lbl_calibratedValue, "15.01256V");
    lv_obj_align(lbl_calibratedValue, LV_ALIGN_TOP_LEFT, XOffset + 140, 131);
    lv_obj_add_style(lbl_calibratedValue, &style_, LV_STATE_DEFAULT);

    lv_obj_t *lbl_rawAVG = lv_label_create(voltageCurrentCalibration);
    lv_obj_t *lbl_calibValueAVG = lv_label_create(voltageCurrentCalibration);
    lv_label_set_text(lbl_rawAVG, "Avg Raw:");
    lv_label_set_text(lbl_calibValueAVG, "Avg Calibrated Value:");
    lv_obj_set_pos(lbl_rawAVG, XOffset, verOffset + 38 * 3 - 15);
    lv_obj_set_pos(lbl_calibValueAVG, XOffset + 140, verOffset + 38 * 3 - 15);
    lv_obj_add_style(lbl_rawAVG, &style_spinboxLbl, LV_STATE_DEFAULT);
    lv_obj_add_style(lbl_calibValueAVG, &style_spinboxLbl, LV_STATE_DEFAULT);

    lbl_rawAVG_ = lv_label_create(voltageCurrentCalibration);
    lbl_calibValueAVG_ = lv_label_create(voltageCurrentCalibration);
    lv_label_set_text(lbl_rawAVG_, "+21657651");
    lv_label_set_text(lbl_calibValueAVG_, "+019.0154");
    lv_obj_set_pos(lbl_rawAVG_, XOffset, verOffset + 38 * 3 - 1);
    lv_obj_set_pos(lbl_calibValueAVG_, XOffset + 140, verOffset + 38 * 3 - 1);
    lv_obj_add_style(lbl_rawAVG_, &style_, LV_STATE_DEFAULT);
    lv_obj_add_style(lbl_calibValueAVG_, &style_, LV_STATE_DEFAULT);

    lv_obj_t *lbl_ER = lv_label_create(voltageCurrentCalibration);
    lv_label_set_text(lbl_ER, "Effective Resolution:");
    lv_obj_set_pos(lbl_ER, XOffset, verOffset + 38 * 4 - 17);
    lv_obj_add_style(lbl_ER, &style_spinboxLbl, LV_STATE_DEFAULT);
    lbl_ER_ = lv_label_create(voltageCurrentCalibration);
    lv_label_set_text(lbl_ER_, "17.23");
    lv_obj_set_pos(lbl_ER_, XOffset, verOffset + 38 * 4 - 4);
    lv_obj_add_style(lbl_ER_, &style_, LV_STATE_DEFAULT);

    lv_obj_t *btn_save = lv_btn_create(voltageCurrentCalibration);
    lv_obj_t *label_btn_save = lv_label_create(btn_save);
    lv_label_set_text(label_btn_save, "Save");
    lv_obj_set_pos(btn_save, 232, 190);
    lv_obj_set_size(btn_save, 54, 26);
    lv_obj_center(label_btn_save);
    lv_obj_remove_style(btn_save, &style_btn_loadSave, LV_STATE_DEFAULT);
    lv_obj_add_style(btn_save, &style_btn_loadSave, LV_STATE_DEFAULT);
    lv_obj_add_event_cb(btn_save, save_cb, LV_EVENT_RELEASED, NULL);

    btn_load = lv_btn_create(voltageCurrentCalibration);
    lv_obj_t *label_btn_load = lv_label_create(btn_load);
    lv_label_set_text(label_btn_load, "Load");
    lv_obj_set_pos(btn_load, 170, 190);
    lv_obj_set_size(btn_load, 54, 26);
    lv_obj_center(label_btn_load);
    lv_obj_remove_style(btn_load, &style_btn_loadSave, LV_STATE_DEFAULT);
    lv_obj_add_style(btn_load, &style_btn_loadSave, LV_STATE_DEFAULT);
    lv_obj_add_event_cb(btn_load, load_cb, LV_EVENT_RELEASED, NULL);

    // Create line
    auto createLine = [&](lv_obj_t *parent, const lv_point_t points[], lv_coord_t width, lv_style_t *style_,
                          int color, lv_coord_t dash_width, lv_coord_t dash_gap,
                          lv_coord_t x_ofs, lv_coord_t y_ofs)
    {
        /*Create an array for the points of the line*/
        /*Create style*/
        lv_style_set_line_width(style_, width);
        lv_style_set_line_color(style_, lv_color_hex(color));

        lv_style_set_line_dash_width(style_, dash_width);
        lv_style_set_line_dash_gap(style_, dash_gap);

        /*Create a line and apply the new style*/
        lv_obj_t *line1;
        line1 = lv_line_create(parent);
        lv_line_set_points(line1, points, 2); /*Set the points*/
        lv_obj_remove_style_all(line1);
        lv_obj_add_style(line1, style_, 0);
        // lv_obj_center(line1);
        lv_obj_align_to(line1, parent, LV_ALIGN_BOTTOM_LEFT, x_ofs + 40, y_ofs - 195);
    };

    int yOffset = 270;
    int xoff = 20;
    static lv_style_t line_style;
    lv_style_init(&line_style);
    static lv_point_t line_points[] = {{0, 80}, {70, 50}};
    createLine(voltageCurrentCalibration, line_points, 1, &line_style, 0xffc107, 5, 0, xoff, yOffset - 5);

    static lv_style_t dash_style;
    lv_style_init(&dash_style);
    static lv_point_t line2_points[] = {{0, 0}, {70, 0}};
    createLine(voltageCurrentCalibration, line2_points, 1, &dash_style, 0xffffff, 5, 6, xoff, yOffset - 30);

    static lv_point_t line3_points[] = {{62, 0}, {62, 40}};
    createLine(voltageCurrentCalibration, line3_points, 1, &dash_style, 0xffffff, 5, 3, xoff, yOffset);

    static lv_point_t line4_points[] = {{0, 0}, {20, 0}};
    createLine(voltageCurrentCalibration, line4_points, 1, &dash_style, 0xffffff, 2, 6, xoff, yOffset - 10);

    static lv_point_t line5_points[] = {{13, 0}, {13, 15}};
    createLine(voltageCurrentCalibration, line5_points, 1, &dash_style, 0xffffff, 2, 3, xoff, yOffset);

    // Rectangular frame
    static lv_style_t line_style2;
    lv_style_init(&line_style2);

    static lv_point_t line6_points[] = {{0, 0}, {70, 0}};
    createLine(voltageCurrentCalibration, line6_points, 1, &line_style2, 0x080808, 5, 0, xoff, yOffset);
    createLine(voltageCurrentCalibration, line6_points, 1, &line_style2, 0x080808, 5, 0, xoff, yOffset - 40);

    static lv_point_t line7_points[] = {{0, 0}, {0, 40}};
    createLine(voltageCurrentCalibration, line7_points, 1, &line_style2, 0x0a0a0a, 5, 0, xoff, yOffset);
    createLine(voltageCurrentCalibration, line7_points, 1, &line_style2, 0x6a6a6a, 5, 0, 70 + xoff, yOffset);

    // static lv_style_t style_lbl;
    // lv_style_init(&style_lbl);
    // lv_style_set_text_font(&style_lbl, &graph_R_8); //
    // lv_style_set_bg_color(&style_lbl, lv_color_hex(0xFF0000));

    auto createLabel = [&](lv_obj_t *parent, const char *labelText, lv_align_t align, lv_coord_t x_ofs, lv_coord_t y_ofs)
    {
        lv_obj_t *_label = lv_label_create(parent);
        lv_obj_remove_style_all(_label);
        lv_label_set_recolor(_label, true); /*Enable re-coloring by commands in the text*/
        lv_label_set_text(_label, labelText);
        lv_obj_align_to(_label, parent, align, x_ofs, y_ofs);
        // lv_obj_add_style(_label, &style_lbl, LV_STATE_DEFAULT);
        // return _label;
    };

    yOffset -= 10;

    createLabel(voltageCurrentCalibration, "#FFFFF7 Code1#", LV_ALIGN_TOP_LEFT, 2, yOffset + -15);
    createLabel(voltageCurrentCalibration, "#FFFFF7 Code2#", LV_ALIGN_TOP_LEFT, 2, yOffset + 5);
    createLabel(voltageCurrentCalibration, "#FFFFF7 Vin1#", LV_ALIGN_TOP_LEFT, 60, yOffset + 30);
    createLabel(voltageCurrentCalibration, "#FFFFF7 Vin2#", LV_ALIGN_TOP_LEFT, 60 + 49, yOffset + 30);

    // lv_obj_t *label_title = lv_label_create(voltageCurrentCalibration);
    lv_obj_t *label_m = lv_label_create(voltageCurrentCalibration);
    lv_obj_t *label_m_num = lv_label_create(voltageCurrentCalibration);
    lv_obj_t *label_m_den = lv_label_create(voltageCurrentCalibration);
    lv_obj_t *label_b = lv_label_create(voltageCurrentCalibration);
    lv_obj_t *label_Vin_calib = lv_label_create(voltageCurrentCalibration);

    // lv_label_set_text(label_title, "Voltage Calibration");
    // lv_obj_align(label_title, LV_ALIGN_TOP_LEFT, 127, 0);

    // memory = ESP.getFreeHeap();
    lv_label_set_text(label_m_num, "Code2 - Code1");
    lv_label_set_text(label_m, "m = --------------------- =");
    lv_label_set_text(label_m_den, "Vin2 - Vin1");

    // LV_LOG_USER("Used memory:%i", memory - ESP.getFreeHeap());
    verPad += yOffset;
    lv_obj_align(label_m_num, LV_ALIGN_TOP_LEFT, 35, verPad + 10);
    lv_obj_align(label_m, LV_ALIGN_TOP_LEFT, 0, verPad + 20);
    lv_obj_align(label_m_den, LV_ALIGN_TOP_LEFT, 50, verPad + 30);

    lbl_voltageCalib_m = lv_label_create(voltageCurrentCalibration);
    lbl_voltageCalib_b = lv_label_create(voltageCurrentCalibration);

    lv_obj_align(lbl_voltageCalib_m, LV_ALIGN_TOP_LEFT, 160, verPad + 20);

    lv_label_set_text(label_b, "b = Code1 - m * Vin1 =");
    lv_obj_align(label_b, LV_ALIGN_TOP_LEFT, 0, verPad + 30 + 30 * 1);
    lv_obj_align(lbl_voltageCalib_b, LV_ALIGN_TOP_LEFT, 160, verPad + 30 + 30 * 1);

    lv_label_set_text(label_Vin_calib, "Vin_cal = (Code-b)/m");
    lv_obj_align(label_Vin_calib, LV_ALIGN_TOP_LEFT, 0, verPad + 30 + 30 * 2);

    lv_obj_t *dummy = lv_label_create(voltageCurrentCalibration);
    lv_label_set_text(dummy, "");
    lv_obj_align(dummy, LV_ALIGN_TOP_LEFT, 65, verPad + 60 + 30 * 3);
}

static lv_obj_t *create_switch_(lv_obj_t *parent, const char *icon, const char *txt, bool chk,
                                lv_event_cb_t event_cb, lv_event_code_t filter, void *user_data)
{
    // lv_obj_t *obj = create_text(parent, icon, txt, LV_MENU_ITEM_BUILDER_VARIANT_1);
    // Setting menu
    lv_obj_t *obj = lv_menu_cont_create(parent);
    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(obj, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // Setting Text
    lv_obj_t *label = lv_label_create(obj);
    lv_label_set_text(label, txt);
    lv_label_set_long_mode(label, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_flex_grow(label, 2);

    lv_obj_t *sw = lv_switch_create(obj);
    lv_obj_add_state(sw, chk ? LV_STATE_CHECKED : 0);
    // lv_obj_set_flex_grow(sw, 15);
    lv_obj_add_flag(sw, LV_OBJ_FLAG_FLEX_IN_NEW_TRACK);
    // lv_obj_set_flex_align(sw, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_END);

    lv_obj_add_event_cb(sw, PRESSED_event_cb, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(sw, RELEASED_event_cb, LV_EVENT_RELEASED, NULL);
    lv_obj_add_event_cb(sw, event_cb, filter, user_data);

    return obj;
}

int8_t lastButton = 0;
static void trackPress(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);
    lastButton = lv_obj_get_index(obj);
    // Serial.println("Pressed");
}

lv_obj_t *slider_Avgs;
void SettingMenu(lv_obj_t *parent)
{

    // g = lv_group_create();
    menu = lv_menu_create(parent);

    lv_color_t bg_color = lv_obj_get_style_bg_color(menu, 0);
    if (lv_color_brightness(bg_color) > 127)
    {
        lv_obj_set_style_bg_color(menu, lv_color_darken(lv_obj_get_style_bg_color(menu, 0), 10), 0);
    }
    else
    {
        lv_obj_set_style_bg_color(menu, lv_color_darken(lv_obj_get_style_bg_color(menu, 0), 50), 0);
    }

    lv_menu_set_mode_root_back_btn(menu, LV_MENU_ROOT_BACK_BTN_DISABLED);

    lv_obj_set_size(menu, 294, 200);
    lv_obj_center(menu);

    // Return => OK

    lv_obj_t *section;

    /*Create sub pages*/
    lv_obj_t *sub_display_page = lv_menu_page_create(menu, NULL);
    lv_obj_set_style_pad_hor(sub_display_page, lv_obj_get_style_pad_left(lv_menu_get_main_header(menu), 0), 0);
    // lv_menu_separator_create(sub_display_page);
    section = lv_menu_section_create(sub_display_page);
    create_slider(section, LV_SYMBOL_SETTINGS, "Backlight", 0, 255, 255, slider_backlight_event_cb, LV_EVENT_VALUE_CHANGED);

    /******************************
     ** Buzzer
     ******************************/
    lv_obj_t *sub_sound_page = lv_menu_page_create(menu, NULL);
    section = lv_menu_section_create(sub_sound_page);
    create_switch_(section, LV_SYMBOL_AUDIO, "Buzzer", buzzerSound, switch_buzzer_event_cb, LV_EVENT_SHORT_CLICKED, NULL);

    /******************************
     ** ADC SPS, ADC # Avgs, Number of digits, Auto BAr-graph
     ******************************/
    lv_obj_t *sub_measure_page = lv_menu_page_create(menu, NULL);
    section = lv_menu_section_create(sub_measure_page);
    create_slider(section, NULL, "ADC SPS", 0, 3, PowerSupply.settingParameters.adcRate, slider_adcRate_event_cb, LV_EVENT_VALUE_CHANGED); // 0:20, 1:90. 2:330,3:1000
    slider_Avgs = create_slider(section, NULL, "ADC # of Avgs", 0, log2(MAX_NO_OF_AVG), PowerSupply.settingParameters.adcNumberOfAvgs, slider_adcAVG_event_cb, LV_EVENT_VALUE_CHANGED);
    create_slider(section, NULL, "Number of Digits", 1, 4, PowerSupply.settingParameters.adcNumberOfDigits, slider_decimalPoints_event_cb, LV_EVENT_VALUE_CHANGED);
    create_switch_(section, NULL, "Auto Bar-Graph", false, switch_buzzer_event_cb, LV_EVENT_VALUE_CHANGED, menu);

    /******************************
     ** Calibration
     ******************************/
    lv_obj_t *sub_calibration_page = lv_menu_page_create(menu, NULL);
    section = lv_menu_section_create(sub_calibration_page);
    create_pushbutton(section, NULL, btn_calibration_ADC_event_cb, LV_SYMBOL_CHARGE, "V/I ADC");
    create_pushbutton(section, NULL, btn_calibration_DAC_event_cb, LV_SYMBOL_CHARGE, "V/I DAC");
    create_pushbutton(section, NULL, LCD_Calibration_cb, "", "LCD Touch");
    create_pushbutton(section, NULL, NULL, "Statistics", "Reset Stats");

    /******************************
     ** Save/Load
     ******************************/
    lv_obj_t *sub_save_load = lv_menu_page_create(menu, NULL);
    section = lv_menu_section_create(sub_save_load);

    /******************************
     ** Version
     ******************************/
    lv_obj_t *sub_software_info_page = lv_menu_page_create(menu, NULL);
    section = lv_menu_section_create(sub_software_info_page);
    lv_obj_t *obj0;
    // create_text(section, NULL, "Version 1.0", LV_MENU_ITEM_BUILDER_VARIANT_1, obj0);

    // Use the SOFTWARE_VERSION macro
    char version_text[50];
    snprintf(version_text, sizeof(version_text), "Version %s", SOFTWARE_VERSION);
    create_text(section, NULL, version_text, LV_MENU_ITEM_BUILDER_VARIANT_1, obj0);

    lv_obj_t *sub_about_page = lv_menu_page_create(menu, NULL);
    section = lv_menu_section_create(sub_about_page);
    obj0 = create_text(section, NULL, "Software information", LV_MENU_ITEM_BUILDER_VARIANT_1, obj0);
    lv_menu_set_load_page_event(menu, obj0, sub_software_info_page);
    root_page = lv_menu_page_create(menu, NULL);
    lv_obj_set_style_pad_hor(root_page, lv_obj_get_style_pad_left(lv_menu_get_main_header(menu), 0), 0);
    section = lv_menu_section_create(root_page);

    obj0 = create_text(section, NULL, "Display", LV_MENU_ITEM_BUILDER_VARIANT_1, obj0);
    lv_menu_set_load_page_event(menu, obj0, sub_display_page);
    lv_obj_add_event_cb(obj0, trackPress, LV_EVENT_RELEASED, NULL);

    obj0 = create_text(section, NULL, "Sound", LV_MENU_ITEM_BUILDER_VARIANT_1, obj0);
    lv_menu_set_load_page_event(menu, obj0, sub_sound_page);
    lv_obj_add_event_cb(obj0, trackPress, LV_EVENT_RELEASED, NULL);

    obj0 = create_text(section, NULL, "Measure", LV_MENU_ITEM_BUILDER_VARIANT_1, obj0);
    lv_menu_set_load_page_event(menu, obj0, sub_measure_page);
    lv_obj_add_event_cb(obj0, trackPress, LV_EVENT_RELEASED, NULL);

    obj0 = create_text(section, NULL, "Calibration", LV_MENU_ITEM_BUILDER_VARIANT_1, obj0);
    lv_menu_set_load_page_event(menu, obj0, sub_calibration_page); //=> Error!
    lv_obj_add_event_cb(obj0, trackPress, LV_EVENT_RELEASED, NULL);

    obj0 = create_text(section, NULL, "Save/Load", LV_MENU_ITEM_BUILDER_VARIANT_1, obj0);
    lv_menu_set_load_page_event(menu, obj0, sub_save_load); //=> Error!
    lv_obj_add_event_cb(obj0, trackPress, LV_EVENT_RELEASED, NULL);

    obj0 = create_text(section, NULL, "About", LV_MENU_ITEM_BUILDER_VARIANT_1, obj0);
    lv_menu_set_load_page_event(menu, obj0, sub_about_page);
    lv_obj_add_event_cb(obj0, trackPress, LV_EVENT_RELEASED, NULL);

    lv_menu_set_sidebar_page(menu, root_page);

    // Adjust size of the sidebar menu
    lv_menu_t *menu_ = (lv_menu_t *)menu;
    lv_obj_set_size(menu_->sidebar, LV_PCT(38), LV_PCT(100));
    lv_event_send(lv_obj_get_child(lv_obj_get_child(lv_menu_get_cur_sidebar_page(menu), 0), 0), LV_EVENT_CLICKED, NULL);
}

static void switch_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    // lv_obj_t * menu = lv_event_get_user_data(e);
    lv_obj_t *menu = (lv_obj_t *)lv_event_get_user_data(e);
    lv_obj_t *obj = lv_event_get_target(e);
    if (code == LV_EVENT_VALUE_CHANGED)
    {

        if (lv_obj_has_state(obj, LV_STATE_CHECKED))
        {
            lv_menu_set_page(menu, NULL);
            lv_menu_set_sidebar_page(menu, root_page);
            lv_event_send(lv_obj_get_child(lv_obj_get_child(lv_menu_get_cur_sidebar_page(menu), 0), 0), LV_EVENT_CLICKED, NULL);
        }
        else
        {
            lv_menu_set_sidebar_page(menu, NULL);
            lv_menu_clear_history(menu); /* Clear history because we will be showing the root page later */
            lv_menu_set_page(menu, root_page);
        }
    }
}

static lv_obj_t *create_text(lv_obj_t *parent, const char *icon, const char *txt,
                             lv_menu_builder_variant_t builder_variant, lv_obj_t *obj)
{
    obj = lv_menu_cont_create(parent);
    // lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_COLUMN);
    // lv_obj_set_flex_align(obj, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    lv_obj_t *img = NULL;
    lv_obj_t *label = NULL;

    if (icon)
    {
        img = lv_img_create(obj);
        lv_img_set_src(img, icon);
    }

    if (txt)
    {
        label = lv_label_create(obj);
        lv_label_set_text(label, txt);
        // lv_label_set_long_mode(label, LV_LABEL_LONG_SCROLL_CIRCULAR);
        lv_obj_set_flex_grow(label, 0);
        lv_obj_set_height(label, 11);
    }

    if (builder_variant == LV_MENU_ITEM_BUILDER_VARIANT_2 && icon && txt)
    {
        lv_obj_add_flag(img, LV_OBJ_FLAG_FLEX_IN_NEW_TRACK);
        lv_obj_swap(img, label);
    }

    return obj;
}

static lv_obj_t *create_setting_item(lv_obj_t *parent, const char *icon, const char *txt,
                                     lv_menu_builder_variant_t builder_variant)
{
    lv_obj_t *obj = lv_menu_cont_create(parent);

    lv_obj_t *img = NULL;
    lv_obj_t *label = NULL;

    if (icon)
    {
        img = lv_img_create(obj);
        lv_img_set_src(img, icon);
    }

    if (txt)
    {
        label = lv_label_create(obj);
        lv_label_set_text(label, txt);
        lv_label_set_long_mode(label, LV_LABEL_LONG_SCROLL_CIRCULAR);
        lv_obj_set_flex_grow(label, 1);
    }

    if (builder_variant == LV_MENU_ITEM_BUILDER_VARIANT_2 && icon && txt)
    {
        lv_obj_add_flag(img, LV_OBJ_FLAG_FLEX_IN_NEW_TRACK);
        lv_obj_swap(img, label);
    }

    return obj;
}
static void slider_backlight_event_cb(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);
    int32_t v = lv_slider_get_value(obj);
    // ledcWrite(lcdBacklightChannel, v);

    // LV_UNUSED(obj);
    // LV_LOG_USER("Clicked: %i", code);

    char buf[8];
    lv_snprintf(buf, sizeof(buf), "%d", v);
    // lv_label_set_text(lv_obj_get_child(lv_obj_get_child(lv_obj_get_parent(obj), 0), 0), buf);

    lv_label_set_text(lv_obj_get_child(lv_obj_get_parent(obj), 1), buf);

    // LV_LOG_USER("Y: %i", lv_obj_get_child_cnt(lv_obj_get_child(lv_obj_get_parent(obj),0)));
}

static void slider_adcRate_event_cb(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);
    uint8_t v = lv_slider_get_value(obj);

    lv_label_create(lv_obj_get_child(obj, 0));

    static std::map<int, const char *> ADC_SPS;

    ADC_SPS[0] = "  20";
    ADC_SPS[1] = "  90";
    ADC_SPS[2] = " 330";
    ADC_SPS[3] = "1000";

    // lv_label_set_text(slider_label2, ADC_SPS.at(v));
    lv_label_set_text(lv_obj_get_child(lv_obj_get_parent(obj), 1), ADC_SPS.at(v));
    PowerSupply.adc.ads1219->setDataRate(atoi(ADC_SPS.at(v)));
    PowerSupply.settingParameters.adcRate = v;

    // Reset statistics too
    PowerSupply.ResetStats();
}

static void slider_adcAVG_event_cb(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);
    uint16_t v = lv_slider_get_value(obj);

    // lv_obj_t *slider_label = lv_label_create(lv_obj_get_child(obj, 0));

    // lv_label_set_text(slider_label, "");
    // lv_obj_set_pos(slider_label, 180, 0);

    char buf[6];
    lv_snprintf(buf, sizeof(buf), "%-d", (uint16_t)std::pow(2, v));

    // lv_label_set_text(slider_label2, ADC_AVG.at(v));
    lv_label_set_text(lv_obj_get_child(lv_obj_get_parent(obj), 1), buf);
    PowerSupply.Voltage.measured.SetWindowSize(std::pow(2, v));
    PowerSupply.Current.measured.SetWindowSize(std::pow(2, v));
    PowerSupply.Power.measured.SetWindowSize(std::pow(2, v));

    PowerSupply.settingParameters.adcNumberOfAvgs = v;
}

static void slider_decimalPoints_event_cb(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);
    uint16_t v = lv_slider_get_value(obj);

    // lv_obj_t *slider_label = lv_label_create(lv_obj_get_child(obj, 0));

    // lv_label_set_text(slider_label, "");
    // lv_obj_set_pos(slider_label, 180, 0);

    char buf[4];
    lv_snprintf(buf, sizeof(buf), "%i", v);

    // lv_label_set_text(slider_label2, ADC_AVG.at(v));
    lv_label_set_text(lv_obj_get_child(lv_obj_get_parent(obj), 1), buf);
    switch (v)
    {
    case 1:
        PowerSupply.Voltage.restrict = "%+05.1f";
        PowerSupply.Current.restrict = "%+05.1f";
        PowerSupply.Power.restrict = "%+05.1f";
        break;
    case 2:
        PowerSupply.Voltage.restrict = "%+06.2f";
        PowerSupply.Current.restrict = "%+06.2f";
        PowerSupply.Power.restrict = "%+06.2f";
        break;
    case 3:
        PowerSupply.Voltage.restrict = "%+07.3f";
        PowerSupply.Current.restrict = "%+07.3f";
        PowerSupply.Power.restrict = "%+07.3f";
        break;
    case 4:
        PowerSupply.Voltage.restrict = "%+08.4f";
        PowerSupply.Current.restrict = "%+08.4f";
        PowerSupply.Power.restrict = "%+08.4f";
        break;

    default:
        PowerSupply.Voltage.restrict = "%+07.3f";
        PowerSupply.Current.restrict = "%+07.3f";
        PowerSupply.Power.restrict = "%+07.3f";
    }
    PowerSupply.settingParameters.adcNumberOfDigits = v;
}

static void switch_buzzer_event_cb(lv_event_t *e)
{

    // static void btn_event_cb(lv_event_t *e)
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);

    lv_obj_get_parent(btn);

    if (code == LV_EVENT_VALUE_CHANGED)
    {
        if (lv_obj_get_state(btn) & LV_STATE_CHECKED)
        {

            // lv_label_set_text(label, "ON");
            buzzerSound = true;
            myTone(NOTE_A5, 10, true);
        }
        else
        {
            myTone(NOTE_A3, 10);
            //  lv_label_set_text(label, "OFF");
            buzzerSound = false;
        }
    }
    /*Create a label below the slider*/

    // LV_UNUSED(obj);
    // LV_LOG_USER("Clicked: %s %i", label, lv_obj_get_state(btn));
}

static void PRESSED_event_cb(lv_event_t *e)
{
    myTone(NOTE_A4, 5);
}
static void RELEASED_event_cb(lv_event_t *e)
{
    myTone(NOTE_A4, 5);
}
static void MainMenu_event_cb(lv_event_t *e)
{
    lv_obj_add_flag(voltageCurrentCalibration, LV_OBJ_FLAG_HIDDEN);
}

static void btn_calibration_ADC_event_cb(lv_event_t *e)
{

    lv_obj_clear_flag(voltageCurrentCalibration, LV_OBJ_FLAG_HIDDEN);
}

static void btn_close_hide_obj_cb(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);
    //  lv_obj_add_flag(lv_obj_get_parent(lv_obj_get_parent(obj)), LV_OBJ_FLAG_HIDDEN);
    // lv_obj_del(lv_obj_get_parent(lv_obj_get_parent(obj)));
    lv_obj_add_flag(lv_obj_get_parent(lv_obj_get_parent(obj)), LV_OBJ_FLAG_HIDDEN);
}
static void event_handler(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);
    LV_LOG_USER("Button %d clicked", (int)lv_obj_get_index(obj));
}

static void btn_calibration_DAC_event_cb(lv_event_t *e)
{

    if (win_dac_already_created)
    {
        lv_obj_clear_flag(win_DAC_calibration, LV_OBJ_FLAG_HIDDEN);
        return;
    }

    win_DAC_calibration = lv_win_create(lv_scr_act(), 36);
    lv_obj_set_size(win_DAC_calibration, 320, 226);
    lv_obj_t *btn;
    // btn = lv_win_add_btn(win_DAC_calibration, LV_SYMBOL_LEFT, 40);
    // lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);

    lv_win_add_title(win_DAC_calibration, "DAC Calibration");

    // btn = lv_win_add_btn(win_DAC_calibration, LV_SYMBOL_RIGHT, 40);
    // lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);

    btn = lv_win_add_btn(win_DAC_calibration, LV_SYMBOL_CLOSE, 60);
    // lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(btn, btn_close_hide_obj_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *cont = lv_win_get_content(win_DAC_calibration); /*Content can be added here*/
    lv_obj_set_style_pad_all(cont, 0, LV_PART_ITEMS);
    lv_obj_set_style_pad_all(cont, 0, LV_PART_MAIN);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *_label = lv_label_create(cont);
    lv_label_set_text(_label, "DAC Code      Output V/C");
    lv_obj_align(_label, LV_ALIGN_OUT_LEFT_TOP, 95, 10);
    lv_obj_set_style_text_font(_label, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(_label, lv_color_hex(0x00FFFF), LV_PART_MAIN | LV_STATE_DEFAULT);

    int xPos = 94;
    int yPos = 40;
    int yOffset = 25;
    static lv_obj_t *zv = spinbox_pro(cont, "#FFFFF7 Zero Voltage:#", 0, 65535, 5, 0, LV_ALIGN_DEFAULT, xPos, yPos + yOffset * 0, 86, 15);
    static lv_obj_t *mv = spinbox_pro(cont, "#FFFFF7 Max Voltage:#", 0, 65535, 5, 0, LV_ALIGN_DEFAULT, xPos, yPos + yOffset * 1, 86, 16);

    static lv_obj_t *zc = spinbox_pro(cont, "#FFFFF7 Zero Current:#", 0, 65535, 5, 0, LV_ALIGN_DEFAULT, xPos, yPos + yOffset * 2.3, 86, 17);
    static lv_obj_t *mc = spinbox_pro(cont, "#FFFFF7 Max Current:#", 0, 65535, 5, 0, LV_ALIGN_DEFAULT, xPos, yPos + yOffset * 3.3, 86, 18);

    // lv_label_set_text(label, ":)");

    auto addLabel_edit = [](lv_obj_t *parent, const char *fmt, double value, int color,
                            lv_coord_t x, lv_coord_t y)
    {
        lv_obj_t *parent_child = lv_obj_get_child(parent, 1);

        lv_obj_align(parent_child, LV_ALIGN_OUT_RIGHT_MID, -90, 0);

        lv_obj_t *label_zero_volt = lv_label_create(parent);
        lv_obj_add_flag(parent, LV_OBJ_FLAG_OVERFLOW_VISIBLE);
        lv_obj_align_to(label_zero_volt, parent, LV_ALIGN_OUT_RIGHT_MID, x, y);
        lv_label_set_text_fmt(label_zero_volt, fmt, value);

        lv_obj_set_style_text_font(label_zero_volt, &graph_R_16, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(label_zero_volt, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
        return label_zero_volt;
    };

    int xOffset = 10;
    addLabel_edit(zv, "+%07.4fV", 0.0, 0, xOffset, 0);
    static lv_obj_t *label_maxV = addLabel_edit(mv, "+%07.4fV", 32.750, 0, xOffset, 0);

    addLabel_edit(zc, "%+06.4fA", 0.0, 0, xOffset, 0);
    static lv_obj_t *label_maxC = addLabel_edit(mc, "%+06.4fA", 6.5535, 0, xOffset, 0);
    dac_data_g = PowerSupply.LoadDACdata("dac_data_");

    PowerSupply.Voltage.adjOffset = dac_data_g.zero_voltage;
    PowerSupply.Voltage.minValue = (-dac_data_g.zero_voltage) ;/// 2000.0;
    PowerSupply.Voltage.maxValue = (dac_data_g.max_voltage - dac_data_g.zero_voltage);// 2000.0;


    PowerSupply.Current.adjOffset = dac_data_g.zero_current;
    PowerSupply.Current.minValue = (-dac_data_g.zero_current); // 10000.0;
    PowerSupply.Current.maxValue = (dac_data_g.max_current - dac_data_g.zero_current); // 10000.0;

    static auto DAC_voltage_calib_change_event_cb = [](lv_event_t *e)
    {
        if (lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED)
        {
            double zv_value = lv_spinbox_get_value(zv);
            double mv_value = lv_spinbox_get_value(mv);
            lv_label_set_text_fmt(label_maxV, "%+6.4fV", (mv_value - zv_value) / 2000.0);

            dac_data_g.zero_voltage = lv_spinbox_get_value(zv);
            dac_data_g.max_voltage = lv_spinbox_get_value(mv);
            dac_data_g.zero_current = lv_spinbox_get_value(zc);
            dac_data_g.max_current = lv_spinbox_get_value(mc);

            PowerSupply.Voltage.adjOffset = dac_data_g.zero_voltage;
            PowerSupply.Voltage.minValue = (-dac_data_g.zero_voltage); // 2000.0;
            PowerSupply.Voltage.maxValue = (mv_value - zv_value);  // 2000.0;
            PowerSupply.SaveDACdata("dac_data_", dac_data_g);
        }
    };

    static auto DAC_current_calib_change_event_cb = [](lv_event_t *e)
    {
        if (lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED)
        {
            double zc_value = lv_spinbox_get_value(zc);
            double mc_value = lv_spinbox_get_value(mc);
            lv_label_set_text_fmt(label_maxC, "%+6.4fV", (mc_value - zc_value) / 10000.0);

            dac_data_g.zero_voltage = lv_spinbox_get_value(zv);
            dac_data_g.max_voltage = lv_spinbox_get_value(mv);
            dac_data_g.zero_current = lv_spinbox_get_value(zc);
            dac_data_g.max_current = lv_spinbox_get_value(mc);

            PowerSupply.Current.adjOffset = dac_data_g.zero_current;
            PowerSupply.Current.minValue = (-dac_data_g.zero_current) / 10000.0;
            PowerSupply.Current.maxValue = (mc_value - zc_value) / 10000.0;
            PowerSupply.SaveDACdata("dac_data_", dac_data_g);
        }
    };

    dac_data_g = PowerSupply.LoadDACdata("dac_data_");

    lv_spinbox_set_value(zv, dac_data_g.zero_voltage);
    lv_spinbox_set_value(mv, dac_data_g.max_voltage);
    lv_spinbox_set_value(zc, dac_data_g.zero_current);
    lv_spinbox_set_value(mc, dac_data_g.max_current);

    // lv_obj_add_event_cb(zv, spinbox_pro_change_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    // auto update_added_label = [](lv_obj_t *spinbox_pro_obj, const char *fmt, double value, int color,
    //                    lv_coord_t x, lv_coord_t y)
    // {
    //     lv_obj_t *label_zero_volt = lv_label_create(parent);
    //     lv_obj_add_flag(parent, LV_OBJ_FLAG_OVERFLOW_VISIBLE);
    //     lv_obj_align_to(label_zero_volt, parent, LV_ALIGN_OUT_RIGHT_MID, x, y);
    //     lv_label_set_text_fmt(label_zero_volt, fmt, value);

    //     lv_obj_set_style_text_font(label_zero_volt, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
    //     lv_obj_set_style_text_color(label_zero_volt, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);

    // };
    lv_obj_add_event_cb(zv, DAC_voltage_calib_change_event_cb, LV_EVENT_VALUE_CHANGED, label_maxV);
    lv_obj_add_event_cb(mv, DAC_voltage_calib_change_event_cb, LV_EVENT_VALUE_CHANGED, label_maxV);

    lv_obj_add_event_cb(zc, DAC_current_calib_change_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(mc, DAC_current_calib_change_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    win_dac_already_created = true;

    return;
    lv_obj_t *DAC_Calibration = lv_obj_create(lv_scr_act());

    lv_obj_set_size(DAC_Calibration, 320, 240);
    lv_obj_align(DAC_Calibration, LV_ALIGN_CENTER, 0, 0);
    // static lv_style_t style_panel;
    // lv_style_init(&style_panel);
    // remove all padding
    // lv_style_set_pad_ver(&style_panel, 0);
    // lv_style_set_pad_hor(&style_panel, 0);
    // lv_obj_add_style(DAC_Calibration, &style_panel, LV_STATE_DEFAULT);

    /********************
     * close button
     ********************/
    lv_obj_t *close_btn = lv_btn_create(DAC_Calibration);
    lv_obj_t *close_btn_label = lv_label_create(close_btn);
    lv_obj_remove_style_all(close_btn_label);
    // lv_obj_remove_style_all(close_btn);

    lv_label_set_text(close_btn_label, " X ");
    lv_obj_center(close_btn_label);

    lv_obj_set_pos(close_btn, 0, 0);
    lv_obj_add_event_cb(close_btn, btn_close_hide_obj_cb, LV_EVENT_RELEASED, NULL);
    lv_obj_set_align(close_btn, LV_ALIGN_TOP_RIGHT);

    lv_obj_set_content_width(close_btn_label, 20);  // The actual width: padding left + 50 + padding right
    lv_obj_set_content_height(close_btn_label, 20); // The actual width: padding top + 30 + padding bottom
}

static void LCD_Calibration_cb(lv_event_t *e)
{
    uint16_t calData[5];
    uint8_t calDataOK = 0;

    // Calibrate
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(20, 0);
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    tft.println("Touch corners as indicated");

    tft.setTextFont(1);
    tft.println();
    delay(2000);
    tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);

    Serial.println();
    Serial.println();
    Serial.println("// Use this calibration code in setup():");
    Serial.print("  uint16_t calData[5] = ");
    Serial.print("{ ");

    for (uint8_t i = 0; i < 5; i++)
    {
        Serial.print(calData[i]);
        if (i < 4)
            Serial.print(", ");
    }

    Serial.println(" };");
    Serial.print("  tft.setTouch(calData);");
    Serial.println();
    Serial.println();
    tft.setTouch(calData);
    Serial.println("tft calibrated by tft.setTouch(calData)");

    tft.println("Calibration complete!");

    delay(2000);
    lv_obj_invalidate(Tabs::tabview);

    // tft.setTextColor(TFT_GREEN, TFT_BLACK);
    // tft.println("Calibration complete!");
    // tft.println("Calibration code sent to Serial port.");
}

static void PRESSED_label_event_cb(lv_event_t *e)
{
    myTone(NOTE_A4, 5);
}

/**
 * Create a slider for menu with icon, text, min max ....
 * @param parent pointer to the parent.
 * @param icon pointer to an icon (not used).
 * @param txt  pointer to the text.
 * @param min min value of the slider.
 * @param max max value of the slider.
 * @param val default value of the slider.
 * @param event_cb pointer to an icon.
 * @param filter pointer to an icon.
 * @param txtDefault pointer to an icon.
 * @return pointer to the new slider menu
 */
lv_obj_t *create_slider(lv_obj_t *parent, const char *icon, const char *txt, int32_t min, int32_t max,
                        int32_t val, lv_event_cb_t event_cb, lv_event_code_t filter)
{
    // Setting menu
    lv_obj_t *obj = lv_menu_cont_create(parent);
    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(obj, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_END);

    // Setting Text
    lv_obj_t *label = lv_label_create(obj);
    lv_label_set_text(label, txt);
    lv_label_set_long_mode(label, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_flex_grow(label, 2);

    // Slider label
    lv_obj_t *slider_valueLabel = lv_label_create(obj);
    lv_label_set_text(slider_valueLabel, std::to_string(val).c_str());
    // lv_obj_set_size(slider_valueLabel, 30,10);

    // Slider
    lv_obj_t *slider = lv_slider_create(obj);
    lv_obj_set_flex_grow(slider, 15);
    lv_obj_add_flag(slider, LV_OBJ_FLAG_FLEX_IN_NEW_TRACK);
    lv_obj_add_event_cb(slider, event_cb, filter, NULL);
    lv_slider_set_range(slider, min, max);
    lv_slider_set_value(slider, val, LV_ANIM_OFF);
    lv_obj_add_event_cb(slider, PRESSED_event_cb, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(slider, RELEASED_event_cb, LV_EVENT_RELEASED, NULL);
    lv_event_send(slider, LV_EVENT_VALUE_CHANGED, NULL);
    return obj;
}

static lv_obj_t *create_switch(lv_obj_t *parent, const char *icon, const char *txt, bool chk, lv_obj_t *obj)
{
    obj = create_text(parent, icon, txt, LV_MENU_ITEM_BUILDER_VARIANT_2, obj);

    // Setting menu
    // lv_obj_t *obj = lv_menu_cont_create(parent);
    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(obj, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_END);

    // Setting Text
    lv_obj_t *label = lv_label_create(obj);
    lv_label_set_text(label, txt);
    lv_label_set_long_mode(label, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_flex_grow(label, 2);

    lv_obj_t *sw = lv_switch_create(obj);
    lv_obj_add_state(sw, chk ? LV_STATE_CHECKED : 0);
    // lv_obj_set_layout(sw,LV_LAYOUT_FLEX);
    // lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_ROW);

    // if (icon == NULL)
    // {
    //     lv_obj_add_flag(sw, LV_OBJ_FLAG_FLEX_IN_NEW_TRACK);
    // }

    // lv_obj_set_x(sw, 1);
    // lv_obj_set_y(sw, 1);
    // if (lv_obj_get_child_cnt(obj) > 2)
    //     lv_obj_add_state(lv_obj_get_child(obj, 2), 0);

    // lv_obj_align(sw, LV_ALIGN_TOP_RIGHT, 210, 80);

    /// lv_obj_set_y(lv_obj_get_child(obj,1),10);

    lv_obj_add_event_cb(sw, PRESSED_event_cb, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(sw, RELEASED_event_cb, LV_EVENT_RELEASED, NULL);

    return obj;
}
// struct _lv_event_dsc_t *lv_obj_add_event_cb(lv_obj_t *obj, lv_event_cb_t event_cb, lv_event_code_t filter,
//                                             void *user_data);

static lv_obj_t *create_pushbutton(lv_obj_t *parent, const char *icon, lv_event_cb_t event_cb, const char *txt, const char *buttonTxt)
{
    // lv_obj_t *obj = create_text(parent, icon, txt, LV_MENU_ITEM_BUILDER_VARIANT_1);

    // Setting menu
    static lv_obj_t *obj = lv_menu_cont_create(parent);
    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_COLUMN);
    // lv_obj_set_flex_align(obj, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_SPACE_EVENLY);

    // Setting Text
    // if (*txt != 0)
    // {
    // lv_obj_t *menuLabel = lv_label_create(obj);
    // lv_label_set_text(menuLabel, txt);
    // lv_label_set_long_mode(menuLabel, LV_LABEL_LONG_SCROLL_CIRCULAR);
    // lv_obj_set_flex_grow(menuLabel, 2);
    // }

    lv_obj_t *btn = lv_btn_create(obj);
    lv_obj_set_size(btn, 120, 30);
    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text(label, buttonTxt);
    lv_obj_center(label);
    // if (*txt != 0)
    // lv_obj_add_flag(btn, LV_OBJ_FLAG_FLEX_IN_NEW_TRACK);

    lv_obj_add_event_cb(btn, PRESSED_event_cb, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(btn, RELEASED_event_cb, LV_EVENT_RELEASED, NULL);
    lv_obj_add_event_cb(btn, event_cb, LV_EVENT_RELEASED, NULL);

    return obj;
}

#endif