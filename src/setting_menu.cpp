#include "setting_menu.h"
#include "calib_log_window.h"
#include "calib_sequencer.h"
#include "calib_internal_leakage.h"
#include "calib_dac.h"
#include "calib_cc_limit.h"
#include "calib_adc.h"
#include "calib_inl.h"
// #include "calib_inl_zc.h"
#include "calib_full_auto.h"
#include "battery_charger.h" // battLeadCalMenu_cb()

// heavy/private includes live ONLY in the .cpp
#include "spinbox_pro.h"
#include "version.h"
#include "buzzer.h"   // provides buzzerSound and myTone()
#include "TFT_eSPI.h" // tft instance
#include "tabs.h"
#include "table_pro.h"
#include "device.hpp"      // provides PowerSupply and dac_data_g
#include "lv_gui_helper.h" // LVLabel, LVButton helpers
#include "disp_perf.h"     // frame-rate overlay switch
#include "spi_quiet.h"     // spiQuietAcquire() for the touch calibration
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
#include "esp_task_wdt.h"

// MonotoneCubicCalibrator.h moved to calib_inl.cpp

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
    log_step("%s", buf);
}

///*****************************************************************************
// ****************** Simple sequencer for auto-measure *************************
// *****************************************************************************

// log_step_begin(), log_step_done(), log_step(), log_clear(), create_log_window() moved to calib_log_window.cpp
// auto_zero_event_cb, autoZeroCurrent_cb moved to calib_adc.cpp

void applyGraphXTicks(); // ui_helpers.cpp - re-applies graph X tick count

namespace
{
}
// ---------- public objects (one definition) ----------

// extern bool lvglChartIsBusy;
extern volatile bool lvglIsBusy;
// Global (or static) residual spline

CalibrationGui Calib_GUI{};

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

    // DAC window widgets moved to calib_dac.cpp

    lv_obj_t *int_total_res = nullptr; // max  current spin

    // pressed/released beeps
    static void PRESSED_event_cb(lv_event_t *) { myTone(NOTE_A4, 5); }
    static void RELEASED_event_cb(lv_event_t *) { myTone(NOTE_A4, 5); }

    // Saved ~2 s after the last change by statisticUpdateInterval(): an NVS
    // write stalls both cores, so not on every slider step. Most of these
    // settings were never saved before unless a setpoint changed afterwards.
    static void settingChanged() { PowerSupply.settingsDirtyAt = millis() | 1; }

} // namespace

void btn_close_hide_obj_cb(lv_event_t *e)
{
    auto *btn = lv_event_get_target(e);
    lv_obj_add_flag(lv_obj_get_parent(lv_obj_get_parent(btn)), LV_OBJ_FLAG_HIDDEN);
    lv_obj_invalidate(lv_scr_act());
}

namespace
{

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

    static void slider_adcRate_event_cb(lv_event_t *e)
    {
        auto *slider = lv_event_get_target(e);
        uint8_t v = lv_slider_get_value(slider);
        // Edits the rate in use: FUN keeps its own (see Device::applyAdcRate)
        auto &s = PowerSupply.settingParameters;
        (PowerSupply.getStatus() == DEVICE::FUN ? s.adcRateFUN : s.adcRate) = v;
        PowerSupply.applyAdcRate();
        settingChanged();
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
        settingChanged();
    }

    static void slider_decimalPoints_event_cb(lv_event_t *e)
    {
        auto *slider = lv_event_get_target(e);
        uint16_t v = lv_slider_get_value(slider);
        char buf[4];
        lv_snprintf(buf, sizeof(buf), "%i", v);
        lv_label_set_text(lv_obj_get_child(lv_obj_get_parent(slider), 1), buf);

        const char *fmt;
        switch (v)
        {
        case 1:  fmt = "%+05.1f"; break;
        case 2:  fmt = "%+06.2f"; break;
        case 3:  fmt = "%+07.3f"; break;
        case 4:  fmt = "%+08.4f"; break;
        default: fmt = "%+07.3f"; break;
        }
        PowerSupply.Voltage.restrict = fmt;
        PowerSupply.Power.restrict = fmt;
        // mA range has its own decimal-count slider/setting below - only apply
        // this one to Current when the A range is active.
        if (!PowerSupply.mA_Active)
            PowerSupply.Current.restrict = fmt;
        PowerSupply.settingParameters.adcNumberOfDigits = v;
        settingChanged();
    }

    // Decimal places for the mA current range, independent of the slider above.
    // Kept separate because mA's effective resolution is ~10 bits lower than
    // A's at the same physical noise (steeper calibration slope) - see
    // calibrationUpdate() in device.cpp.
    static void slider_decimalPoints_mA_event_cb(lv_event_t *e)
    {
        auto *slider = lv_event_get_target(e);
        uint16_t v = lv_slider_get_value(slider);
        char buf[4];
        lv_snprintf(buf, sizeof(buf), "%i", v);
        lv_label_set_text(lv_obj_get_child(lv_obj_get_parent(slider), 1), buf);

        const char *fmt;
        switch (v)
        {
        case 1:  fmt = "%+05.1f"; break;
        case 2:  fmt = "%+06.2f"; break;
        case 3:  fmt = "%+07.3f"; break;
        case 4:  fmt = "%+08.4f"; break;
        // 5 decimals trades the leading integer digit for a fifth decimal
        // (+7.12345 instead of +07.1234) - same 8-char width. Only valid
        // because the mA range tops out at 8.192; see digitFormat() in
        // device.cpp.
        case 5:  fmt = "%+08.5f"; break;
        default: fmt = "%+07.3f"; break;
        }
        if (PowerSupply.mA_Active)
            PowerSupply.Current.restrict = fmt;
        PowerSupply.settingParameters.adcNumberOfDigits_mA = v;
        settingChanged();
    }

    // TEMPORARY experiment: ADS1219 PGA x4 on current conversions only.
    // Intentionally not written to settingParameters, so it is never saved and
    // always returns to x1 on the next boot.
    static void switch_iPga_event_cb(lv_event_t *e)
    {
        if (lv_event_get_code(e) != LV_EVENT_VALUE_CHANGED)
            return;
        auto *sw = lv_event_get_target(e);
        const bool x4 = lv_obj_has_state(sw, LV_STATE_CHECKED);
        PowerSupply.iPgaX4 = x4;
        // No on-screen "x4" marker by request - the switch state and this log
        // line are the only indication. OVL still reflects the scaled range.
        Serial.printf("\nI PGA: x%d", x4 ? 4 : 1);
    }

    static void switch_show_fps_event_cb(lv_event_t *e)
    {
        dispPerfSetSaved(lv_obj_has_state(lv_event_get_target(e), LV_STATE_CHECKED));
    }

    static void switch_discard_first_event_cb(lv_event_t *e)
    {
        auto *sw = lv_event_get_target(e);
        PowerSupply.settingParameters.adcDiscardFirst = lv_obj_has_state(sw, LV_STATE_CHECKED);
        Serial.printf("\nDiscard 1st conversion: %s", PowerSupply.settingParameters.adcDiscardFirst ? "ON" : "OFF");
        settingChanged();
    }

    static void switch_buzzer_event_cb(lv_event_t *e)
    {
        if (lv_event_get_code(e) != LV_EVENT_SHORT_CLICKED)
            return;
        auto *sw = lv_event_get_target(e);
        if (lv_obj_get_state(sw) & LV_STATE_CHECKED)
        {
            buzzerSound = true;
            PowerSupply.settingParameters.buzzer = true;
            myTone(NOTE_A5, 10, true);
        }
        else
        {
            buzzerSound = false;
            PowerSupply.settingParameters.buzzer = false;
            myTone(NOTE_A3, 10);
        }
        PowerSupply.SaveSetting();  // Save buzzer state
    }

    // Beep on Power Change / Beep on Error: user_data points at the setting
    static void switch_beep_flag_event_cb(lv_event_t *e)
    {
        bool *flag = static_cast<bool *>(lv_event_get_user_data(e));
        *flag = lv_obj_has_state(lv_event_get_target(e), LV_STATE_CHECKED);
        PowerSupply.SaveSetting();
    }

    // General save/load callbacks used by settings menu and internal calibration window
    static void startup_behavior_event_cb(lv_event_t *e)
    {
        auto *dd = lv_event_get_target(e);
        uint16_t selected = lv_dropdown_get_selected(dd);
        PowerSupply.settingParameters.startupBehavior = static_cast<StartupBehavior>(selected);
        PowerSupply.SaveSetting();

        Serial.printf("\nStartup behavior set to: %d", selected);
    }

    // Event callbacks for new sliders and switches
    static void switch_timer_enable_event_cb(lv_event_t *e)
    {
        auto *sw = lv_event_get_target(e);
        bool enabled = lv_obj_has_state(sw, LV_STATE_CHECKED);
        PowerSupply.settingParameters.timerEnabled = enabled;
        settingChanged();
        // Takes effect now, not only at the next output-on
        PowerSupply.timerStartTime = (enabled && PowerSupply.getStatus() != DEVICE::OFF) ? millis() : 0;
        Serial.printf("\nTimer enabled: %d", enabled);
    }

    static void slider_timer_duration_event_cb(lv_event_t *e)
    {
        auto *slider = lv_event_get_target(e);
        int32_t v = lv_slider_get_value(slider);
        char buf[8];
        lv_snprintf(buf, sizeof(buf), "%d", v);
        lv_label_set_text(lv_obj_get_child(lv_obj_get_parent(slider), 1), buf);
        PowerSupply.settingParameters.timerDurationSeconds = v * 60;  // Convert minutes to seconds
        settingChanged();
        Serial.printf("\nTimer duration set to: %d seconds", PowerSupply.settingParameters.timerDurationSeconds);
    }

    static void slider_voltage_limit_event_cb(lv_event_t *e)
    {
        auto *slider = lv_event_get_target(e);
        int32_t v = lv_slider_get_value(slider);
        char buf[10];
        lv_snprintf(buf, sizeof(buf), "%d.%dV", v / 10, v % 10); // slider in 0.1 V
        lv_label_set_text(lv_obj_get_child(lv_obj_get_parent(slider), 1), buf);
        PowerSupply.settingParameters.voltageLimitMax = v / 10.0f;
        settingChanged();
    }

    static void slider_current_limit_event_cb(lv_event_t *e)
    {
        auto *slider = lv_event_get_target(e);
        int32_t v = lv_slider_get_value(slider);
        char buf[10];
        lv_snprintf(buf, sizeof(buf), "%d.%02dA", v / 100, v % 100); // slider in 0.01 A
        lv_label_set_text(lv_obj_get_child(lv_obj_get_parent(slider), 1), buf);
        PowerSupply.settingParameters.currentLimitMax = v / 100.0f;
        settingChanged();
    }

    static void switch_ovp_enable_event_cb(lv_event_t *e)
    {
        auto *sw = lv_event_get_target(e);
        bool enabled = lv_obj_has_state(sw, LV_STATE_CHECKED);
        PowerSupply.settingParameters.ovpEnabled = enabled;
        settingChanged();
        Serial.printf("\nOVP enabled: %d", enabled);
    }

    static void switch_output_fault_event_cb(lv_event_t *e)
    {
        const bool enabled = lv_obj_has_state(lv_event_get_target(e), LV_STATE_CHECKED);
        PowerSupply.settingParameters.outputFaultProtOff = enabled ? 0 : 1;
        settingChanged();
    }

    static void switch_ocp_enable_event_cb(lv_event_t *e)
    {
        auto *sw = lv_event_get_target(e);
        bool enabled = lv_obj_has_state(sw, LV_STATE_CHECKED);
        PowerSupply.settingParameters.ocpEnabled = enabled;
        settingChanged();
        Serial.printf("\nOCP enabled: %d", enabled);
    }

    static void slider_output_delay_event_cb(lv_event_t *e)
    {
        auto *slider = lv_event_get_target(e);
        int32_t v = lv_slider_get_value(slider);
        char buf[8];
        lv_snprintf(buf, sizeof(buf), "%dms", v);
        lv_label_set_text(lv_obj_get_child(lv_obj_get_parent(slider), 1), buf);
        PowerSupply.settingParameters.outputDelayMs = v;
        settingChanged();
    }

    static void slider_autosave_event_cb(lv_event_t *e)
    {
        auto *slider = lv_event_get_target(e);
        int32_t v = lv_slider_get_value(slider);
        char buf[12];
        if (v == 0)
            lv_snprintf(buf, sizeof(buf), "OFF");
        else
            lv_snprintf(buf, sizeof(buf), "%dmin", v);
        lv_label_set_text(lv_obj_get_child(lv_obj_get_parent(slider), 1), buf);
        PowerSupply.settingParameters.autoSaveIntervalMinutes = v;
        settingChanged();
    }

    // Graph X-axis mode toggle callback
    static void switch_graph_xaxis_mode_event_cb(lv_event_t *e)
    {
        auto *sw = lv_event_get_target(e);
        bool timeMode = lv_obj_has_state(sw, LV_STATE_CHECKED);
        PowerSupply.settingParameters.graphXaxisTimeMode = timeMode;
        settingChanged();
        applyGraphXTicks(); // label count depends on the mode
        Serial.printf("\nGraph X-axis mode: %s", timeMode ? "Time" : "Points");
    }

    static void switch_graph_autostop_event_cb(lv_event_t *e)
    {
        auto *sw = lv_event_get_target(e);
        PowerSupply.settingParameters.graphAutoStop = lv_obj_has_state(sw, LV_STATE_CHECKED);
        settingChanged();
        Serial.printf("\nGraph auto-stop: %s", PowerSupply.settingParameters.graphAutoStop ? "ON" : "OFF");
    }

    // Graph time span slider callback
    static void slider_graph_timespan_event_cb(lv_event_t *e)
    {
        auto *slider = lv_event_get_target(e);
        int32_t v = lv_slider_get_value(slider);
        char buf[16];

        // Convert slider value (0-11) to time spans
        // 0=5s, 1=10s, 2=30s, 3=60s(1m), 4=5m, 5=10m, 6=30m, 7=1h, 8=2h, 9=4h, 10=8h, 11=12h
        const uint16_t timeSpans[] = { 5, 10, 30, 60, 300, 600, 1800, 3600, 7200, 14400, 28800, 43200};
        const char* labels[] = {"5s", "10s", "30s", "1m", "5m", "10m", "30m", "1h", "2h", "4h", "8h", "12h"};

        if (v >= 0 && v < 12)
        {
            PowerSupply.settingParameters.graphTimeSpanSeconds = timeSpans[v];
            settingChanged();
            applyGraphXTicks(); // divisions are chosen per span
            lv_snprintf(buf, sizeof(buf), "%s", labels[v]);
            lv_label_set_text(lv_obj_get_child(lv_obj_get_parent(slider), 1), buf);
            Serial.printf("\nGraph time span set to: %s (%d seconds)", labels[v], timeSpans[v]);
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

        // CRITICAL: Validate array bounds after loading (LoadCalibrationData already validates, but double-check)
        if (PowerSupply.CalBank.empty() || PowerSupply.bankCalibId < 0 ||
            PowerSupply.bankCalibId >= (int8_t)PowerSupply.CalBank.size())
        {
            Serial.printf("\nERROR in load_cb: Invalid bankCalibId=%d after load", PowerSupply.bankCalibId);
            return;
        }

        if (PowerSupply.mA_Active < 0 || PowerSupply.mA_Active > 1)
        {
            Serial.printf("\nERROR in load_cb: Invalid mA_Active=%d", PowerSupply.mA_Active);
            return;
        }

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
            lv_obj_invalidate(Calib_GUI.Current.code_1);
            lv_obj_invalidate(Calib_GUI.Current.code_2);
            lv_obj_invalidate(Calib_GUI.Current.vin_1);
            lv_obj_invalidate(Calib_GUI.Current.vin_2);
        }
        if (PowerSupply.gui.calibration.win_int_current_calibration && !lv_obj_has_flag(PowerSupply.gui.calibration.win_int_current_calibration, LV_OBJ_FLAG_HIDDEN))
        {
            // Update both internal resistor spinboxes
            if (Calib_GUI.internalLeakage_A)
                lv_spinbox_set_value(Calib_GUI.internalLeakage_A, PowerSupply.CalBank[PowerSupply.bankCalibId].internalLeakage[0] * 1000.0);
            if (Calib_GUI.internalLeakage_mA)
                lv_spinbox_set_value(Calib_GUI.internalLeakage_mA, PowerSupply.CalBank[PowerSupply.bankCalibId].internalLeakage[1] * 1000.0);
        }

        // Refresh DAC calibration window spinboxes
        refreshDacCalibSpinboxes();
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

    // build_adc_calibration_window() moved to calib_adc.cpp

    // DAC handlers - separate callbacks for A and mA ranges
    static void ADC_internalRes_A_change_cb(lv_event_t *e)
    {
        if (lv_event_get_code(e) != LV_EVENT_VALUE_CHANGED)
            return;

        // CRITICAL: Validate array bounds
        if (PowerSupply.CalBank.empty() || PowerSupply.bankCalibId < 0 ||
            PowerSupply.bankCalibId >= (int8_t)PowerSupply.CalBank.size())
        {
            Serial.printf("\nERROR in ADC_internalRes_A_change_cb: Invalid bankCalibId=%d",
                         PowerSupply.bankCalibId);
            return;
        }

        double val = lv_spinbox_get_value(Calib_GUI.internalLeakage_A) / 1000.0;
        PowerSupply.CalBank[PowerSupply.bankCalibId].internalLeakage[0] = val;  // [0] = A range
    }

    static void ADC_internalRes_mA_change_cb(lv_event_t *e)
    {
        if (lv_event_get_code(e) != LV_EVENT_VALUE_CHANGED)
            return;

        // CRITICAL: Validate array bounds
        if (PowerSupply.CalBank.empty() || PowerSupply.bankCalibId < 0 ||
            PowerSupply.bankCalibId >= (int8_t)PowerSupply.CalBank.size())
        {
            Serial.printf("\nERROR in ADC_internalRes_mA_change_cb: Invalid bankCalibId=%d",
                         PowerSupply.bankCalibId);
            return;
        }

        double val = lv_spinbox_get_value(Calib_GUI.internalLeakage_mA) / 1000.0;
        PowerSupply.CalBank[PowerSupply.bankCalibId].internalLeakage[1] = val;  // [1] = mA range
    }

    // DAC handlers moved to calib_dac.cpp

    // LCD touch calibration kept private
    // Saved to NVS and loaded by initializeTouch() at boot (it used to be lost
    // on every restart). Takes the SPI slot for the whole run, so in low-noise
    // mode the ADC simply waits.
    static void LCD_Calibration_cb(lv_event_t *)
    {
        uint16_t calData[5];
        spiQuietAcquire();
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
        spiQuietEnd();
        tft.setTouch(calData);
        PowerSupply.StoreMem.begin("my-app", false);
        PowerSupply.StoreMem.putBytes("touch_cal", calData, sizeof(calData));
        PowerSupply.StoreMem.end();
        lv_obj_invalidate(lv_scr_act());
    }

    static lv_obj_t *create_dropdown(lv_obj_t *parent, const char *txt, const char *options,
                                     uint16_t selected, lv_event_cb_t event_cb)
    {
        lv_obj_t *obj = lv_menu_cont_create(parent);
        lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_style_pad_all(obj, 3, LV_PART_MAIN);

        auto *label = lv_label_create(obj);
        lv_label_set_text(label, txt);
        lv_obj_set_width(label, LV_PCT(100));

        auto *dropdown = lv_dropdown_create(obj);
        lv_dropdown_set_options(dropdown, options);
        lv_dropdown_set_selected(dropdown, selected);
        lv_obj_set_width(dropdown, LV_PCT(100));
        if (event_cb)
            lv_obj_add_event_cb(dropdown, event_cb, LV_EVENT_VALUE_CHANGED, nullptr);
        lv_obj_add_event_cb(dropdown, PRESSED_event_cb, LV_EVENT_PRESSED, nullptr);
        lv_obj_add_event_cb(dropdown, RELEASED_event_cb, LV_EVENT_RELEASED, nullptr);

        return dropdown;
    }

    // Classic tornado-siren wail: motor spin-up, hold at top pitch,
    // coast-down. Pitch follows the rotor's exponential speed curve.
    // Siren shape, edited live from the Beeper page spinboxes (not saved:
    // back to these defaults after a restart)
    struct SirenParams
    {
        int32_t fLo = 400, fHi = 1100;               // Hz
        int32_t rise = 4000, hold = 2000, fall = 5000; // ms
        int32_t riseTau = 1000;                      // ms, spin-up time constant
        int32_t fallK = 30;                          // x0.1, coast-down decay over the fall
        int32_t cycles = 2;                          // wails per demo press
    } s_siren;

    lv_timer_t *s_sirenTimer = nullptr;
    uint32_t s_sirenStart = 0;
    uint8_t s_sirenCycles = 0;
    void (*s_sirenDone)(bool) = nullptr;

    static void sirenStop(bool finished = false)
    {
        if (!s_sirenTimer)
            return;
        lv_timer_del(s_sirenTimer);
        s_sirenTimer = nullptr;
        sweepStop();
        auto done = s_sirenDone;
        s_sirenDone = nullptr;
        if (done)
            done(finished);
    }

    static void siren_timer_cb(lv_timer_t *)
    {
        const float fLo = s_siren.fLo, fHi = s_siren.fHi;
        const uint32_t rise = s_siren.rise, hold = s_siren.hold, fall = s_siren.fall;
        const float tau = s_siren.riseTau, k = s_siren.fallK / 10.0f;
        const uint32_t cycle = rise + hold + fall;
        uint32_t ms = millis() - s_sirenStart;
        if (ms >= s_sirenCycles * cycle || !buzzerSound)
        {
            sirenStop(ms >= s_sirenCycles * cycle);
            return;
        }
        ms %= cycle;
        // (float) first: -rise on the unsigned wrapped to ~4e9, fTop was -inf
        // and the hold and fall played a 100 Hz buzz (sweepSet's floor).
        const float fTop = fLo + (fHi - fLo) * (1 - expf(-(float)rise / tau));
        float f;
        if (ms < rise)
            f = fLo + (fHi - fLo) * (1 - expf(-(float)ms / tau));
        else if (ms < rise + hold)
            f = fTop;
        else
        {
            // coast-down
            const float x = (float)(ms - rise - hold) / fall; // 0..1
            f = fLo + (fTop - fLo) * expf(-k * x);
        }
        sweepSet(f);
    }

    // Demo button: two wails; press again to stop.
    static void btn_siren_demo_cb(lv_event_t *)
    {
        remove_selected_spinbox(); // hand the encoders back to page scrolling
        sirenDemoToggle();
    }

    static void siren_spin_cb(lv_event_t *e)
    {
        *static_cast<int32_t *>(lv_event_get_user_data(e)) = lv_spinbox_get_value(lv_event_get_target(e));
    }

    // One row per siren parameter: label left, spinbox right. Touch a spinbox
    // to select it; encoder 1 changes the value, encoder 2 the digit.
    static void create_siren_spinboxes(lv_obj_t *section)
    {
        struct Row { const char *label; int32_t *val; int32_t min, max; uint8_t digits, sep; };
        const Row rows[] = {
            {"Low Hz", &s_siren.fLo, 100, 5000, 4, 0},
            {"High Hz", &s_siren.fHi, 100, 9999, 4, 0},
            {"Rise ms", &s_siren.rise, 100, 30000, 5, 0},
            {"Rise tau ms", &s_siren.riseTau, 50, 9999, 4, 0},
            {"Hold ms", &s_siren.hold, 0, 30000, 5, 0},
            {"Fall ms", &s_siren.fall, 100, 30000, 5, 0},
            {"Fall decay", &s_siren.fallK, 1, 99, 2, 1},
            {"Wails", &s_siren.cycles, 1, 20, 2, 0},
        };
        constexpr lv_coord_t rowH = 28;
        lv_obj_t *cont = lv_obj_create(section);
        lv_obj_remove_style_all(cont);
        lv_obj_set_size(cont, LV_PCT(100), rowH * (sizeof(rows) / sizeof(rows[0])) + 6);
        lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
        uint8_t id = 40;
        lv_coord_t y = 4;
        for (const Row &r : rows)
        {
            lv_obj_t *sb = spinbox_pro(cont, "", r.min, r.max, r.digits, r.sep, LV_ALIGN_TOP_RIGHT, -6, y, 64, id++, &graph_R_16);
            lv_obj_align(sb, LV_ALIGN_TOP_RIGHT, -6, y); // align_to ran before cont had its width
            lv_spinbox_set_value(sb, *r.val);
            lv_obj_add_event_cb(sb, siren_spin_cb, LV_EVENT_VALUE_CHANGED, r.val);
            lv_obj_t *lbl = lv_label_create(cont);
            lv_label_set_text(lbl, r.label);
            lv_obj_align(lbl, LV_ALIGN_TOP_LEFT, 8, y + 4);
            y += rowH;
        }
    }

    static lv_obj_t *create_button_item(lv_obj_t *parent, lv_event_cb_t cb, const char *buttonTxt)
    {
        lv_obj_t *obj = lv_menu_cont_create(parent);
        lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_style_pad_all(obj, 3, LV_PART_ITEMS);
        lv_obj_set_style_pad_all(obj, 3, LV_PART_MAIN);

        auto *btn = lv_btn_create(obj);
        lv_obj_set_size(btn, 120, 30);

        // Add blue background for focused/pressed state
        lv_obj_set_style_bg_color(btn, lv_palette_main(LV_PALETTE_BLUE), LV_STATE_FOCUSED);
        lv_obj_set_style_bg_color(btn, lv_palette_main(LV_PALETTE_BLUE), LV_STATE_PRESSED);

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

void sirenPlay(uint8_t cycles, void (*onDone)(bool))
{
    sirenStop();
    if (!buzzerSound)
    {
        if (onDone)
            onDone(true);
        return;
    }
    s_sirenStart = millis();
    s_sirenCycles = cycles;
    s_sirenDone = onDone;
    sweepStart(s_siren.fLo);
    s_sirenTimer = lv_timer_create(siren_timer_cb, 20, nullptr);
}

void sirenCancel() { sirenStop(); }

// Beeper page button and SYST:BEEP:SIR
void sirenDemoToggle()
{
    if (s_sirenTimer)
        sirenStop();
    else
        sirenPlay(s_siren.cycles);
}

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
    // No "Backlight" slider: the backlight has no PWM pin wired, it did nothing
    {
        lv_obj_t *cont = create_switch_(section, nullptr, "Show FPS", PowerSupply.settingParameters.showFps == 1,
                                        switch_show_fps_event_cb, LV_EVENT_VALUE_CHANGED, nullptr);
        dispPerfSetSwitch(lv_obj_get_child(cont, 1)); // child 0 label, 1 switch
    }


    // Measure
    lv_obj_t *sub_measure = lv_menu_page_create(PowerSupply.gui.setting_menu, nullptr);
    section = lv_menu_section_create(sub_measure);
    PowerSupply.gui.slider_adcRate = create_slider(section, nullptr, "ADC SPS", 0, 4, PowerSupply.settingParameters.adcRate, slider_adcRate_event_cb, LV_EVENT_VALUE_CHANGED);
    PowerSupply.gui.slider_Avgs = create_slider(section, nullptr, "Averaging", 0, (int)std::log2(MAX_NO_OF_AVG), PowerSupply.settingParameters.adcNumberOfAvgs, slider_adcAVG_event_cb, LV_EVENT_VALUE_CHANGED);
    create_slider(section, nullptr, "Number of Digits", 1, 4, PowerSupply.settingParameters.adcNumberOfDigits, slider_decimalPoints_event_cb, LV_EVENT_VALUE_CHANGED);
    // Max 5 (not 4 like the general slider): at 5 the format drops the leading
    // integer digit, which only works for the mA range's 0-8.192 span.
    create_slider(section, nullptr, "mA Digits", 1, 5, PowerSupply.settingParameters.adcNumberOfDigits_mA, slider_decimalPoints_mA_event_cb, LV_EVENT_VALUE_CHANGED);
    // Convert each V and I twice and keep only the second result (halves the rate)
    create_switch_(section, nullptr, "Discard 1st Conversion", PowerSupply.settingParameters.adcDiscardFirst, switch_discard_first_event_cb, LV_EVENT_VALUE_CHANGED, nullptr);
    // The experimental "I PGA x4" switch is off the menu (PowerSupply.iPgaX4
    // stays x1); switch_iPga_event_cb() is kept for bringing it back.
    create_button_item(section, [](lv_event_t *) { PowerSupply.ResetStats(); }, "Reset Stats");

    // Calibration
    lv_obj_t *sub_cal = lv_menu_page_create(PowerSupply.gui.setting_menu, nullptr);
    section = lv_menu_section_create(sub_cal);
    create_button_item(section, btn_calibration_ADC_voltage_event_cb, "ADC Voltage");
    create_button_item(section, btn_calibration_ADC_current_event_cb, "ADC Current");
    create_button_item(section, open_dac_calibration_cb, "DAC Zero/Max");
    create_button_item(section, internal_leakage_calibration_cb, "Leakage");
    create_button_item(section, cc_limit_calibration_cb, "CC Limit (short)");
    create_button_item(section, ADC_INL_Voltage_calibration_cb, "Voltage INL");
    // create_button_item(section, ADC_INL_ZC_calibration_cb, "ADC INL ZC_CAL");
    create_button_item(section, full_auto_calibration_cb, "Full Auto Cal");
    create_button_item(section, battLeadCalMenu_cb, "Lead Resistance");
    create_button_item(section, LCD_Calibration_cb, "Touch Screen");

    // Save/Load
    lv_obj_t *sub_save_load = lv_menu_page_create(PowerSupply.gui.setting_menu, nullptr);
    section = lv_menu_section_create(sub_save_load);

    create_button_item(section, load_cb, "Load Cal.");
    create_button_item(section, save_cb, "Save Cal.");

    // Startup behavior dropdown (placed after Load/Save for better spacing)
    create_dropdown(section, "Power at Startup",
                   "Always Off\nAlways On\nLast Status",
                   static_cast<uint16_t>(PowerSupply.settingParameters.startupBehavior),
                   startup_behavior_event_cb);

    // Timer Function (feature #4)
    lv_obj_t *sub_timer = lv_menu_page_create(PowerSupply.gui.setting_menu, nullptr);
    section = lv_menu_section_create(sub_timer);
    create_switch_(section, nullptr, "Auto-Off Timer", PowerSupply.settingParameters.timerEnabled, switch_timer_enable_event_cb, LV_EVENT_VALUE_CHANGED, nullptr);
    create_slider(section, nullptr, "Timer Duration (min)", 1, 480, PowerSupply.settingParameters.timerDurationSeconds / 60, slider_timer_duration_event_cb, LV_EVENT_VALUE_CHANGED);

    // Power Management (features #5, #6, #7, #13 + voltage/current limits)
    lv_obj_t *sub_power_mgmt = lv_menu_page_create(PowerSupply.gui.setting_menu, nullptr);
    section = lv_menu_section_create(sub_power_mgmt);
    create_switch_(section, nullptr, "OVP Enable", PowerSupply.settingParameters.ovpEnabled, switch_ovp_enable_event_cb, LV_EVENT_VALUE_CHANGED, nullptr);
    // 0.1 V / 0.01 A steps; min above 0, where noise alone would trip it. The
    // old whole-unit sliders also truncated a fractional limit set over SCPI
    // when the menu was built.
    create_slider(section, nullptr, "OVP Level", 1, 330, (int32_t)lroundf(PowerSupply.settingParameters.voltageLimitMax * 10), slider_voltage_limit_event_cb, LV_EVENT_VALUE_CHANGED);
    create_switch_(section, nullptr, "OCP Enable", PowerSupply.settingParameters.ocpEnabled, switch_ocp_enable_event_cb, LV_EVENT_VALUE_CHANGED, nullptr);
    create_slider(section, nullptr, "OCP Level", 1, 650, (int32_t)lroundf(PowerSupply.settingParameters.currentLimitMax * 100), slider_current_limit_event_cb, LV_EVENT_VALUE_CHANGED);
    create_switch_(section, nullptr, "Power Lost / Thermal", PowerSupply.settingParameters.outputFaultProtOff != 1, switch_output_fault_event_cb, LV_EVENT_VALUE_CHANGED, nullptr);
    create_slider(section, nullptr, "Output Delay (ms)", 0, 5000, PowerSupply.settingParameters.outputDelayMs, slider_output_delay_event_cb, LV_EVENT_VALUE_CHANGED);
    create_slider(section, nullptr, "Auto-save Interval (min)", 0, 60, PowerSupply.settingParameters.autoSaveIntervalMinutes, slider_autosave_event_cb, LV_EVENT_VALUE_CHANGED);
    create_text(section, nullptr, "Energy Counter and Power-On\nDuration shown on main screen", 0, nullptr);

    // Graph Settings
    lv_obj_t *sub_graph = lv_menu_page_create(PowerSupply.gui.setting_menu, nullptr);
    section = lv_menu_section_create(sub_graph);
    create_switch_(section, nullptr, "X-axis Time Mode", PowerSupply.settingParameters.graphXaxisTimeMode, switch_graph_xaxis_mode_event_cb, LV_EVENT_VALUE_CHANGED, nullptr);

    // Find initial slider position from graphTimeSpanSeconds
    int initialSliderPos = 3;  // Default to 60s (1m)
    const uint16_t timeSpans[] = {5, 10, 30, 60, 300, 600, 1800, 3600, 7200, 14400, 28800, 43200};
    for (int i = 0; i < 12; i++)
    {
        if (PowerSupply.settingParameters.graphTimeSpanSeconds == timeSpans[i])
        {
            initialSliderPos = i;
            break;
        }
    }
    create_slider(section, nullptr, "Time Span", 0, 11, initialSliderPos, slider_graph_timespan_event_cb, LV_EVENT_VALUE_CHANGED);
    create_text(section, nullptr, "Time span: 5s to 12h\nOnly used when Time Mode enabled", 0, nullptr);
    create_switch_(section, nullptr, "Auto-Stop after Full Span", PowerSupply.settingParameters.graphAutoStop, switch_graph_autostop_event_cb, LV_EVENT_VALUE_CHANGED, nullptr);

    // Beeper Control (feature #11) - Buzzer enable at top, volume removed
    lv_obj_t *sub_beeper_detail = lv_menu_page_create(PowerSupply.gui.setting_menu, nullptr);
    section = lv_menu_section_create(sub_beeper_detail);
    create_switch_(section, nullptr, "Buzzer Enable", buzzerSound, switch_buzzer_event_cb, LV_EVENT_SHORT_CLICKED, nullptr);
    create_switch_(section, nullptr, "Beep on Power Change", PowerSupply.settingParameters.beeperOnPowerChange, switch_beep_flag_event_cb, LV_EVENT_VALUE_CHANGED, &PowerSupply.settingParameters.beeperOnPowerChange);
    create_switch_(section, nullptr, "Beep on Error", PowerSupply.settingParameters.beeperOnError, switch_beep_flag_event_cb, LV_EVENT_VALUE_CHANGED, &PowerSupply.settingParameters.beeperOnError);
    create_text(section, nullptr, "Tornado Siren Demo", 0, nullptr);
    create_button_item(section, btn_siren_demo_cb, "Play / Stop");
    create_siren_spinboxes(section);

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

    itm = create_text(section, nullptr, "Graph", 1, nullptr);
    lv_menu_set_load_page_event(PowerSupply.gui.setting_menu, itm, sub_graph);

    itm = create_text(section, nullptr, "Beeper", 1, nullptr);
    lv_menu_set_load_page_event(PowerSupply.gui.setting_menu, itm, sub_beeper_detail);

    itm = create_text(section, nullptr, "Measure", 1, nullptr);
    lv_menu_set_load_page_event(PowerSupply.gui.setting_menu, itm, sub_measure);

    itm = create_text(section, nullptr, "Calibration", 1, nullptr);
    lv_menu_set_load_page_event(PowerSupply.gui.setting_menu, itm, sub_cal);

    itm = create_text(section, nullptr, "Timer", 1, nullptr);
    lv_menu_set_load_page_event(PowerSupply.gui.setting_menu, itm, sub_timer);

    itm = create_text(section, nullptr, "Protection", 1, nullptr);
    lv_menu_set_load_page_event(PowerSupply.gui.setting_menu, itm, sub_power_mgmt);

    itm = create_text(section, nullptr, "Load/Save", 1, nullptr);
    lv_menu_set_load_page_event(PowerSupply.gui.setting_menu, itm, sub_save_load);

    itm = create_text(section, nullptr, "About", 1, nullptr);
    lv_menu_set_load_page_event(PowerSupply.gui.setting_menu, itm, sub_about_page);

    lv_menu_set_sidebar_page(PowerSupply.gui.setting_menu, root_page);
    auto *m = (lv_menu_t *)PowerSupply.gui.setting_menu;
    lv_obj_set_size(m->sidebar, LV_PCT(38), LV_PCT(100));
    // create_slider() sends VALUE_CHANGED to show each value: nothing changed
    PowerSupply.settingsDirtyAt = 0;

    // Simulate click on first item
    lv_event_send(lv_obj_get_child(lv_obj_get_child(lv_menu_get_cur_sidebar_page(PowerSupply.gui.setting_menu), 0), 0),
                  LV_EVENT_CLICKED, nullptr);
}

// ---------- exported callbacks moved to calib modules ----------
// btn_calibration_ADC_voltage_event_cb() and btn_calibration_ADC_current_event_cb() moved to calib_adc.cpp
// close_log_cb() moved to calib_log_window.cpp
// measure_ctx_t, SeqStep, SeqRunner, seq_start(), seq_cb() moved to calib_sequencer.cpp
// LeakageCalCtx, start_leakage_resistance_measurement(), start_current_zero_calibration() moved to calib_internal_leakage.cpp

static void event_cb(lv_event_t *e)
{
    Serial.println("[IntRes] CB entered");
    lv_obj_t *obj = lv_event_get_current_target(e);
    if (lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED)
    {
        Serial.println("[IntRes] VALUE_CHANGED");
        // Get button text BEFORE closing msgbox (to avoid use-after-free)
        const char *txt = lv_msgbox_get_active_btn_text(obj);
        Serial.printf("[IntRes] btn='%s'\n", txt ? txt : "NULL");
        bool is_ok = (txt && strcmp(txt, "OK") == 0);

        // Now safe to close the msgbox
        Serial.println("[IntRes] calling msgbox_close_deferred");
        msgbox_close_deferred(obj);
        Serial.println("[IntRes] msgbox_close_deferred returned");

        if (is_ok)
        {
            // Prevent multiple triggers - check global flag
            if (g_calibration_in_progress) {
                Serial.println("Leakage resistance: Calibration already in progress, ignoring");
                return;
            }

            // CRITICAL: Validate array bounds before starting
            if (PowerSupply.CalBank.empty() || PowerSupply.bankCalibId < 0 ||
                PowerSupply.bankCalibId >= (int8_t)PowerSupply.CalBank.size() ||
                PowerSupply.mA_Active < 0 || PowerSupply.mA_Active > 1)
            {
                Serial.printf("\nERROR in AutoMeasureTotalRes_cb: Invalid indices! bankCalibId=%d, mA_Active=%d",
                             PowerSupply.bankCalibId, PowerSupply.mA_Active);
                return;
            }

            // Set flag to prevent concurrent calibrations
            g_calibration_in_progress = true;
            esp_task_wdt_reset();

            // Hide the calibration window so the log window is visible
            if (PowerSupply.gui.calibration.win_int_current_calibration &&
                lv_obj_is_valid(PowerSupply.gui.calibration.win_int_current_calibration))
                lv_obj_add_flag(PowerSupply.gui.calibration.win_int_current_calibration, LV_OBJ_FLAG_HIDDEN);

            // Create log window FIRST (before blocking or modifying data)
            create_log_window("Zero + Leakage");
            log_reset();
            log_clear();

            esp_task_wdt_reset();

            // Start the calibration sequence (runs in LVGL timer)
            start_leakage_resistance_measurement(nullptr);
        }
    }
}

// Safe msgbox close — hide only, never delete.
// Calling lv_obj_del from any event callback (directly or via timer/animation)
// corrupts LVGL internal structures. Hidden msgboxes stay in memory;
// Warning_msgbox is called at most a handful of times per boot so the leak
// is negligible.
void msgbox_close_deferred(lv_obj_t *mbox)
{
    if (!mbox || !lv_obj_is_valid(mbox)) return;

    lv_obj_add_flag(mbox, LV_OBJ_FLAG_HIDDEN);
    lv_obj_t *backdrop = lv_obj_get_parent(mbox);
    if (backdrop) lv_obj_add_flag(backdrop, LV_OBJ_FLAG_HIDDEN);
    lv_obj_invalidate(lv_scr_act());
}

bool calib_check_current_setpoint(float min_mA)
{
    // Compute current setpoint in mA regardless of A/mA range
    // adjFactor always converts DAC codes → Amps in both A and mA ranges
    double setpoint = (PowerSupply.Current.adjValue - PowerSupply.Current.adjOffset)
                      / PowerSupply.Current.adjFactor;
    double setpoint_mA = setpoint * 1000.0;  // A → mA, always

    if (setpoint_mA >= min_mA)
        return true;

    // Show a blocking error msgbox — user must set current before calibrating
    static const char *btn[] = {"OK", ""};
    char msg[64];
    snprintf(msg, sizeof(msg), "Set current >= %.0f mA before calibrating!", min_mA);
    lv_obj_t *mbox = lv_msgbox_create(NULL, "Current Too Low", msg, btn, false);
    lv_obj_center(mbox);
    // dismiss-only: wire OK to the deferred-close helper
    lv_obj_add_event_cb(mbox, [](lv_event_t *e) {
        if (lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED)
            msgbox_close_deferred(lv_event_get_current_target(e));
    }, LV_EVENT_VALUE_CHANGED, nullptr);

    Serial.printf("[CAL] Blocked: current setpoint %.2f mA < %.0f mA\n", setpoint_mA, min_mA);
    return false;
}

void Message_msgbox(const char *title, const char *text)
{
    static const char *btn[] = {"OK", ""};
    lv_obj_t *mbox = lv_msgbox_create(NULL, title, text, btn, false);
    lv_obj_center(mbox);
    lv_obj_add_event_cb(mbox, [](lv_event_t *e) {
        if (lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED)
            msgbox_close_deferred(lv_event_get_current_target(e));
    }, LV_EVENT_VALUE_CHANGED, nullptr);
}

bool calib_check_output_on()
{
    // Current zero/leakage are measured at 1..32 V. With the output off the
    // output sits just below 0 V, where pre-tracking SMPS pulses get through to
    // the shunt (-30..-50 uA on the mA range), so nothing useful can be measured.
    if (PowerSupply.getStatus() != DEVICE::OFF)
        return true;

    static const char *btn[] = {"OK", ""};
    lv_obj_t *mbox = lv_msgbox_create(NULL, "Output Off", "Turn the output ON (nothing connected) before calibrating!", btn, false);
    lv_obj_center(mbox);
    lv_obj_add_event_cb(mbox, [](lv_event_t *e) {
        if (lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED)
            msgbox_close_deferred(lv_event_get_current_target(e));
    }, LV_EVENT_VALUE_CHANGED, nullptr);

    Serial.println("[CAL] Blocked: output off");
    return false;
}

void Warning_msgbox(const char *title, lv_event_cb_t event_cb)
{
    esp_task_wdt_reset();

    static const char *btns[] = {"OK", "Cancel", ""};
    lv_obj_t *mbox1 = lv_msgbox_create(NULL, title, "Disconnect any load!", btns, false);

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
}

lv_obj_t *mbox;
static void AutoMeasureTotalRes_cb(lv_event_t *)
{
    esp_task_wdt_reset();
    if (!calib_check_output_on()) return;
    Warning_msgbox("Auto Measure", event_cb);
}

// Open/create the Internal Current Calibration window (shows both A and mA ranges)
void internal_leakage_calibration_cb(lv_event_t *)
{
    // CRITICAL: Validate array bounds before accessing calibration data
    if (PowerSupply.CalBank.empty() || PowerSupply.bankCalibId < 0 ||
        PowerSupply.bankCalibId >= (int8_t)PowerSupply.CalBank.size())
    {
        Serial.printf("\nERROR in internal_leakage_calibration_cb: Invalid bankCalibId=%d",
                     PowerSupply.bankCalibId);
        return;
    }

    if (PowerSupply.gui.calibration.win_int_current_calibration)
    {
        // Update spinbox values when reopening (with null checks)
        if (Calib_GUI.internalLeakage_A)
            lv_spinbox_set_value(Calib_GUI.internalLeakage_A, 1000.0 * PowerSupply.CalBank[PowerSupply.bankCalibId].internalLeakage[0]);
        if (Calib_GUI.internalLeakage_mA)
            lv_spinbox_set_value(Calib_GUI.internalLeakage_mA, 1000.0 * PowerSupply.CalBank[PowerSupply.bankCalibId].internalLeakage[1]);
        lv_obj_clear_flag(PowerSupply.gui.calibration.win_int_current_calibration, LV_OBJ_FLAG_HIDDEN);
        return;
    }

    PowerSupply.gui.calibration.win_int_current_calibration = lv_win_create(lv_scr_act(), 36);
    lv_obj_set_size(PowerSupply.gui.calibration.win_int_current_calibration, 320, 226);
    lv_win_add_title(PowerSupply.gui.calibration.win_int_current_calibration, "Internal Leakage Calibration");
    auto *close = lv_win_add_btn(PowerSupply.gui.calibration.win_int_current_calibration, LV_SYMBOL_CLOSE, 60);
    lv_obj_add_event_cb(close, btn_close_hide_obj_cb, LV_EVENT_CLICKED, nullptr);
    auto *cont = lv_win_get_content(PowerSupply.gui.calibration.win_int_current_calibration);
    lv_obj_set_style_pad_all(cont, 0, LV_PART_ITEMS);
    lv_obj_set_style_pad_all(cont, 0, LV_PART_MAIN);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);

    int xPos = 10, yPos = 20, yOffset = 48;

    // Resistor for A range
    // Range allows negatives: at high V some hardware sources current INTO the
    // sense path (an inverse bias that scales linearly with output voltage), and
    // the same I = V/R model covers it with R < 0. The measurement in
    // calib_internal_leakage.cpp already produces and clamps a signed Rtot, and
    // DACUpdate()'s compensation is signed - a range_min of 0 here was the only
    // thing forcing those measurements to clamp to zero.
    Calib_GUI.internalLeakage_A = spinbox_pro(cont, "#FFFFF7 Leakage [A] (kOhm):#", -999'999'999, 999'999'999, 9, 6, LV_ALIGN_DEFAULT, xPos, yPos, 150, 21, &graph_R_16);
    lv_spinbox_set_value(Calib_GUI.internalLeakage_A, 1000.0 * PowerSupply.CalBank[PowerSupply.bankCalibId].internalLeakage[0]);
    lv_obj_add_event_cb(Calib_GUI.internalLeakage_A, ADC_internalRes_A_change_cb, LV_EVENT_VALUE_CHANGED, nullptr);

    // Resistor for mA range (see note on the A range above)
    Calib_GUI.internalLeakage_mA = spinbox_pro(cont, "#FFFFF7 Leakage [mA] (kOhm):#", -999'999'999, 999'999'999, 9, 6, LV_ALIGN_DEFAULT, xPos, yPos + yOffset, 150, 22, &graph_R_16);
    lv_spinbox_set_value(Calib_GUI.internalLeakage_mA, 1000.0 * PowerSupply.CalBank[PowerSupply.bankCalibId].internalLeakage[1]);
    lv_obj_add_event_cb(Calib_GUI.internalLeakage_mA, ADC_internalRes_mA_change_cb, LV_EVENT_VALUE_CHANGED, nullptr);

    Serial.printf("\nLeakage [A]: %f kOhm", PowerSupply.CalBank[PowerSupply.bankCalibId].internalLeakage[0]);
    Serial.printf("\nLeakage [mA]: %f kOhm", PowerSupply.CalBank[PowerSupply.bankCalibId].internalLeakage[1]);

    LVButton btnAutoMeasureTotalRes(cont, "Auto Leakage", 10, yPos + yOffset * 2 - 10, 120, 30, nullptr, AutoMeasureTotalRes_cb);

    lv_point_t btn_pos{80, 155};
    LVButton btnLoad(cont, "Load", btn_pos.x, btn_pos.y, 75, 30, nullptr, load_cb);
    LVButton btnSave(cont, "Save", btn_pos.x + 85, btn_pos.y, 75, 30, nullptr, save_cb);
}

// INL calibration (globals, FSM, GUI) moved to calib_inl.cpp



// open_dac_calibration_cb() moved to calib_dac.cpp

namespace
{

    // autoZeroCurrent_cb, auto_zero_event_cb moved to calib_adc.cpp
    // start_current_zero_calibration() implementation moved to calib_internal_leakage.cpp

} // namespace
