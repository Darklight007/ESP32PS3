#include "calib_rate_match.h"
#include "calib_log_window.h"
#include "calib_sequencer.h"
#include "calib_adc.h"    // g_calibration_in_progress, calib_low_noise_begin/end
#include "setting_menu.h" // msgbox_close_deferred()
#include "device.hpp"
#include "esp_task_wdt.h"
#include <cmath>
#include <vector>

extern Device PowerSupply;

// ─── Trim tables ────────────────────────────────────────────────────────────
// Defaults: measured by hand 2026-11-12 (see device.cpp readVoltage comment).
static constexpr float kDefGain[5] = {
    1.00000f,
    1.00000f * (1.0f + 0.0063e-3f),
    32.0000f / 31.9985f * (1.0f + 0.0130e-3f),
    32.0000f / 31.9945f * (1.0f + 0.0167e-3f),
    32.0000f / 31.9882f * (1.0f + 0.0626e-3f)};
static constexpr float kDefOff[5] = {0.0f, 0.047e-3f, -0.006e-3f, -0.233e-3f, -0.810e-3f};

volatile float g_rateGain[5] = {kDefGain[0], kDefGain[1], kDefGain[2], kDefGain[3], kDefGain[4]};
volatile float g_rateOff[5] = {kDefOff[0], kDefOff[1], kDefOff[2], kDefOff[3], kDefOff[4]};

struct RateTrim
{
    uint32_t magic;
    float gain[5];
    float off[5];
};
static constexpr uint32_t kMagic = 0x5A7E0001;
static constexpr const char *kKey = "rate_trim";

static void publish(const float *g, const float *o)
{
    for (int r = 0; r < 5; r++)
    {
        g_rateGain[r] = g[r];
        g_rateOff[r] = o[r];
    }
}

static bool trim_valid(const RateTrim &t)
{
    if (t.magic != kMagic)
        return false;
    for (int r = 0; r < 5; r++)
        if (!(fabsf(t.gain[r] - 1.0f) < 0.002f) || !(fabsf(t.off[r]) < 0.01f))
            return false;
    return true;
}

void rateMatchLoad()
{
    RateTrim t{};
    PowerSupply.StoreMem.begin("my-app", true);
    const size_t got = PowerSupply.StoreMem.getBytes(kKey, &t, sizeof(t));
    PowerSupply.StoreMem.end();
    if (got == sizeof(t) && trim_valid(t))
    {
        publish(t.gain, t.off);
        Serial.printf("\nSpeed Match trim loaded");
    }
}

void rateMatchClear()
{
    publish(kDefGain, kDefOff);
    PowerSupply.StoreMem.begin("my-app", false);
    PowerSupply.StoreMem.remove(kKey);
    PowerSupply.StoreMem.end();
    Serial.println("\nSpeed Match trim cleared (defaults)");
}

String rateMatchString()
{
    String s;
    for (int r = 0; r < 5; r++)
    {
        char buf[40];
        snprintf(buf, sizeof(buf), "%s%.7f,%.4f", r ? ";" : "", (double)g_rateGain[r], 1000.0 * g_rateOff[r]);
        s += buf;
    }
    return s;
}
// ────────────────────────────────────────────────────────────────────────────

// ─── Sequence ───────────────────────────────────────────────────────────────
static constexpr double kV[2] = {2.0, 29.0};
static constexpr double kMaxLoadA = 0.005; // more = a load is connected
#define RM_SETTLE_V_MS 6000 // after a voltage step
#define RM_SETTLE_R_MS 2500 // after a speed change (filters refill)

static double s_mean[2][5];
static int s_n = 0;
static double s_sum = 0;
static float s_prevG[5], s_prevO[5];
static bool s_failed = false;
static uint16_t s_savedV = 0;
static lv_timer_t *s_timer = nullptr;
static lv_timer_t *s_sampler = nullptr;
static std::vector<SeqStep> s_steps;

static void set_volts(double v) { PowerSupply.Voltage.SetUpdate(v * PowerSupply.Voltage.adjFactor + PowerSupply.Voltage.adjOffset); }

static void set_rate(uint8_t r)
{
    PowerSupply.settingParameters.adcRate = r;
    PowerSupply.applyAdcRate();
}

// Mean of the 512-sample window, taken every 100 ms over the measuring step
static void sample_cb(lv_timer_t *)
{
    s_sum += PowerSupply.Voltage.Statistics.Mean();
    s_n++;
}

static void fail(const char *fmt, double a)
{
    s_failed = true;
    log_step(fmt, a);
    Serial.printf("\nSpeed Match failed: ");
    Serial.printf(fmt, a);
    if (s_timer && s_timer->user_data)
        static_cast<SeqRunner *>(s_timer->user_data)->index = s_steps.size() - 2;
}

static void rm_finish()
{
    esp_task_wdt_reset();
    if (s_sampler)
    {
        lv_timer_del(s_sampler);
        s_sampler = nullptr;
    }

    if (!s_failed)
    {
        // Quiet is the reference: true = g * m + o through both voltages
        RateTrim t{};
        t.magic = kMagic;
        t.gain[0] = 1.0f;
        t.off[0] = 0.0f;
        const double q0 = s_mean[0][0], q1 = s_mean[1][0];
        for (int r = 1; r < 5; r++)
        {
            const double m0 = s_mean[0][r], m1 = s_mean[1][r];
            const double g = (q1 - q0) / (m1 - m0);
            t.gain[r] = g;
            t.off[r] = q0 - g * m0;
            log_step("%s: %+.2f/%+.2f mV -> fixed", r == 1 ? "Slow" : r == 2 ? "Normal" : r == 3 ? "Fast" : "Turbo",
                     1000.0 * (m0 - q0), 1000.0 * (m1 - q1));
            Serial.printf("\n  rate %d: @2V %+.3f mV @29V %+.3f mV gain %.7f off %+.4f mV",
                          r, 1000.0 * (m0 - q0), 1000.0 * (m1 - q1), g, 1000.0 * t.off[r]);
        }
        if (trim_valid(t))
        {
            publish(t.gain, t.off);
            PowerSupply.StoreMem.begin("my-app", false);
            PowerSupply.StoreMem.putBytes(kKey, &t, sizeof(t));
            PowerSupply.StoreMem.end();
            log_step("Saved, all speeds match Quiet");
        }
        else
        {
            s_failed = true;
            log_step("Result out of range, nothing changed");
        }
    }
    if (s_failed)
        publish(s_prevG, s_prevO);

    PowerSupply.Voltage.SetUpdate((int)s_savedV);
    set_rate(0); // calib_low_noise_end() restores the user's speed
    calib_low_noise_end();
    s_timer = nullptr;
    g_calibration_in_progress = false;

    lv_timer_t *close_t = lv_timer_create(close_log_cb, s_failed ? 15000 : 10000, nullptr);
    if (close_t)
        lv_timer_set_repeat_count(close_t, 1);
}

static void build_steps()
{
    static const char *names[5] = {"Quiet", "Slow", "Normal", "Fast", "Turbo"};
    static char labels[2][6][32];
    for (int k = 0; k < 2; k++)
    {
        snprintf(labels[k][5], sizeof(labels[k][5]), "Setting %gV", kV[k]);
        s_steps.push_back({labels[k][5], RM_SETTLE_V_MS, 0,
                           [k]() { set_rate(0); set_volts(kV[k]); }, nullptr});
        for (int r = 0; r < 5; r++)
        {
            snprintf(labels[k][r], sizeof(labels[k][r]), "%gV %s", kV[k], names[r]);
            // Quiet and Slow give ~10-20 readings/s: measure longer
            const uint32_t meas = r < 2 ? 12000 : 6000;
            s_steps.push_back({labels[k][r], RM_SETTLE_R_MS + meas, 0,
                               [r]() {
                                   set_rate(r);
                                   s_sum = 0;
                                   s_n = 0;
                               },
                               [k, r]() {
                                   s_mean[k][r] = s_n ? s_sum / s_n : NAN;
                                   const double a = fabs(PowerSupply.Current.measured.Mean() *
                                                         (PowerSupply.mA_Active ? 0.001 : 1.0));
                                   if (a > kMaxLoadA)
                                       fail("Load connected (%.1f mA): remove it", 1000.0 * a);
                                   else if (!std::isfinite(s_mean[k][r]) || fabs(s_mean[k][r] - kV[k]) > 0.2)
                                       fail("Output not at set voltage (%.3fV)", s_mean[k][r]);
                               }});
        }
    }
    s_steps.push_back({"Finish", 100, 100, nullptr, rm_finish});
}

// Gate the sampler: only count while the current step is past its settle time
static uint32_t s_stepStart = 0;
static size_t s_lastIndex = (size_t)-1;
static void gated_sample_cb(lv_timer_t *t)
{
    if (!s_timer || !s_timer->user_data)
        return;
    const size_t idx = static_cast<SeqRunner *>(s_timer->user_data)->index;
    if (idx != s_lastIndex)
    {
        s_lastIndex = idx;
        s_stepStart = millis();
        return;
    }
    if (millis() - s_stepStart < RM_SETTLE_R_MS)
        return;
    // First tick past the settle time: drop samples from before the change
    static size_t s_resetFor = (size_t)-1;
    if (s_resetFor != idx)
    {
        s_resetFor = idx;
        PowerSupply.Voltage.Statistics.ResetStats();
        return;
    }
    sample_cb(t);
}

bool start_rate_match_calibration()
{
    if (PowerSupply.getStatus() == DEVICE::OFF || PowerSupply.getStatus() == DEVICE::FUN)
        return false;

    esp_task_wdt_reset();
    s_failed = false;
    s_savedV = PowerSupply.Voltage.adjValue;
    calib_low_noise_begin();
    for (int r = 0; r < 5; r++)
    {
        s_prevG[r] = g_rateGain[r];
        s_prevO[r] = g_rateOff[r];
    }
    // Measure untrimmed
    static const float one[5] = {1, 1, 1, 1, 1}, zero[5] = {0, 0, 0, 0, 0};
    publish(one, zero);

    if (s_steps.empty())
        build_steps();
    s_timer = lv_timer_create(seq_cb, 50, nullptr);
    s_lastIndex = (size_t)-1;
    s_sampler = lv_timer_create(gated_sample_cb, 100, nullptr);
    if (!s_timer || !s_sampler)
    {
        Serial.println("ERROR: Failed to create Speed Match timers (LVGL heap full)");
        s_failed = true;
        rm_finish();
        return false;
    }
    seq_start(s_timer, s_steps.data(), s_steps.size(), nullptr);
    return true;
}
// ────────────────────────────────────────────────────────────────────────────

static void rm_msgbox_cb(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_current_target(e);
    const char *txt = lv_msgbox_get_active_btn_text(obj);
    const bool start = txt && strcmp(txt, "Start") == 0;
    const bool clear = txt && strcmp(txt, "Default") == 0;
    msgbox_close_deferred(obj);

    if (clear)
    {
        rateMatchClear();
        return;
    }
    if (!start || g_calibration_in_progress)
        return;
    g_calibration_in_progress = true;
    create_log_window("Speed Match");
    log_reset();
    log_clear();
    if (!start_rate_match_calibration())
    {
        log_step("Turn the output ON first");
        g_calibration_in_progress = false;
    }
}

void rate_match_calibration_cb(lv_event_t *)
{
    esp_task_wdt_reset();
    static const char *btns[] = {"Start", "Default", "Cancel", ""};
    lv_obj_t *mbox = lv_msgbox_create(NULL, "Speed Match",
                                      "Disconnect the load, output ON.\n"
                                      "Measures every ADC speed at 2 V and 29 V\n"
                                      "and matches them to Quiet (~2 min).",
                                      btns, false);
    lv_obj_add_event_cb(mbox, rm_msgbox_cb, LV_EVENT_VALUE_CHANGED, nullptr);
    lv_obj_center(mbox);
}
