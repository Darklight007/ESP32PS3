#include "calib_inl.h"
#include "setting_menu.h"
#include "device.hpp"
#include "spinbox_pro.h"
#include "lv_gui_helper.h"
#include "table_pro.h"
#include "myFonts.h"
#include "MonotoneCubicCalibrator.h"
#include <vector>
#include <cstdio>
#include <cstring>

// External references
extern Device PowerSupply;

// =============================================================================
// Globals
// =============================================================================

MonotoneCubicCalibrator g_voltINL;
bool g_voltINL_ready = false;

// Optional preload calibration data (0..32V, 35 pts)
lv_obj_t *table_inl = nullptr;
static const double MEASURED[] = {-.0045, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 32.5};
static const double TRUE_IDEAL[] = {-.0045, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 32.5};
constexpr size_t NPTS = sizeof(MEASURED) / sizeof(MEASURED[0]);
static_assert(NPTS == sizeof(TRUE_IDEAL) / sizeof(TRUE_IDEAL[0]), "knot sizes must match");

// =============================================================================
// Debug helper
// =============================================================================

void INL_dbg(const char *fmt, ...)
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

// =============================================================================
// FSM (Finite State Machine) for auto-calibration
// =============================================================================

struct INL_FSM
{
    static constexpr int NPTS = sizeof(MEASURED) / sizeof(MEASURED[0]);
    double x_raw[NPTS];  // measured (uncorrected) volts  ← X
    double y_true[NPTS]; // ideal/commanded volts         ← Y
    int i;               // 0..34
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

// =============================================================================
// GUI update helpers
// =============================================================================

uint16_t last_adjValue;

void inl_gui_prepare()
{
    last_adjValue = PowerSupply.Voltage.adjValue;
    // Set start (0V)
    PowerSupply.Voltage.SetUpdate(0.0 * PowerSupply.Voltage.adjFactor + PowerSupply.Voltage.adjOffset);

    // Safety check: only update GUI if window is visible
    if (PowerSupply.gui.calibration.win_ADC_INL_Voltage_calibration &&
        !lv_obj_has_flag(PowerSupply.gui.calibration.win_ADC_INL_Voltage_calibration, LV_OBJ_FLAG_HIDDEN))
    {
        PowerSupply.gui.calibration.inl.lbl_inl_state->set_text("#FFFF00 Running");
    }
}

void inl_gui_set(double v_cmd)
{
    PowerSupply.Voltage.SetUpdate(v_cmd * PowerSupply.Voltage.adjFactor + PowerSupply.Voltage.adjOffset);
    PowerSupply.Voltage.measured.ResetStats();

    // Safety check: only update GUI if window is visible
    if (PowerSupply.gui.calibration.win_ADC_INL_Voltage_calibration &&
        !lv_obj_has_flag(PowerSupply.gui.calibration.win_ADC_INL_Voltage_calibration, LV_OBJ_FLAG_HIDDEN))
    {
        table_set_selected_row(table_inl, inl.i + 1);
        lv_bar_set_value(PowerSupply.gui.calibration.inl.bar_progress, inl.i, LV_ANIM_OFF);
        PowerSupply.gui.calibration.inl.lbl_bar_progress->set_text_fmt("Progress: %i%%", inl.i * 100 / NPTS);
    }
}

void inl_gui_measure(double measure, double v_cmd)
{
    if (table_inl && lv_obj_is_valid(table_inl))
    {
        lv_table_set_cell_value_fmt(table_inl, inl.i + 1, 1, "%+08.4f", inl.y_true[inl.i]);
        lv_table_set_cell_value_fmt(table_inl, inl.i + 1, 2, "%+09.5f", measure);
        lv_table_set_cell_value_fmt(table_inl, inl.i + 1, 3, "%+02.2f", (measure - v_cmd) * 30518.043793393); // 10^6/33.7675
    }
}

void inl_gui_compute()
{
    PowerSupply.Voltage.SetUpdate(last_adjValue);

    // Safety check: only update GUI if window is visible
    if (PowerSupply.gui.calibration.win_ADC_INL_Voltage_calibration &&
        !lv_obj_has_flag(PowerSupply.gui.calibration.win_ADC_INL_Voltage_calibration, LV_OBJ_FLAG_HIDDEN))
    {
        PowerSupply.gui.calibration.inl.lbl_inl_state->set_text("#FFFF00 Done");
        PowerSupply.gui.calibration.inl.lbl_bar_progress->set_text_fmt("Progress: 100%%");
        lv_bar_set_value(PowerSupply.gui.calibration.inl.bar_progress, NPTS, LV_ANIM_OFF);
        table_set_selected_row(table_inl, 1);
    }
}

// =============================================================================
// INL FSM timer callback
// =============================================================================

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
        {
            INL_dbg("[INL] SETTLE  +%u ms", unsigned(since(inl.t0)));
            inl.ph = INL_FSM::MEASURE;
        }
    }
    break;

    case INL_FSM::MEASURE:
    {
        double measure = PowerSupply.Voltage.measured.Mean(); // volts
        inl.x_raw[inl.i] = measure; // X = ideal (linearized) volts
        INL_dbg("[INL] MEASURE i=%d  ideal=%.6fV  true=%.6fV", inl.i, measure, inl.y_true[inl.i]);

        inl_gui_measure(measure, inl.y_true[inl.i]);

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

// =============================================================================
// Start INL calibration
// =============================================================================

static void INL_start(lv_event_t *e)
{
    INL_dbg("[INL] INL_start callback triggered");

    if (!e)
    {
        INL_dbg("[INL] ERROR: Null event");
        return;
    }

    lv_obj_t *obj = lv_event_get_current_target(e);
    if (!obj || !lv_obj_is_valid(obj))
    {
        INL_dbg("[INL] ERROR: Invalid msgbox object");
        return;
    }

    if (lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED)
    {
        INL_dbg("[INL] VALUE_CHANGED event received");

        // Get button text BEFORE closing msgbox (to avoid use-after-free)
        const char *txt = lv_msgbox_get_active_btn_text(obj);

        // COPY the button text to local buffer (extra safety)
        char btn_text[16] = {0};
        if (txt)
        {
            strncpy(btn_text, txt, sizeof(btn_text) - 1);
            INL_dbg("[INL] Button pressed: '%s'", btn_text);
        }
        else
        {
            INL_dbg("[INL] WARNING: Null button text");
        }

        // Now safe to close the msgbox
        lv_msgbox_close_async(obj);
        INL_dbg("[INL] Msgbox closed");

        // Check the COPIED string (not the freed pointer!)
        bool is_ok = (btn_text[0] != '\0' && strcmp(btn_text, "OK") == 0);

        if (is_ok)
        {
            INL_dbg("[INL] Starting INL calibration");
            g_voltINL_ready = false;
            if (inl.timer)
            {
                INL_dbg("[INL] Deleting existing timer");
                lv_timer_del(inl.timer);
                inl.timer = nullptr;
            }
            inl = INL_FSM{};
            inl.ph = INL_FSM::PREPARE;
            inl.timer = lv_timer_create(INL_timer_cb, 10, nullptr); // 10ms tick
            INL_dbg("[INL] Timer created, calibration started");
        }
        else
        {
            INL_dbg("[INL] User cancelled - no action taken");
        }
    }
    else
    {
        INL_dbg("[INL] Non-VALUE_CHANGED event: %d", lv_event_get_code(e));
    }
}

static void ADC_INL_VCalib_cb(lv_event_t *)
{
    Warning_msgbox("ADC INL Calibration", INL_start);
}

// =============================================================================
// INL window close handler - stops timer to prevent crashes
// =============================================================================

static void inl_close_window_cb(lv_event_t *e)
{
    // Stop the calibration timer if running
    if (inl.timer)
    {
        INL_dbg("[INL] Window closed - stopping timer");
        lv_timer_del(inl.timer);
        inl.timer = nullptr;
    }

    // Reset FSM to idle state
    inl.ph = INL_FSM::DONE;

    // Hide the window (standard close behavior)
    auto *btn = lv_event_get_target(e);
    lv_obj_add_flag(lv_obj_get_parent(lv_obj_get_parent(btn)), LV_OBJ_FLAG_HIDDEN);
}

// =============================================================================
// Radio button and style helpers
// =============================================================================

static lv_style_t style_radio;
static lv_style_t style_radio_chk;
static uint32_t active_index_1 = 0;
static uint32_t active_index_2 = 0;

static void radio_event_handler(lv_event_t *e)
{
    auto *active_id = static_cast<uint32_t *>(lv_event_get_user_data(e));
    lv_obj_t *cont = lv_event_get_current_target(e);
    lv_obj_t *act_cb = lv_event_get_target(e);
    lv_obj_t *old_cb = lv_obj_get_child(cont, *active_id);

    if (act_cb == cont)
        return;

    lv_obj_clear_state(old_cb, LV_STATE_CHECKED);
    lv_obj_add_state(act_cb, LV_STATE_CHECKED);
    *active_id = lv_obj_get_index(act_cb);
    LV_LOG_USER("Selected radio buttons: %d, %d", (int)active_index_1, (int)active_index_2);
}

static lv_obj_t *radiobutton_create(lv_obj_t *parent, const char *txt, lv_style_t *style_text)
{
    lv_obj_t *obj = lv_checkbox_create(parent);
    lv_checkbox_set_text(obj, txt);
    lv_obj_add_flag(obj, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_obj_add_style(obj, &style_radio, LV_PART_INDICATOR);
    lv_obj_add_style(obj, &style_radio_chk, LV_PART_INDICATOR | LV_STATE_CHECKED);
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

// =============================================================================
// Main INL calibration window
// =============================================================================

void ADC_INL_Voltage_calibration_cb(lv_event_t *)
{
    // Re-open existing window if already created
    if (PowerSupply.gui.calibration.win_ADC_INL_Voltage_calibration)
    {
        lv_obj_clear_flag(PowerSupply.gui.calibration.win_ADC_INL_Voltage_calibration, LV_OBJ_FLAG_HIDDEN);
        return;
    }

    uint8_t y_off = 5;
    uint8_t x_off = y_off;

    // Create window shell
    PowerSupply.gui.calibration.win_ADC_INL_Voltage_calibration = lv_win_create(lv_scr_act(), 36);
    lv_obj_set_size(PowerSupply.gui.calibration.win_ADC_INL_Voltage_calibration, 320, 226);
    lv_win_add_title(PowerSupply.gui.calibration.win_ADC_INL_Voltage_calibration, "ADC INL Voltage Calibration");

    lv_obj_t *btn_close = lv_win_add_btn(PowerSupply.gui.calibration.win_ADC_INL_Voltage_calibration, LV_SYMBOL_CLOSE, 60);
    lv_obj_add_event_cb(btn_close, inl_close_window_cb, LV_EVENT_CLICKED, nullptr);

    // Content container
    lv_obj_t *cont = lv_win_get_content(PowerSupply.gui.calibration.win_ADC_INL_Voltage_calibration);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(cont, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(cont, 0, LV_PART_ITEMS);

    // Load persisted calibration data
    PowerSupply.LoadCalibrationData();

    // Styles (init once)
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

    // Event handler for checkbox-toggled enable/disable
    static auto set_siblings_enabled_by_checkbox = [](lv_event_t *e)
    {
        INL_dbg("[INL] Checkbox event triggered");

        lv_obj_t *chk = lv_event_get_target(e);
        if (!chk || !lv_obj_is_valid(chk))
        {
            INL_dbg("[INL] ERROR: Invalid checkbox object");
            return;
        }

        lv_obj_t *parent = lv_obj_get_parent(chk);
        if (!parent || !lv_obj_is_valid(parent))
        {
            INL_dbg("[INL] ERROR: Invalid parent object");
            return;
        }

        const bool enabled = lv_obj_has_state(chk, LV_STATE_CHECKED);
        const bool disabled = !enabled;

        INL_dbg("[INL] Checkbox state: %s", enabled ? "ENABLED" : "DISABLED");

        if (disabled)
        {
            // CRITICAL: Stop calibration timer FIRST before any UI changes
            if (inl.timer)
            {
                INL_dbg("[INL] Stopping calibration timer");
                lv_timer_del(inl.timer);
                inl.timer = nullptr;
            }
            // Reset FSM to idle state
            inl.ph = INL_FSM::DONE;
            g_voltINL_ready = false;
            INL_dbg("[INL] Timer stopped, FSM reset");
        }

        // Now safe to modify UI (timer is stopped if it was running)
        ensure_disabled_style_once();

        uint32_t n = lv_obj_get_child_cnt(parent);
        INL_dbg("[INL] Processing %u child objects", (unsigned)n);

        for (uint32_t i = 0; i < n; ++i)
        {
            lv_obj_t *ch = lv_obj_get_child(parent, i);
            if (!ch || ch == chk)
                continue;

            if (!lv_obj_is_valid(ch))
            {
                INL_dbg("[INL] WARNING: Skipping invalid child %u", (unsigned)i);
                continue;
            }

            if (disabled)
                lv_obj_add_flag(ch, LV_OBJ_FLAG_HIDDEN);
            else
                lv_obj_clear_flag(ch, LV_OBJ_FLAG_HIDDEN);
            lv_obj_invalidate(ch);
        }

        if (enabled)
        {
            // Re-enabling: Load calibration data
            INL_dbg("[INL] Re-enabling: loading calibration data");
            PowerSupply.LoadCalibrationData();

            try
            {
                std::vector<double> X(PowerSupply.CalBank[PowerSupply.bankCalibId].adc_inl_measure,
                                      PowerSupply.CalBank[PowerSupply.bankCalibId].adc_inl_measure + NPTS);
                std::vector<double> Y(PowerSupply.CalBank[PowerSupply.bankCalibId].adc_inl_ideal,
                                      PowerSupply.CalBank[PowerSupply.bankCalibId].adc_inl_ideal + NPTS);
                g_voltINL.setPoints(X, Y);
                g_voltINL.build();
                g_voltINL_ready = true;
                INL_dbg("[INL] Calibration data loaded successfully");
            }
            catch (...)
            {
                INL_dbg("[INL] ERROR: Failed to load calibration data");
                g_voltINL_ready = false;
            }
        }

        INL_dbg("[INL] Checkbox event complete");
    };

    // Header: checkbox + status/progress labels
    PowerSupply.gui.calibration.inl.chbx_inl_activate = lv_checkbox_create(cont);
    lv_checkbox_set_text(PowerSupply.gui.calibration.inl.chbx_inl_activate, "INL Calibration");
    lv_obj_set_pos(PowerSupply.gui.calibration.inl.chbx_inl_activate, 3, y_off);
    lv_obj_add_event_cb(PowerSupply.gui.calibration.inl.chbx_inl_activate, set_siblings_enabled_by_checkbox, LV_EVENT_SHORT_CLICKED, nullptr);
    lv_obj_add_state(PowerSupply.gui.calibration.inl.chbx_inl_activate, LV_STATE_CHECKED);

    LVLabel_class lbl_status = LVLabel_class(cont, "#FFFFF7 Status:", PowerSupply.gui.calibration.inl.chbx_inl_activate, 0, y_off, &style_lbl);
    PowerSupply.gui.calibration.inl.lbl_inl_state = new LVLabel_class(cont, "#FFFF00 Idle", lbl_status.get_lv_obj(), 0, 0, &style_lbl);
    PowerSupply.gui.calibration.inl.lbl_inl_state->align_to(LV_ALIGN_OUT_RIGHT_TOP, lbl_status, y_off * 2, 0);
    PowerSupply.gui.calibration.inl.lbl_bar_progress = new LVLabel_class(cont, "#FFFFF7 Progress: 0%#", lbl_status.get_lv_obj(), 0, y_off, &style_lbl);

    // Progress bar
    PowerSupply.gui.calibration.inl.bar_progress = lv_bar_create(cont);
    lv_obj_set_size(PowerSupply.gui.calibration.inl.bar_progress, 126, 5);
    lv_bar_set_value(PowerSupply.gui.calibration.inl.bar_progress, 0, LV_ANIM_OFF);
    lv_obj_align_to(PowerSupply.gui.calibration.inl.bar_progress, PowerSupply.gui.calibration.inl.lbl_bar_progress->get_lv_obj(), LV_ALIGN_OUT_BOTTOM_LEFT, 0, y_off);
    lv_bar_set_range(PowerSupply.gui.calibration.inl.bar_progress, 1, NPTS);

    lv_style_init(&style_radio);
    lv_style_set_radius(&style_radio, LV_RADIUS_CIRCLE);
    lv_style_init(&style_radio_chk);
    lv_style_set_bg_img_src(&style_radio_chk, NULL);

    // Measurement source radio buttons
    LVLabel_class lbl_measurement_source = LVLabel_class(cont, "#FFFFF7 Measurement Source:", PowerSupply.gui.calibration.inl.bar_progress, 0, 2 * y_off, &style_lbl);

    lv_obj_t *cont2 = lv_obj_create(cont);
    lv_obj_set_flex_flow(cont2, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_size(cont2, lv_pct(40), lv_pct(40));
    lv_obj_set_x(cont2, lv_pct(10));
    lv_obj_set_y(cont2, lv_pct(60));
    lv_obj_add_event_cb(cont2, radio_event_handler, LV_EVENT_CLICKED, &active_index_2);

    radiobutton_create(cont2, "Internal DAC", &style_lbl);
    lv_obj_t *rb_dmm = radiobutton_create(cont2, "DMM", &style_lbl);
    lv_obj_add_state(rb_dmm, LV_STATE_DISABLED);
    lv_obj_add_state(lv_obj_get_child(cont2, 0), LV_STATE_CHECKED);
    lv_obj_align_to(cont2, lbl_measurement_source.get_lv_obj(), LV_ALIGN_OUT_BOTTOM_LEFT, x_off * 2, y_off);

    lv_obj_set_style_pad_all(cont2, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_row(cont2, 5, LV_PART_MAIN);
    lv_obj_set_style_pad_column(cont2, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(cont2, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(cont2, LV_OPA_TRANSP, LV_PART_MAIN);

    PowerSupply.gui.calibration.inl.settle_time = spinbox_pro(cont, "#FFFFF7 Settle Time[s]:#", 5, 100, 3, 2, LV_ALIGN_DEFAULT, 240 + x_off, y_off, 60, 20, &graph_R_16);
    lv_obj_align(lv_obj_get_child(PowerSupply.gui.calibration.inl.settle_time, 1), LV_ALIGN_OUT_RIGHT_MID, -90, 0);
    lv_spinbox_set_value(PowerSupply.gui.calibration.inl.settle_time, 30);

    // Action button
    LVButton ADC_INL(cont, "Auto Calib", 0, 0, 126, 35, nullptr, ADC_INL_VCalib_cb);
    ADC_INL.set_align(LV_ALIGN_TOP_LEFT);
    lv_obj_align_to(ADC_INL.get_lv_obj(), rb_dmm, LV_ALIGN_OUT_BOTTOM_LEFT, -x_off * 2, 2 * y_off);

    // Table
    static lv_style_t style_stats;
    lv_style_init(&style_stats);

    table_inl = table_pro(cont, &style_stats, &graph_R_8, LV_ALIGN_DEFAULT, 154, 0 + 17 * 2, 200, 17 * 9, 0, y_off);

    lv_obj_set_style_pad_all(table_inl, 0, LV_PART_MAIN);

    lv_table_set_cell_value_fmt(table_inl, 0, 0, "#");
    lv_table_set_cell_value_fmt(table_inl, 0, 1, "Ideal");
    lv_table_set_cell_value_fmt(table_inl, 0, 2, "Measured");

    for (int i = 0; i < NPTS; ++i)
    {
        lv_table_set_cell_value_fmt(table_inl, i + 1, 0, "%02i", i);
        lv_table_set_cell_value_fmt(table_inl, i + 1, 1, "%+08.4f", TRUE_IDEAL[i]);
        lv_table_set_cell_value_fmt(table_inl, i + 1, 2, "%+09.5f", MEASURED[i]);
    }

    lv_table_set_col_width(table_inl, 0, 30);
    lv_table_set_col_width(table_inl, 1, 56);
    lv_table_set_col_width(table_inl, 2, 76);
    table_autofit_columns(table_inl);

    lv_table_set_cell_value_fmt(table_inl, 0, 3, "INL[ppmFSR]");
    lv_table_set_col_width(table_inl, 3, 86);

    lv_obj_add_event_cb(table_inl, table_draw_cell_event_cb2, LV_EVENT_DRAW_PART_BEGIN, NULL);

    lv_obj_set_style_pad_ver(cont, 0, LV_PART_ITEMS);
    lv_obj_set_style_pad_all(cont, 0, LV_PART_MAIN);
}
