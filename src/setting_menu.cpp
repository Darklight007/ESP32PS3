#include "setting_menu.h"

// heavy/private includes live ONLY in the .cpp
#include "spinbox_pro.h"
#include "version.h"
#include "buzzer.h"   // provides buzzerSound and myTone()
#include "TFT_eSPI.h" // tft instance
#include "tabs.h"
#include "table_pro.h"
#include "device.hpp"      // provides PowerSupply and dac_data_g
#include "lv_gui_helper.h" // LVLabel, LVButton helpers
#include <float.h>

#include <cmath>
#include <map>
#include <cstdio>
#include <cstring> // for memcmp, memcpy
#include <cstdint>
#include <cstddef>
#include <functional>
#include <cstdarg>
#include <vector>

#include "MonotoneCubicCalibrator.h"

// --- Debug helpers ---
#if defined(ARDUINO) && defined(ESP32)
#include <Arduino.h>
#define DBG_FREE_HEAP() ESP.getFreeHeap()
#else
#define DBG_FREE_HEAP() 0u
#endif

static void dbg_printf(const char *fmt, ...)
{
    char buf[192];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
#if defined(ARDUINO)
    Serial.println(buf);
#endif
    // Optional LVGL log
    extern void log_step(const char *, ...);
    log_step("%s", buf); // remove if you do not have log_step
}

///*****************************************************************************
// ****************** Simple sequencer for auto-measure *************************
// *****************************************************************************

static char s_logbuf[2048];
static size_t s_len = 0;
static bool s_pending = false; // true after _begin(), cleared by _done()
// ---------- Reuse your existing logger ----------
void log_step_begin(const char *fmt, ...);
void log_step_done();
void log_step(const char *fmt, ...);
namespace
{
    static void auto_zero_event_cb(lv_event_t *e);

    static void autoZeroCurrent_cb(lv_event_t *);
}
// ---------- public objects (one definition) ----------

// extern bool lvglChartIsBusy;
extern bool lvglIsBusy;
// Global (or static) residual spline

setting_GUI Calib_GUI{};

static lv_obj_t *log_win;
static lv_obj_t *log_label;

static void close_log_cb(lv_timer_t *t)
{
    if (log_win)
    {
        lv_obj_del(log_win);
        log_win = NULL;
        log_label = NULL;
    }
    lv_timer_del(t);
}

// Replace your log_step() with this pair

static inline void log_update_label()
{
    s_logbuf[s_len] = '\0';
    lv_label_set_text(log_label, s_logbuf);
    lv_obj_scroll_to_view(log_label, LV_ANIM_OFF);
    myTone(NOTE_A3, 50, false);
}

// 1) Print: "1.  Setting volt to 0v ...\n"
void log_step_begin(const char *fmt, ...)
{
    // auto-close any previous pending step
    if (s_pending)
    {
        // replace the previous " ...\n" with " done!\n"
        // (safe even if buffer tight; see _done code)
        // You can call log_step_done(); but inline is faster.
        const char tail[] = " ...\n";
        const size_t tail_len = sizeof(tail) - 1;
        if (s_len >= tail_len && std::memcmp(s_logbuf + s_len - tail_len, tail, tail_len) == 0)
        {
            s_len -= tail_len;
            const char done[] = " done!\n";
            size_t add = strlen(done);
            if (s_len + add >= sizeof(s_logbuf))
                add = sizeof(s_logbuf) - s_len - 1;
            std::memcpy(s_logbuf + s_len, done, add);
            s_len += add;
        }
        s_pending = false;
    }

    // format the new line head: "1.  Setting volt to 0v"
    char line[256];
    va_list args;
    va_start(args, fmt);
    const int n = vsnprintf(line, sizeof(line), fmt, args);
    va_end(args);

    // append to the big buffer + " ...\n"
    const char ell[] = " ...\n";
    const size_t need = (size_t)((n > 0 ? n : 0)) + sizeof(ell) - 1;

    // truncate if needed
    size_t avail = sizeof(s_logbuf) - s_len - 1;
    size_t to_copy = avail > need ? (size_t)(n > 0 ? n : 0) : (avail > (sizeof(ell) - 1) ? avail - (sizeof(ell) - 1) : 0);

    if (to_copy > 0)
    {
        std::memcpy(s_logbuf + s_len, line, to_copy);
        s_len += to_copy;
    }
    // add " ...\n" if there is room
    size_t add = sizeof(ell) - 1;
    if (s_len + add >= sizeof(s_logbuf))
        add = sizeof(s_logbuf) - s_len - 1;
    std::memcpy(s_logbuf + s_len, ell, add);
    s_len += add;

    s_pending = true;
    log_update_label();
    myTone(NOTE_A5, 100, false);
}

// 2) Replace trailing " ...\n" with " done!\n" on the same line
void log_step_done()
{
    if (!s_pending)
        return;

    const char tail[] = " ...\n";
    const size_t tail_len = sizeof(tail) - 1;

    if (s_len >= tail_len && std::memcmp(s_logbuf + s_len - tail_len, tail, tail_len) == 0)
    {
        // remove " ...\n"
        s_len -= tail_len;

        // append " done!\n"
        const char done[] = " done!\n";
        size_t add = sizeof(done) - 1;
        if (s_len + add >= sizeof(s_logbuf))
            add = sizeof(s_logbuf) - s_len - 1;
        std::memcpy(s_logbuf + s_len, done, add);
        s_len += add;

        log_update_label();
    }
    s_pending = false;
    myTone(NOTE_A3, 50, false);
}

void log_step(const char *fmt, ...)
{
    if (!lvglIsBusy)
    {
        char buf[256];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);

        const char *old = lv_label_get_text(log_label);
        static char new_txt[1024];
        snprintf(new_txt, sizeof(new_txt), "%s%s\n", old, buf);

        lv_label_set_text(log_label, new_txt);
        lv_obj_scroll_to_view(log_label, LV_ANIM_OFF);
    }
}

void log_clear()
{
    if (log_label)
    {
        lv_label_set_text(log_label, "");              // wipe text
        lv_obj_scroll_to_y(log_label, 0, LV_ANIM_OFF); // scroll back to top
    }
}

static void create_log_window()
{
    log_win = lv_win_create(lv_scr_act(), 40);
    lv_win_add_title(log_win, "Measuring total internal resistor");

    lv_obj_t *cont = lv_win_get_content(log_win);
    log_label = lv_label_create(cont);
    lv_label_set_text(log_label, "");
    lv_label_set_long_mode(log_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(log_label, lv_pct(100));
}

// setting_GUI Calib_GUI.Current{};

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

    lv_obj_t *int_total_res = nullptr; // max  current spin

    // pressed/released beeps
    static void PRESSED_event_cb(lv_event_t *) { myTone(NOTE_A4, 5); }
    static void RELEASED_event_cb(lv_event_t *) { myTone(NOTE_A4, 5); }

    static void btn_close_hide_obj_cb(lv_event_t *e)
    {
        auto *btn = lv_event_get_target(e);
        lv_obj_add_flag(lv_obj_get_parent(lv_obj_get_parent(btn)), LV_OBJ_FLAG_HIDDEN);
    }

    // static void trackPress(lv_event_t *e)
    // {
    //     auto *obj = lv_event_get_target(e);
    //     lastButton = lv_obj_get_index(obj);
    // }

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

        if (PowerSupply.gui.calibration.win_ADC_voltage_calibration && !lv_obj_has_flag(PowerSupply.gui.calibration.win_ADC_voltage_calibration, LV_OBJ_FLAG_HIDDEN))
        {
            auto &v = PowerSupply.CalBank[PowerSupply.bankCalibId].vCal;
            lv_spinbox_set_value(Calib_GUI.Voltage.code_1, v.code_1);
            lv_spinbox_set_value(Calib_GUI.Voltage.code_2, v.code_2);
            lv_spinbox_set_value(Calib_GUI.Voltage.vin_1, (int32_t)llround(10000.0 * v.value_1));
            lv_spinbox_set_value(Calib_GUI.Voltage.vin_2, (int32_t)llround(10000.0 * v.value_2));
        }
        if (PowerSupply.gui.calibration.win_ADC_current_calibration && !lv_obj_has_flag(PowerSupply.gui.calibration.win_ADC_current_calibration, LV_OBJ_FLAG_HIDDEN))
        {
            auto &i = PowerSupply.CalBank[PowerSupply.bankCalibId].iCal;
            lv_spinbox_set_value(Calib_GUI.Current.code_1, i[PowerSupply.mA_Active].code_1);
            lv_spinbox_set_value(Calib_GUI.Current.code_2, i[PowerSupply.mA_Active].code_2);
            lv_spinbox_set_value(Calib_GUI.Current.vin_1, (int32_t)llround(10000.0 * i[PowerSupply.mA_Active].value_1));
            lv_spinbox_set_value(Calib_GUI.Current.vin_2, (int32_t)llround(10000.0 * i[PowerSupply.mA_Active].value_2));
        }
        if (PowerSupply.gui.calibration.win_int_current_calibration && !lv_obj_has_flag(PowerSupply.gui.calibration.win_int_current_calibration, LV_OBJ_FLAG_HIDDEN))
        {
            // Update internal resistor value in the spinbox
            lv_spinbox_set_value(Calib_GUI.internalLeakage, PowerSupply.CalBank[PowerSupply.bankCalibId].internalLeakage * 1000.0); // Convert to mOhms
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

    // // shared ADC window builder
    // struct CalPrefill
    // {
    //     int32_t code1, code2;
    //     double val1, val2;
    //     const char *unit;
    // };
    static void start_current_zeros(lv_event_t *);

    void build_adc_calibration_window(lv_obj_t **win_holder,
                                      const char *title,
                                      setting_ &gui,
                                      const CalPrefill &pf)
    {
        if (*win_holder)
        {
            lv_spinbox_set_value(gui.code_1, pf.code1);
            lv_spinbox_set_value(gui.code_2, pf.code2);
            lv_spinbox_set_value(gui.vin_1, (int32_t)llround(10000.0 * pf.val1));
            lv_spinbox_set_value(gui.vin_2, (int32_t)llround(10000.0 * pf.val2));

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

        gui.vin_1 = spinbox_pro(cont, "#FFFFF7 Vin1:#", -10000, 400000, 6, 2, LV_ALIGN_TOP_LEFT, offset.x, offset.y, w, 10, &graph_R_16);
        gui.code_1 = spinbox_pro(cont, "#FFFFF7 Code1:#", -10000, 8388608, 7, 0, LV_ALIGN_TOP_LEFT, offset.x, offset.y + row, w, 11, &graph_R_16);
        gui.vin_2 = spinbox_pro(cont, "#FFFFF7 Vin2:#", -10000, 400000, 6, 2, LV_ALIGN_TOP_LEFT, offset.x + col, offset.y, w, 12, &graph_R_16);
        gui.code_2 = spinbox_pro(cont, "#FFFFF7 Code2:#", -10000, 8388608, 7, 0, LV_ALIGN_TOP_LEFT, offset.x + col, offset.y + row, w, 13, &graph_R_16);

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

        LVButton btnAutoZeros(cont, "Auto zeros", btn_pos.x +5, btn_pos.y+31, 2 * 54, 26, nullptr, autoZeroCurrent_cb);

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

        // LVButton btnAutoZeros(cont, "Auto zeros", btn_pos.x + 62 - 54 - 54 / 2, verPad + 125, 2 * 54, 26, nullptr, autoZeroCurrent_cb);
    }

    // DAC handlers
    static void ADC_iinternalRes_calib_change_event_cb(lv_event_t *e)
    {
        if (lv_event_get_code(e) != LV_EVENT_VALUE_CHANGED)
            return;
        double internalLeakage = lv_spinbox_get_value(Calib_GUI.internalLeakage) / 1000.0; // Convert from mOhms to Ohms
        PowerSupply.CalBank[PowerSupply.bankCalibId].internalLeakage = internalLeakage;
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
        PowerSupply.Voltage.SetEncoderUpdate();
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
        PowerSupply.Current.SetEncoderUpdate();
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
    create_button_item(section, internal_current_calibration_cb, "Inter. Current");
    create_button_item(section, ADC_INL_Voltage_calibration_cb, "ADC INL V_CAL");

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
    // lv_obj_add_event_cb(itm, trackPress, LV_EVENT_RELEASED, nullptr);

    itm = create_text(section, nullptr, "Sound", 1, nullptr);
    lv_menu_set_load_page_event(PowerSupply.gui.setting_menu, itm, sub_sound);

    itm = create_text(section, nullptr, "Measure", 1, nullptr);
    lv_menu_set_load_page_event(PowerSupply.gui.setting_menu, itm, sub_measure);

    itm = create_text(section, nullptr, "Calibration", 1, nullptr);
    lv_menu_set_load_page_event(PowerSupply.gui.setting_menu, itm, sub_cal);

    itm = create_text(section, nullptr, "Save/Load", 1, nullptr);
    lv_menu_set_load_page_event(PowerSupply.gui.setting_menu, itm, sub_save_load);

    itm = create_text(section, nullptr, "About", 1, nullptr);
    lv_menu_set_load_page_event(PowerSupply.gui.setting_menu, itm, sub_about_page);

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
    build_adc_calibration_window(&PowerSupply.gui.calibration.win_ADC_voltage_calibration, "ADC Voltage Calibration", Calib_GUI.Voltage, pf);
}

void btn_calibration_ADC_current_event_cb(lv_event_t *)
{
    auto &cal = PowerSupply.CalBank[PowerSupply.bankCalibId].iCal;
    CalPrefill pf{cal[PowerSupply.mA_Active].code_1, cal[PowerSupply.mA_Active].code_2, cal[PowerSupply.mA_Active].value_1, cal[PowerSupply.mA_Active].value_2, "A"};
    build_adc_calibration_window(&PowerSupply.gui.calibration.win_ADC_current_calibration, "ADC Current Calibration [A]", Calib_GUI.Current, pf);
}

static void close_log_cb(lv_timer_t *t);

// ---------- Your measurement context ----------
struct measure_ctx_t
{
    int step = 0; // unused by sequencer, keep if you want
    double v0 = 0.0;
    double v1 = 0.0;
};

// ---------- Sequencer types ----------
struct SeqStep
{
    const char *label;             // e.g. "Setting voltage to 0V"
    uint32_t wait_ms_begin_to_end; // delay before we call end()
    uint32_t wait_ms_after_end;    // cooldown before next step
    std::function<void()> begin;   // optional
    std::function<void()> end;     // optional
};

struct SeqRunner
{
    lv_timer_t *timer = nullptr;
    const SeqStep *steps = nullptr;
    size_t count = 0;
    size_t index = 0;     // current step
    bool in_begin = true; // phase: begin or end
    measure_ctx_t *mctx = nullptr;
};

// Forward
static void seq_cb(lv_timer_t *t);

// Create and start the runner
static SeqRunner *seq_start(lv_timer_t *t, const SeqStep *steps, size_t count, measure_ctx_t *mctx)
{
    auto *r = static_cast<SeqRunner *>(lv_mem_alloc(sizeof(SeqRunner)));
    r->timer = t;
    r->steps = steps;
    r->count = count;
    r->index = 0;
    r->in_begin = true;
    r->mctx = mctx;
    t->user_data = r; // bind runner to timer

    // run first tick "now"
    lv_timer_set_repeat_count(t, -1);
    lv_timer_set_period(t, 1);
    return r;
}

// The LVGL timer callback drives the sequence
static void seq_cb(lv_timer_t *t)
{
    auto *r = static_cast<SeqRunner *>(t->user_data);
    if (!r || r->index >= r->count)
    {
        // done
        lv_timer_del(t);
        if (r)
            lv_mem_free(r);
        return;
    }

    const SeqStep &s = r->steps[r->index];

    if (r->in_begin)
    {
        // BEGIN PHASE
        log_step_begin("%u.  %s", (unsigned)(r->index + 1), s.label);
        if (s.begin)
            s.begin(); // user begin action
        lv_timer_set_period(t, s.wait_ms_begin_to_end);
        r->in_begin = false; // next time: end phase
        return;
    }
    else
    {
        // END PHASE
        log_step_done();
        if (s.end)
            s.end(); // user end action
        lv_timer_set_period(t, s.wait_ms_after_end);
        r->in_begin = true; // next time: begin of next step
        r->index += 1;

        if (r->index >= r->count)
        {
            // finished whole table; stop timer
            lv_timer_del(t);
            lv_mem_free(r);
        }
        return;
    }
}

struct CurrentCalCtx
{
    double v0 = 0, v1 = 0;
};
static void start_current_totalR()
{
    auto *c = (CurrentCalCtx *)lv_mem_alloc(sizeof(CurrentCalCtx));
    *c = CurrentCalCtx{};

    static const SeqStep steps[] = {
        {"Setting voltage to 0V", 1500, 500,
         []()
         { PowerSupply.Voltage.SetUpdate(0.0 * PowerSupply.Voltage.adjFactor + PowerSupply.Voltage.adjOffset); }, nullptr},
        {"Reset statistics", 1000, 1500,
         []()
         { PowerSupply.Current.Statistics.ResetStats(); }, nullptr},
        // {"2nd Reset statistics", 1000, 1500,
        //  []()
        //  { PowerSupply.ResetStats(); }, nullptr},
        {"Measuring current at 0V", 30000, 1500,
         nullptr, [c]()
         { c->v0 = PowerSupply.Current.Statistics.Mean(); }},
        {"Setting voltage to 32V", 1500, 500,
         []()
         {
             PowerSupply.Voltage.SetUpdate(32.0 * PowerSupply.Voltage.adjFactor + PowerSupply.Voltage.adjOffset);
         },
         nullptr},
        {"Reset statistics", 1500, 1500,
         []()
         { PowerSupply.Current.Statistics.ResetStats(); }, nullptr},

        // {"2nd Reset statistics", 1000, 1500,
        //  []()
        //  { PowerSupply.ResetStats(); }, nullptr},

        {"Measuring current at 32V", 30000, 1000,
         nullptr, [c]()
         { c->v1 = PowerSupply.Current.Statistics.Mean(); }},
        {"Finalize", 100, 100, nullptr,
         [c]()
         {
            blockAll = true;
             log_step("           i0 = %+1.6f", c->v0);
             log_step("           i1 = %+1.6f", c->v1);
             double Rtot = (PowerSupply.mA_Active ? 1000.0 : 1.0) * 32.0f / (c->v1 - c->v0) / 1000.0f;
             log_step("Measured Res: %4.3fk", Rtot);

             lv_spinbox_set_value(Calib_GUI.internalLeakage, 1000.0f * Rtot);
             PowerSupply.CalBank[PowerSupply.bankCalibId].internalLeakage = Rtot;
             PowerSupply.Voltage.SetUpdate(0.0 * PowerSupply.Voltage.adjFactor + PowerSupply.Voltage.adjOffset);
             lv_timer_t *close_t = lv_timer_create(close_log_cb, 6000, nullptr);

             lv_timer_set_repeat_count(close_t, 1);
             lv_mem_free(c);
             blockAll = false;
         }},
    };

    lv_timer_t *t = lv_timer_create(seq_cb, 1, nullptr);
    seq_start(t, steps, sizeof(steps) / sizeof(steps[0]), nullptr);
}

static void event_cb(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_current_target(e);
    if (lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED)
    {
        const char *txt = lv_msgbox_get_active_btn_text(obj);
        lv_msgbox_close(obj);
        if (txt && strcmp(txt, "OK") == 0)
        {
            blockAll = true;
            create_log_window();
            // start_auto_measure();
            PowerSupply.CalBank[PowerSupply.bankCalibId].internalLeakage = FLT_MAX;
            s_len = 0;
            s_logbuf[0] = '\0';
            s_pending = false;
            log_clear();
            start_current_totalR();
            blockAll = false;
        }
    }
}

void Warning_msgbox(const char *title, lv_event_cb_t event_cb)
{
    // static const char *btns[] = {"OK", "Cancel", ""};
    // lv_obj_t *mbox1 = lv_msgbox_create(NULL, title, "Disconnect any load!", btns, true);
    // lv_obj_add_event_cb(mbox1, event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    // lv_obj_center(mbox1);

    // static const char *btns[] = {"OK", "Cancel", ""};
    // lv_obj_t *mbox1 = lv_msgbox_create(
    //     NULL, title,        "#FF0000 Disconnect any load!#", // 🔴 red text
    //     btns,     true);
    // lv_obj_add_event_cb(mbox1, event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    // // Enable recolor so LVGL applies the #RRGGBB tags
    // lv_label_set_recolor(lv_msgbox_get_text(mbox1), true);
    // lv_obj_center(mbox1);

    static const char *btns[] = {"OK", "Cancel", ""};
    lv_obj_t *mbox1 = lv_msgbox_create(NULL, title, "Disconnect any load!", btns, true);

    // Style for red background + white text
    static lv_style_t style_warn;
    static bool inited = false;
    if (!inited)
    {
        lv_style_init(&style_warn);
        lv_style_set_bg_color(&style_warn, lv_color_hex(0xFF0000)); // red background
        lv_style_set_bg_opa(&style_warn, LV_OPA_COVER);
        lv_style_set_text_color(&style_warn, lv_color_hex(0xFFFFFF)); // white text
        inited = true;
    }

    // Apply style to the text area of the msgbox
    lv_obj_t *txt = lv_msgbox_get_text(mbox1);
    lv_obj_add_style(txt, &style_warn, LV_PART_MAIN);

    lv_obj_add_event_cb(mbox1, event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_center(mbox1);

    // static const char *btns[] = {"OK", "Cancel", ""};
    // lv_obj_t *mbox1 = lv_msgbox_create(NULL, title, "Disconnect any load!", btns, true);

    // // ── Style (red background, white text)
    // static lv_style_t style_warn;
    // static bool inited = false;
    // if (!inited)
    // {
    //     lv_style_init(&style_warn);
    //     lv_style_set_bg_color(&style_warn, lv_color_hex(0xFF0000)); // 🔴 red
    //     lv_style_set_bg_opa(&style_warn, LV_OPA_COVER);
    //     lv_style_set_text_color(&style_warn, lv_color_hex(0xFFFFFF)); // ⚪ white
    //     inited = true;
    // }

    // // Apply to title bar
    // lv_obj_t *title_obj = lv_msgbox_get_title(mbox1);
    // if (title_obj)
    //     lv_obj_add_style(title_obj, &style_warn, LV_PART_MAIN);

    // // Apply to text body
    // lv_obj_t *txt_obj = lv_msgbox_get_text(mbox1);
    // if (txt_obj)
    //     lv_obj_add_style(txt_obj, &style_warn, LV_PART_MAIN);

    // // (Optional) Apply same style to buttons if you want red/white buttons too
    // // lv_obj_t *btns_cont = lv_msgbox_get_btns(mbox1);
    // // if (btns_cont)
    // //     lv_obj_add_style(btns_cont, &style_warn, LV_PART_MAIN);

    // lv_obj_add_event_cb(mbox1, event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    // lv_obj_center(mbox1);
}

lv_obj_t *mbox;
static void AutoMeasureTotalRes_cb(lv_event_t *)
{
    // static const char *btns[] = {"OK", "Cancel", ""};

    // mbox = lv_msgbox_create(NULL, "Auto Measure", "Disconnect any load!", btns, true);
    // lv_obj_add_event_cb(mbox, mbox_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    // lv_obj_center(mbox);
    Warning_msgbox("Auto Measure", event_cb);
}

// Open/create the DAC calibration window
void internal_current_calibration_cb(lv_event_t *)
{
    if (PowerSupply.gui.calibration.win_int_current_calibration)
    {
        lv_obj_clear_flag(PowerSupply.gui.calibration.win_int_current_calibration, LV_OBJ_FLAG_HIDDEN);
        return;
    }

    PowerSupply.gui.calibration.win_int_current_calibration = lv_win_create(lv_scr_act(), 36);
    lv_obj_set_size(PowerSupply.gui.calibration.win_int_current_calibration, 320, 226);
    lv_win_add_title(PowerSupply.gui.calibration.win_int_current_calibration, "Internal Current Calibration");
    auto *close = lv_win_add_btn(PowerSupply.gui.calibration.win_int_current_calibration, LV_SYMBOL_CLOSE, 60);
    lv_obj_add_event_cb(close, btn_close_hide_obj_cb, LV_EVENT_CLICKED, nullptr);
    auto *cont = lv_win_get_content(PowerSupply.gui.calibration.win_int_current_calibration);
    lv_obj_set_style_pad_all(cont, 0, LV_PART_ITEMS);
    lv_obj_set_style_pad_all(cont, 0, LV_PART_MAIN);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);

    int xPos = 10, xOffset = 10, yPos = 25, yOffset = 25;

    // int_total_res
    Calib_GUI.internalLeakage = spinbox_pro(cont, "#FFFFF7 Total Internal Resistor (kΩ):#", 0, 999'999'999, 9, 6, LV_ALIGN_DEFAULT, xPos, yPos + yOffset * 0, 150, 21, &graph_R_16);

    PowerSupply.LoadCalibrationData();
    // lv_spinbox_set_value(intRes, 40'000.123*1000.0);
    lv_spinbox_set_value(Calib_GUI.internalLeakage, 1000.0 * PowerSupply.CalBank[PowerSupply.bankCalibId].internalLeakage);

    Serial.printf("\nInternal Current Calibration: %f", PowerSupply.CalBank[PowerSupply.bankCalibId].internalLeakage);

    PowerSupply.CalBank[PowerSupply.bankCalibId].internalLeakage = lv_spinbox_get_value(Calib_GUI.internalLeakage) / 1000.0;

    lv_obj_add_event_cb(Calib_GUI.internalLeakage, ADC_iinternalRes_calib_change_event_cb, LV_EVENT_VALUE_CHANGED, nullptr);

    LVButton btnAutoMeasureTotalRes(cont, "Auto Measure", 10, xOffset + 50, 120, 35, nullptr, AutoMeasureTotalRes_cb);

    lv_point_t btn_pos{80, 120};
    LVButton btnLoad(cont, "Load", btn_pos.x, btn_pos.y, 75, 35, nullptr, load_cb);
    LVButton btnSave(cont, "Save", btn_pos.x + 75 + xOffset, btn_pos.y, 75, 35, nullptr, save_cb);
}
// --- Globals ---
// ==== Globals (keep ONE definition of each) ====
// ================= INL CAL (FSM, no std::function, no vectors) =================
// #include "MonotoneCubicCalibrator.h"

// --- ONE global calibrator in the whole project ---
MonotoneCubicCalibrator g_voltINL;
bool g_voltINL_ready = false;

// --- Debug (Serial only — no LVGL calls here) ---
static void INL_dbg(const char *fmt, ...)
{
    char b[160];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(b, sizeof(b), fmt, ap);
    va_end(ap);
#ifdef ARDUINO
    Serial.println(b);
#endif
}

// --- Optional preload (0..32V, 33 pts) ---

lv_obj_t *table_inl;
static const double MEASURED[] = {-.0045, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 32.5};
static const double TRUE_IDEAL[] = {-.0045, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 32.5};
constexpr size_t NPTS = sizeof(MEASURED) / sizeof(MEASURED[0]);
static_assert(NPTS == sizeof(TRUE_IDEAL) / sizeof(TRUE_IDEAL[0]), "knot sizes must match");

// ================= FSM (LVGL timer) =================
struct INL_FSM
{
    static constexpr int NPTS = sizeof(MEASURED) / sizeof(MEASURED[0]);
    ;
    double x_raw[NPTS];  // measured (uncorrected) volts  ← X
    double y_true[NPTS]; // ideal/commanded volts         ← Y
    int i;               // 0..32
    enum Phase : uint8_t
    {
        PREPARE,
        SET,
        SETTLE,
        MEASURE,
        COMPUTE,
        DONE
    } ph;
    uint32_t t0;
    lv_timer_t *timer;
    double step_V;
} static inl;

static inline uint32_t now_ms() { return lv_tick_get(); }
static inline uint32_t since(uint32_t t) { return lv_tick_elaps(t); }
uint16_t last_adjValue;
void inl_gui_prepare()
{
    if (!lvglIsBusy)
    {
        last_adjValue = PowerSupply.Voltage.adjValue;
        // Set start (0V)
        PowerSupply.Voltage.SetUpdate(0.0 * PowerSupply.Voltage.adjFactor + PowerSupply.Voltage.adjOffset);
        PowerSupply.gui.calibration.inl.lbl_inl_state->set_text("#FFFF00 Running");
    }
}

void inl_gui_set(double v_cmd)
{

    table_set_selected_row(table_inl, inl.i + 1);
    PowerSupply.Voltage.SetUpdate(v_cmd * PowerSupply.Voltage.adjFactor + PowerSupply.Voltage.adjOffset);
    PowerSupply.Voltage.measured.ResetStats();
    lv_bar_set_value(PowerSupply.gui.calibration.inl.bar_progress, inl.i, LV_ANIM_OFF);
    // lv_label_set_text_fmt(PowerSupply.gui.calibration.inl.lbl_bar_progress->get_lv_obj, "Progress: %i%%", inl.i * 100 / NPTS);
    PowerSupply.gui.calibration.inl.lbl_bar_progress->set_text_fmt("Progress: %i%%", inl.i * 100 / NPTS);
}

void inl_gui_measure(double measure, double v_cmd)
{
    if (!lvglIsBusy)
    {
        lv_table_set_cell_value_fmt(table_inl, inl.i + 1, 1, "%+08.4f", inl.y_true[inl.i]);
        lv_table_set_cell_value_fmt(table_inl, inl.i + 1, 2, "%+09.5f", measure);
        lv_table_set_cell_value_fmt(table_inl, inl.i + 1, 3, "%+02.2f", (measure - v_cmd) * 30518.043793393); // 10^6/33.7675
    }
}

void inl_gui_compute()
{
    if (!lvglIsBusy)
    {
        PowerSupply.gui.calibration.inl.lbl_inl_state->set_text("#FFFF00 Done");
        PowerSupply.gui.calibration.inl.lbl_bar_progress->set_text_fmt("Progress: 100%%");

        lv_bar_set_value(PowerSupply.gui.calibration.inl.bar_progress, NPTS, LV_ANIM_OFF);
        table_set_selected_row(table_inl, 1);
        PowerSupply.Voltage.SetUpdate(last_adjValue);
    }
}

static void INL_timer_cb(lv_timer_t *)
{
    switch (inl.ph)
    {
    case INL_FSM::PREPARE:
    {
        for (int k = 0; k < INL_FSM::NPTS; ++k)
        {
            inl.x_raw[k] = 0;
            inl.y_true[k] = 0;
        }
        inl.i = 0;
        inl.step_V = 32.0 / (INL_FSM::NPTS - 1); // 1.0 V
        INL_dbg("[INL] PREPARE: 33+ pts, settle 3000 ms");

        inl_gui_prepare();
        inl.ph = INL_FSM::SET;
    }
    break;

    case INL_FSM::SET:
    {
        double v_cmd = TRUE_IDEAL[inl.i]; // TRUE (setpoint) volts

        if (v_cmd > 32.0)                                 // Set the max
            v_cmd = PowerSupply.Voltage.maxValue * .0005; // Maxvolate
        // v_cmd = 32.7675 - PowerSupply.dac_data.zero_voltage * .0005; // se t to max of range

        INL_dbg("[INL] SET     i=%d  v_cmd=%.3f", inl.i, v_cmd);
        inl_gui_set(v_cmd);

        inl.y_true[inl.i] = v_cmd; // Y = true (commanded) volts

        inl.t0 = now_ms();
        inl.ph = INL_FSM::SETTLE;
    }
    break;

    case INL_FSM::SETTLE:
    {
        if (since(inl.t0) >= (uint32_t)100 * lv_spinbox_get_value(PowerSupply.gui.calibration.inl.settle_time))
        { // 2 s settle (per your request)
            INL_dbg("[INL] SETTLE  +%u ms", unsigned(since(inl.t0)));
            inl.ph = INL_FSM::MEASURE;
        }
    }
    break;

    case INL_FSM::MEASURE:
    {
        // const double v_cmd = TRUE_IDEAL[inl.i]; // TRUE (setpoint) volts

        // Mean() already = ideal volts (converted from raw)
        double measure = PowerSupply.Voltage.measured.Mean(); // volts
        // If this is mV in your build, uncomment: ideal *= 0.001;

        inl.x_raw[inl.i] = measure; // X = ideal (linearized) volts
        // inl.y_true[inl.i] = v_cmd;  // Y = true (commanded) volts
        INL_dbg("[INL] MEASURE i=%d  ideal=%.6fV  true=%.6fV", inl.i, measure, inl.y_true[inl.i] /*Ideal!*/);

        inl_gui_measure(measure, inl.y_true[inl.i]);

        // PowerSupply.CalBank[PowerSupply.bankCalibId].adc_inl[inl.i] = measure;

        if (++inl.i >= INL_FSM::NPTS)
            inl.ph = INL_FSM::COMPUTE;
        else
            inl.ph = INL_FSM::SET;
    }
    break;

    case INL_FSM::COMPUTE:
    {
        INL_dbg("[INL] COMPUTE begin");

        std::vector<double> X(inl.x_raw, inl.x_raw + INL_FSM::NPTS); // ideal volts (Mean)
        std::vector<double> Y(TRUE_IDEAL, TRUE_IDEAL + NPTS);        // true volts (set)
        g_voltINL.setPoints(X, Y);
        g_voltINL.build();
        g_voltINL_ready = true;
        for (int i = 0; i < NPTS; i++)
        {
            PowerSupply.CalBank[PowerSupply.bankCalibId].adc_inl_measure[i] = inl.x_raw[i];
            PowerSupply.CalBank[PowerSupply.bankCalibId].adc_inl_ideal[i] = inl.y_true[i];
        }

        INL_dbg("[INL] COMPUTE done (ready=%d)", int(g_voltINL_ready));
        inl_gui_compute();

        PowerSupply.SaveCalibrationData();

        // Serial.printf("\nLoading calibration data");
        // PowerSupply.LoadCalibrationData();
        // for (int i = 0; i < NPTS; i++)
        //     Serial.printf("\nMeasuere%f Ideal%f",
        //                   PowerSupply.CalBank[PowerSupply.bankCalibId].adc_inl_measure[i],
        //                   PowerSupply.CalBank[PowerSupply.bankCalibId].adc_inl_ideal[i]); // g_voltINL.printKnotTable();

        inl.ph = INL_FSM::DONE;
    }
    break;

    case INL_FSM::DONE:
    {
        INL_dbg("[INL] DONE");
        if (inl.timer)
        {
            lv_timer_del(inl.timer);
            inl.timer = nullptr;
        }
    }
    break;
    }
}

// --- Public API ---
// inline void INL_start()
static void INL_start(lv_event_t *e)
{

    lv_obj_t *obj = lv_event_get_current_target(e);
    if (lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED)
    {
        const char *txt = lv_msgbox_get_active_btn_text(obj);
        lv_msgbox_close(obj);
        if (txt && strcmp(txt, "OK") == 0)
        {
            g_voltINL_ready = false;
            if (inl.timer)
            {
                lv_timer_del(inl.timer);
                inl.timer = nullptr;
            }
            inl = INL_FSM{};
            inl.ph = INL_FSM::PREPARE;
            inl.timer = lv_timer_create(INL_timer_cb, 10, nullptr); // 10ms tick; very light
            INL_dbg("[INL] START (LVGL timer)");
        }
    }
}

//******************************************************************* */
static void ADC_INL_VCalib_cb(lv_event_t *)
{
    Warning_msgbox("ADC INL Calibration", INL_start);
}

static lv_style_t style_radio;
static lv_style_t style_radio_chk;
static uint32_t active_index_1 = 0;
static uint32_t active_index_2 = 0;

static void radio_event_handler(lv_event_t *e)
{
    auto *active_id = static_cast<uint32_t *>(lv_event_get_user_data(e)); // ✅ cast

    lv_obj_t *cont = lv_event_get_current_target(e);
    lv_obj_t *act_cb = lv_event_get_target(e);
    lv_obj_t *old_cb = lv_obj_get_child(cont, *active_id);

    /*Do nothing if the container was clicked*/
    if (act_cb == cont)
        return;

    lv_obj_clear_state(old_cb, LV_STATE_CHECKED); /*Uncheck the previous radio button*/
    lv_obj_add_state(act_cb, LV_STATE_CHECKED);   /*Uncheck the current radio button*/

    *active_id = lv_obj_get_index(act_cb);

    LV_LOG_USER("Selected radio buttons: %d, %d", (int)active_index_1, (int)active_index_2);
}

static lv_obj_t *radiobutton_create(lv_obj_t *parent,
                                    const char *txt,
                                    lv_style_t *style_text)
{
    lv_obj_t *obj = lv_checkbox_create(parent);
    lv_checkbox_set_text(obj, txt);

    lv_obj_add_flag(obj, LV_OBJ_FLAG_EVENT_BUBBLE);

    // indicator (circle/box)
    lv_obj_add_style(obj, &style_radio, LV_PART_INDICATOR);
    lv_obj_add_style(obj, &style_radio_chk, LV_PART_INDICATOR | LV_STATE_CHECKED);

    // text (MAIN part)
    if (style_text)
        lv_obj_add_style(obj, style_text, LV_PART_MAIN);

    return obj;
}

static lv_style_t s_dis_main;
static bool s_dis_inited = false;

static void ensure_disabled_style_once()
{
    if (s_dis_inited)
        return;
    s_dis_inited = true;
    lv_style_init(&s_dis_main);
    lv_style_set_bg_opa(&s_dis_main, LV_OPA_30);
    lv_style_set_bg_color(&s_dis_main, lv_palette_lighten(LV_PALETTE_GREY, 2));
    lv_style_set_text_color(&s_dis_main, lv_palette_darken(LV_PALETTE_GREY, 3));
    lv_style_set_border_color(&s_dis_main, lv_palette_darken(LV_PALETTE_GREY, 2));
    lv_style_set_border_opa(&s_dis_main, LV_OPA_80);
}

void ADC_INL_Voltage_calibration_cb(lv_event_t *)
{
    // ─────────────────────────────────────────────────────────────────────────────
    // [0] Re-open existing window (if already created)
    // ─────────────────────────────────────────────────────────────────────────────
    if (PowerSupply.gui.calibration.win_ADC_INL_Voltage_calibration)
    {
        lv_obj_clear_flag(PowerSupply.gui.calibration.win_ADC_INL_Voltage_calibration, LV_OBJ_FLAG_HIDDEN);
        return;
    }
    uint8_t y_off = 5;
    uint8_t x_off = y_off;
    // ─────────────────────────────────────────────────────────────────────────────
    // [1] Create window shell (title + close)
    // ─────────────────────────────────────────────────────────────────────────────
    PowerSupply.gui.calibration.win_ADC_INL_Voltage_calibration = lv_win_create(lv_scr_act(), 36);
    lv_obj_set_size(PowerSupply.gui.calibration.win_ADC_INL_Voltage_calibration, 320, 226);
    lv_win_add_title(PowerSupply.gui.calibration.win_ADC_INL_Voltage_calibration, "ADC INL Voltage Calibration");

    lv_obj_t *btn_close = lv_win_add_btn(PowerSupply.gui.calibration.win_ADC_INL_Voltage_calibration, LV_SYMBOL_CLOSE, 60);
    lv_obj_add_event_cb(btn_close, btn_close_hide_obj_cb, LV_EVENT_CLICKED, nullptr);

    // ─────────────────────────────────────────────────────────────────────────────
    // [2] Content container (layout/padding/scroll)
    // ─────────────────────────────────────────────────────────────────────────────
    lv_obj_t *cont = lv_win_get_content(PowerSupply.gui.calibration.win_ADC_INL_Voltage_calibration);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(cont, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(cont, 0, LV_PART_ITEMS);

    // ─────────────────────────────────────────────────────────────────────────────
    // [3] Load persisted calibration data (if any)
    // ─────────────────────────────────────────────────────────────────────────────
    PowerSupply.LoadCalibrationData();
    // lv_spinbox_set_value(intRes, 40'000.123 * 1000.0);

    // ─────────────────────────────────────────────────────────────────────────────
    // [4] Styles (labels / values) - init once
    // ─────────────────────────────────────────────────────────────────────────────
    static lv_style_t style_lbl, style_val;
    static bool styles_inited = false;
    if (!styles_inited)
    {
        lv_style_init(&style_lbl);
        lv_style_set_text_font(&style_lbl, &lv_font_montserrat_10);

        lv_style_init(&style_val);
        lv_style_set_text_font(&style_val, &graph_R_16);
        lv_style_set_text_color(&style_val, lv_color_hex(0xFFFFFF));

        styles_inited = true;
    }

    // btn_close_hide_obj_cb(lv_event_t *e);
    //    auto dropdownOpenCb = [](lv_event_t *e)
    //     {
    //         dropdown_active = true; // Set flag when dropdown is opened
    //     };
    // 2) Event: toggle siblings based on checkbox state
    static auto set_siblings_enabled_by_checkbox = [](lv_event_t *e)
    {
        ensure_disabled_style_once();

        lv_obj_t *chk = lv_event_get_target(e);
        lv_obj_t *parent = lv_obj_get_parent(chk);
        if (!parent)
            return;

        const bool enabled = lv_obj_has_state(chk, LV_STATE_CHECKED);
        const bool disabled = !enabled;

        uint32_t n = lv_obj_get_child_cnt(parent);
        for (uint32_t i = 0; i < n; ++i)
        {
            lv_obj_t *ch = lv_obj_get_child(parent, i);
            if (ch == chk)
                continue; // keep checkbox interactive

            if (disabled)
            {
                // // functionally disabled
                // lv_obj_add_state(ch, LV_STATE_DISABLED);
                // lv_obj_clear_flag(ch, LV_OBJ_FLAG_CLICK_FOCUSABLE);
                // // visibly dimmed
                // lv_obj_add_style(ch, &s_dis_main, LV_PART_MAIN | LV_STATE_DISABLED);
                lv_obj_add_flag(ch, LV_OBJ_FLAG_HIDDEN);
                //  g_voltINL_ready = false;
            }
            else
            {
                // // re-enable
                // lv_obj_clear_state(ch, LV_STATE_DISABLED);
                // // do not force focusable back—only add if you know it had it
                // lv_obj_remove_style(ch, &s_dis_main, LV_PART_MAIN | LV_STATE_DISABLED);

                lv_obj_clear_flag(ch, LV_OBJ_FLAG_HIDDEN);
                //  g_voltINL_ready = true;
            }
            lv_obj_invalidate(ch);
        }

        if (disabled)
            g_voltINL_ready = false;
        else
            g_voltINL_ready = true;
    };

    // ─────────────────────────────────────────────────────────────────────────────
    // [5] Header row: checkbox + status/progress labels
    // ─────────────────────────────────────────────────────────────────────────────
    PowerSupply.gui.calibration.inl.chbx_inl_activate = lv_checkbox_create(cont);
    lv_checkbox_set_text(PowerSupply.gui.calibration.inl.chbx_inl_activate, "INL Calibration");
    lv_obj_set_pos(PowerSupply.gui.calibration.inl.chbx_inl_activate, 3, y_off);
    lv_obj_add_event_cb(PowerSupply.gui.calibration.inl.chbx_inl_activate, set_siblings_enabled_by_checkbox, LV_EVENT_SHORT_CLICKED, nullptr);
    // after creating children & checkbox
    lv_obj_add_state(PowerSupply.gui.calibration.inl.chbx_inl_activate, LV_STATE_CHECKED);

    LVLabel_class lbl_status = LVLabel_class(cont, "#FFFFF7 Status:", PowerSupply.gui.calibration.inl.chbx_inl_activate, 0, y_off, &style_lbl);
    PowerSupply.gui.calibration.inl.lbl_inl_state = new LVLabel_class(cont, "#FFFF00 Idle", lbl_status.get_lv_obj(), 0, 0, &style_lbl);
    PowerSupply.gui.calibration.inl.lbl_inl_state->align_to(LV_ALIGN_OUT_RIGHT_TOP, lbl_status, y_off * 2, 0);
    PowerSupply.gui.calibration.inl.lbl_bar_progress = new LVLabel_class(cont, "#FFFFF7 Progress: 0%#", lbl_status.get_lv_obj(), 0, y_off, &style_lbl);

    // ─────────────────────────────────────────────────────────────────────────────
    // [6] Progress bar
    // ─────────────────────────────────────────────────────────────────────────────
    PowerSupply.gui.calibration.inl.bar_progress = lv_bar_create(cont);
    lv_obj_set_size(PowerSupply.gui.calibration.inl.bar_progress, 126, 5);
    lv_bar_set_value(PowerSupply.gui.calibration.inl.bar_progress, 0, LV_ANIM_OFF);
    lv_obj_align_to(PowerSupply.gui.calibration.inl.bar_progress, PowerSupply.gui.calibration.inl.lbl_bar_progress->get_lv_obj(), LV_ALIGN_OUT_BOTTOM_LEFT, 0, y_off);

    lv_bar_set_range(PowerSupply.gui.calibration.inl.bar_progress, 1, NPTS);

    lv_style_init(&style_radio);
    lv_style_set_radius(&style_radio, LV_RADIUS_CIRCLE);

    lv_style_init(&style_radio_chk);
    lv_style_set_bg_img_src(&style_radio_chk, NULL);

    // ─────────────────────────────────────────────────────────────────────────────
    // [6.5] Progress bar
    // ─────────────────────────────────────────────────────────────────────────────

    LVLabel_class lbl_measurement_source = LVLabel_class(cont, "#FFFFF7 Measurement Source:", PowerSupply.gui.calibration.inl.bar_progress, 0, 2 * y_off, &style_lbl);

    char buf[32];

    lv_obj_t *cont2 = lv_obj_create(cont);
    lv_obj_set_flex_flow(cont2, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_size(cont2, lv_pct(40), lv_pct(40));
    lv_obj_set_x(cont2, lv_pct(10));
    lv_obj_set_y(cont2, lv_pct(60));
    lv_obj_add_event_cb(cont2, radio_event_handler, LV_EVENT_CLICKED, &active_index_2);

    radiobutton_create(cont2, "Internal DAC", &style_lbl);

    lv_obj_t *rb_dmm = radiobutton_create(cont2, "DMM", &style_lbl);

    // make it inactive (grayed out, ignores clicks/focus)
    lv_obj_add_state(rb_dmm, LV_STATE_DISABLED);
    /*Make the first checkbox checked*/
    lv_obj_add_state(lv_obj_get_child(cont2, 0), LV_STATE_CHECKED);
    lv_obj_align_to(cont2, lbl_measurement_source.get_lv_obj(), LV_ALIGN_OUT_BOTTOM_LEFT, x_off * 2, y_off);

    // Zero container padding (inner padding of the box)
    lv_obj_set_style_pad_all(cont2, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_left(cont2, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_right(cont2, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_top(cont2, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_bottom(cont2, 0, LV_PART_MAIN);

    // Zero gaps between flex items
    lv_obj_set_style_pad_row(cont2, 5, LV_PART_MAIN);    // gap along cross axis
    lv_obj_set_style_pad_column(cont2, 0, LV_PART_MAIN); // gap along main axis

    // (Optional) No border/background so it hugs tightly
    lv_obj_set_style_border_width(cont2, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(cont2, LV_OPA_TRANSP, LV_PART_MAIN);

    PowerSupply.gui.calibration.inl.settle_time = spinbox_pro(cont, "#FFFFF7 Settle Time[s]:#", 5, 100, 3, 2, LV_ALIGN_DEFAULT, 240 + x_off, y_off, 60, 20, &graph_R_16);
    lv_obj_align(lv_obj_get_child(PowerSupply.gui.calibration.inl.settle_time, 1), LV_ALIGN_OUT_RIGHT_MID, -90, 0);
    lv_spinbox_set_value(PowerSupply.gui.calibration.inl.settle_time, 30);

    // ─────────────────────────────────────────────────────────────────────────────
    // [7] Action button: "Auto Calib"
    // ─────────────────────────────────────────────────────────────────────────────
    LVButton ADC_INL(cont, "Auto Calib", 0, 0, 126, 35, nullptr, ADC_INL_VCalib_cb);
    ADC_INL.set_align(LV_ALIGN_TOP_LEFT);
    lv_obj_align_to(ADC_INL.get_lv_obj(), rb_dmm, LV_ALIGN_OUT_BOTTOM_LEFT, -x_off * 2, 2 * y_off);

    // ─────────────────────────────────────────────────────────────────────────────
    // [8] Table: create + basic style
    // ─────────────────────────────────────────────────────────────────────────────
    static lv_style_t style_stats;
    lv_style_init(&style_stats);

    // table_inl:  (x=154, y=3, w=200, h=186, border=0, rows=5)  ← params per your table_pro()
    table_inl = table_pro(cont, &style_stats, &graph_R_8,
                          LV_ALIGN_DEFAULT, 154, 0 + 17 * 2, 200, 17 * 9, 0, y_off);

    // Zero container padding (inner padding of the box)
    lv_obj_set_style_pad_all(table_inl, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_left(table_inl, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_right(table_inl, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_top(table_inl, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_bottom(table_inl, 0, LV_PART_MAIN);

    // Headers
    lv_table_set_cell_value_fmt(table_inl, 0, 0, "#");
    lv_table_set_cell_value_fmt(table_inl, 0, 1, "Ideal");
    lv_table_set_cell_value_fmt(table_inl, 0, 2, "Measured");

    // ─────────────────────────────────────────────────────────────────────────────
    // [9] Table: populate rows
    // ─────────────────────────────────────────────────────────────────────────────
    for (int i = 0; i < NPTS; ++i)
    {
        lv_table_set_cell_value_fmt(table_inl, i + 1, 0, "%02i", i);
        lv_table_set_cell_value_fmt(table_inl, i + 1, 1, "%+08.4f", TRUE_IDEAL[i]);
        lv_table_set_cell_value_fmt(table_inl, i + 1, 2, "%+09.5f", MEASURED[i]);
    }

    // ─────────────────────────────────────────────────────────────────────────────
    // [10] Table: sizing (fixed cols then auto-fit)
    // ─────────────────────────────────────────────────────────────────────────────
    lv_table_set_col_width(table_inl, 0, 30);
    lv_table_set_col_width(table_inl, 1, 56);
    lv_table_set_col_width(table_inl, 2, 76);

    // After filling text, let columns auto-fit to content (helper you added)
    table_autofit_columns(table_inl);

    // Optional extra header (4th column title) and width tweak
    lv_table_set_cell_value_fmt(table_inl, 0, 3, "INL[ppmFSR]");
    lv_table_set_col_width(table_inl, 3, 86); // keep Measured column readable

    // ─────────────────────────────────────────────────────────────────────────────
    // [11] Events (draw/highlight, touch if needed)
    // ─────────────────────────────────────────────────────────────────────────────
    // lv_obj_add_event_cb(table_inl, table_touch_event_cb,      LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(table_inl, table_draw_cell_event_cb2, LV_EVENT_DRAW_PART_BEGIN, NULL);

    // ─────────────────────────────────────────────────────────────────────────────
    // [12] Final layout nits (avoid duplicates)
    // ─────────────────────────────────────────────────────────────────────────────
    lv_obj_set_style_pad_ver(cont, 0, LV_PART_ITEMS);
    lv_obj_set_style_pad_all(cont, 0, LV_PART_MAIN);
}

// Open/create the DAC calibration window
void open_dac_calibration_cb(lv_event_t *)
{
    if (PowerSupply.gui.calibration.win_DAC_calibration)
    {
        lv_obj_clear_flag(PowerSupply.gui.calibration.win_DAC_calibration, LV_OBJ_FLAG_HIDDEN);
        return;
    }

    PowerSupply.gui.calibration.win_DAC_calibration = lv_win_create(lv_scr_act(), 36);
    lv_obj_set_size(PowerSupply.gui.calibration.win_DAC_calibration, 320, 226);
    lv_win_add_title(PowerSupply.gui.calibration.win_DAC_calibration, "DAC Calibration");
    auto *close = lv_win_add_btn(PowerSupply.gui.calibration.win_DAC_calibration, LV_SYMBOL_CLOSE, 60);
    lv_obj_add_event_cb(close, btn_close_hide_obj_cb, LV_EVENT_CLICKED, nullptr);
    auto *cont = lv_win_get_content(PowerSupply.gui.calibration.win_DAC_calibration);
    lv_obj_set_style_pad_all(cont, 0, LV_PART_ITEMS);
    lv_obj_set_style_pad_all(cont, 0, LV_PART_MAIN);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);

    auto *title = lv_label_create(cont);
    lv_label_set_text(title, "DAC Code      Output V/C");
    lv_obj_align(title, LV_ALIGN_OUT_LEFT_TOP, 95, 10);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(title, lv_color_hex(0x00FFFF), LV_PART_MAIN | LV_STATE_DEFAULT);

    int xPos = 94, yPos = 40, yOffset = 25, xOffset = 10;

    s_zv = spinbox_pro(cont, "#FFFFF7 Zero Voltage:#", 0, 65535, 5, 0, LV_ALIGN_DEFAULT, xPos, yPos + yOffset * 0, 86, 15, &graph_R_16);
    s_mv = spinbox_pro(cont, "#FFFFF7 Max  Voltage:#", 0, 65535, 5, 0, LV_ALIGN_DEFAULT, xPos, yPos + yOffset * 1, 86, 16, &graph_R_16);
    s_zc = spinbox_pro(cont, "#FFFFF7 Zero Current:#", 0, 65535, 5, 0, LV_ALIGN_DEFAULT, xPos, yPos + yOffset * 2.3, 86, 17, &graph_R_16);
    s_mc = spinbox_pro(cont, "#FFFFF7 Max  Current:#", 0, 65535, 5, 0, LV_ALIGN_DEFAULT, xPos, yPos + yOffset * 3.3, 86, 18, &graph_R_16);

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

namespace
{

    static void autoZeroCurrent_cb(lv_event_t *)
    {
        Warning_msgbox("Auto Zero", auto_zero_event_cb);
    }

    static void auto_zero_event_cb(lv_event_t *e)
    {
        lv_obj_t *obj = lv_event_get_current_target(e);
        if (lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED)
        {
            const char *txt = lv_msgbox_get_active_btn_text(obj);
            lv_msgbox_close(obj);
            if (txt && strcmp(txt, "OK") == 0)
            {
                create_log_window();
                s_len = 0;
                s_logbuf[0] = '\0';
                s_pending = false;
                log_clear();
                start_current_zeros(nullptr);
            }
        }
    }

    static void start_current_zeros(lv_event_t *e)
    {
        // auto *c = (CurrentCalCtx *)lv_mem_alloc(sizeof(CurrentCalCtx));
        static int32_t c0_raw = 0;  

        static const SeqStep steps[] = {
            {"Setting voltage to 0V", 1500, 500,
             []()
             { PowerSupply.Voltage.SetUpdate(0.0 * PowerSupply.Voltage.adjFactor + PowerSupply.Voltage.adjOffset); }, nullptr},
            {"Reset statistics", 1000, 1500,
             []()
             { PowerSupply.Current.rawValueStats.ResetStats(); }, nullptr},
            // {"2nd Reset statistics", 1000, 1500,
            //  []()
            //  { PowerSupply.ResetStats(); }, nullptr},
            {"Measuring current at 0V", 10000, 1500,
             nullptr, [&]()
             {
                 c0_raw = PowerSupply.Current.rawValueStats.Mean();
                 Serial.printf("\n Code 1 at zero current:%i", c0_raw);
             }},
            {"Setting code for 0.0A", 1500, 500, [&]()
             {
                 lv_spinbox_set_value(Calib_GUI.Current.code_1, c0_raw);
             },
             nullptr},
            {"Finalize", 1500, 1500, [&]()
             {
                 lv_timer_t *close_t = lv_timer_create(close_log_cb, 6000, nullptr);
                 lv_timer_set_repeat_count(close_t, 1);

                 lv_spinbox_set_value(Calib_GUI.Current.code_1, c0_raw);

                 auto &cal = PowerSupply.CalBank[PowerSupply.bankCalibId].iCal;
                 cal[PowerSupply.mA_Active].code_1 = c0_raw;

                 Serial.printf("\n Code 1 at zero current:%i", c0_raw);
             },
             nullptr},

            // {"2nd Reset statistics", 1000, 1500,
            //  []()
            //  { PowerSupply.ResetStats(); }, nullptr},

        };

        lv_timer_t *t = lv_timer_create(seq_cb, 1, nullptr);
        seq_start(t, steps, sizeof(steps) / sizeof(steps[0]), nullptr);
    }
} // namespace
