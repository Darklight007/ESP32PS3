#include "calib_internal_current.h"
#include "calib_log_window.h"
#include "calib_sequencer.h"
#include "device.hpp"
#include "setting_menu.h"
#include "esp_task_wdt.h"
#include <Arduino.h>
#include <float.h>

// External references
extern Device PowerSupply;
extern setting_GUI Calib_GUI;
extern bool blockAll;

// Context for total resistance measurement
struct CurrentCalCtx
{
    double v0 = 0, v1 = 0;
};

void start_current_totalR()
{
    auto *c = (CurrentCalCtx *)lv_mem_alloc(sizeof(CurrentCalCtx));
    *c = CurrentCalCtx{};
    esp_task_wdt_reset();
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
        {"Measuring current at 0V", 60000, 1500,
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

        {"Measuring current at 32V", 60000, 1000,
         nullptr, [c]()
         { c->v1 = PowerSupply.Current.Statistics.Mean(); }},
        {"Finalize", 100, 100, nullptr,
         [c]()
         {
             esp_task_wdt_reset();
             blockAll = true;
             log_step("           i0 = %+1.6f", c->v0);
             log_step("           i1 = %+1.6f", c->v1);
             double Rtot = (PowerSupply.mA_Active ? 1000.0 : 1.0) * 32.0f / (c->v1 - c->v0) / 1000.0f;
             log_step("Measured Res: %4.3fk", Rtot);

             lv_spinbox_set_value(Calib_GUI.internalLeakage, 1000.0f * Rtot);
             PowerSupply.CalBank[PowerSupply.bankCalibId].internalLeakage = Rtot;
             PowerSupply.Voltage.SetUpdate(0.0 * PowerSupply.Voltage.adjFactor + PowerSupply.Voltage.adjOffset);
             lv_timer_t *close_t = lv_timer_create(close_log_cb, 6000, nullptr);
             esp_task_wdt_reset();
             lv_timer_set_repeat_count(close_t, 1);
             lv_mem_free(c);
             blockAll = false;
         }},
    };

    lv_timer_t *t = lv_timer_create(seq_cb, 50, nullptr);
    seq_start(t, steps, sizeof(steps) / sizeof(steps[0]), nullptr);
}

void start_current_zeros(lv_event_t *e)
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

    lv_timer_t *t = lv_timer_create(seq_cb, 50, nullptr);
    seq_start(t, steps, sizeof(steps) / sizeof(steps[0]), nullptr);
}
