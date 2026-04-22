#include "calib_full_auto.h"
#include "calib_internal_leakage.h"
#include "calib_inl.h"
#include "calib_log_window.h"
#include "setting_menu.h"
#include "device.hpp"
#include "buzzer.h"
#include "esp_task_wdt.h"
#include <Arduino.h>
#include <float.h>

extern Device PowerSupply;
extern bool g_calibration_in_progress;

// Full Auto Calibration state machine
// Polls g_calibration_in_progress and is_inl_calibration_running() to detect
// when each sub-calibration finishes, then starts the next one.

namespace {

enum class FullAutoPhase : uint8_t {
    IDLE,
    ZERO_VOLTAGE,
    WAIT_ZERO_VOLTAGE,
    ZERO_CURRENT_A,
    WAIT_ZERO_CURRENT_A,
    SWITCH_TO_MA_FOR_ZERO,
    ZERO_CURRENT_MA,
    WAIT_ZERO_CURRENT_MA,
    SWITCH_TO_A_FOR_LEAKAGE,
    LEAKAGE_A,
    WAIT_LEAKAGE_A,
    SWITCH_TO_MA,
    LEAKAGE_MA,
    WAIT_LEAKAGE_MA,
    INL,
    WAIT_INL,
    DONE
};

static FullAutoPhase phase = FullAutoPhase::IDLE;
static lv_timer_t *auto_timer = nullptr;
static int8_t saved_mA_Active = 0;

static void switch_to_range(int range)
{
    PowerSupply.mA_Active = range;
    digitalWrite(PowerSupply.AmA_Pin, PowerSupply.mA_Active);
    PowerSupply.calibrationUpdate();
}

static void full_auto_timer_cb(lv_timer_t *t)
{
    esp_task_wdt_reset();

    switch (phase) {

    case FullAutoPhase::ZERO_VOLTAGE:
        Serial.println("[FullAuto] Step 1/6: Auto Zero Voltage");
        log_set_title("1/6 Zero Voltage");
        log_reset();
        log_clear();
        // Ensure A range for voltage zero
        if (PowerSupply.mA_Active != 0)
            switch_to_range(0);
        g_calibration_in_progress = true;
        start_voltage_zero_calibration(nullptr);
        phase = FullAutoPhase::WAIT_ZERO_VOLTAGE;
        break;

    case FullAutoPhase::WAIT_ZERO_VOLTAGE:
        if (!g_calibration_in_progress) {
            phase = FullAutoPhase::ZERO_CURRENT_A;
        }
        break;

    case FullAutoPhase::ZERO_CURRENT_A:
        Serial.println("[FullAuto] Step 2/6: Auto Zero Current [A]");
        log_set_title("2/6 Zero Current [A]");
        log_reset();
        log_clear();
        if (PowerSupply.mA_Active != 0)
            switch_to_range(0);
        g_calibration_in_progress = true;
        start_current_zero_calibration(nullptr);
        phase = FullAutoPhase::WAIT_ZERO_CURRENT_A;
        break;

    case FullAutoPhase::WAIT_ZERO_CURRENT_A:
        if (!g_calibration_in_progress) {
            phase = FullAutoPhase::SWITCH_TO_MA_FOR_ZERO;
        }
        break;

    case FullAutoPhase::SWITCH_TO_MA_FOR_ZERO:
        Serial.println("[FullAuto] Switching to mA range for zero current");
        switch_to_range(1);
        phase = FullAutoPhase::ZERO_CURRENT_MA;
        break;

    case FullAutoPhase::ZERO_CURRENT_MA:
        Serial.println("[FullAuto] Step 3/6: Auto Zero Current [mA]");
        log_set_title("3/6 Zero Current [mA]");
        log_reset();
        log_clear();
        g_calibration_in_progress = true;
        start_current_zero_calibration(nullptr);
        phase = FullAutoPhase::WAIT_ZERO_CURRENT_MA;
        break;

    case FullAutoPhase::WAIT_ZERO_CURRENT_MA:
        if (!g_calibration_in_progress) {
            phase = FullAutoPhase::SWITCH_TO_A_FOR_LEAKAGE;
        }
        break;

    case FullAutoPhase::SWITCH_TO_A_FOR_LEAKAGE:
        Serial.println("[FullAuto] Switching to A range for leakage");
        switch_to_range(0);
        phase = FullAutoPhase::LEAKAGE_A;
        break;

    case FullAutoPhase::LEAKAGE_A:
        Serial.println("[FullAuto] Step 4/6: Internal Resistance [A]");
        log_set_title("4/6 Int. Resistance [A]");
        log_reset();
        log_clear();
        g_calibration_in_progress = true;
        PowerSupply.CalBank[PowerSupply.bankCalibId].internalLeakage[0] = FLT_MAX;
        start_leakage_resistance_measurement(nullptr);
        phase = FullAutoPhase::WAIT_LEAKAGE_A;
        break;

    case FullAutoPhase::WAIT_LEAKAGE_A:
        if (!g_calibration_in_progress) {
            phase = FullAutoPhase::SWITCH_TO_MA;
        }
        break;

    case FullAutoPhase::SWITCH_TO_MA:
        Serial.println("[FullAuto] Switching to mA range");
        switch_to_range(1);
        phase = FullAutoPhase::LEAKAGE_MA;
        break;

    case FullAutoPhase::LEAKAGE_MA:
        Serial.println("[FullAuto] Step 5/6: Int. Resistance [mA]");
        log_set_title("5/6 Int. Resistance [mA]");
        log_reset();
        log_clear();
        g_calibration_in_progress = true;
        PowerSupply.CalBank[PowerSupply.bankCalibId].internalLeakage[1] = FLT_MAX;
        start_leakage_resistance_measurement(nullptr);
        phase = FullAutoPhase::WAIT_LEAKAGE_MA;
        break;

    case FullAutoPhase::WAIT_LEAKAGE_MA:
        if (!g_calibration_in_progress) {
            // Switch back to A for INL
            switch_to_range(0);
            phase = FullAutoPhase::INL;
        }
        break;

    case FullAutoPhase::INL:
        Serial.println("[FullAuto] Step 6/6: ADC INL Calibration");
        // Open the INL calibration window (shows table, progress bar, etc.)
        ADC_INL_Voltage_calibration_cb(nullptr);
        start_inl_calibration();
        phase = FullAutoPhase::WAIT_INL;
        break;

    case FullAutoPhase::WAIT_INL:
        if (!is_inl_calibration_running()) {
            // Hide the INL window now that calibration is done
            if (PowerSupply.gui.calibration.win_ADC_INL_Voltage_calibration &&
                lv_obj_is_valid(PowerSupply.gui.calibration.win_ADC_INL_Voltage_calibration))
            {
                lv_obj_add_flag(PowerSupply.gui.calibration.win_ADC_INL_Voltage_calibration, LV_OBJ_FLAG_HIDDEN);
            }
            phase = FullAutoPhase::DONE;
        }
        break;

    case FullAutoPhase::DONE: {
        Serial.println("[FullAuto] Complete!");
        log_set_title("Calibration Complete");
        log_reset();
        log_clear();
        log_step("All 6 steps completed successfully.");

        // Restore original mA/A range
        if (PowerSupply.mA_Active != saved_mA_Active) {
            switch_to_range(saved_mA_Active);
            Serial.printf("[FullAuto] Restored %s range\n", saved_mA_Active ? "mA" : "A");
        }

        // Save all calibration data
        PowerSupply.SaveCalibrationData();

        // Set voltage to 0V
        PowerSupply.Voltage.SetUpdate(0.0 * PowerSupply.Voltage.adjFactor + PowerSupply.Voltage.adjOffset);

        myTone(NOTE_A5, 200, true);
        myTone(NOTE_A4, 200, true);

        g_calibration_in_progress = false;
        g_log_window_keep_open = false;
        phase = FullAutoPhase::IDLE;

        // Schedule log window close
        lv_timer_t *close_t = lv_timer_create(close_log_cb, 6000, nullptr);
        lv_timer_set_repeat_count(close_t, 1);

        // Stop this timer
        lv_timer_del(t);
        auto_timer = nullptr;
        break;
    }

    case FullAutoPhase::IDLE:
    default:
        break;
    }
}

static void full_auto_msgbox_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_VALUE_CHANGED)
        return;

    lv_obj_t *obj = lv_event_get_current_target(e);
    const char *btn_txt = lv_msgbox_get_active_btn_text(obj);
    bool is_ok = (btn_txt && strcmp(btn_txt, "OK") == 0);

    msgbox_close_deferred(obj);

    if (!is_ok)
        return;

    if (g_calibration_in_progress) {
        Serial.println("[FullAuto] Calibration already in progress");
        return;
    }

    // Validate
    if (PowerSupply.CalBank.empty() || PowerSupply.bankCalibId < 0 ||
        PowerSupply.bankCalibId >= (int8_t)PowerSupply.CalBank.size())
    {
        Serial.println("[FullAuto] ERROR: Invalid calibration bank");
        return;
    }

    Serial.println("[FullAuto] Starting full auto calibration sequence");

    g_calibration_in_progress = true;
    g_log_window_keep_open = true;
    saved_mA_Active = PowerSupply.mA_Active;

    // Create log window
    create_log_window("Full Auto Calibration");
    log_reset();
    log_clear();

    // Start the state machine
    phase = FullAutoPhase::ZERO_VOLTAGE;
    auto_timer = lv_timer_create(full_auto_timer_cb, 200, nullptr);
}

} // namespace

void full_auto_calibration_cb(lv_event_t *)
{
    if (!calib_check_current_setpoint(10.0f)) return;
    Warning_msgbox("Full Auto Calibration", full_auto_msgbox_cb);
}
