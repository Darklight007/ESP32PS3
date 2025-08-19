#include "setting_menu.h"

// heavy/private includes live ONLY in the .cpp
#include "spinbox_pro.h"
#include "version.h"
#include "buzzer.h"   // provides buzzerSound and myTone()
#include "TFT_eSPI.h" // tft instance
#include "tabs.h"
#include "device.h"        // provides PowerSupply and dac_data_g
#include "lv_gui_helper.h" // LVLabel, LVButton helpers

#include <cmath>
#include <map>
#include <cstdio>

// ---------- public objects (one definition) ----------

lv_obj_t *myTextBox = nullptr;
int8_t lastButton = 0;

setting_GUI Calib_GUI_voltage{};
setting_GUI Calib_GUI_current{};

// ---------- internal state / helpers ----------
namespace
{
    struct
    {
        int current_index{};
        int ids[10]{};
        int count{};
        int digit_count[10]{};
        int id_index{};
        lv_obj_t *btn_plus[10]{};
        lv_obj_t *btn_minus[10]{};
    } spinboxes;

    // styles used during focus/highlight
    lv_style_t style_hide;
    lv_style_t style_unhide;
    lv_style_t style_border_def;
    lv_style_t style_border_select;
    bool styles_inited = false;

    // DAC window widgets we need in its callbacks
    lv_obj_t *s_zv = nullptr; // zero voltage spin
    lv_obj_t *s_mv = nullptr; // max  voltage spin
    lv_obj_t *s_zc = nullptr; // zero current spin
    lv_obj_t *s_mc = nullptr; // max  current spin

    lv_obj_t *s_label_maxV = nullptr;
    lv_obj_t *s_label_maxC = nullptr;

    // pressed/released beeps
    static void PRESSED_event_cb(lv_event_t *) { myTone(NOTE_A4, 5); }
    static void RELEASED_event_cb(lv_event_t *) { myTone(NOTE_A4, 5); }

    static void btn_close_hide_obj_cb(lv_event_t *e)
    {
        auto *btn = lv_event_get_target(e);
        lv_obj_add_flag(lv_obj_get_parent(lv_obj_get_parent(btn)), LV_OBJ_FLAG_HIDDEN);
    }

    static void trackPress(lv_event_t *e)
    {
        auto *obj = lv_event_get_target(e);
        lastButton = lv_obj_get_index(obj);
    }

    static void trackChild(lv_event_t *e)
    {
        auto *obj = lv_event_get_target(e);
        spinboxes.current_index = lv_obj_get_index(obj);

        if (!styles_inited)
        {
            lv_style_init(&style_hide);
            lv_style_init(&style_unhide);
            lv_style_init(&style_border_def);
            lv_style_init(&style_border_select);
            styles_inited = true;
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

        for (int i = 0; i < spinboxes.count; ++i)
        {
            if (spinboxes.ids[i] != spinboxes.current_index)
            {
                auto *sib = lv_obj_get_child(lv_obj_get_parent(obj), spinboxes.ids[i]);
                lv_obj_remove_style(sib, &style_unhide, LV_PART_CURSOR);
                lv_obj_remove_style(sib, &style_hide, LV_PART_CURSOR);
                lv_obj_remove_style(sib, &style_border_def, LV_STATE_DEFAULT);

                lv_obj_add_style(sib, &style_hide, LV_PART_CURSOR);
                lv_obj_add_style(sib, &style_border_def, LV_STATE_DEFAULT);
            }
            else
            {
                spinboxes.id_index = i;
            }
        }
    }

    // // Switch handler for root/sidebar toggle (kept for reference)
    // static void switch_handler(lv_event_t* e)
    // {
    //     if (lv_event_get_code(e) != LV_EVENT_VALUE_CHANGED) return;
    //     auto* m   = static_cast<lv_obj_t*>(lv_event_get_user_data(e));
    //     auto* obj = lv_event_get_target(e);

    //     if (lv_obj_has_state(obj, LV_STATE_CHECKED))
    //     {
    //         lv_menu_set_page(m, nullptr);
    //         lv_menu_set_sidebar_page(m, root_page);
    //         lv_event_send(lv_obj_get_child(lv_obj_get_child(lv_menu_get_cur_sidebar_page(m), 0), 0),
    //                       LV_EVENT_CLICKED, nullptr);
    //     }
    //     else
    //     {
    //         lv_menu_set_sidebar_page(m, nullptr);
    //         lv_menu_clear_history(m);
    //         lv_menu_set_page(m, root_page);
    //     }
    // }

    // ------ UI builders ------
    static lv_obj_t *create_text(lv_obj_t *parent, const char *icon, const char *txt,
                                 uint8_t builder_variant, lv_obj_t *obj)
    {
        (void)icon;
        obj = lv_menu_cont_create(parent);

        lv_obj_t *label = nullptr;
        if (txt)
        {
            label = lv_label_create(obj);
            lv_label_set_text(label, txt);
            lv_obj_set_flex_grow(label, 0);
            lv_obj_set_height(label, 11);
        }

        // builder_variant 1 or 2 (see LVGL menu docs). Using raw ints avoids macro deps.
        (void)builder_variant;
        return obj;
    }

    static lv_obj_t *create_switch_(lv_obj_t *parent, const char * /*icon*/, const char *txt, bool chk,
                                    lv_event_cb_t event_cb, lv_event_code_t filter, void *user_data)
    {
        lv_obj_t *obj = lv_menu_cont_create(parent);
        lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(obj, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

        auto *label = lv_label_create(obj);
        lv_label_set_text(label, txt);
        lv_label_set_long_mode(label, LV_LABEL_LONG_SCROLL_CIRCULAR);
        lv_obj_set_flex_grow(label, 2);

        auto *sw = lv_switch_create(obj);
        if (chk)
            lv_obj_add_state(sw, LV_STATE_CHECKED);
        lv_obj_add_flag(sw, LV_OBJ_FLAG_FLEX_IN_NEW_TRACK);

        lv_obj_add_event_cb(sw, PRESSED_event_cb, LV_EVENT_PRESSED, nullptr);
        lv_obj_add_event_cb(sw, RELEASED_event_cb, LV_EVENT_RELEASED, nullptr);
        lv_obj_add_event_cb(sw, event_cb, filter, user_data);
        return obj;
    }

    static void slider_backlight_event_cb(lv_event_t *e)
    {
        auto *slider = lv_event_get_target(e);
        int32_t v = lv_slider_get_value(slider);
        char buf[8];
        lv_snprintf(buf, sizeof(buf), "%d", v);
        lv_label_set_text(lv_obj_get_child(lv_obj_get_parent(slider), 1), buf);
        // ledcWrite(lcdBacklightChannel, v); // if you wire it up
    }

    static void slider_adcRate_event_cb(lv_event_t *e)
    {
        auto *slider = lv_event_get_target(e);
        uint8_t v = lv_slider_get_value(slider);
        static std::map<int, const char *> ADC_SPS{
            {0, "  20"}, {1, "  90"}, {2, " 330"}, {3, "1000"}};
        lv_label_set_text(lv_obj_get_child(lv_obj_get_parent(slider), 1), ADC_SPS.at(v));
        PowerSupply.adc.ads1219->setDataRate(atoi(ADC_SPS.at(v)));
        PowerSupply.settingParameters.adcRate = v;
        PowerSupply.ResetStats();
    }

    static void slider_adcAVG_event_cb(lv_event_t *e)
    {
        auto *slider = lv_event_get_target(e);
        uint16_t v = lv_slider_get_value(slider);
        char buf[6];
        lv_snprintf(buf, sizeof(buf), "%-d", (uint16_t)std::pow(2, v));
        lv_label_set_text(lv_obj_get_child(lv_obj_get_parent(slider), 1), buf);

        PowerSupply.Voltage.measured.SetWindowSize(std::pow(2, v));
        PowerSupply.Current.measured.SetWindowSize(std::pow(2, v));
        PowerSupply.Power.measured.SetWindowSize(std::pow(2, v));
        PowerSupply.settingParameters.adcNumberOfAvgs = v;
    }

    static void slider_decimalPoints_event_cb(lv_event_t *e)
    {
        auto *slider = lv_event_get_target(e);
        uint16_t v = lv_slider_get_value(slider);
        char buf[4];
        lv_snprintf(buf, sizeof(buf), "%i", v);
        lv_label_set_text(lv_obj_get_child(lv_obj_get_parent(slider), 1), buf);

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
        if (lv_event_get_code(e) != LV_EVENT_SHORT_CLICKED)
            return;
        auto *sw = lv_event_get_target(e);
        if (lv_obj_get_state(sw) & LV_STATE_CHECKED)
        {
            buzzerSound = true;
            myTone(NOTE_A5, 10, true);
        }
        else
        {
            buzzerSound = false;
            myTone(NOTE_A3, 10);
        }
    }

    static void save_cb(lv_event_t *)
    {
        PowerSupply.SaveCalibrationData();
        PRESSED_event_cb(nullptr);
    }

    static void load_cb(lv_event_t *)
    {
        PowerSupply.LoadCalibrationData();
        PRESSED_event_cb(nullptr);

        if (PowerSupply.gui.win_ADC_voltage_calibration && !lv_obj_has_flag(PowerSupply.gui.win_ADC_voltage_calibration, LV_OBJ_FLAG_HIDDEN))
        {
            auto &v = PowerSupply.CalBank[PowerSupply.bankCalibId].vCal;
            lv_spinbox_set_value(Calib_GUI_voltage.code_1, v.code_1);
            lv_spinbox_set_value(Calib_GUI_voltage.code_2, v.code_2);
            lv_spinbox_set_value(Calib_GUI_voltage.vin_1, (int32_t)llround(10000.0 * v.value_1));
            lv_spinbox_set_value(Calib_GUI_voltage.vin_2, (int32_t)llround(10000.0 * v.value_2));
        }
        if (PowerSupply.gui.win_ADC_current_calibration && !lv_obj_has_flag(PowerSupply.gui.win_ADC_current_calibration, LV_OBJ_FLAG_HIDDEN))
        {
            auto &i = PowerSupply.CalBank[PowerSupply.bankCalibId].iCal;
            lv_spinbox_set_value(Calib_GUI_current.code_1, i.code_1);
            lv_spinbox_set_value(Calib_GUI_current.code_2, i.code_2);
            lv_spinbox_set_value(Calib_GUI_current.vin_1, (int32_t)llround(10000.0 * i.value_1));
            lv_spinbox_set_value(Calib_GUI_current.vin_2, (int32_t)llround(10000.0 * i.value_2));
        }
    }

    // slider factory
    static lv_obj_t *create_slider(lv_obj_t *parent, const char * /*icon*/, const char *txt,
                                   int32_t min, int32_t max, int32_t val,
                                   lv_event_cb_t event_cb, lv_event_code_t filter)
    {
        lv_obj_t *obj = lv_menu_cont_create(parent);
        lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(obj, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_END);

        auto *label = lv_label_create(obj);
        lv_label_set_text(label, txt);
        lv_label_set_long_mode(label, LV_LABEL_LONG_SCROLL_CIRCULAR);
        lv_obj_set_flex_grow(label, 2);

        auto *val_lbl = lv_label_create(obj);
        lv_label_set_text(val_lbl, "0");

        auto *slider = lv_slider_create(obj);
        lv_obj_set_flex_grow(slider, 15);
        lv_obj_add_flag(slider, LV_OBJ_FLAG_FLEX_IN_NEW_TRACK);
        lv_obj_add_event_cb(slider, event_cb, filter, nullptr);
        lv_obj_add_event_cb(slider, PRESSED_event_cb, LV_EVENT_PRESSED, nullptr);
        lv_obj_add_event_cb(slider, RELEASED_event_cb, LV_EVENT_RELEASED, nullptr);
        lv_slider_set_range(slider, min, max);
        lv_slider_set_value(slider, val, LV_ANIM_OFF);
        lv_event_send(slider, LV_EVENT_VALUE_CHANGED, nullptr);
        return obj;
    }

    // shared ADC window builder
    struct CalPrefill
    {
        int32_t code1, code2;
        double val1, val2;
        const char *unit;
    };

    static void build_adc_calibration_window(lv_obj_t **win_holder,
                                             const char *title,
                                             setting_GUI &gui,
                                             const CalPrefill &pf)
    {
        if (*win_holder)
        {
            lv_obj_clear_flag(*win_holder, LV_OBJ_FLAG_HIDDEN);
            return;
        }

        lv_obj_t *win = lv_win_create(lv_scr_act(), 32);
        *win_holder = win;
        lv_obj_set_size(win, 320, 226);
        lv_win_add_title(win, title); // LVGL 8 win API.

        lv_obj_t *btn = lv_win_add_btn(win, LV_SYMBOL_CLOSE, 60);
        lv_obj_add_event_cb(btn, btn_close_hide_obj_cb, LV_EVENT_CLICKED, nullptr);

        lv_obj_t *cont = lv_win_get_content(win);
        lv_obj_set_style_pad_all(cont, 0, LV_PART_ITEMS);
        lv_obj_set_style_pad_all(cont, 0, LV_PART_MAIN);

        static lv_style_t style_lbl, style_val;
        static bool styles_inited{};
        if (!styles_inited)
        {
            lv_style_init(&style_lbl);
            lv_style_set_text_font(&style_lbl, &lv_font_montserrat_10);
            lv_style_init(&style_val);
            lv_style_set_text_font(&style_val, &graph_R_16);
            lv_style_set_text_color(&style_val, lv_color_hex(0xFFFFFF));
            styles_inited = true;
        }

        lv_point_t offset{10, 20};
        lv_coord_t w = 110, col = w + 30, row = 38;
        lv_point_t pad{5, 5};

        gui.vin_1 = spinbox_pro(cont, "#FFFFF7 Vin1:#", -10000, 400000, 6, 2, LV_ALIGN_TOP_LEFT, offset.x, offset.y, w, 10);
        gui.code_1 = spinbox_pro(cont, "#FFFFF7 Code1:#", -10000, 8388608, 7, 0, LV_ALIGN_TOP_LEFT, offset.x, offset.y + row, w, 11);
        gui.vin_2 = spinbox_pro(cont, "#FFFFF7 Vin2:#", -10000, 400000, 6, 2, LV_ALIGN_TOP_LEFT, offset.x + col, offset.y, w, 12);
        gui.code_2 = spinbox_pro(cont, "#FFFFF7 Code2:#", -10000, 8388608, 7, 0, LV_ALIGN_TOP_LEFT, offset.x + col, offset.y + row, w, 13);

        lv_spinbox_set_value(gui.code_1, pf.code1);
        lv_spinbox_set_value(gui.code_2, pf.code2);
        lv_spinbox_set_value(gui.vin_1, (int32_t)llround(10000.0 * pf.val1));
        lv_spinbox_set_value(gui.vin_2, (int32_t)llround(10000.0 * pf.val2));

        auto *lbl_raw = LVLabel::create(cont, "Raw Code:", gui.code_1, 0, pad.y, &style_lbl);
        gui.lbl_rawCode = LVLabel::create(cont, "#FFFFF7 00000000#", lbl_raw, 0, pad.y - 2, &style_val);

        auto *lbl_rawA = LVLabel::create(cont, "Avg Raw:", gui.lbl_rawCode, 0, pad.y, &style_lbl);
        gui.lbl_rawAVG_ = LVLabel::create(cont, "+00000000", lbl_rawA, 0, pad.y - 2, &style_val);

        auto *lbl_calV = LVLabel::create(cont, "Calibrated Value:", gui.code_2, 0, pad.y, &style_lbl);
        gui.lbl_calibratedValue = LVLabel::create(cont, (pf.unit && pf.unit[0] == 'A') ? "0.0000A" : "0.0000V",
                                                  lbl_calV, 0, pad.y - 2, &style_val);

        auto *lbl_cvA = LVLabel::create(cont, "Avg Calibrated Value:", gui.lbl_calibratedValue, 0, pad.y, &style_lbl);
        gui.lbl_calibValueAVG_ = LVLabel::create(cont, "+00.0000", lbl_cvA, 0, pad.y - 2, &style_val);

        auto *lbl_er = LVLabel::create(cont, "Effective Resolution:", gui.lbl_rawAVG_, 0, pad.y, &style_lbl);
        gui.lbl_ER = LVLabel::create(cont, "00.00", lbl_er, 0, pad.y - 2, &style_val);

        lv_point_t btn_pos{140 + offset.x, 160};
        LVButton btnSave(cont, "Save", btn_pos.x + 62, btn_pos.y, 54, 26, nullptr, save_cb);
        LVButton btnLoad(cont, "Load", btn_pos.x, btn_pos.y, 54, 26, nullptr, load_cb);

        // m, b, and Vin_cal notes (simple labels)
        auto *label_m = lv_label_create(cont);
        auto *label_m_num = lv_label_create(cont);
        auto *label_m_den = lv_label_create(cont);
        auto *label_b = lv_label_create(cont);
        auto *label_vin = lv_label_create(cont);

        lv_label_set_text(label_m_num, "Code2 - Code1");
        lv_label_set_text(label_m, "m = --------------------- =");
        lv_label_set_text(label_m_den, "Vin2 - Vin1");

        int verPad = 160 + 50;
        lv_obj_align(label_m_num, LV_ALIGN_TOP_LEFT, 35, verPad + 10);
        lv_obj_align(label_m, LV_ALIGN_TOP_LEFT, 0, verPad + 20);
        lv_obj_align(label_m_den, LV_ALIGN_TOP_LEFT, 50, verPad + 30);

        gui.lbl_calib_m = lv_label_create(cont);
        gui.lbl_calib_b = lv_label_create(cont);
        lv_obj_align(gui.lbl_calib_m, LV_ALIGN_TOP_LEFT, 160, verPad + 20);

        lv_label_set_text(label_b, "b = Code1 - m * Vin1 =");
        lv_obj_align(label_b, LV_ALIGN_TOP_LEFT, 0, verPad + 60);
        lv_obj_align(gui.lbl_calib_b, LV_ALIGN_TOP_LEFT, 160, verPad + 60);

        lv_label_set_text(label_vin, "Vin_cal = (Code-b)/m");
        lv_obj_align(label_vin, LV_ALIGN_TOP_LEFT, 0, verPad + 90);
    }

    // DAC handlers
    static void DAC_voltage_calib_change_event_cb(lv_event_t *e)
    {
        if (lv_event_get_code(e) != LV_EVENT_VALUE_CHANGED)
            return;
        double zv = lv_spinbox_get_value(s_zv);
        double mv = lv_spinbox_get_value(s_mv);
        lv_label_set_text_fmt(s_label_maxV, "%+6.4fV", (mv - zv) / 2000.0);

        PowerSupply.dac_data.zero_voltage = lv_spinbox_get_value(s_zv);
        PowerSupply.dac_data.max_voltage = lv_spinbox_get_value(s_mv);
        PowerSupply.dac_data.zero_current = lv_spinbox_get_value(s_zc);
        PowerSupply.dac_data.max_current = lv_spinbox_get_value(s_mc);

        PowerSupply.Voltage.adjOffset = PowerSupply.dac_data.zero_voltage;
        PowerSupply.Voltage.minValue = (-PowerSupply.dac_data.zero_voltage);
        PowerSupply.Voltage.maxValue = (mv - zv);
        PowerSupply.SaveDACdata("dac_data_", PowerSupply.dac_data);
    }

    static void DAC_current_calib_change_event_cb(lv_event_t *e)
    {
        if (lv_event_get_code(e) != LV_EVENT_VALUE_CHANGED)
            return;
        double zc = lv_spinbox_get_value(s_zc);
        double mc = lv_spinbox_get_value(s_mc);
        lv_label_set_text_fmt(s_label_maxC, "%+6.4fA", (mc - zc) / 10000.0);

        PowerSupply.dac_data.zero_voltage = lv_spinbox_get_value(s_zv);
        PowerSupply.dac_data.max_voltage = lv_spinbox_get_value(s_mv);
        PowerSupply.dac_data.zero_current = lv_spinbox_get_value(s_zc);
        PowerSupply.dac_data.max_current = lv_spinbox_get_value(s_mc);

        PowerSupply.Current.adjOffset = PowerSupply.dac_data.zero_current;
        PowerSupply.Current.minValue = (-PowerSupply.dac_data.zero_current) / 10000.0;
        PowerSupply.Current.maxValue = (mc - zc) / 10000.0;
        PowerSupply.SaveDACdata("dac_data_", PowerSupply.dac_data);
    }

    // LCD touch calibration kept private
    static void LCD_Calibration_cb(lv_event_t *)
    {
        uint16_t calData[5];
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
        tft.setTouch(calData);
        lv_obj_invalidate(Tabs::tabview);
    }

    static lv_obj_t *create_button_item(lv_obj_t *parent, lv_event_cb_t cb, const char *buttonTxt)
    {
        lv_obj_t *obj = lv_menu_cont_create(parent);
        lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_style_pad_all(obj, 3, LV_PART_ITEMS);
        lv_obj_set_style_pad_all(obj, 3, LV_PART_MAIN);

        auto *btn = lv_btn_create(obj);
        lv_obj_set_size(btn, 120, 30);
        auto *label = lv_label_create(btn);
        lv_label_set_text(label, buttonTxt);
        lv_obj_center(label);

        lv_obj_add_event_cb(btn, PRESSED_event_cb, LV_EVENT_PRESSED, nullptr);
        lv_obj_add_event_cb(btn, RELEASED_event_cb, LV_EVENT_RELEASED, nullptr);
        if (cb)
            lv_obj_add_event_cb(btn, cb, LV_EVENT_RELEASED, nullptr);
        return obj;
    }

} // namespace

// ---------- small helpers exported ----------
double get_m(double code1, double code2, double vin1, double vin2) { return (code2 - code1) / (vin2 - vin1); }
double get_b(double code, double m, double vin) { return code - m * vin; }

// ---------- public: build menus ----------
void SettingMenu(lv_obj_t *parent)
{
    PowerSupply.gui.setting_menu = lv_menu_create(parent); // lv_menu API.

    lv_color_t bg = lv_obj_get_style_bg_color(PowerSupply.gui.setting_menu, 0);
    lv_obj_set_style_bg_color(PowerSupply.gui.setting_menu, lv_color_darken(bg, lv_color_brightness(bg) > 127 ? 10 : 50), 0);
    lv_menu_set_mode_root_back_btn(PowerSupply.gui.setting_menu, LV_MENU_ROOT_BACK_BTN_DISABLED);
    lv_obj_set_size(PowerSupply.gui.setting_menu, 294, 200);
    lv_obj_center(PowerSupply.gui.setting_menu);

    // Display page
    lv_obj_t *sub_display = lv_menu_page_create(PowerSupply.gui.setting_menu, nullptr);
    lv_obj_set_style_pad_hor(sub_display, lv_obj_get_style_pad_left(lv_menu_get_main_header(PowerSupply.gui.setting_menu), 0), 0);
    auto *section = lv_menu_section_create(sub_display);
    create_slider(section, nullptr, "Backlight", 0, 255, 255, slider_backlight_event_cb, LV_EVENT_VALUE_CHANGED);

    // Sound
    lv_obj_t *sub_sound = lv_menu_page_create(PowerSupply.gui.setting_menu, nullptr);
    section = lv_menu_section_create(sub_sound);
    create_switch_(section, nullptr, "Buzzer", buzzerSound, switch_buzzer_event_cb, LV_EVENT_SHORT_CLICKED, nullptr);

    // Measure
    lv_obj_t *sub_measure = lv_menu_page_create(PowerSupply.gui.setting_menu, nullptr);
    section = lv_menu_section_create(sub_measure);
    create_slider(section, nullptr, "ADC SPS", 0, 3, PowerSupply.settingParameters.adcRate, slider_adcRate_event_cb, LV_EVENT_VALUE_CHANGED);
    PowerSupply.gui.slider_Avgs = create_slider(section, nullptr, "ADC # of Avgs", 0, (int)std::log2(MAX_NO_OF_AVG), PowerSupply.settingParameters.adcNumberOfAvgs, slider_adcAVG_event_cb, LV_EVENT_VALUE_CHANGED);
    create_slider(section, nullptr, "Number of Digits", 1, 4, PowerSupply.settingParameters.adcNumberOfDigits, slider_decimalPoints_event_cb, LV_EVENT_VALUE_CHANGED);
    create_switch_(section, nullptr, "Auto Bar-Graph", false, switch_buzzer_event_cb, LV_EVENT_VALUE_CHANGED, PowerSupply.gui.setting_menu);

    // Calibration
    lv_obj_t *sub_cal = lv_menu_page_create(PowerSupply.gui.setting_menu, nullptr);
    section = lv_menu_section_create(sub_cal);
    create_button_item(section, btn_calibration_ADC_voltage_event_cb, "ADC Voltage");
    create_button_item(section, btn_calibration_ADC_current_event_cb, "ADC Current");
    create_button_item(section, open_dac_calibration_cb, "V/I DAC");
    create_button_item(section, nullptr /* Stats reset wiring */, "Reset Stats");
    create_button_item(section, LCD_Calibration_cb, "LCD Touch");

    // Save/Load
    lv_obj_t *sub_save_load = lv_menu_page_create(PowerSupply.gui.setting_menu, nullptr);
    section = lv_menu_section_create(sub_save_load);
    create_button_item(section, load_cb, "Load");
    create_button_item(section, save_cb, "Save");

    // Version/About/Sidebar
    lv_obj_t *sub_software_info_page = lv_menu_page_create(PowerSupply.gui.setting_menu, nullptr);
    section = lv_menu_section_create(sub_software_info_page);

    char version_text[48];
    snprintf(version_text, sizeof(version_text), "Version %s", SOFTWARE_VERSION);
    create_text(section, nullptr, version_text, 1, nullptr);

    lv_obj_t *sub_about_page = lv_menu_page_create(PowerSupply.gui.setting_menu, nullptr);
    section = lv_menu_section_create(sub_about_page);
    auto *itm = create_text(section, nullptr, "Software information", 1, nullptr);
    lv_menu_set_load_page_event(PowerSupply.gui.setting_menu, itm, sub_software_info_page);

    // Root page + items
    lv_obj_t *root_page = lv_menu_page_create(PowerSupply.gui.setting_menu, nullptr);
    lv_obj_set_style_pad_hor(root_page, lv_obj_get_style_pad_left(lv_menu_get_main_header(PowerSupply.gui.setting_menu), 0), 0);
    section = lv_menu_section_create(root_page);

    itm = create_text(section, nullptr, "Display", 1, nullptr);
    lv_menu_set_load_page_event(PowerSupply.gui.setting_menu, itm, sub_display);
    lv_obj_add_event_cb(itm, trackPress, LV_EVENT_RELEASED, nullptr);

    itm = create_text(section, nullptr, "Sound", 1, nullptr);
    lv_menu_set_load_page_event(PowerSupply.gui.setting_menu, itm, sub_sound);
    lv_obj_add_event_cb(itm, trackPress, LV_EVENT_RELEASED, nullptr);

    itm = create_text(section, nullptr, "Measure", 1, nullptr);
    lv_menu_set_load_page_event(PowerSupply.gui.setting_menu, itm, sub_measure);
    lv_obj_add_event_cb(itm, trackPress, LV_EVENT_RELEASED, nullptr);

    itm = create_text(section, nullptr, "Calibration", 1, nullptr);
    lv_menu_set_load_page_event(PowerSupply.gui.setting_menu, itm, sub_cal);
    lv_obj_add_event_cb(itm, trackPress, LV_EVENT_RELEASED, nullptr);

    itm = create_text(section, nullptr, "Save/Load", 1, nullptr);
    lv_menu_set_load_page_event(PowerSupply.gui.setting_menu, itm, sub_save_load);
    lv_obj_add_event_cb(itm, trackPress, LV_EVENT_RELEASED, nullptr);

    itm = create_text(section, nullptr, "About", 1, nullptr);
    lv_menu_set_load_page_event(PowerSupply.gui.setting_menu, itm, sub_about_page);
    lv_obj_add_event_cb(itm, trackPress, LV_EVENT_RELEASED, nullptr);

    lv_menu_set_sidebar_page(PowerSupply.gui.setting_menu, root_page);
    auto *m = (lv_menu_t *)PowerSupply.gui.setting_menu;
    lv_obj_set_size(m->sidebar, LV_PCT(38), LV_PCT(100));
    // Simulate click on first item
    lv_event_send(lv_obj_get_child(lv_obj_get_child(lv_menu_get_cur_sidebar_page(PowerSupply.gui.setting_menu), 0), 0),
                  LV_EVENT_CLICKED, nullptr);
}

// ---------- exported callbacks used by other files ----------
void btn_calibration_ADC_voltage_event_cb(lv_event_t *)
{
    auto &cal = PowerSupply.CalBank[PowerSupply.bankCalibId].vCal;
    CalPrefill pf{cal.code_1, cal.code_2, cal.value_1, cal.value_2, "V"};
    build_adc_calibration_window(&PowerSupply.gui.win_ADC_voltage_calibration, "ADC Voltage Calibration", Calib_GUI_voltage, pf);
}

void btn_calibration_ADC_current_event_cb(lv_event_t *)
{
    auto &cal = PowerSupply.CalBank[PowerSupply.bankCalibId].iCal;
    CalPrefill pf{cal.code_1, cal.code_2, cal.value_1, cal.value_2, "A"};
    build_adc_calibration_window(&PowerSupply.gui.win_ADC_current_calibration, "ADC Current Calibration", Calib_GUI_current, pf);
}

// Open/create the DAC calibration window
void open_dac_calibration_cb(lv_event_t *)
{
    if (PowerSupply.gui.win_DAC_calibration)
    {
        lv_obj_clear_flag(PowerSupply.gui.win_DAC_calibration, LV_OBJ_FLAG_HIDDEN);
        return;
    }

    PowerSupply.gui.win_DAC_calibration = lv_win_create(lv_scr_act(), 36);
    lv_obj_set_size(PowerSupply.gui.win_DAC_calibration, 320, 226);
    lv_win_add_title(PowerSupply.gui.win_DAC_calibration, "DAC Calibration");
    auto *close = lv_win_add_btn(PowerSupply.gui.win_DAC_calibration, LV_SYMBOL_CLOSE, 60);
    lv_obj_add_event_cb(close, btn_close_hide_obj_cb, LV_EVENT_CLICKED, nullptr);
    auto *cont = lv_win_get_content(PowerSupply.gui.win_DAC_calibration);
    lv_obj_set_style_pad_all(cont, 0, LV_PART_ITEMS);
    lv_obj_set_style_pad_all(cont, 0, LV_PART_MAIN);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);

    auto *title = lv_label_create(cont);
    lv_label_set_text(title, "DAC Code      Output V/C");
    lv_obj_align(title, LV_ALIGN_OUT_LEFT_TOP, 95, 10);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(title, lv_color_hex(0x00FFFF), LV_PART_MAIN | LV_STATE_DEFAULT);

    int xPos = 94, yPos = 40, yOffset = 25, xOffset = 10;

    s_zv = spinbox_pro(cont, "#FFFFF7 Zero Voltage:#", 0, 65535, 5, 0, LV_ALIGN_DEFAULT, xPos, yPos + yOffset * 0, 86, 15);
    s_mv = spinbox_pro(cont, "#FFFFF7 Max  Voltage:#", 0, 65535, 5, 0, LV_ALIGN_DEFAULT, xPos, yPos + yOffset * 1, 86, 16);
    s_zc = spinbox_pro(cont, "#FFFFF7 Zero Current:#", 0, 65535, 5, 0, LV_ALIGN_DEFAULT, xPos, yPos + yOffset * 2.3, 86, 17);
    s_mc = spinbox_pro(cont, "#FFFFF7 Max  Current:#", 0, 65535, 5, 0, LV_ALIGN_DEFAULT, xPos, yPos + yOffset * 3.3, 86, 18);

    auto addLabel = [&](lv_obj_t *parent, const char *fmt, double value) -> lv_obj_t *
    {
        lv_obj_t *parent_child = lv_obj_get_child(parent, 1);
        lv_obj_align(parent_child, LV_ALIGN_OUT_RIGHT_MID, -90, 0);
        auto *lbl = lv_label_create(parent);
        lv_obj_add_flag(parent, LV_OBJ_FLAG_OVERFLOW_VISIBLE);
        lv_obj_align_to(lbl, parent, LV_ALIGN_OUT_RIGHT_MID, xOffset, 0);
        lv_label_set_text_fmt(lbl, fmt, value);
        lv_obj_set_style_text_font(lbl, &graph_R_16, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(lbl, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
        return lbl;
    };

    addLabel(s_zv, "+%07.4fV", 0.0);
    s_label_maxV = addLabel(s_mv, "+%07.4fV", 32.750);

    addLabel(s_zc, "%+06.4fA", 0.0);
    s_label_maxC = addLabel(s_mc, "%+06.4fA", 6.5535);

    // Load persisted DAC data and update limits
    PowerSupply.dac_data = PowerSupply.LoadDACdata("dac_data_");
    lv_spinbox_set_value(s_zv, PowerSupply.dac_data.zero_voltage);
    lv_spinbox_set_value(s_mv, PowerSupply.dac_data.max_voltage);
    lv_spinbox_set_value(s_zc, PowerSupply.dac_data.zero_current);
    lv_spinbox_set_value(s_mc, PowerSupply.dac_data.max_current);

    PowerSupply.Voltage.adjOffset = PowerSupply.dac_data.zero_voltage;
    PowerSupply.Voltage.minValue = (-PowerSupply.dac_data.zero_voltage);
    PowerSupply.Voltage.maxValue = (PowerSupply.dac_data.max_voltage - PowerSupply.dac_data.zero_voltage);

    PowerSupply.Current.adjOffset = PowerSupply.dac_data.zero_current;
    PowerSupply.Current.minValue = (-PowerSupply.dac_data.zero_current);
    PowerSupply.Current.maxValue = (PowerSupply.dac_data.max_current - PowerSupply.dac_data.zero_current);

    lv_obj_add_event_cb(s_zv, DAC_voltage_calib_change_event_cb, LV_EVENT_VALUE_CHANGED, s_label_maxV);
    lv_obj_add_event_cb(s_mv, DAC_voltage_calib_change_event_cb, LV_EVENT_VALUE_CHANGED, s_label_maxV);
    lv_obj_add_event_cb(s_zc, DAC_current_calib_change_event_cb, LV_EVENT_VALUE_CHANGED, nullptr);
    lv_obj_add_event_cb(s_mc, DAC_current_calib_change_event_cb, LV_EVENT_VALUE_CHANGED, nullptr);
}
