#include "calib_cc_limit.h"
#include "calib_log_window.h"
#include "calib_sequencer.h"
#include "calib_adc.h"     // g_calibration_in_progress, calib_low_noise_begin/end
#include "input_handler.h" // setCurrentRange()
#include "setting_menu.h"  // msgbox_close_deferred()
#include "device.hpp"
#include "esp_task_wdt.h"
#include <atomic>
#include <cmath>
#include <vector>

extern Device PowerSupply;

// ─── Procedure ──────────────────────────────────────────────────────────────
// Limits stepped with the output shorted, A range. Nothing between 0.1 and
// 1 A: into a dead short at 0.4-0.7 A the current fell 45-190 mA short of the
// limit and wandered (2026-11-01, likely the pre-tracking SMPS sawtooth
// leaving too little headroom at 0 V) - such a point would bend the table.
// Below the first point the first point's ratio is used, so in the mA region
// (where the 10 mOhm CC shunt carries only uV and op-amp offset dominates)
// the change stays under one DAC step (0.1 mA).
static constexpr double kCcSetA[] = {0.1, 1.0, 2.0, 3.0, 4.0, 5.0};
static constexpr int kCcN = sizeof(kCcSetA) / sizeof(kCcSetA[0]);
static constexpr double kCcVset = 2.5;    // V setpoint: CC headroom over wire + leads
static constexpr double kCcIdleA = 0.010; // between points, so nothing heats up
static constexpr double kCcShortMaxV = 0.25; // above this the output isn't shorted
static constexpr double kCcMaxDev = 0.03;    // actual vs set beyond 3% = not a CC limit

#define CC_SETTLE_MS  6000
#define CC_MEASURE_MS 10000
#define CC_COOL_MS    3000
// ────────────────────────────────────────────────────────────────────────────

// ─── Correction table ───────────────────────────────────────────────────────
// Read by DACUpdate() on Core 0, written on Core 1: two buffers, and the
// pointer is swapped only after the new one is complete.
struct CcTable
{
    uint32_t magic;
    uint8_t n;
    float set[8]; // limit commanded, A
    float act[8]; // current measured, A (strictly increasing)
};
static constexpr uint32_t kCcMagic = 0xCC11A001;
static constexpr const char *kCcKey = "cc_lim";

static CcTable s_buf[2];
static int s_bufNext = 0;
static std::atomic<const CcTable *> s_active{nullptr};

static bool table_valid(const CcTable &t)
{
    if (t.magic != kCcMagic || t.n < 2 || t.n > 8)
        return false;
    for (int i = 0; i < t.n; i++)
    {
        if (!(t.set[i] > 0) || !(t.act[i] > 0))
            return false;
        if (i && !(t.act[i] > t.act[i - 1] && t.set[i] > t.set[i - 1]))
            return false;
    }
    return true;
}

static void table_publish(const CcTable &t)
{
    s_buf[s_bufNext] = t;
    s_active.store(&s_buf[s_bufNext]);
    s_bufNext ^= 1;
}

double ccLimitCorrect(double amps)
{
    const CcTable *t = s_active.load();
    if (!t)
        return amps;
    // Below the first point: same ratio, shrinking the correction to 0 at 0 A
    if (amps <= t->act[0])
        return amps * t->set[0] / t->act[0];
    int i = 0;
    while (i < t->n - 2 && amps > t->act[i + 1])
        i++;
    // Inside a segment, or past the last point along the last segment
    return t->set[i] + (amps - t->act[i]) * (t->set[i + 1] - t->set[i]) / (t->act[i + 1] - t->act[i]);
}

void ccLimitLoad()
{
    CcTable t{};
    PowerSupply.StoreMem.begin("my-app", true);
    const size_t got = PowerSupply.StoreMem.getBytes(kCcKey, &t, sizeof(t));
    PowerSupply.StoreMem.end();
    if (got == sizeof(t) && table_valid(t))
    {
        table_publish(t);
        Serial.printf("\nCC limit table: %d points", t.n);
    }
}

static void table_save(const CcTable &t)
{
    PowerSupply.StoreMem.begin("my-app", false);
    PowerSupply.StoreMem.putBytes(kCcKey, &t, sizeof(t));
    PowerSupply.StoreMem.end();
}

void ccLimitClear()
{
    s_active.store(nullptr);
    PowerSupply.StoreMem.begin("my-app", false);
    PowerSupply.StoreMem.remove(kCcKey);
    PowerSupply.StoreMem.end();
    Serial.println("\nCC limit table cleared");
}

String ccLimitTableString()
{
    const CcTable *t = s_active.load();
    String s;
    if (!t)
        return s;
    for (int i = 0; i < t->n; i++)
    {
        char buf[32];
        snprintf(buf, sizeof(buf), "%s%.4f,%.5f", i ? ";" : "", t->set[i], t->act[i]);
        s += buf;
    }
    return s;
}
// ────────────────────────────────────────────────────────────────────────────

// ─── Sequence ───────────────────────────────────────────────────────────────
static double s_act[kCcN];
static double s_v[kCcN];
static const CcTable *s_prev = nullptr; // table in use before the run
static bool s_failed = false;
static bool s_savedMA = false;
static uint16_t s_savedV = 0, s_savedI = 0;
static lv_timer_t *s_timer = nullptr;
static std::vector<SeqStep> s_steps;

static void set_volts(double v) { PowerSupply.Voltage.SetUpdate(v * PowerSupply.Voltage.adjFactor + PowerSupply.Voltage.adjOffset); }
static void set_amps(double a) { PowerSupply.Current.SetUpdate(a * PowerSupply.Current.adjFactor + PowerSupply.Current.adjOffset); }

// Stop at the current step: the sequencer has no abort, so point it at the
// step before "Finish"; seq_cb() advances to Finish after this end() returns.
static void fail(const char *fmt, double a, double b)
{
    s_failed = true;
    log_step(fmt, a, b);
    Serial.printf("\nCC limit cal failed: ");
    Serial.printf(fmt, a, b);
    if (s_timer && s_timer->user_data)
        static_cast<SeqRunner *>(s_timer->user_data)->index = s_steps.size() - 2;
}

static void cc_finish()
{
    esp_task_wdt_reset();
    set_amps(kCcIdleA);

    if (!s_failed)
    {
        CcTable t{};
        t.magic = kCcMagic;
        t.n = kCcN;
        for (int i = 0; i < kCcN; i++)
        {
            t.set[i] = kCcSetA[i];
            t.act[i] = s_act[i];
            log_step("%.1fA -> %.4fA (%+.1fmA)", kCcSetA[i], s_act[i], 1000.0 * (s_act[i] - kCcSetA[i]));
            Serial.printf("\n  set %.3fA act %.5fA V %.4f", kCcSetA[i], s_act[i], s_v[i]);
        }
        if (table_valid(t))
        {
            table_publish(t);
            table_save(t);
            log_step("Saved, limit corrected");
        }
        else
        {
            s_failed = true;
            log_step("Not monotonic, nothing changed");
        }
    }
    if (s_failed)
        s_active.store(s_prev); // keep what was there

    // Back to the user's setpoints and range
    PowerSupply.Voltage.SetUpdate((int)s_savedV);
    PowerSupply.Current.SetUpdate((int)s_savedI);
    if (s_savedMA)
        setCurrentRange(true);
    calib_low_noise_end();
    s_timer = nullptr;
    g_calibration_in_progress = false;

    lv_timer_t *close_t = lv_timer_create(close_log_cb, s_failed ? 15000 : 8000, nullptr);
    if (close_t)
        lv_timer_set_repeat_count(close_t, 1);
}

static void build_steps()
{
    static char labels[kCcN][2][32];
    s_steps.push_back({"Preparing (A range)", 3000, 500,
                       []() {
                           set_amps(kCcIdleA);
                           set_volts(kCcVset);
                       },
                       nullptr});
    for (int i = 0; i < kCcN; i++)
    {
        snprintf(labels[i][0], sizeof(labels[i][0]), "Limit %gA", kCcSetA[i]);
        snprintf(labels[i][1], sizeof(labels[i][1]), "Measuring at %gA", kCcSetA[i]);
        s_steps.push_back({labels[i][0], CC_SETTLE_MS, 0,
                           [i]() { set_amps(kCcSetA[i]); }, nullptr});
        s_steps.push_back({labels[i][1], CC_MEASURE_MS, CC_COOL_MS,
                           []() {
                               PowerSupply.Current.Statistics.ResetStats();
                               PowerSupply.Voltage.Statistics.ResetStats();
                           },
                           [i]() {
                               s_act[i] = PowerSupply.Current.Statistics.Mean();
                               s_v[i] = PowerSupply.Voltage.Statistics.Mean();
                               set_amps(kCcIdleA);
                               if (s_v[i] > kCcShortMaxV)
                                   fail("Output not shorted (%.2fV at %.1fA)", s_v[i], kCcSetA[i]);
                               else if (fabs(s_act[i] / kCcSetA[i] - 1.0) > kCcMaxDev)
                                   fail("%.3fA at %.1fA set: not a CC limit", s_act[i], kCcSetA[i]);
                           }});
    }
    s_steps.push_back({"Finish", 100, 100, nullptr, cc_finish});
}

bool start_cc_limit_calibration()
{
    if (PowerSupply.getStatus() == DEVICE::OFF || PowerSupply.getStatus() == DEVICE::FUN)
        return false;

    esp_task_wdt_reset();
    s_failed = false;
    s_savedV = PowerSupply.Voltage.adjValue;
    s_savedI = PowerSupply.Current.adjValue;
    s_savedMA = PowerSupply.mA_Active;
    if (s_savedMA)
        setCurrentRange(false);
    calib_low_noise_begin();
    // Measure the uncorrected limit
    s_prev = s_active.load();
    s_active.store(nullptr);

    if (s_steps.empty())
        build_steps();
    s_timer = lv_timer_create(seq_cb, 50, nullptr);
    if (!s_timer)
    {
        Serial.println("ERROR: Failed to create CC limit seq timer (LVGL heap full)");
        s_failed = true;
        cc_finish();
        return false;
    }
    seq_start(s_timer, s_steps.data(), s_steps.size(), nullptr);
    return true;
}
// ────────────────────────────────────────────────────────────────────────────

static void cc_msgbox_cb(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_current_target(e);
    const char *txt = lv_msgbox_get_active_btn_text(obj);
    const bool start = txt && strcmp(txt, "Start") == 0;
    const bool clear = txt && strcmp(txt, "Clear") == 0;
    msgbox_close_deferred(obj);

    if (clear)
    {
        ccLimitClear();
        return;
    }
    if (!start || g_calibration_in_progress)
        return;
    g_calibration_in_progress = true;
    create_log_window("CC Limit (short)");
    log_reset();
    log_clear();
    if (!start_cc_limit_calibration())
    {
        log_step("Turn the output ON first");
        g_calibration_in_progress = false;
    }
}

void cc_limit_calibration_cb(lv_event_t *)
{
    esp_task_wdt_reset();
    static const char *btns[] = {"Start", "Clear", "Cancel", ""};
    char msg[160];
    snprintf(msg, sizeof(msg),
             "Short the output with a thick wire, output ON.\n"
             "Steps the limit 0.1-5 A (~2 min).\n%s",
             s_active.load() ? "Table stored." : "No table stored.");
    lv_obj_t *mbox = lv_msgbox_create(NULL, "CC Limit Calibration", msg, btns, false);
    lv_obj_add_event_cb(mbox, cc_msgbox_cb, LV_EVENT_VALUE_CHANGED, nullptr);
    lv_obj_center(mbox);
}
