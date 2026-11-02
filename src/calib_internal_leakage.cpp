#include "calib_internal_leakage.h"
#include "calib_log_window.h"
#include "calib_sequencer.h"
#include "calib_adc.h"  // For g_calibration_in_progress
#include "device.hpp"
#include "setting_menu.h"
#include "esp_task_wdt.h"
#include <Arduino.h>
#include <float.h>
#include <cmath>
#include <vector>

// ─── Timing constants ──────────────────────────────────────────────────────
// Increase LEAKAGE_MEASURE_MS if ADC noise is high and you need more averaging.
#define LEAKAGE_MEASURE_MS   20000   // ADC accumulation time per voltage point (ms)
#define LEAKAGE_SETTLE_MS    5000    // Settling time after a setpoint change (ms)
// ────────────────────────────────────────────────────────────────────────────

// Zero + leakage fit points, shared by both A and mA ranges (mA_Active only
// selects which iCal[]/internalLeakage[] slot gets the result).
// The measured current is  I = I0 + V/R : the zero offset plus the internal
// leakage path (sense divider etc.) that loads the output through the shunt.
// Both come from one least-squares line through these points.
// No point below 0.5 V: near 0 V pulses from the pre-tracking SMPS get through
// to the shunt (measured on the mA range: -50 +-20 uA at 0 V, -19 uA at 0.1 V,
// -8 uA at 0.25 V, on the line again from 0.5 V), so a 0 V reading is neither
// the zero nor a valid leakage point.
static constexpr double kLeakageV[] = {1.0, 8.0, 16.0, 24.0, 32.0};
static constexpr int kLeakageN = sizeof(kLeakageV) / sizeof(kLeakageV[0]);

// ─── Countdown timer ────────────────────────────────────────────────────────
static lv_timer_t *s_countdown_timer = nullptr;
static int         s_countdown_secs  = 0;

static void countdown_tick_cb(lv_timer_t *)
{
    if (s_countdown_secs > 0) s_countdown_secs--;
    char buf[48];
    snprintf(buf, sizeof(buf), LV_SYMBOL_REFRESH " Measuring... %ds left", s_countdown_secs);
    log_set_countdown(buf);
}

static void start_countdown(int total_secs)
{
    s_countdown_secs = total_secs;
    if (s_countdown_timer) { lv_timer_del(s_countdown_timer); s_countdown_timer = nullptr; }
    s_countdown_timer = lv_timer_create(countdown_tick_cb, 1000, nullptr);
    countdown_tick_cb(nullptr); // show immediately
}

static void stop_countdown()
{
    if (s_countdown_timer) { lv_timer_del(s_countdown_timer); s_countdown_timer = nullptr; }
    log_set_countdown(nullptr); // hide countdown label
}
// ────────────────────────────────────────────────────────────────────────────

// External references
extern Device PowerSupply;
extern CalibrationGui Calib_GUI;
extern volatile bool blockAll;

// Per-point results of the zero + leakage fit
static double g_leak_code[kLeakageN]; // mean raw ADC code (x1 PGA)
static double g_leak_v[kLeakageN];    // mean measured output voltage

static MovingStatistics &active_raw_stats()
{
    // A and mA ranges use different ADC gain, so each has its own raw-code
    // window (see DispObject.h) - use whichever range is active.
    return PowerSupply.mA_Active ? PowerSupply.Current.rawValueStats_mA
                                 : PowerSupply.Current.rawValueStats;
}

// Codes per (A or mA) of the active range for a zero code `zeroCode`, with the
// stored gain point. Same formula as Device::calibrationUpdate() with value_1 = 0.
static double gain_codes_per_unit(const calibPoints &c, double zeroCode)
{
    return (c.code_2 - zeroCode) / c.value_2;
}

// Leakage R (kOhm, as stored in internalLeakage[]) from the fitted slope in
// codes per volt. Device::readCurrent() subtracts V/R mA in the mA range and
// V/(R*1000) A in the A range.
static double leakage_R_from_slope(double codesPerVolt, double codesPerUnit)
{
    const double unitsPerVolt = codesPerVolt / codesPerUnit;
    return 1.0 / (unitsPerVolt * (PowerSupply.mA_Active ? 1.0 : 1000.0));
}

static void leakage_finalize()
{
    esp_task_wdt_reset();

    // Least-squares line code = c0 + k*V through the measured points
    double mv = 0, mc = 0;
    for (int i = 0; i < kLeakageN; i++) { mv += g_leak_v[i]; mc += g_leak_code[i]; }
    mv /= kLeakageN; mc /= kLeakageN;
    double sxy = 0, sxx = 0;
    for (int i = 0; i < kLeakageN; i++)
    {
        sxy += (g_leak_v[i] - mv) * (g_leak_code[i] - mc);
        sxx += (g_leak_v[i] - mv) * (g_leak_v[i] - mv);
    }
    const double k = sxy / sxx;
    const double c0 = mc - k * mv;

    if (PowerSupply.CalBank.empty() || PowerSupply.bankCalibId < 0 ||
        PowerSupply.bankCalibId >= (int8_t)PowerSupply.CalBank.size() ||
        PowerSupply.mA_Active < 0 || PowerSupply.mA_Active > 1)
    {
        Serial.printf("\nERROR in leakage finalize: Invalid indices! bankCalibId=%d, mA_Active=%d",
                      PowerSupply.bankCalibId, PowerSupply.mA_Active);
        PowerSupply.Voltage.SetUpdate(0.0 * PowerSupply.Voltage.adjFactor + PowerSupply.Voltage.adjOffset);
        calib_low_noise_end();
        g_calibration_in_progress = false;
        return;
    }
    auto &bank = PowerSupply.CalBank[PowerSupply.bankCalibId];
    auto &cal = bank.iCal[PowerSupply.mA_Active];
    const char *unit = PowerSupply.mA_Active ? "uA" : "mA"; // residuals x1000

    const double m = gain_codes_per_unit(cal, c0);
    double Rtot = leakage_R_from_slope(k, m);
    // Allow negative R: some hardware sources current INTO the sense path,
    // scaling linearly with V. Same I = V/R model handles it with R<0.
    // Clamp magnitude to spinbox range (+-9999 kOhm).
    if (!std::isfinite(Rtot)) Rtot = 0.0;
    if (Rtot >  9999.0) Rtot =  9999.0;
    if (Rtot < -9999.0) Rtot = -9999.0;

    // Residuals show whether the leakage really is linear in V
    double worst = 0;
    for (int i = 0; i < kLeakageN; i++)
    {
        const double r = 1000.0 * (g_leak_code[i] - (c0 + k * g_leak_v[i])) / m;
        if (fabs(r) > fabs(worst)) worst = r;
        Serial.printf("\n  %6.3fV code %10.1f resid %+7.3f%s", g_leak_v[i], g_leak_code[i], r, unit);
    }
    const double oldZero = 1000.0 * (cal.code_1 - c0) / m;
    log_step("Zero code %d -> %d (%+.2f%s)", cal.code_1, (int)lround(c0), oldZero, unit);
    log_step("Leakage R: %.3fk (was %.3fk)", Rtot, bank.internalLeakage[PowerSupply.mA_Active]);
    log_step("Linearity: worst %+.2f%s", worst, unit);
    Serial.printf("\nLeakage fit: c0=%.1f k=%.3f code/V m=%.3f R=%.4fk worst=%+.3f%s",
                  c0, k, m, Rtot, worst, unit);

    // Zero point is (0, c0); the gain point (value_2, code_2) is untouched
    cal.value_1 = 0.0;
    cal.code_1 = (int32_t)lround(c0);
    bank.internalLeakage[PowerSupply.mA_Active] = Rtot;
    PowerSupply.calibrationUpdate();
    PowerSupply.Current.measured.ResetStats();

    // Update the spinboxes that show these values
    lv_obj_t *rBox = PowerSupply.mA_Active ? Calib_GUI.internalLeakage_mA : Calib_GUI.internalLeakage_A;
    if (rBox && lv_obj_is_valid(rBox))
        lv_spinbox_set_value(rBox, 1000.0f * Rtot);
    if (Calib_GUI.Current.code_1 && lv_obj_is_valid(Calib_GUI.Current.code_1))
        lv_spinbox_set_value(Calib_GUI.Current.code_1, cal.code_1);
    if (Calib_GUI.Current.vin_1 && lv_obj_is_valid(Calib_GUI.Current.vin_1))
        lv_spinbox_set_value(Calib_GUI.Current.vin_1, 0);

    PowerSupply.Voltage.SetUpdate(0.0 * PowerSupply.Voltage.adjFactor + PowerSupply.Voltage.adjOffset);
    calib_low_noise_end();

    lv_timer_t *close_t = lv_timer_create(close_log_cb, 6000, nullptr);
    if (close_t)
        lv_timer_set_repeat_count(close_t, 1);

    Serial.println("\nLeakage measurement completed successfully");
    g_calibration_in_progress = false;
}

void start_leakage_resistance_measurement(lv_event_t *)
{
    // CRITICAL: Validate array indices before proceeding
    if (PowerSupply.CalBank.empty() ||
        PowerSupply.bankCalibId < 0 ||
        PowerSupply.bankCalibId >= (int8_t)PowerSupply.CalBank.size())
    {
        Serial.printf("\nERROR: Invalid bankCalibId=%d, CalBank.size()=%d",
                     PowerSupply.bankCalibId, PowerSupply.CalBank.size());
        g_calibration_in_progress = false;
        Message_msgbox("Calibration Error", "Invalid calibration bank.");
        return;
    }

    if (PowerSupply.mA_Active < 0 || PowerSupply.mA_Active > 1)
    {
        Serial.printf("\nERROR: Invalid mA_Active=%d (must be 0 or 1)", PowerSupply.mA_Active);
        g_calibration_in_progress = false;
        Message_msgbox("Calibration Error", "Invalid current range.");
        return;
    }

    Serial.printf("\nStarting zero + leakage fit: bankCalibId=%d, mA_Active=%d",
                 PowerSupply.bankCalibId, PowerSupply.mA_Active);

    esp_task_wdt_reset();
    calib_low_noise_begin();

    // Two steps per point (set V, measure) plus Finalize. Raw codes are used,
    // so the stored leakage compensation and REL offset don't enter the fit.
    static char labels[kLeakageN][2][32];
    static std::vector<SeqStep> steps;
    if (steps.empty())
    {
        for (int i = 0; i < kLeakageN; i++)
        {
            snprintf(labels[i][0], sizeof(labels[i][0]), "Setting voltage to %gV", kLeakageV[i]);
            snprintf(labels[i][1], sizeof(labels[i][1]), "Measuring current at %gV", kLeakageV[i]);
            steps.push_back({labels[i][0], LEAKAGE_SETTLE_MS, 500,
                             [i]() { PowerSupply.Voltage.SetUpdate(kLeakageV[i] * PowerSupply.Voltage.adjFactor + PowerSupply.Voltage.adjOffset); },
                             nullptr});
            steps.push_back({labels[i][1], LEAKAGE_MEASURE_MS, 500,
                             []() {
                                 active_raw_stats().ResetStats();
                                 PowerSupply.Voltage.Statistics.ResetStats();
                                 start_countdown(LEAKAGE_MEASURE_MS / 1000);
                             },
                             [i]() {
                                 stop_countdown();
                                 g_leak_code[i] = active_raw_stats().Mean();
                                 g_leak_v[i] = PowerSupply.Voltage.Statistics.Mean();
                             }});
        }
        steps.push_back({"Finalize", 100, 100, nullptr, leakage_finalize});
    }

    lv_timer_t *t = lv_timer_create(seq_cb, 50, nullptr);
    if (!t) {
        Serial.println("ERROR: Failed to create leakage seq timer (LVGL heap full)");
        calib_low_noise_end();
        g_calibration_in_progress = false;
        return;
    }
    seq_start(t, steps.data(), steps.size(), nullptr);
}

// Global storage for auto-zero calibration result (avoids lambda capture issues)
static int32_t g_zero_current_code = 0;

// Current auto-zero is measured at kZeroV, not 0 V (see kLeakageV), or at V-Set
// if the user said no (see calib_adc.cpp). Either way the leakage current V/R
// at the measured voltage is taken out of the zero code, so it needs the
// stored leakage R; without one the zero carries V/R of error.
static constexpr double kZeroV = 1.0;
static bool g_current_zero_set_v = true;
static char g_current_zero_step1[40] = "Setting voltage to 1V";
static double g_zero_current_v = 0;

void current_zero_set_voltage(bool set, const char *step_label)
{
    g_current_zero_set_v = set;
    snprintf(g_current_zero_step1, sizeof(g_current_zero_step1), "%s", step_label);
}

// Leakage current at voltage v in raw codes of the active range, 0 if no R stored.
static double leakage_codes_at(double v, bool *haveR)
{
    const double R = PowerSupply.CalBank[PowerSupply.bankCalibId].internalLeakage[PowerSupply.mA_Active];
    *haveR = std::isfinite(R) && std::abs(R) > 0.001 && std::abs(R) < 1e9;
    if (!*haveR)
        return 0.0;
    return PowerSupply.Current.calib_m * v / (R * (PowerSupply.mA_Active ? 1.0 : 1000.0));
}

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
        Message_msgbox("Calibration Error", "Invalid calibration bank.");
        return;
    }

    if (PowerSupply.mA_Active < 0 || PowerSupply.mA_Active > 1)
    {
        Serial.printf("\nERROR: Invalid mA_Active=%d (must be 0 or 1)", PowerSupply.mA_Active);
        g_calibration_in_progress = false;
        Message_msgbox("Calibration Error", "Invalid current range.");
        return;
    }

    Serial.printf("\nStarting current zero calibration: bankCalibId=%d, mA_Active=%d, CalBank.size()=%d",
                 PowerSupply.bankCalibId, PowerSupply.mA_Active, PowerSupply.CalBank.size());

    calib_low_noise_begin();

    static const SeqStep steps[] = {
        {g_current_zero_step1, 5000, 500,
         []()
         {
             if (g_current_zero_set_v)
                 PowerSupply.Voltage.SetUpdate(kZeroV * PowerSupply.Voltage.adjFactor + PowerSupply.Voltage.adjOffset);
         }, nullptr},
        {"Reset statistics", 1000, 1500,
         []()
         {
             // A and mA ranges use different ADC gain, so each has its own
             // rawValueStats instance (see DispObject.h) - reset whichever is active.
             if (PowerSupply.mA_Active) PowerSupply.Current.rawValueStats_mA.ResetStats();
             else PowerSupply.Current.rawValueStats.ResetStats();
             PowerSupply.Voltage.Statistics.ResetStats();
         }, nullptr},
        {"Measuring current", 20000, 1500,
         []() { start_countdown(20); },
         []() { stop_countdown();
             const double raw = PowerSupply.mA_Active ? PowerSupply.Current.rawValueStats_mA.Mean()
                                                      : PowerSupply.Current.rawValueStats.Mean();
             g_zero_current_v = PowerSupply.Voltage.Statistics.Mean();
             bool haveR;
             const double leak = leakage_codes_at(g_zero_current_v, &haveR);
             g_zero_current_code = (int32_t)lround(raw - leak);
             log_step("At %.3fV, leakage %.1f codes", g_zero_current_v, leak);
             if (!haveR && fabs(g_zero_current_v) > 0.05)
                 log_step("No leakage R: run Auto Leakage");
             Serial.printf("\n Code 1 at zero current:%i (raw %.1f, V %.4f, leak %.1f)",
                           g_zero_current_code, raw, g_zero_current_v, leak);
         }},
        {"Setting code for 0.0A", 1500, 500, []()
         {
             Serial.printf("\nSetting code_1 preview to %i", g_zero_current_code);
             if (Calib_GUI.Current.code_1 && lv_obj_is_valid(Calib_GUI.Current.code_1)) {
                 lv_spinbox_set_value(Calib_GUI.Current.code_1, g_zero_current_code);
                 Serial.printf("\nPreview updated successfully");
             }
         },
         nullptr},
        {"Finalize", 1500, 1500, []()
         {
             esp_task_wdt_reset();

             // CRITICAL: Save cal data FIRST before any UI operations that could fail
             if (!PowerSupply.CalBank.empty() &&
                 PowerSupply.bankCalibId >= 0 &&
                 PowerSupply.bankCalibId < (int8_t)PowerSupply.CalBank.size() &&
                 PowerSupply.mA_Active >= 0 &&
                 PowerSupply.mA_Active <= 1)
             {
                 auto &cal = PowerSupply.CalBank[PowerSupply.bankCalibId].iCal;
                 cal[PowerSupply.mA_Active].value_1 = 0.0; // code_1 is the zero-current code
                 cal[PowerSupply.mA_Active].code_1 = g_zero_current_code;
                 PowerSupply.calibrationUpdate();
                 PowerSupply.Current.measured.ResetStats();
                 Serial.printf("\n Code 1 at zero current saved: %i", g_zero_current_code);
             }
             else
             {
                 Serial.printf("\nERROR in finalize: Invalid indices! bankCalibId=%d, CalBank.size()=%d, mA_Active=%d",
                              PowerSupply.bankCalibId, PowerSupply.CalBank.size(), PowerSupply.mA_Active);
             }

             // Measured at 1 V: leave the output at 0 V as before
             if (g_current_zero_set_v)
                 PowerSupply.Voltage.SetUpdate(0.0 * PowerSupply.Voltage.adjFactor + PowerSupply.Voltage.adjOffset);
             calib_low_noise_end();

             // Reset calibration flag before UI ops so a crash here doesn't lock out future runs
             g_calibration_in_progress = false;

             esp_task_wdt_reset();
             Serial.printf("\nSetting code_1 spinbox to %i", g_zero_current_code);
             if (Calib_GUI.Current.code_1 && lv_obj_is_valid(Calib_GUI.Current.code_1)) {
                 lv_spinbox_set_value(Calib_GUI.Current.code_1, g_zero_current_code);
                 Serial.printf("\ncode_1 spinbox updated");
             }
             if (Calib_GUI.Current.vin_1 && lv_obj_is_valid(Calib_GUI.Current.vin_1))
                 lv_spinbox_set_value(Calib_GUI.Current.vin_1, 0);

             esp_task_wdt_reset();
             lv_timer_t *close_t = lv_timer_create(close_log_cb, 6000, nullptr);
             if (close_t)
                 lv_timer_set_repeat_count(close_t, 1);
             else
                 Serial.println("\nWARNING: Failed to create close timer (low LVGL heap)");
         },
         nullptr},
    };

    lv_timer_t *t = lv_timer_create(seq_cb, 50, nullptr);
    if (!t) {
        Serial.println("ERROR: Failed to create current zero seq timer (LVGL heap full)");
        calib_low_noise_end();
        g_calibration_in_progress = false;
        return;
    }
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
        Message_msgbox("Calibration Error", "Invalid calibration bank.");
        return;
    }

    Serial.printf("\nStarting voltage zero calibration: bankCalibId=%d, CalBank.size()=%d",
                 PowerSupply.bankCalibId, PowerSupply.CalBank.size());

    calib_low_noise_begin();

    static const SeqStep steps[] = {
        {"Setting voltage to 0V", 1500, 500,
         []()
         { PowerSupply.Voltage.SetUpdate(0.0 * PowerSupply.Voltage.adjFactor + PowerSupply.Voltage.adjOffset); }, nullptr},
        {"Reset statistics", 1000, 1500,
         []()
         { PowerSupply.Voltage.rawValueStats.ResetStats(); }, nullptr},
        {"Measuring voltage at 0V", 10000, 1500,
         []() { start_countdown(10); },
         []() { stop_countdown();
             g_zero_voltage_code = PowerSupply.Voltage.rawValueStats.Mean();
             Serial.printf("\n Code 1 at zero voltage:%i", g_zero_voltage_code);
         }},
        {"Setting code for 0.0V", 1500, 500, []()
         {
             if (Calib_GUI.Voltage.code_1 && lv_obj_is_valid(Calib_GUI.Voltage.code_1))
                 lv_spinbox_set_value(Calib_GUI.Voltage.code_1, g_zero_voltage_code);
         },
         nullptr},
        {"Finalize", 1500, 1500, []()
         {
             esp_task_wdt_reset();

             // CRITICAL: Save cal data FIRST before any UI operations that could fail
             if (!PowerSupply.CalBank.empty() &&
                 PowerSupply.bankCalibId >= 0 &&
                 PowerSupply.bankCalibId < (int8_t)PowerSupply.CalBank.size())
             {
                 auto &cal = PowerSupply.CalBank[PowerSupply.bankCalibId].vCal;
                 cal.code_1 = g_zero_voltage_code;
                 Serial.printf("\n Code 1 at zero voltage saved: %i", g_zero_voltage_code);
             }
             else
             {
                 Serial.printf("\nERROR in finalize: Invalid bankCalibId=%d, CalBank.size()=%d",
                              PowerSupply.bankCalibId, PowerSupply.CalBank.size());
             }

             calib_low_noise_end();

             // Reset calibration flag before UI ops so a crash here doesn't lock out future runs
             g_calibration_in_progress = false;

             esp_task_wdt_reset();
             Serial.printf("\nSetting code_1 spinbox to %i", g_zero_voltage_code);
             if (Calib_GUI.Voltage.code_1 && lv_obj_is_valid(Calib_GUI.Voltage.code_1)) {
                 lv_spinbox_set_value(Calib_GUI.Voltage.code_1, g_zero_voltage_code);
                 Serial.printf("\ncode_1 spinbox updated");
             }

             esp_task_wdt_reset();
             lv_timer_t *close_t = lv_timer_create(close_log_cb, 6000, nullptr);
             if (close_t)
                 lv_timer_set_repeat_count(close_t, 1);
             else
                 Serial.println("\nWARNING: Failed to create close timer (low LVGL heap)");
         },
         nullptr},
    };

    lv_timer_t *t = lv_timer_create(seq_cb, 50, nullptr);
    if (!t) {
        Serial.println("ERROR: Failed to create voltage zero seq timer (LVGL heap full)");
        calib_low_noise_end();
        g_calibration_in_progress = false;
        return;
    }
    seq_start(t, steps, sizeof(steps) / sizeof(steps[0]), nullptr);
}
