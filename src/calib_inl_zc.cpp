// Zero-current INL calibration — commented out pending investigation
#if 0
#include "calib_inl_zc.h"
#include "calib_inl.h"         // for INL_dbg, Warning_msgbox
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

extern Device PowerSupply;

// =============================================================================
// Globals
// =============================================================================

MonotoneCubicCalibrator g_voltINL_ZC;
bool g_voltINL_ZC_ready = false;

static lv_obj_t *table_inl_zc = nullptr;

// Same voltage table as regular INL

static const double ZC_TRUE_IDEAL[] = {-.0045, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
    13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 32.5};
static const double ZC_MEASURED[] = {-.0045, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
    13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 32.5};
constexpr size_t ZC_NPTS = sizeof(ZC_TRUE_IDEAL) / sizeof(ZC_TRUE_IDEAL[0]);

// =============================================================================
// FSM
// =============================================================================

struct INLZC_FSM
{
    static constexpr int NPTS = ZC_NPTS;
    double x_raw[ZC_NPTS];
    double y_true[ZC_NPTS];
    int i;
    enum Phase : uint8_t { PREPARE, SET, SETTLE, MEASURE, COMPUTE, DONE } ph;
    uint32_t t0;
    lv_timer_t *timer;
    uint16_t saved_V_adjValue;
    uint16_t saved_I_adjValue;
} static zc;

static inline uint32_t now_ms()    { return lv_tick_get(); }
static inline uint32_t since(uint32_t t) { return lv_tick_elaps(t); }

// =============================================================================
// GUI helpers
// =============================================================================

static void zc_gui_prepare()
{
    zc.saved_V_adjValue = PowerSupply.Voltage.adjValue;
    zc.saved_I_adjValue = PowerSupply.Current.adjValue;

    // Force voltage to 0V to start
    PowerSupply.Voltage.SetUpdate(0.0 * PowerSupply.Voltage.adjFactor + PowerSupply.Voltage.adjOffset);
    // Force current to 0 (no load) for both A and mA ranges
    PowerSupply.Current.SetUpdate(0.0 * PowerSupply.Current.adjFactor + PowerSupply.Current.adjOffset);

    if (PowerSupply.gui.calibration.win_ADC_INL_ZC_calibration &&
        !lv_obj_has_flag(PowerSupply.gui.calibration.win_ADC_INL_ZC_calibration, LV_OBJ_FLAG_HIDDEN))
    {
        PowerSupply.gui.calibration.inl_zc.lbl_inl_state->set_text("#FFFF00 Running");
    }
}

static void zc_gui_set(double v_cmd)
{
    PowerSupply.Voltage.SetUpdate(v_cmd * PowerSupply.Voltage.adjFactor + PowerSupply.Voltage.adjOffset);
    // Keep current at zero on every step
    PowerSupply.Current.SetUpdate(0.0 * PowerSupply.Current.adjFactor + PowerSupply.Current.adjOffset);
    PowerSupply.Voltage.measured.ResetStats();

    if (PowerSupply.gui.calibration.win_ADC_INL_ZC_calibration &&
        !lv_obj_has_flag(PowerSupply.gui.calibration.win_ADC_INL_ZC_calibration, LV_OBJ_FLAG_HIDDEN))
    {
        table_set_selected_row(table_inl_zc, zc.i + 1);
        lv_bar_set_value(PowerSupply.gui.calibration.inl_zc.bar_progress, zc.i, LV_ANIM_OFF);
        PowerSupply.gui.calibration.inl_zc.lbl_bar_progress->set_text_fmt(
            "Progress: %i%%", zc.i * 100 / INLZC_FSM::NPTS);
    }
}

static void zc_gui_measure(double measure, double v_cmd)
{
    if (table_inl_zc && lv_obj_is_valid(table_inl_zc))
    {
        lv_table_set_cell_value_fmt(table_inl_zc, zc.i + 1, 1, "%+08.4f", zc.y_true[zc.i]);
        lv_table_set_cell_value_fmt(table_inl_zc, zc.i + 1, 2, "%+09.5f", measure);
        lv_table_set_cell_value_fmt(table_inl_zc, zc.i + 1, 3, "%+02.2f",
            (measure - v_cmd) * 30518.043793393);
    }
}

static void zc_gui_compute()
{
    // Restore voltage and current to saved values
    PowerSupply.Voltage.SetUpdate(zc.saved_V_adjValue);
    PowerSupply.Current.SetUpdate(zc.saved_I_adjValue);

    if (PowerSupply.gui.calibration.win_ADC_INL_ZC_calibration &&
        !lv_obj_has_flag(PowerSupply.gui.calibration.win_ADC_INL_ZC_calibration, LV_OBJ_FLAG_HIDDEN))
    {
        PowerSupply.gui.calibration.inl_zc.lbl_inl_state->set_text("#FFFF00 Done");
        PowerSupply.gui.calibration.inl_zc.lbl_bar_progress->set_text_fmt("Progress: 100%%");
        lv_bar_set_value(PowerSupply.gui.calibration.inl_zc.bar_progress, ZC_NPTS, LV_ANIM_OFF);
        table_set_selected_row(table_inl_zc, 1);
    }
}

// =============================================================================
// FSM timer callback
// =============================================================================

static void INLZC_timer_cb(lv_timer_t *)
{
    switch (zc.ph)
    {
    case INLZC_FSM::PREPARE:
        for (int k = 0; k < INLZC_FSM::NPTS; ++k) { zc.x_raw[k] = 0; zc.y_true[k] = 0; }
        zc.i = 0;
        INL_dbg("[INLZC] PREPARE: %d pts, settle 3000ms", INLZC_FSM::NPTS);
        zc_gui_prepare();
        zc.ph = INLZC_FSM::SET;
        break;

    case INLZC_FSM::SET:
    {
        double v_cmd = ZC_TRUE_IDEAL[zc.i];
        INL_dbg("[INLZC] SET i=%d v=%.3f", zc.i, v_cmd);
        zc_gui_set(v_cmd);
        zc.y_true[zc.i] = v_cmd;
        zc.t0 = now_ms();
        zc.ph = INLZC_FSM::SETTLE;
    }
    break;

    case INLZC_FSM::SETTLE:
    {
        uint32_t settle_val = (PowerSupply.gui.calibration.inl_zc.settle_time &&
                               lv_obj_is_valid(PowerSupply.gui.calibration.inl_zc.settle_time))
                              ? lv_spinbox_get_value(PowerSupply.gui.calibration.inl_zc.settle_time)
                              : 30;
        if (since(zc.t0) >= (uint32_t)100 * settle_val)
        {
            INL_dbg("[INLZC] SETTLE +%u ms", unsigned(since(zc.t0)));
            zc.ph = INLZC_FSM::MEASURE;
        }
    }
    break;

    case INLZC_FSM::MEASURE:
    {
        double measure = PowerSupply.Voltage.measured.Mean();
        zc.x_raw[zc.i] = measure;
        INL_dbg("[INLZC] MEASURE i=%d raw=%.6f cmd=%.6f", zc.i, measure, zc.y_true[zc.i]);
        zc_gui_measure(measure, zc.y_true[zc.i]);
        if (++zc.i >= INLZC_FSM::NPTS)
            zc.ph = INLZC_FSM::COMPUTE;
        else
            zc.ph = INLZC_FSM::SET;
    }
    break;

    case INLZC_FSM::COMPUTE:
    {
        INL_dbg("[INLZC] COMPUTE begin");

        if (PowerSupply.CalBank.empty() || PowerSupply.bankCalibId < 0 ||
            PowerSupply.bankCalibId >= (int8_t)PowerSupply.CalBank.size())
        {
            INL_dbg("[INLZC] ERROR: Invalid bankCalibId=%d", PowerSupply.bankCalibId);
            g_voltINL_ZC_ready = false;
            zc.ph = INLZC_FSM::DONE;
            break;
        }

        std::vector<double> X(zc.x_raw, zc.x_raw + INLZC_FSM::NPTS);
        std::vector<double> Y(ZC_TRUE_IDEAL, ZC_TRUE_IDEAL + ZC_NPTS);
        g_voltINL_ZC.setPoints(X, Y);
        g_voltINL_ZC.build();
        g_voltINL_ZC_ready = true;

        for (int i = 0; i < (int)ZC_NPTS; i++)
        {
            PowerSupply.CalBank[PowerSupply.bankCalibId].adc_inl_zc_measure[i] = zc.x_raw[i];
            PowerSupply.CalBank[PowerSupply.bankCalibId].adc_inl_zc_ideal[i]   = zc.y_true[i];
        }

        INL_dbg("[INLZC] COMPUTE done");
        zc_gui_compute();
        PowerSupply.SaveCalibrationData();
        zc.ph = INLZC_FSM::DONE;
    }
    break;

    case INLZC_FSM::DONE:
        INL_dbg("[INLZC] DONE");
        if (zc.timer)
        {
            lv_timer_del(zc.timer);
            zc.timer = nullptr;
        }
        break;
    }
}

// =============================================================================
// Start callback (from warning msgbox)
// =============================================================================

static void INLZC_start(lv_event_t *e)
{
    if (!e) return;

    lv_obj_t *obj = lv_event_get_current_target(e);
    if (!obj || !lv_obj_is_valid(obj)) return;

    if (lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED)
    {
        const char *txt = lv_msgbox_get_active_btn_text(obj);
        char btn_text[16] = {0};
        if (txt) strncpy(btn_text, txt, sizeof(btn_text) - 1);
        msgbox_close_deferred(obj);

        if (strcmp(btn_text, "OK") == 0)
        {
            INL_dbg("[INLZC] Starting ZC calibration");
            g_voltINL_ZC_ready = false;
            if (zc.timer) { lv_timer_del(zc.timer); zc.timer = nullptr; }
            zc = INLZC_FSM{};
            zc.ph = INLZC_FSM::PREPARE;
            zc.timer = lv_timer_create(INLZC_timer_cb, 10, nullptr);
        }
        else
        {
            INL_dbg("[INLZC] Cancelled");
        }
    }
}

static void INLZC_start_btn_cb(lv_event_t *e)
{
    Warning_msgbox("ADC INL Zero-Current Calibration", INLZC_start);
}

// =============================================================================
// Window close handler
// =============================================================================

static void inl_zc_close_window_cb(lv_event_t *e)
{
    if (zc.timer)
    {
        INL_dbg("[INLZC] Window closed - stopping timer, restoring output");
        // Restore current before stopping
        PowerSupply.Voltage.SetUpdate(zc.saved_V_adjValue);
        PowerSupply.Current.SetUpdate(zc.saved_I_adjValue);
        lv_timer_del(zc.timer);
        zc.timer = nullptr;
    }
    zc.ph = INLZC_FSM::DONE;

    auto *btn = lv_event_get_target(e);
    lv_obj_add_flag(lv_obj_get_parent(lv_obj_get_parent(btn)), LV_OBJ_FLAG_HIDDEN);
    lv_obj_invalidate(lv_scr_act());
}

// =============================================================================
// Calibration window
// =============================================================================

void ADC_INL_ZC_calibration_cb(lv_event_t *)
{
    if (PowerSupply.gui.calibration.win_ADC_INL_ZC_calibration)
    {
        lv_obj_clear_flag(PowerSupply.gui.calibration.win_ADC_INL_ZC_calibration, LV_OBJ_FLAG_HIDDEN);
        return;
    }

    uint8_t y_off = 5, x_off = 5;

    PowerSupply.gui.calibration.win_ADC_INL_ZC_calibration = lv_win_create(lv_scr_act(), 36);
    lv_obj_set_size(PowerSupply.gui.calibration.win_ADC_INL_ZC_calibration, 320, 226);
    lv_win_add_title(PowerSupply.gui.calibration.win_ADC_INL_ZC_calibration,
                     "ADC INL Zero-Current Calibration");

    lv_obj_t *btn_close = lv_win_add_btn(
        PowerSupply.gui.calibration.win_ADC_INL_ZC_calibration, LV_SYMBOL_CLOSE, 60);
    lv_obj_add_event_cb(btn_close, inl_zc_close_window_cb, LV_EVENT_CLICKED, nullptr);

    lv_obj_t *cont = lv_win_get_content(PowerSupply.gui.calibration.win_ADC_INL_ZC_calibration);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(cont, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(cont, 0, LV_PART_ITEMS);

    PowerSupply.LoadCalibrationData();

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

    // Status / progress row
    LVLabel_class lbl_status = LVLabel_class(cont, "#FFFFF7 Status:", nullptr, 3, y_off, &style_lbl);
    PowerSupply.gui.calibration.inl_zc.lbl_inl_state =
        new LVLabel_class(cont, "#FFFF00 Idle", lbl_status.get_lv_obj(), 0, 0, &style_lbl);
    PowerSupply.gui.calibration.inl_zc.lbl_inl_state->align_to(
        LV_ALIGN_OUT_RIGHT_TOP, lbl_status, y_off * 2, 0);

    PowerSupply.gui.calibration.inl_zc.lbl_bar_progress =
        new LVLabel_class(cont, "#FFFFF7 Progress: 0%#", lbl_status.get_lv_obj(), 0, y_off, &style_lbl);

    PowerSupply.gui.calibration.inl_zc.bar_progress = lv_bar_create(cont);
    lv_obj_set_size(PowerSupply.gui.calibration.inl_zc.bar_progress, 126, 5);
    lv_bar_set_value(PowerSupply.gui.calibration.inl_zc.bar_progress, 0, LV_ANIM_OFF);
    lv_obj_align_to(PowerSupply.gui.calibration.inl_zc.bar_progress,
        PowerSupply.gui.calibration.inl_zc.lbl_bar_progress->get_lv_obj(),
        LV_ALIGN_OUT_BOTTOM_LEFT, 0, y_off);
    lv_bar_set_range(PowerSupply.gui.calibration.inl_zc.bar_progress, 1, (int)ZC_NPTS);

    // Info label
    LVLabel_class lbl_info = LVLabel_class(cont, "#FFAA00 I forced=0 at each step#",
        PowerSupply.gui.calibration.inl_zc.bar_progress, 0, y_off * 2, &style_lbl);

    // Settle time spinbox
    PowerSupply.gui.calibration.inl_zc.settle_time =
        spinbox_pro(cont, "#FFFFF7 Settle[s]:#", 5, 100, 3, 2, LV_ALIGN_DEFAULT,
                    240 + x_off, y_off, 60, 20, &graph_R_16);
    lv_obj_align(lv_obj_get_child(PowerSupply.gui.calibration.inl_zc.settle_time, 1),
        LV_ALIGN_OUT_RIGHT_MID, -90, 0);
    lv_spinbox_set_value(PowerSupply.gui.calibration.inl_zc.settle_time, 30);

    // Auto-calibrate button
    LVButton btn_zc(cont, "Auto Calib ZC", 0, 0, 140, 35, nullptr, INLZC_start_btn_cb);
    btn_zc.set_align(LV_ALIGN_TOP_LEFT);
    lv_obj_align_to(btn_zc.get_lv_obj(), lbl_info.get_lv_obj(),
        LV_ALIGN_OUT_BOTTOM_LEFT, 0, 2 * y_off);

    // Table
    static lv_style_t style_stats;
    lv_style_init(&style_stats);
    table_inl_zc = table_pro(cont, &style_stats, &graph_R_8, LV_ALIGN_DEFAULT,
        154, 17 * 2, 200, 17 * 9, 0, y_off);
    lv_obj_set_style_pad_all(table_inl_zc, 0, LV_PART_MAIN);

    lv_table_set_cell_value_fmt(table_inl_zc, 0, 0, "#");
    lv_table_set_cell_value_fmt(table_inl_zc, 0, 1, "Ideal");
    lv_table_set_cell_value_fmt(table_inl_zc, 0, 2, "Measured");
    lv_table_set_cell_value_fmt(table_inl_zc, 0, 3, "INL[ppmFSR]");

    for (int i = 0; i < (int)ZC_NPTS; ++i)
    {
        lv_table_set_cell_value_fmt(table_inl_zc, i + 1, 0, "%02i", i);
        lv_table_set_cell_value_fmt(table_inl_zc, i + 1, 1, "%+08.4f", ZC_TRUE_IDEAL[i]);
        lv_table_set_cell_value_fmt(table_inl_zc, i + 1, 2, "%+09.5f", ZC_MEASURED[i]);
        lv_table_set_cell_value_fmt(table_inl_zc, i + 1, 3, "0.00");
    }

    lv_table_set_col_width(table_inl_zc, 0, 30);
    lv_table_set_col_width(table_inl_zc, 1, 56);
    lv_table_set_col_width(table_inl_zc, 2, 76);
    lv_table_set_col_width(table_inl_zc, 3, 86);
    table_autofit_columns(table_inl_zc);

    lv_obj_add_event_cb(table_inl_zc, table_draw_cell_event_cb2, LV_EVENT_DRAW_PART_BEGIN, NULL);
    lv_obj_set_style_pad_ver(cont, 0, LV_PART_ITEMS);
    lv_obj_set_style_pad_all(cont, 0, LV_PART_MAIN);
}

// =============================================================================
// Rebuild from saved data (called at startup)
// =============================================================================

void rebuildINLZCFromCalibration()
{
    INL_dbg("[INLZC] Checking saved ZC calibration data...");

    if (PowerSupply.CalBank.empty() || PowerSupply.bankCalibId < 0 ||
        PowerSupply.bankCalibId >= (int8_t)PowerSupply.CalBank.size())
    {
        INL_dbg("[INLZC] ERROR: Invalid bankCalibId=%d", PowerSupply.bankCalibId);
        g_voltINL_ZC_ready = false;
        return;
    }

    const double *measure = PowerSupply.CalBank[PowerSupply.bankCalibId].adc_inl_zc_measure;
    const double *ideal   = PowerSupply.CalBank[PowerSupply.bankCalibId].adc_inl_zc_ideal;

    double sum_diff = 0.0;
    for (size_t i = 0; i < ZC_NPTS; i++)
        sum_diff += fabs(measure[i] - ideal[i]);

    if (sum_diff <= 0.001)
    {
        INL_dbg("[INLZC] No valid ZC INL calibration data (identity or zeros)");
        g_voltINL_ZC_ready = false;
        return;
    }

    std::vector<double> X(measure, measure + ZC_NPTS);
    std::vector<double> Y(ideal,   ideal   + ZC_NPTS);
    g_voltINL_ZC.setPoints(X, Y);
    g_voltINL_ZC.build();
    g_voltINL_ZC_ready = true;

    INL_dbg("[INLZC] ZC interpolator rebuilt (ready=1)");
}
#endif // Zero-current INL — pending investigation
