#include "calib_internal_leakage.h"
#include "calib_log_window.h"
#include "calib_sequencer.h"
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
             blockAll = true;
             log_step("           i0 = %+1.6f", g_leakage_i_at_0v);
             log_step("           i1 = %+1.6f", g_leakage_i_at_32v);
             double Rtot = (PowerSupply.mA_Active ? 1000.0 : 1.0) * 32.0f / (g_leakage_i_at_32v - g_leakage_i_at_0v) / 1000.0f;
             log_step("Measured Res: %4.3fk", Rtot);

             // Update the correct spinbox based on current mode (with null checks)
             if (PowerSupply.mA_Active) {
                 if (Calib_GUI.internalLeakage_mA)
                     lv_spinbox_set_value(Calib_GUI.internalLeakage_mA, 1000.0f * Rtot);
             } else {
                 if (Calib_GUI.internalLeakage_A)
                     lv_spinbox_set_value(Calib_GUI.internalLeakage_A, 1000.0f * Rtot);
             }
             PowerSupply.CalBank[PowerSupply.bankCalibId].internalLeakage[PowerSupply.mA_Active] = Rtot;
             PowerSupply.Voltage.SetUpdate(0.0 * PowerSupply.Voltage.adjFactor + PowerSupply.Voltage.adjOffset);
             lv_timer_t *close_t = lv_timer_create(close_log_cb, 6000, nullptr);
             esp_task_wdt_reset();
             lv_timer_set_repeat_count(close_t, 1);
             blockAll = false;
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
             if (Calib_GUI.Current.code_1)
                 lv_spinbox_set_value(Calib_GUI.Current.code_1, g_zero_current_code);
         },
         nullptr},
        {"Finalize", 1500, 1500, []()
         {
             lv_timer_t *close_t = lv_timer_create(close_log_cb, 6000, nullptr);
             lv_timer_set_repeat_count(close_t, 1);

             if (Calib_GUI.Current.code_1)
                 lv_spinbox_set_value(Calib_GUI.Current.code_1, g_zero_current_code);

             auto &cal = PowerSupply.CalBank[PowerSupply.bankCalibId].iCal;
             cal[PowerSupply.mA_Active].code_1 = g_zero_current_code;

             Serial.printf("\n Code 1 at zero current:%i", g_zero_current_code);
         },
         nullptr},
    };

    lv_timer_t *t = lv_timer_create(seq_cb, 50, nullptr);
    seq_start(t, steps, sizeof(steps) / sizeof(steps[0]), nullptr);
}
