#include "calib_internal_leakage.h"
#include "calib_log_window.h"
#include "calib_sequencer.h"
#include "calib_adc.h"  // For g_calibration_in_progress
#include "device.hpp"
#include "setting_menu.h"
#include "esp_task_wdt.h"
#include <Arduino.h>
#include <float.h>

// External references
extern Device PowerSupply;
extern CalibrationGui Calib_GUI;
extern bool blockAll;

// Global storage for leakage resistance measurement (avoids lambda capture issues with static arrays)
static double g_leakage_i_at_0v = 0;
static double g_leakage_i_at_32v = 0;

void start_leakage_resistance_measurement(lv_event_t *)
{
    // Reset globals before starting
    g_leakage_i_at_0v = 0;
    g_leakage_i_at_32v = 0;

    // CRITICAL: Validate array indices before proceeding
    if (PowerSupply.CalBank.empty() ||
        PowerSupply.bankCalibId < 0 ||
        PowerSupply.bankCalibId >= (int8_t)PowerSupply.CalBank.size())
    {
        Serial.printf("\nERROR: Invalid bankCalibId=%d, CalBank.size()=%d",
                     PowerSupply.bankCalibId, PowerSupply.CalBank.size());
        g_calibration_in_progress = false;
        Warning_msgbox("Error: Invalid calibration bank", [](lv_event_t*){});
        return;
    }

    if (PowerSupply.mA_Active < 0 || PowerSupply.mA_Active > 1)
    {
        Serial.printf("\nERROR: Invalid mA_Active=%d (must be 0 or 1)", PowerSupply.mA_Active);
        g_calibration_in_progress = false;
        Warning_msgbox("Error: Invalid current range", [](lv_event_t*){});
        return;
    }

    Serial.printf("\nStarting leakage measurement: bankCalibId=%d, mA_Active=%d, CalBank.size()=%d",
                 PowerSupply.bankCalibId, PowerSupply.mA_Active, PowerSupply.CalBank.size());

    esp_task_wdt_reset();
    static const SeqStep steps[] = {
        {"Setting voltage to 0V", 1500, 500,
         []()
         { PowerSupply.Voltage.SetUpdate(0.0 * PowerSupply.Voltage.adjFactor + PowerSupply.Voltage.adjOffset); }, nullptr},
        {"Reset statistics", 1000, 1500,
         []()
         { PowerSupply.Current.Statistics.ResetStats(); }, nullptr},
        {"Measuring current at 0V", 60000, 1500,
         nullptr, []()
         { g_leakage_i_at_0v = PowerSupply.Current.Statistics.Mean(); }},
        {"Setting voltage to 32V", 1500, 500,
         []()
         {
             PowerSupply.Voltage.SetUpdate(32.0 * PowerSupply.Voltage.adjFactor + PowerSupply.Voltage.adjOffset);
         },
         nullptr},
        {"Reset statistics", 1500, 1500,
         []()
         { PowerSupply.Current.Statistics.ResetStats(); }, nullptr},
        {"Measuring current at 32V", 60000, 1000,
         nullptr, []()
         { g_leakage_i_at_32v = PowerSupply.Current.Statistics.Mean(); }},
        {"Finalize", 100, 100, nullptr,
         []()
         {
             esp_task_wdt_reset();

             // Log measurement results
             log_step("           i0 = %+1.6f", g_leakage_i_at_0v);
             log_step("           i1 = %+1.6f", g_leakage_i_at_32v);
             double Rtot = (PowerSupply.mA_Active ? 1000.0 : 1.0) * 32.0f / (g_leakage_i_at_32v - g_leakage_i_at_0v) / 1000.0f;
             log_step("Measured Res: %4.3fk", Rtot);

             esp_task_wdt_reset();

             // CRITICAL: Validate indices before array access
             if (PowerSupply.CalBank.empty() ||
                 PowerSupply.bankCalibId < 0 ||
                 PowerSupply.bankCalibId >= (int8_t)PowerSupply.CalBank.size() ||
                 PowerSupply.mA_Active < 0 ||
                 PowerSupply.mA_Active > 1)
             {
                 Serial.printf("\nERROR in leakage finalize: Invalid indices! bankCalibId=%d, CalBank.size()=%d, mA_Active=%d",
                              PowerSupply.bankCalibId, PowerSupply.CalBank.size(), PowerSupply.mA_Active);
                 g_calibration_in_progress = false;
                 return;
             }

             // Save the measured resistance value
             PowerSupply.CalBank[PowerSupply.bankCalibId].internalLeakage[PowerSupply.mA_Active] = Rtot;

             // Update the spinbox with the measured value (with null checks)
             Serial.printf("\nUpdating spinbox: mA_Active=%d", PowerSupply.mA_Active);
             if (PowerSupply.mA_Active) {
                 if (Calib_GUI.internalLeakage_mA) {
                     Serial.printf("\nSetting mA spinbox to %f", 1000.0f * Rtot);
                     lv_spinbox_set_value(Calib_GUI.internalLeakage_mA, 1000.0f * Rtot);
                     Serial.printf("\nmA spinbox updated");
                 }
             } else {
                 if (Calib_GUI.internalLeakage_A) {
                     Serial.printf("\nSetting A spinbox to %f", 1000.0f * Rtot);
                     lv_spinbox_set_value(Calib_GUI.internalLeakage_A, 1000.0f * Rtot);
                     Serial.printf("\nA spinbox updated");
                 }
             }

             // Set voltage back to 0V
             PowerSupply.Voltage.SetUpdate(0.0 * PowerSupply.Voltage.adjFactor + PowerSupply.Voltage.adjOffset);

             esp_task_wdt_reset();

             // Schedule log window to close after 6 seconds
             lv_timer_t *close_t = lv_timer_create(close_log_cb, 6000, nullptr);
             if (close_t) {
                 lv_timer_set_repeat_count(close_t, 1);
             }

             esp_task_wdt_reset();

             // Reset calibration flag to allow subsequent calibrations
             g_calibration_in_progress = false;
         }},
    };

    lv_timer_t *t = lv_timer_create(seq_cb, 50, nullptr);
    seq_start(t, steps, sizeof(steps) / sizeof(steps[0]), nullptr);
}

// Global storage for auto-zero calibration result (avoids lambda capture issues)
static int32_t g_zero_current_code = 0;

void start_current_zero_calibration(lv_event_t *e)
{
    // Reset the global before starting
    g_zero_current_code = 0;

    // CRITICAL: Validate array indices before proceeding
    if (PowerSupply.CalBank.empty() ||
        PowerSupply.bankCalibId < 0 ||
        PowerSupply.bankCalibId >= (int8_t)PowerSupply.CalBank.size())
    {
        Serial.printf("\nERROR: Invalid bankCalibId=%d, CalBank.size()=%d",
                     PowerSupply.bankCalibId, PowerSupply.CalBank.size());
        g_calibration_in_progress = false;
        Warning_msgbox("Error: Invalid calibration bank", [](lv_event_t*){});
        return;
    }

    if (PowerSupply.mA_Active < 0 || PowerSupply.mA_Active > 1)
    {
        Serial.printf("\nERROR: Invalid mA_Active=%d (must be 0 or 1)", PowerSupply.mA_Active);
        g_calibration_in_progress = false;
        Warning_msgbox("Error: Invalid current range", [](lv_event_t*){});
        return;
    }

    Serial.printf("\nStarting current zero calibration: bankCalibId=%d, mA_Active=%d, CalBank.size()=%d",
                 PowerSupply.bankCalibId, PowerSupply.mA_Active, PowerSupply.CalBank.size());

    static const SeqStep steps[] = {
        {"Setting voltage to 0V", 1500, 500,
         []()
         { PowerSupply.Voltage.SetUpdate(0.0 * PowerSupply.Voltage.adjFactor + PowerSupply.Voltage.adjOffset); }, nullptr},
        {"Reset statistics", 1000, 1500,
         []()
         { PowerSupply.Current.rawValueStats.ResetStats(); }, nullptr},
        {"Measuring current at 0V", 10000, 1500,
         nullptr, []()
         {
             g_zero_current_code = PowerSupply.Current.rawValueStats.Mean();
             Serial.printf("\n Code 1 at zero current:%i", g_zero_current_code);
         }},
        {"Setting code for 0.0A", 1500, 500, []()
         {
             Serial.printf("\nSetting code_1 preview to %i", g_zero_current_code);
             if (Calib_GUI.Current.code_1) {
                 lv_spinbox_set_value(Calib_GUI.Current.code_1, g_zero_current_code);
                 Serial.printf("\nPreview updated successfully");
             } else {
                 Serial.println("\nWARNING: Calib_GUI.Current.code_1 is NULL");
             }
         },
         nullptr},
        {"Finalize", 1500, 1500, []()
         {
             esp_task_wdt_reset();

             lv_timer_t *close_t = lv_timer_create(close_log_cb, 6000, nullptr);
             lv_timer_set_repeat_count(close_t, 1);

             esp_task_wdt_reset();
             Serial.printf("\nSetting code_1 spinbox to %i", g_zero_current_code);
             if (Calib_GUI.Current.code_1) {
                 lv_spinbox_set_value(Calib_GUI.Current.code_1, g_zero_current_code);
                 Serial.printf("\ncode_1 spinbox updated");
             }

             esp_task_wdt_reset();
             // CRITICAL: Re-validate indices before array access (they could change during sequence)
             if (PowerSupply.CalBank.empty() ||
                 PowerSupply.bankCalibId < 0 ||
                 PowerSupply.bankCalibId >= (int8_t)PowerSupply.CalBank.size() ||
                 PowerSupply.mA_Active < 0 ||
                 PowerSupply.mA_Active > 1)
             {
                 Serial.printf("\nERROR in finalize: Invalid indices! bankCalibId=%d, CalBank.size()=%d, mA_Active=%d",
                              PowerSupply.bankCalibId, PowerSupply.CalBank.size(), PowerSupply.mA_Active);
                 g_calibration_in_progress = false;
                 return;
             }

             auto &cal = PowerSupply.CalBank[PowerSupply.bankCalibId].iCal;
             cal[PowerSupply.mA_Active].code_1 = g_zero_current_code;

             esp_task_wdt_reset();
             Serial.printf("\n Code 1 at zero current:%i", g_zero_current_code);

             // Reset calibration flag to allow subsequent calibrations
             g_calibration_in_progress = false;
         },
         nullptr},
    };

    lv_timer_t *t = lv_timer_create(seq_cb, 50, nullptr);
    seq_start(t, steps, sizeof(steps) / sizeof(steps[0]), nullptr);
}

// Global storage for voltage auto-zero calibration result
static int32_t g_zero_voltage_code = 0;

void start_voltage_zero_calibration(lv_event_t *e)
{
    // Reset the global before starting
    g_zero_voltage_code = 0;

    // CRITICAL: Validate array indices before proceeding
    if (PowerSupply.CalBank.empty() ||
        PowerSupply.bankCalibId < 0 ||
        PowerSupply.bankCalibId >= (int8_t)PowerSupply.CalBank.size())
    {
        Serial.printf("\nERROR: Invalid bankCalibId=%d, CalBank.size()=%d",
                     PowerSupply.bankCalibId, PowerSupply.CalBank.size());
        g_calibration_in_progress = false;
        Warning_msgbox("Error: Invalid calibration bank", [](lv_event_t*){});
        return;
    }

    Serial.printf("\nStarting voltage zero calibration: bankCalibId=%d, CalBank.size()=%d",
                 PowerSupply.bankCalibId, PowerSupply.CalBank.size());

    static const SeqStep steps[] = {
        {"Setting voltage to 0V", 1500, 500,
         []()
         { PowerSupply.Voltage.SetUpdate(0.0 * PowerSupply.Voltage.adjFactor + PowerSupply.Voltage.adjOffset); }, nullptr},
        {"Reset statistics", 1000, 1500,
         []()
         { PowerSupply.Voltage.rawValueStats.ResetStats(); }, nullptr},
        {"Measuring voltage at 0V", 10000, 1500,
         nullptr, []()
         {
             g_zero_voltage_code = PowerSupply.Voltage.rawValueStats.Mean();
             Serial.printf("\n Code 1 at zero voltage:%i", g_zero_voltage_code);
         }},
        {"Setting code for 0.0V", 1500, 500, []()
         {
             if (Calib_GUI.Voltage.code_1)
                 lv_spinbox_set_value(Calib_GUI.Voltage.code_1, g_zero_voltage_code);
         },
         nullptr},
        {"Finalize", 1500, 1500, []()
         {
             esp_task_wdt_reset();

             lv_timer_t *close_t = lv_timer_create(close_log_cb, 6000, nullptr);
             lv_timer_set_repeat_count(close_t, 1);

             esp_task_wdt_reset();
             Serial.printf("\nSetting code_1 spinbox to %i", g_zero_voltage_code);
             if (Calib_GUI.Voltage.code_1) {
                 lv_spinbox_set_value(Calib_GUI.Voltage.code_1, g_zero_voltage_code);
                 Serial.printf("\ncode_1 spinbox updated");
             }

             esp_task_wdt_reset();
             // CRITICAL: Re-validate indices before array access (they could change during sequence)
             if (PowerSupply.CalBank.empty() ||
                 PowerSupply.bankCalibId < 0 ||
                 PowerSupply.bankCalibId >= (int8_t)PowerSupply.CalBank.size())
             {
                 Serial.printf("\nERROR in finalize: Invalid bankCalibId=%d, CalBank.size()=%d",
                              PowerSupply.bankCalibId, PowerSupply.CalBank.size());
                 g_calibration_in_progress = false;
                 return;
             }

             auto &cal = PowerSupply.CalBank[PowerSupply.bankCalibId].vCal;
             cal.code_1 = g_zero_voltage_code;

             esp_task_wdt_reset();
             Serial.printf("\n Code 1 at zero voltage:%i", g_zero_voltage_code);

             // Reset calibration flag to allow subsequent calibrations
             g_calibration_in_progress = false;
         },
         nullptr},
    };

    lv_timer_t *t = lv_timer_create(seq_cb, 50, nullptr);
    seq_start(t, steps, sizeof(steps) / sizeof(steps[0]), nullptr);
}
