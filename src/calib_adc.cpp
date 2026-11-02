#include "calib_adc.h"
#include "calib_internal_leakage.h"
#include "calib_log_window.h"
#include "setting_menu.h"
#include "device.hpp"
#include "spinbox_pro.h"
#include "lv_gui_helper.h"
#include "myFonts.h"
#include "buzzer.h"
#include "esp_task_wdt.h"
#include "spi_quiet.h"
#include <cmath>
#include <cstring>

// External references
extern Device PowerSupply;
extern CalibrationGui Calib_GUI;

// Global flag to prevent concurrent calibrations (defined here, declared in header)
bool g_calibration_in_progress = false;

namespace
{
    int s_lowNoiseDepth = 0;
    uint8_t s_savedAdcRate = 0;
    bool s_savedPgaX4 = false;

    void setAdcRate(uint8_t v)
    {
        static const char *labels[] = {" ~10", " ~20", " ~70", "~200", "~500"};
        const int rates[] = {20, 20, 90, 330, 1000};
        if (v > 4) return;
        PowerSupply.settingParameters.adcRate = v;
        PowerSupply.adc.ads1219->setDataRate(rates[v]);
        PowerSupply.adjustAdcTaskPriority();
        PowerSupply.ResetStats();
        // slider_adcRate is the container: child 1 = value label, child 2 = slider
        if (lv_obj_t *c = PowerSupply.gui.slider_adcRate)
        {
            if (lv_obj_t *sl = lv_obj_get_child(c, 2)) lv_slider_set_value(sl, v, LV_ANIM_OFF);
            if (lv_obj_t *lb = lv_obj_get_child(c, 1)) lv_label_set_text(lb, labels[v]);
        }
    }
}

bool calib_low_noise_active() { return s_lowNoiseDepth > 0; }

void calib_low_noise_begin()
{
    if (s_lowNoiseDepth++ > 0) return;
    s_savedAdcRate = PowerSupply.settingParameters.adcRate;
    s_savedPgaX4 = PowerSupply.iPgaX4;
    PowerSupply.iPgaX4 = false; // raw codes must be x1 codes (see Device::readCurrent)
    SpiQuiet::pauseBlocked.store(true);
    PowerSupply.Voltage.setCeiling = INFINITY; // no OVP ceiling (see enforceSoftwareLimits())
    if (s_savedAdcRate != 0) setAdcRate(0);
    Serial.printf("\n[Calib] low-noise: ADC rate 0 (was %d), PGA x1", s_savedAdcRate);
}

void calib_low_noise_end()
{
    if (s_lowNoiseDepth == 0 || --s_lowNoiseDepth > 0) return;
    SpiQuiet::pauseBlocked.store(false);
    PowerSupply.iPgaX4 = s_savedPgaX4;
    if (s_savedAdcRate != 0) setAdcRate(s_savedAdcRate);
    Serial.printf("\n[Calib] restored ADC rate %d", s_savedAdcRate);
}

namespace
{
    // Forward declaration
    static void auto_zero_event_cb(lv_event_t *e);

    // Callbacks for save/load buttons in ADC calibration window
    static void save_cb(lv_event_t *)
    {
        PowerSupply.SaveCalibrationData();
        myTone(NOTE_A4, 200);
    }

    static void load_cb(lv_event_t *)
    {
        PowerSupply.LoadCalibrationData();
        myTone(NOTE_A4, 200);

        // Update voltage calibration spinboxes if window is visible
        if (PowerSupply.gui.calibration.win_ADC_voltage_calibration &&
            !lv_obj_has_flag(PowerSupply.gui.calibration.win_ADC_voltage_calibration, LV_OBJ_FLAG_HIDDEN))
        {
            auto &v = PowerSupply.CalBank[PowerSupply.bankCalibId].vCal;
            if (Calib_GUI.Voltage.code_1) { lv_spinbox_set_value(Calib_GUI.Voltage.code_1, v.code_1); lv_obj_invalidate(Calib_GUI.Voltage.code_1); }
            if (Calib_GUI.Voltage.code_2) { lv_spinbox_set_value(Calib_GUI.Voltage.code_2, v.code_2); lv_obj_invalidate(Calib_GUI.Voltage.code_2); }
            if (Calib_GUI.Voltage.vin_1) { lv_spinbox_set_value(Calib_GUI.Voltage.vin_1, (int32_t)llround(10000.0 * v.value_1)); lv_obj_invalidate(Calib_GUI.Voltage.vin_1); }
            if (Calib_GUI.Voltage.vin_2) { lv_spinbox_set_value(Calib_GUI.Voltage.vin_2, (int32_t)llround(10000.0 * v.value_2)); lv_obj_invalidate(Calib_GUI.Voltage.vin_2); }
        }

        // Update current calibration spinboxes if window is visible
        if (PowerSupply.gui.calibration.win_ADC_current_calibration &&
            !lv_obj_has_flag(PowerSupply.gui.calibration.win_ADC_current_calibration, LV_OBJ_FLAG_HIDDEN))
        {
            auto &i = PowerSupply.CalBank[PowerSupply.bankCalibId].iCal;
            if (Calib_GUI.Current.code_1) { lv_spinbox_set_value(Calib_GUI.Current.code_1, i[PowerSupply.mA_Active].code_1); lv_obj_invalidate(Calib_GUI.Current.code_1); }
            if (Calib_GUI.Current.code_2) { lv_spinbox_set_value(Calib_GUI.Current.code_2, i[PowerSupply.mA_Active].code_2); lv_obj_invalidate(Calib_GUI.Current.code_2); }
            if (Calib_GUI.Current.vin_1) { lv_spinbox_set_value(Calib_GUI.Current.vin_1, (int32_t)llround(10000.0 * i[PowerSupply.mA_Active].value_1)); lv_obj_invalidate(Calib_GUI.Current.vin_1); }
            if (Calib_GUI.Current.vin_2) { lv_spinbox_set_value(Calib_GUI.Current.vin_2, (int32_t)llround(10000.0 * i[PowerSupply.mA_Active].value_2)); lv_obj_invalidate(Calib_GUI.Current.vin_2); }
        }
    }

    // Context markers for auto_zero_event_cb to know which calibration is active
    static enum class AutoZeroContext { FOR_CURRENT, FOR_VOLTAGE } g_autozero_context = AutoZeroContext::FOR_CURRENT;

    static void autoZeroCurrent_cb(lv_event_t *)
    {
        if (!calib_check_current_setpoint(10.0f)) return;  // needs real load to calibrate current ADC
        if (!calib_check_output_on()) return;               // zero is measured at 1 V
        g_autozero_context = AutoZeroContext::FOR_CURRENT;
        myTone(NOTE_A4, 200);
        Warning_msgbox("Auto Zero Current", auto_zero_event_cb);
    }

    static void autoZeroVoltage_cb(lv_event_t *)
    {
        if (!calib_check_current_setpoint(10.0f)) return;
        g_autozero_context = AutoZeroContext::FOR_VOLTAGE;
        myTone(NOTE_A4, 200);
        Warning_msgbox("Auto Zero Voltage", auto_zero_event_cb);
    }

    // Open the log window and start the selected auto-zero sequence
    static void run_auto_zero()
    {
        // Reset watchdog before long operation
        esp_task_wdt_reset();

        create_log_window("Auto Zero Calibration");
        log_reset();
        log_clear();

        // Call the appropriate calibration function based on context
        if (g_autozero_context == AutoZeroContext::FOR_VOLTAGE) {
            Serial.println("Starting voltage auto-zero calibration");
            start_voltage_zero_calibration(nullptr);
        } else {
            Serial.println("Starting current auto-zero calibration");
            start_current_zero_calibration(nullptr);
        }

        Serial.println("Auto-zero calibration started");
    }

    static void auto_zero_event_cb(lv_event_t *e)
    {
        if (lv_event_get_code(e) != LV_EVENT_VALUE_CHANGED)
            return;

        lv_obj_t *obj = lv_event_get_current_target(e);
        if (!obj) return;

        // Prevent multiple triggers - check global flag
        if (g_calibration_in_progress) {
            Serial.println("Auto-zero: Calibration already in progress, ignoring");
            return;
        }

        // Get button text to check which button was pressed
        const char *btn_txt = lv_msgbox_get_active_btn_text(obj);

        Serial.printf("Auto-zero: button pressed = '%s'\n", btn_txt ? btn_txt : "NULL");

        // Check if OK button was pressed (compare text, not index)
        bool is_ok = (btn_txt && strcmp(btn_txt, "OK") == 0);

        // Close message box
        msgbox_close_deferred(obj);

        if (!is_ok) {
            Serial.println("Auto-zero: Not OK, returning");
            return;
        }

        Serial.println("Auto-zero OK pressed - starting calibration");

        // Check system resources first
        size_t free_heap = heap_caps_get_free_size(MALLOC_CAP_8BIT);
        size_t largest_block = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);

        Serial.printf("Auto-zero: Free heap: %d, Largest block: %d\n",
                     free_heap, largest_block);

        if (free_heap < 30000 || largest_block < 10000) {
            Serial.println("ERROR: System resources too low for calibration");
            Message_msgbox("Calibration Error", "Not enough free memory.");
            return;
        }

        // Set global flag - will be cleared by calibration completion
        g_calibration_in_progress = true;

        if (g_autozero_context == AutoZeroContext::FOR_CURRENT)
        {
            // The zero current is measured at 1 V, not 0 V: near 0 V pulses
            // from the pre-tracking SMPS get through to the shunt. The leakage V/R
            // at the measured voltage is subtracted (calib_internal_leakage.cpp).
            // Output is on (checked at the button). V-Set below 0.5 V: go to 1 V. Otherwise ask.
            const double vset = (PowerSupply.Voltage.adjValue - PowerSupply.Voltage.adjOffset) / PowerSupply.Voltage.adjFactor;
            if (vset < 0.5)
            {
                current_zero_set_voltage(true, "Setting voltage to 1V");
                run_auto_zero();
                return;
            }
            static const char *btns[] = {"Yes", "No", ""};
            lv_obj_t *mb = lv_msgbox_create(NULL, "Auto Zero Current", "Set V to 1 V for the measurement?", btns, false);
            lv_obj_center(mb);
            lv_obj_add_event_cb(mb, [](lv_event_t *e)
            {
                lv_obj_t *mb = lv_event_get_current_target(e);
                const char *t = lv_msgbox_get_active_btn_text(mb);
                if (!t)
                    return;
                const bool yes = strcmp(t, "Yes") == 0;
                msgbox_close_deferred(mb);
                current_zero_set_voltage(yes, yes ? "Setting voltage to 1V" : "Voltage left at V-Set");
                run_auto_zero();
            }, LV_EVENT_VALUE_CHANGED, nullptr);
            return;
        }

        run_auto_zero();
    }
} // end anonymous namespace

// Remove the duplicate declaration - use the one from calib_adc.h
void build_adc_calibration_window(lv_obj_t **win_holder,
                                   const char *title,
                                   AdcCalibrationControls &gui,
                                   const CalPrefill &pf,
                                   lv_event_cb_t autozero_cb)
{
    if (*win_holder)
    {
        lv_spinbox_set_value(gui.code_1, pf.code1);
        lv_spinbox_set_value(gui.code_2, pf.code2);
        lv_spinbox_set_value(gui.vin_1, (int32_t)llround(10000.0 * pf.val1));
        lv_spinbox_set_value(gui.vin_2, (int32_t)llround(10000.0 * pf.val2));

        // Force visual refresh of all spinboxes (lv_spinbox_set_value may skip
        // redraw when the textarea label area isn't fully invalidated)
        lv_obj_invalidate(gui.code_1);
        lv_obj_invalidate(gui.code_2);
        lv_obj_invalidate(gui.vin_1);
        lv_obj_invalidate(gui.vin_2);

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

    lv_point_t offset{10, 14};
    lv_coord_t w = 110, col = w + 30, row = 36;
    lv_point_t pad{5, 4};

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

    lv_point_t btn_pos{140 + offset.x, offset.y + 125};
    LVButton btnSave(cont, "Save", btn_pos.x + 62, btn_pos.y, 54, 25, nullptr, save_cb);
    LVButton btnLoad(cont, "Load", btn_pos.x, btn_pos.y, 54, 25, nullptr, load_cb);

    // Show "Auto Zero" button with context-specific callback
    if (autozero_cb)
    {
        LVButton btnAutoZeros(cont, "Auto Zero", btn_pos.x , btn_pos.y + 28, 2 * 54+62-54, 25, nullptr, autozero_cb);
    }

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

// Button callbacks
void btn_calibration_ADC_voltage_event_cb(lv_event_t *)
{
    auto &cal = PowerSupply.CalBank[PowerSupply.bankCalibId].vCal;
    CalPrefill pf{cal.code_1, cal.code_2, cal.value_1, cal.value_2, "V"};
    build_adc_calibration_window(&PowerSupply.gui.calibration.win_ADC_voltage_calibration, "ADC Voltage Calibration", Calib_GUI.Voltage, pf, autoZeroVoltage_cb);
}

void btn_calibration_ADC_current_event_cb(lv_event_t *)
{
    auto &cal = PowerSupply.CalBank[PowerSupply.bankCalibId].iCal;
    CalPrefill pf{cal[PowerSupply.mA_Active].code_1, cal[PowerSupply.mA_Active].code_2, cal[PowerSupply.mA_Active].value_1, cal[PowerSupply.mA_Active].value_2, "A"};
    build_adc_calibration_window(&PowerSupply.gui.calibration.win_ADC_current_calibration, "ADC Current Calibration [A]", Calib_GUI.Current, pf, autoZeroCurrent_cb);
}