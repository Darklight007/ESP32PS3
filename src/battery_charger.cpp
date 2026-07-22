// Li-ion battery CC/CV charger + tester
// - CC/CV charge with pre-charge (trickle) for deep-discharged cells
// - Charge counter (mAh / Wh) and elapsed time
// - Internal resistance (IR): 5-point current sweep + least-squares fit,
//   both as a standalone test and as a periodic dip during charging
// - Termination on taper current, over-voltage fault, safety timeout
//
// The PSU itself is the CC/CV loop: we set V = cells*V/cell and I = charge
// current, the analog CC/CV crossover does the rest. This module only
// supervises, integrates charge and decides when to stop.
//
// THREADING: everything here runs on Core 1 (LVGL events + main-loop tick).

#include "battery_charger.h"
#include "device.hpp"
#include "globals.h"
#include "spinbox_pro.h"
#include "buzzer.h"
#include "intervals.h"
#include "setting_menu.h" // msgbox_close_deferred(), for the lead-R calibration confirm dialog
#include <Preferences.h>
#include <Arduino.h>
#include <cmath>

extern Device PowerSupply;

// ---------- Settings (raw spinbox units) ----------
static int32_t s_cells = 1;      // 1..8 series cells
static int32_t s_mv_cell = 4200; // CV target per cell [mV]
static int32_t s_chg_mA = 500;   // charge current [mA]
static int32_t s_term_mA = 50;   // termination (taper) current [mA]
static int32_t s_timeout_min = 300; // safety timeout [min]

// ---------- State machine ----------
enum class BatState { IDLE, TRICKLE, CC_CV, IR_SWEEP, DONE, FAULT, LEAD_CAL };
static BatState s_state = BatState::IDLE;
static const char *stateName(BatState s)
{
    switch (s) {
    case BatState::IDLE: return "IDLE";
    case BatState::TRICKLE: return "PRE-CHG";
    case BatState::CC_CV: return "CC/CV";
    case BatState::IR_SWEEP: return "IR";
    case BatState::DONE: return "DONE";
    case BatState::LEAD_CAL: return "LEADCAL";
    default: return "FAULT";
    }
}

static double s_mAh = 0, s_Wh = 0;
static unsigned long s_startMs = 0, s_lastIntMs = 0, s_phaseMs = 0;
static int s_termCount = 0;
static double s_ir_mOhm = NAN;
static const char *s_faultMsg = "";

// This test is deliberately battery-agnostic — it's also used as a smoke test
// against a plain resistive load. Two regimes:
//  - Current actually swept by a useful amount: least-squares slope R = dV/dI,
//    same as before — this is the precise path, used for real battery IR.
//  - Current barely moved (compliance-limited: high-resistance load, or nothing
//    at all on the terminals): the slope's denominator is near-zero and noise-
//    dominated, so fall back to a plain Ohm's-law point estimate R = V/I on the
//    settled readings instead. For a big resistor or an open circuit this
//    naturally comes out large (hundreds of ohms to kOhm+) rather than being
//    rejected — a very high result IS the correct answer in that case, not an
//    error state.
static double computeIrMOhm(const double *I, const double *V, int nPts)
{
    double iMin = I[0], iMax = I[0];
    for (int i = 1; i < nPts; i++) { if (I[i] < iMin) iMin = I[i]; if (I[i] > iMax) iMax = I[i]; }

    if ((iMax - iMin) >= 0.02) { // >=20mA spread: enough dynamic range for the precise slope fit
        double sumI = 0, sumV = 0, sumII = 0, sumIV = 0;
        for (int i = 0; i < nPts; i++) {
            sumI += I[i]; sumV += V[i];
            sumII += I[i] * I[i]; sumIV += I[i] * V[i];
        }
        double n = nPts;
        double denom = n * sumII - sumI * sumI;
        if (fabs(denom) > 1e-9) {
            double mOhm = (n * sumIV - sumI * sumV) / denom * 1000.0;
            if (std::isfinite(mOhm) && mOhm >= 0.0) return mOhm;
        }
        // Slope came out unusable despite the spread check — fall through.
    }

    // Point estimate: average of the settled (I, V) readings.
    double sumI = 0, sumV = 0;
    for (int i = 0; i < nPts; i++) { sumI += I[i]; sumV += V[i]; }
    double iAvg = sumI / nPts, vAvg = sumV / nPts;
    if (iAvg < 1e-5) return 9.9e6; // no measurable current at all -> essentially open
    return (vAvg / iAvg) * 1000.0;
}

// Standalone IR test: sweep several current levels (not just 2), then fit a
// line to all the (I,V) points. Least-squares slope is far less sensitive to
// noise at any single point than a bare two-point secant.
static const double kIrFracs[] = {0.15, 0.35, 0.6, 0.85, 1.0}; // fractions of Charge [A]
static const int kIrPoints = sizeof(kIrFracs) / sizeof(kIrFracs[0]);
static double s_irStepI[kIrPoints], s_irStepV[kIrPoints];
static int s_irStepIdx = 0;
// Settle time per point: with #Avgs=1 (confirmed on the status bar — no
// rolling multi-sample window to refill), a reading is fresh as soon as the
// CC/CV loop restabilizes and one new ADC sample lands. 250ms covers both
// with margin. Raise this back up if SPS/#Avgs ever changes to something
// with real averaging.
static const unsigned long kIrDwellMs = 500;

// Live IR during charge: every 10 s dip the current setpoint to the low level
// for ~1.2 s and compute IR from the two (V, I) points.
static unsigned long s_liveIrNextMs = 0, s_liveIrPhaseMs = 0, s_liveIrCooldownMs = 0;
static int s_liveIrPhase = 0;  // 0 = normal charging, 1 = mid-sweep
static double s_liveIrAmps0 = 0; // actual operating current at the moment the sweep started

// ---------- UI ----------
static lv_obj_t *s_mah_label = nullptr;   // details line, left: charge counter
static lv_obj_t *s_timer_label = nullptr; // details line, middle: elapsed time
static lv_obj_t *s_ir_label = nullptr;    // details line, right: internal resistance
static lv_obj_t *s_fault_label = nullptr; // full-width, replaces the details line when s_faultMsg is set
static lv_obj_t *s_big_label = nullptr;    // large V/A readout
static lv_obj_t *s_chip = nullptr;         // colored state badge
static lv_obj_t *s_chip_label = nullptr;
static lv_obj_t *s_bar = nullptr;       // charge progress bar (pro-charger style)
static lv_obj_t *s_pct_label = nullptr; // percent estimate
static lv_obj_t *s_chg_btn = nullptr;
static lv_obj_t *s_chg_btn_label = nullptr;
static lv_obj_t *s_ir_btn = nullptr;
static lv_obj_t *s_tab = nullptr;
static lv_obj_t *s_wh_label = nullptr;  // panel, replaces the timer's old spot
static lv_obj_t *s_chem_dd = nullptr;   // battery chemistry preset dropdown
static lv_obj_t *s_target_sb = nullptr; // Target [V] spinbox (chem presets write to this)

// Chemistry presets: name shown in the dropdown -> V/cell target in mV.
// mv < 0 ("Custom") leaves whatever the user already dialed into Target [V].
// Values are full-charge/CV targets per cell (sourced 2026-07: manufacturer
// datasheets + battery-university.com):
//   Li-ion 4.20V, Li-HV 4.35V, LiFePO4 3.65V — standard CC/CV tops.
//   NiMH/NiCd don't really use CV (they terminate on -dV/dt); 1.45V is a
//   safe peak-voltage cap for a CV-only charger like this one, not a "nominal"
//   voltage (nominal NiMH/NiCd is 1.2V — that's what's printed on the cell).
struct ChemPreset { const char *name; int mv; };
static const ChemPreset s_chemPresets[] = {
    {"Li-ion",   4200},
    {"Li-HV",    4350},
    {"LiFePO4",  3650},
    {"LTO",      2400},
    {"NiMH",     1450},
    {"NiCd",     1450},
    {"Custom",     -1},
};
static const int s_numChemPresets = sizeof(s_chemPresets) / sizeof(s_chemPresets[0]);

static lv_color_t stateColor(const char *nm)
{
    if (!strcmp(nm, "CC")) return lv_color_hex(0xFFA000);      // amber
    if (!strcmp(nm, "CV")) return lv_color_hex(0x0090B0);      // cyan
    if (!strcmp(nm, "IR")) return lv_color_hex(0x7040C0);      // purple dip
    if (!strcmp(nm, "LEADCAL")) return lv_color_hex(0x7040C0); // same purple as IR
    if (!strcmp(nm, "PRE-CHG")) return lv_color_hex(0xE06000); // orange
    if (!strcmp(nm, "CC/CV")) return lv_color_hex(0xFFA000);
    if (!strcmp(nm, "DONE")) return lv_color_hex(0x00A048);    // green
    if (!strcmp(nm, "FAULT")) return lv_color_hex(0xC80000);   // red
    if (!strcmp(nm, "HIGH V!")) return lv_color_hex(0xC80000); // red: ext output above Batt target
    if (!strcmp(nm, "EXT ON")) return lv_color_hex(0xB07000);  // output on, not supervised
    return lv_color_hex(0x505050);                             // IDLE grey
}

static double targetVolts();
static double chargeAmps();

// Charge % estimate: CC phase maps Vbat 3.0V->target as 0..80%,
// CV phase maps current taper Ichg->cutoff as 80..100%.
static double chargePct(double vbat, double amps)
{
    if (s_state == BatState::DONE) return 100.0;
    double vt = targetVolts();
    double pct;
    if (vbat < vt * 0.985)
        pct = 80.0 * (vbat - 3.0) / fmax(0.1, vt - 3.0);
    else {
        double hi = chargeAmps(), lo = s_term_mA / 1000.0;
        pct = 80.0 + 20.0 * (1.0 - (amps - lo) / fmax(0.001, hi - lo));
    }
    return fmin(100.0, fmax(0.0, pct));
}

// Spinbox ids inside the Batt tab (searched per-parent, keep unique here)
enum { ID_CELLS = 10, ID_VCELL, ID_ICHG, ID_ITERM, ID_TMOUT };

// ---------- Helpers ----------
static bool outputIsOn()
{
    DEVICE st = PowerSupply.getStatus();
    return st == DEVICE::ON || st == DEVICE::VC || st == DEVICE::CC;
}

static void ensureOutput(bool on)
{
    if (outputIsOn() != on)
        PowerSupply.toggle(); // full path: button event + setStatus (Core 1 only)
}

static void setSetpoints(double volts, double amps)
{
    int32_t vc = (int32_t)lround(volts * PowerSupply.Voltage.adjFactor + PowerSupply.Voltage.adjOffset);
    int32_t ic = (int32_t)lround(amps * PowerSupply.Current.adjFactor + PowerSupply.Current.adjOffset);
    vc = constrain(vc, 0, 65535);
    ic = constrain(ic, 0, 65535);
    // bypassLock=true: this is the charger's own CC/CV control, not user
    // input. SetUpdate() has its own built-in lock check (blocks all callers
    // by default) — the auto-lock engaged for THIS exact charge/test would
    // otherwise silently block the charger from ever changing its own
    // setpoint, freezing the IR sweep's current steps and any CC/CV action.
    PowerSupply.Voltage.SetUpdate(vc, true);
    PowerSupply.Current.SetUpdate(ic, true);
}

static double measuredAmps()
{
    double i = PowerSupply.Current.measured.value;
    if (PowerSupply.mA_Active)
        i /= 1000.0;
    return i;
}

static void readSettings()
{
    s_cells = 1; // single-cell only (Cells spinbox removed)
    s_mv_cell = get_spinbox_data_by_id(s_tab, ID_VCELL);
    s_chg_mA = get_spinbox_data_by_id(s_tab, ID_ICHG);
    s_term_mA = get_spinbox_data_by_id(s_tab, ID_ITERM);
    s_timeout_min = get_spinbox_data_by_id(s_tab, ID_TMOUT);
}

static void saveSettings()
{
    Preferences p;
    p.begin("batt", false);
    p.putInt("c", s_cells);
    p.putInt("vc", s_mv_cell);
    p.putInt("ic", s_chg_mA);
    p.putInt("it", s_term_mA);
    p.putInt("to", s_timeout_min);
    p.end();
}

static void loadSettings()
{
    Preferences p;
    p.begin("batt", true);
    s_cells = p.getInt("c", 1);
    s_mv_cell = p.getInt("vc", 4200);
    s_chg_mA = p.getInt("ic", 500);
    s_term_mA = p.getInt("it", 50);
    s_timeout_min = p.getInt("to", 300);
    p.end();
}

static double targetVolts() { return s_cells * (s_mv_cell / 1000.0); }
static double chargeAmps() { return s_chg_mA / 1000.0; }
static double trickleAmps() { return fmax(0.05, chargeAmps() / 10.0); }

static void syncChargeBtn(bool checked)
{
    if (!s_chg_btn) return;
    if (checked) lv_obj_add_state(s_chg_btn, LV_STATE_CHECKED);
    else lv_obj_clear_state(s_chg_btn, LV_STATE_CHECKED);
    if (s_chg_btn_label)
        lv_label_set_text(s_chg_btn_label, checked ? "STOP" : "CHARGE");
}

static void stopAll(BatState endState, const char *msg = "")
{
    ensureOutput(false);
    s_state = endState;
    s_faultMsg = msg;
    syncChargeBtn(false);
    if (endState == BatState::DONE) { myTone(NOTE_A5, 200, true); myTone(NOTE_A5, 200, true); }
    if (endState == BatState::FAULT) { myTone(NOTE_C5, 400, true); }
}

static bool prechecksOk()
{
    if (PowerSupply.getStatus() == DEVICE::FUN) { s_faultMsg = "FGen active"; return false; }
    if (PowerSupply.mA_Active) { s_faultMsg = "switch to A range"; return false; }
    return true;
}

bool batteryChargerActive()
{
    return s_state == BatState::TRICKLE || s_state == BatState::CC_CV ||
           s_state == BatState::IR_SWEEP || s_state == BatState::LEAD_CAL;
}

// ---------- Start actions ----------
static void startCharge()
{
    readSettings();
    saveSettings();
    if (!prechecksOk()) { s_state = BatState::FAULT; syncChargeBtn(false); return; }

    double vbat = PowerSupply.Voltage.measured.value; // output off → battery voltage
    double vt = targetVolts();
    if (vbat > vt + 0.1) { stopAll(BatState::FAULT, "Vbat > target"); return; }

    s_mAh = 0; s_Wh = 0;
    s_startMs = millis();
    s_lastIntMs = 0;
    s_termCount = 0;
    s_faultMsg = "";
    s_liveIrPhase = 0;
    s_liveIrNextMs = millis() + 10000;

    if (vbat < 2.5 * s_cells && vbat > 0.5) {
        setSetpoints(vt, trickleAmps()); // deep discharged → gentle pre-charge
        s_state = BatState::TRICKLE;
    } else {
        setSetpoints(vt, chargeAmps());
        s_state = BatState::CC_CV;
    }
    ensureOutput(true);
    syncChargeBtn(true);
}

// ---------- Lead-resistance calibration (Settings > Calibration menu) ----------
// Short the PS output leads together and measure the residual lead+contact
// resistance, then persist it so it can be subtracted from every future
// battery IR reading. Runs as its own dedicated state (not reusing IR_SWEEP)
// so it can be tuned purely for calibration accuracy without touching the
// battery IR test at all:
//   - Higher test current (2A vs the battery test's fractions of Charge[A]):
//     V = I*R, so more current directly multiplies the tiny voltage signal
//     from a few-mOhm short against the ADC's roughly fixed noise floor —
//     the single biggest lever on precision here.
//   - More sweep points (10) across a wider current range: the standard
//     error of a least-squares slope shrinks as the spread of the x-values
//     (current, here) grows, for a given noise level.
//   - Each recorded point is the average of every raw sample taken during
//     its whole dwell window, not just one instantaneous reading at the end.
//   - The entire sweep runs 3 times and the resulting R values are averaged
//     (further sqrt(N) noise reduction) — and if the 3 runs disagree by more
//     than a loose tolerance, the result says so instead of quietly trusting
//     a possibly-bad measurement (loose short, flaky contact, etc).
static float s_leadR_mOhm = 0.0f;
static const double kLeadCalTargetV = 2.0; // safe headroom for a near-zero-ohm short
static const double kLeadCalMaxA = 2.0;    // raised from 0.3A: ~6.7x more signal for the same lead R
static const double kLeadCalFracs[] = {0.05, 0.15, 0.25, 0.35, 0.45, 0.55, 0.65, 0.75, 0.875, 1.0};
static const int kLeadCalPoints = sizeof(kLeadCalFracs) / sizeof(kLeadCalFracs[0]);
static double s_leadCalI[kLeadCalPoints], s_leadCalV[kLeadCalPoints];
static int s_leadCalStepIdx = 0;
static const unsigned long kLeadCalDwellMs = 600; // ~2-3 raw samples averaged per point at the 250ms tick rate
static double s_leadCalSumI = 0, s_leadCalSumV = 0;
static int s_leadCalSampleCount = 0;
static const int kLeadCalRepeats = 3;
static int s_leadCalRepeatIdx = 0;
static double s_leadCalRepeatMOhm[kLeadCalRepeats];
static lv_obj_t *s_leadCalMbox = nullptr;  // progress window shown while the sweep runs
static lv_obj_t *s_leadCalWin = nullptr;   // persistent "current value + recalibrate" window
static lv_obj_t *s_leadCalValueLabel = nullptr;

static void updateLeadCalValueLabel()
{
    if (!s_leadCalValueLabel || !lv_obj_is_valid(s_leadCalValueLabel)) return;
    lv_label_set_text_fmt(s_leadCalValueLabel, "%.1f mΩ", (double)s_leadR_mOhm); // graph_R_16 has the Ω glyph
}

static void loadLeadR()
{
    Preferences p;
    p.begin("batt", true);
    s_leadR_mOhm = p.getFloat("lr", 0.0f);
    p.end();
}

static void saveLeadR()
{
    Preferences p;
    p.begin("batt", false);
    p.putFloat("lr", s_leadR_mOhm);
    p.end();
}

void startBattLeadCal()
{
    if (batteryChargerActive()) return; // don't interrupt an active charge/IR test
    s_leadCalStepIdx = 0;
    s_leadCalRepeatIdx = 0;
    s_leadCalSumI = 0; s_leadCalSumV = 0; s_leadCalSampleCount = 0;

    // Progress window (no buttons — the sweep is short and self-terminating).
    // Updated each tick from batteryTick(); replaced by the result box at the end.
    // No title: the "PS Lead Resistance" window is still open behind this.
    s_leadCalMbox = lv_msgbox_create(NULL, "", "Measuring...  sweep 1/3, step 1/10", NULL, false);
    lv_obj_set_width(s_leadCalMbox, 240);
    lv_obj_center(s_leadCalMbox);

    setSetpoints(kLeadCalTargetV, fmax(0.02, kLeadCalMaxA * kLeadCalFracs[0]));
    ensureOutput(true);
    s_phaseMs = millis();
    s_state = BatState::LEAD_CAL;
}

static void leadCalConfirm_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_VALUE_CHANGED) return;
    lv_obj_t *obj = lv_event_get_current_target(e);
    if (!obj) return;
    const char *btn_txt = lv_msgbox_get_active_btn_text(obj);
    bool is_ok = (btn_txt && strcmp(btn_txt, "OK") == 0);
    msgbox_close_deferred(obj);
    if (is_ok) startBattLeadCal();
}

static void leadCalRecalibrate_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    // Title left off — the window open behind this dialog already says
    // "PS Lead Resistance" in its own title bar; repeating it here just reads
    // as a rendering glitch. Text kept short and given an explicit width so
    // it wraps at word boundaries instead of splitting mid-word.
    static const char *btns[] = {"OK", "Cancel", ""};
    lv_obj_t *mbox = lv_msgbox_create(NULL, "", "Short the leads together,\nthen press OK.", btns, false);
    lv_obj_set_width(mbox, 240);
    lv_obj_add_event_cb(mbox, leadCalConfirm_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_center(mbox);
}

// Settings > Calibration entry point: a persistent window showing the current
// calibrated lead resistance plus a button to remeasure it, instead of jumping
// straight into a one-shot confirm dialog with nothing to look at afterward.
void battLeadCalMenu_cb(lv_event_t *)
{
    if (s_leadCalWin && lv_obj_is_valid(s_leadCalWin)) {
        updateLeadCalValueLabel();
        lv_obj_clear_flag(s_leadCalWin, LV_OBJ_FLAG_HIDDEN);
        return;
    }

    // Match the other calibration windows' near-full-screen size (320x226) —
    // anything smaller lets the Settings menu behind it show through the gaps.
    s_leadCalWin = lv_win_create(lv_scr_act(), 36);
    lv_obj_set_size(s_leadCalWin, 320, 226);
    lv_win_add_title(s_leadCalWin, "PS Lead Resistance");
    auto *close = lv_win_add_btn(s_leadCalWin, LV_SYMBOL_CLOSE, 60);
    lv_obj_add_event_cb(close, btn_close_hide_obj_cb, LV_EVENT_CLICKED, nullptr);
    auto *cont = lv_win_get_content(s_leadCalWin);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *caption = lv_label_create(cont);
    lv_label_set_text(caption, "Current calibrated value:");
    lv_obj_align(caption, LV_ALIGN_TOP_MID, 0, 16);

    s_leadCalValueLabel = lv_label_create(cont);
    lv_obj_set_style_text_font(s_leadCalValueLabel, &graph_R_16, 0);
    lv_obj_align(s_leadCalValueLabel, LV_ALIGN_TOP_MID, 0, 40);
    updateLeadCalValueLabel();

    lv_obj_t *hint = lv_label_create(cont);
    lv_label_set_long_mode(hint, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(hint, 280);
    lv_obj_set_style_text_align(hint, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(hint, "Subtracted from every \nbattery IR reading.");
    lv_obj_align(hint, LV_ALIGN_TOP_MID, 0, 78);

    lv_obj_t *btn = lv_btn_create(cont);
    lv_obj_set_size(btn, 180, 40);
    lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, 120);
    lv_obj_t *btnLbl = lv_label_create(btn);
    lv_label_set_text(btnLbl, "Recalibrate");
    lv_obj_center(btnLbl);
    lv_obj_add_event_cb(btn, leadCalRecalibrate_cb, LV_EVENT_CLICKED, nullptr);
}

static void startIrTest()
{
    readSettings();
    saveSettings();
    if (!prechecksOk()) { s_state = BatState::FAULT; return; }
    if (batteryChargerActive()) return; // not while charging

    // Deliberately no "is a battery attached" gate here — this test doubles as a
    // smoke test against a plain resistive load (or nothing at all), and
    // computeIrMOhm() already reports a correctly large value for either case
    // instead of needing to detect/reject them up front.
    s_ir_mOhm = NAN;
    s_faultMsg = "";
    s_irStepIdx = 0;
    setSetpoints(targetVolts(), fmax(0.05, chargeAmps() * kIrFracs[0]));
    ensureOutput(true);
    s_phaseMs = millis();
    s_state = BatState::IR_SWEEP;
}

// ---------- Periodic tick (Core 1) ----------
static void batteryTick()
{
    // User killed the output (O key / touch) while we were running → abort
    if (batteryChargerActive() && !outputIsOn()) {
        s_state = BatState::IDLE;
        syncChargeBtn(false);
    }

    double vbat = PowerSupply.Voltage.measured.value;
    double amps = measuredAmps();
    double vt = targetVolts();
    unsigned long now = millis();

    switch (s_state) {
    case BatState::TRICKLE:
    case BatState::CC_CV: {
        // Integrate charge
        if (s_lastIntMs != 0) {
            double dtH = (now - s_lastIntMs) / 3600000.0;
            if (amps > 0) {
                s_mAh += amps * 1000.0 * dtH;
                s_Wh += amps * vbat * dtH;
            }
        }
        s_lastIntMs = now;

        // Safety
        if (vbat > vt + 0.25 * s_cells) { stopAll(BatState::FAULT, "over-voltage"); break; }
        if (s_timeout_min > 0 && (now - s_startMs) > (unsigned long)s_timeout_min * 60000UL) {
            stopAll(BatState::FAULT, "timeout"); break;
        }

        if (s_state == BatState::TRICKLE) {
            if (vbat >= 3.0 * s_cells) { // recovered → full current
                setSetpoints(vt, chargeAmps());
                s_state = BatState::CC_CV;
            }
            break;
        }

        // Live IR: periodic 5-point sweep while still charging (same
        // least-squares fit as the standalone IR TEST). Levels scale off the
        // ACTUAL operating current (not the charge setting) and only ever
        // dip below what's already flowing, so it works throughout the CV
        // taper without asking for more current than the loop is already
        // giving. Skipped below ~60mA — too little current to get usable dI.
        if (s_liveIrPhase == 0 && now >= s_liveIrNextMs && amps > 0.06) {
            s_liveIrPhase = 1;
            s_irStepIdx = 0;
            s_liveIrAmps0 = amps;
            setSetpoints(vt, fmax(0.02, s_liveIrAmps0 * kIrFracs[0]));
            s_liveIrPhaseMs = now;
        } else if (s_liveIrPhase == 1 && now - s_liveIrPhaseMs >= kIrDwellMs) {
            s_irStepI[s_irStepIdx] = amps;
            s_irStepV[s_irStepIdx] = vbat;
            s_irStepIdx++;
            if (s_irStepIdx >= kIrPoints) {
                // R = +slope, not -slope: this is a CHARGER (current flows into
                // the battery), so terminal V rises with I (V = Voc_chg + I*R) —
                // opposite of the textbook discharge convention (V = Voc - I*R).
                s_ir_mOhm = computeIrMOhm(s_irStepI, s_irStepV, kIrPoints);
                setSetpoints(vt, chargeAmps()); // restore full charge-current ceiling
                s_liveIrPhase = 0;
                // Let the setpoint settle before the bar reacts again. 300ms wasn't
                // quite enough margin beyond the loop's own restabilize time (the
                // per-step dwell during the sweep itself is 500ms) — the first
                // reading right at the edge of the old cooldown could still catch
                // amps mid-transition back to the real taper current, showing up
                // as a visible jump/jitter right after each dip.
                s_liveIrCooldownMs = now + 600;
                // Space sweeps out as the pack fills: frequent early on, rare near full
                // (avoids interrupting the charge every 10s once it barely matters).
                int pct = chargePct(vbat, amps);
                s_liveIrNextMs = now + (pct < 50 ? 10000 : 60000);
            } else {
                setSetpoints(vt, fmax(0.02, s_liveIrAmps0 * kIrFracs[s_irStepIdx]));
                s_liveIrPhaseMs = now;
            }
        }

        // Termination: in CV (voltage reached) and current tapered below limit.
        // Never count during an IR dip — the forced low current would fake it.
        if (s_liveIrPhase == 0 &&
            vbat >= vt * 0.985 && amps >= 0 && amps <= s_term_mA / 1000.0)
            s_termCount++;
        else if (s_liveIrPhase == 0)
            s_termCount = 0;
        if (s_termCount >= 8) // ~2 s of consecutive taper hits
            stopAll(BatState::DONE);
        break;
    }

    case BatState::IR_SWEEP:
        if (now - s_phaseMs >= kIrDwellMs) { // this point settled
            s_irStepI[s_irStepIdx] = amps;
            s_irStepV[s_irStepIdx] = vbat;
            s_irStepIdx++;
            if (s_irStepIdx >= kIrPoints) {
                // Least-squares line V = a + b*I over all points; R = +b for a
                // CHARGER (current into the battery raises terminal V, unlike
                // the textbook discharge convention V = Voc - I*R). Falls back to
                // a V/I point estimate when current couldn't be swept — see
                // computeIrMOhm(): that's the resistor/no-load smoke-test path,
                // and correctly reports a large value there rather than an error.
                s_ir_mOhm = computeIrMOhm(s_irStepI, s_irStepV, kIrPoints);
                ensureOutput(false);
                s_state = BatState::IDLE;
                myTone(NOTE_A3, 100, true);
            } else {
                setSetpoints(vt, fmax(0.05, chargeAmps() * kIrFracs[s_irStepIdx]));
                s_phaseMs = now;
            }
        }
        break;

    case BatState::LEAD_CAL:
        // Accumulate every tick (not just at dwell-end) so each recorded point
        // is the average of several raw samples instead of one noisy read.
        s_leadCalSumI += amps;
        s_leadCalSumV += vbat;
        s_leadCalSampleCount++;

        if (s_leadCalMbox && lv_obj_is_valid(s_leadCalMbox)) {
            lv_obj_t *txt = lv_msgbox_get_text(s_leadCalMbox);
            if (txt) lv_label_set_text_fmt(txt, "Measuring...  sweep %d/%d, step %d/%d",
                                           s_leadCalRepeatIdx + 1, kLeadCalRepeats,
                                           s_leadCalStepIdx + 1, kLeadCalPoints);
        }

        if (now - s_phaseMs >= kLeadCalDwellMs) { // this point settled
            s_leadCalI[s_leadCalStepIdx] = s_leadCalSumI / s_leadCalSampleCount;
            s_leadCalV[s_leadCalStepIdx] = s_leadCalSumV / s_leadCalSampleCount;
            s_leadCalStepIdx++;
            s_leadCalSumI = 0; s_leadCalSumV = 0; s_leadCalSampleCount = 0;

            if (s_leadCalStepIdx >= kLeadCalPoints) {
                // One full sweep done. Either kick off the next repeat, or —
                // if that was the last one — average all repeats and finish.
                s_leadCalRepeatMOhm[s_leadCalRepeatIdx] = computeIrMOhm(s_leadCalI, s_leadCalV, kLeadCalPoints);
                s_leadCalRepeatIdx++;

                if (s_leadCalRepeatIdx < kLeadCalRepeats) {
                    s_leadCalStepIdx = 0;
                    setSetpoints(kLeadCalTargetV, fmax(0.02, kLeadCalMaxA * kLeadCalFracs[0]));
                    s_phaseMs = now;
                } else {
                    double sum = 0;
                    for (int i = 0; i < kLeadCalRepeats; i++) sum += s_leadCalRepeatMOhm[i];
                    double mOhm = sum / kLeadCalRepeats;

                    // Repeatability check: how much do the individual sweeps
                    // disagree with each other? Flags a loose short, flaky
                    // contact, or a genuinely noisy measurement instead of
                    // silently trusting an average that hides real spread.
                    double maxDev = 0;
                    for (int i = 0; i < kLeadCalRepeats; i++)
                        maxDev = fmax(maxDev, fabs(s_leadCalRepeatMOhm[i] - mOhm));
                    bool noisy = maxDev > fmax(0.5, mOhm * 0.15);

                    float oldLeadR = s_leadR_mOhm;
                    s_leadR_mOhm = (float)mOhm;
                    saveLeadR();
                    updateLeadCalValueLabel(); // refresh if the window is still open behind the result box

                    if (s_leadCalMbox && lv_obj_is_valid(s_leadCalMbox)) {
                        msgbox_close_deferred(s_leadCalMbox);
                        s_leadCalMbox = nullptr;
                    }
                    // Short, one-thought-per-line — graph_R_16 has almost no
                    // line-height of its own, so more than a few short lines
                    // reads as a solid, cramped block without extra spacing below.
                    char resBuf[96];
                    if (mOhm > 2000.0)
                        snprintf(resBuf, sizeof(resBuf),
                                 "Measured: %.0f mΩ\nToo high - retry", mOhm);
                    else if (noisy)
                        snprintf(resBuf, sizeof(resBuf),
                                 "Measured: %.1f mΩ\nNoisy - retry?", mOhm);
                    else
                        snprintf(resBuf, sizeof(resBuf),
                                 "Measured: %.1f mΩ\nWas:    %.1f mΩ", mOhm, oldLeadR);
                    // No title (the window behind already says "PS Lead Resistance"),
                    // explicit width so text wraps at word boundaries, and the
                    // graph_R_16 font so the Ω glyph above actually renders instead
                    // of showing up as an empty box (the default msgbox font lacks it).
                    static const char *okBtn[] = {"OK", ""};
                    lv_obj_t *res = lv_msgbox_create(NULL, "", resBuf, okBtn, false);
                    lv_obj_set_width(res, 260);
                    lv_obj_t *resTxt = lv_msgbox_get_text(res);
                    lv_obj_set_style_text_font(resTxt, &graph_R_16, 0);
                    lv_obj_set_style_text_line_space(resTxt, 8, 0);
                    lv_obj_add_event_cb(res, [](lv_event_t *e) {
                        if (lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED)
                            msgbox_close_deferred(lv_event_get_current_target(e));
                    }, LV_EVENT_VALUE_CHANGED, nullptr);
                    lv_obj_center(res);

                    ensureOutput(false);
                    s_state = BatState::IDLE;
                    myTone(NOTE_A3, 100, true);
                }
            } else {
                setSetpoints(kLeadCalTargetV, fmax(0.02, kLeadCalMaxA * kLeadCalFracs[s_leadCalStepIdx]));
                s_phaseMs = now;
            }
        }
        break;

    default:
        break;
    }

    // Status labels (only when the Utility page objects exist)
    if (s_mah_label && lv_obj_is_valid(s_mah_label) && !blockAll) {
        char tstr[16] = "0:00:00";
        if (batteryChargerActive() || s_state == BatState::DONE) {
            unsigned long secs = (now - s_startMs) / 1000;
            snprintf(tstr, sizeof(tstr), "%lu:%02lu:%02lu", secs / 3600, (secs % 3600) / 60, secs % 60);
        }
        char mah[16]; // "001mAh" while <1000, collapses to e.g. "1.2Ah" beyond
        if (s_mAh > 999.0) snprintf(mah, sizeof(mah), "%.1fAh", s_mAh / 1000.0);
        else snprintf(mah, sizeof(mah), "%03.0fmAh", s_mAh);

        char ir[16]; // "---mΩ" (not tested yet), else scales mΩ -> Ω -> kΩ as it grows.
                     // A resistor or open-terminal smoke test lands in the kΩ range —
                     // that's the correct answer here, not an error state.
        // Subtract the calibrated lead+contact resistance (Settings > Calibration >
        // "Batt Lead R") so the displayed number is the battery's own IR, not the
        // test leads'. Kept as a display-time correction (raw s_ir_mOhm untouched)
        // so re-calibrating the leads doesn't require re-measuring the battery.
        double irShown = std::isnan(s_ir_mOhm) ? NAN : fmax(0.0, s_ir_mOhm - s_leadR_mOhm);
        if (std::isnan(irShown)) snprintf(ir, sizeof(ir), "---.-mΩ");
        else if (irShown > 999999.0) snprintf(ir, sizeof(ir), ">999kΩ");
        else if (irShown > 999.0) {
            double ohms = irShown / 1000.0;
            if (ohms > 999.0) snprintf(ir, sizeof(ir), "%.1fkΩ", ohms / 1000.0);
            else snprintf(ir, sizeof(ir), "%.1fΩ", ohms);
        }
        else snprintf(ir, sizeof(ir), "%.1fmΩ", irShown);

        // Show the actual regulation mode while charging (device knows:
        // CC = current loop active, VC = voltage loop active → CV)
        const char *nm = stateName(s_state);
        if (s_state == BatState::IDLE && outputIsOn())
        {
            nm = "EXT ON"; // output running from another page — press CHARGE to adopt/supervise
            // Safety: setpoint left high from another test (e.g. 32 V) with a
            // battery on the terminals — flash red HIGH V! and beep until fixed.
            static bool warnedHighV = false;
            if (vbat > vt + 0.3) {
                nm = "HIGH V!";
                if (!warnedHighV) { myTone(NOTE_C5, 300, true); myTone(NOTE_C5, 300, true); warnedHighV = true; }
            } else
                warnedHighV = false;
        }
        if (s_state == BatState::CC_CV) {
            DEVICE st = PowerSupply.getStatus();
            if (st == DEVICE::CC) nm = "CC";
            else if (st == DEVICE::VC || st == DEVICE::ON) nm = "CV";
        }

        // Visible proof the sweep is really stepping through each current
        // level, not just one blurred "IR" the whole time — same progress
        // readout for both the live in-charge dip and the standalone test.
        char chipBuf[12];
        bool isIrChip = s_state == BatState::IR_SWEEP || s_liveIrPhase != 0;
        if (isIrChip) {
            snprintf(chipBuf, sizeof(chipBuf), "IR %d/%d", s_irStepIdx + 1, kIrPoints);
            nm = chipBuf;
        }
        lv_label_set_text(s_chip_label, nm);
        lv_obj_set_style_bg_color(s_chip, stateColor(isIrChip ? "IR" : nm), LV_PART_MAIN);
        lv_label_set_text_fmt(s_big_label, "%6.3fV %6.3fA", vbat, amps); // %6.3f reserves the minus-sign slot: width constant, V never shifts
        // Progress bar + % — only meaningful while supervising (or done)
        // Progress bar + % — only updated while supervising (or done) AND NOT during live IR measurement dips
        if ((batteryChargerActive() || s_state == BatState::DONE) &&
            s_liveIrPhase == 0 && now >= s_liveIrCooldownMs) {
            double pct = chargePct(vbat, amps);
            lv_bar_set_value(s_bar, (int)lround(pct), LV_ANIM_OFF); // bar itself is integer-resolution
            lv_obj_set_style_bg_color(s_bar, stateColor(nm), LV_PART_INDICATOR);
            lv_label_set_text_fmt(s_pct_label, "%.1f%%", pct);
        }
        lv_label_set_text(s_timer_label, tstr);

        // A long fault string appended to the right-anchored IR label would grow
        // leftward over the mAh/Wh labels and turn into an unreadable pile-up
        // (this is what "messy" looked like). Instead, swap the whole details
        // line for one full-width message while a fault is set.
        bool fault = s_faultMsg[0] != '\0';
        if (fault) {
            lv_obj_add_flag(s_mah_label, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(s_wh_label, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(s_ir_label, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(s_fault_label, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text_fmt(s_fault_label, "FAULT: %s", s_faultMsg);
        } else {
            lv_obj_add_flag(s_fault_label, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(s_mah_label, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(s_wh_label, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(s_ir_label, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text(s_mah_label, mah);
            lv_label_set_text_fmt(s_wh_label, "%.2fWh", s_Wh);
            lv_label_set_text_fmt(s_ir_label, "IR:%s", ir);
        }
    }
}

// Auto-save Target V / Charge / Cutoff / Timeout even if the user never
// starts a charge or IR test (those are the only two places that used to
// call saveSettings()) — same reasoning as the FunGen auto-save in main.cpp:
// covers dialing in new values and power-cycling without an explicit save.
static void autoSaveSettingsIfDirty()
{
    if (!s_tab) return;
    int32_t vc = get_spinbox_data_by_id(s_tab, ID_VCELL);
    int32_t ic = get_spinbox_data_by_id(s_tab, ID_ICHG);
    int32_t it = get_spinbox_data_by_id(s_tab, ID_ITERM);
    int32_t to = get_spinbox_data_by_id(s_tab, ID_TMOUT);
    if (vc != s_mv_cell || ic != s_chg_mA || it != s_term_mA || to != s_timeout_min) {
        s_mv_cell = vc; s_chg_mA = ic; s_term_mA = it; s_timeout_min = to;
        saveSettings();
    }
}

// Auto-lock V/I while a charge, IR test, or lead calibration is actively
// driving the output — an accidental encoder turn or keypad entry on the
// Main page shouldn't be able to step on a running test. Only unlocks again
// whatever WE locked: a lock the user had already set manually before
// starting is left alone when the test finishes.
static bool s_autoLockedV = false, s_autoLockedI = false;

static void updateAutoLock()
{
    if (batteryChargerActive()) {
        if (!PowerSupply.Voltage.getLock()) { PowerSupply.Voltage.setLock(true); s_autoLockedV = true; }
        if (!PowerSupply.Current.getLock()) { PowerSupply.Current.setLock(true); s_autoLockedI = true; }
    } else {
        if (s_autoLockedV) { PowerSupply.Voltage.setLock(false); s_autoLockedV = false; }
        if (s_autoLockedI) { PowerSupply.Current.setLock(false); s_autoLockedI = false; }
    }
}

void BatteryChargerInterval(unsigned long interval)
{
    static unsigned long timer_ = {0};
    schedule(&batteryTick, interval, timer_);
    updateAutoLock();

    // Debounced: only touches flash every 2s while dirty, idempotent when clean.
    static unsigned long saveTimer_ = {0};
    schedule(&autoSaveSettingsIfDirty, 2000, saveTimer_);
}

// ---------- UI creation ----------
static void chg_btn_event_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_VALUE_CHANGED) return;
    if (lv_obj_has_state(s_chg_btn, LV_STATE_CHECKED))
        startCharge();
    else if (batteryChargerActive())
        stopAll(BatState::IDLE);
    syncChargeBtn(lv_obj_has_state(s_chg_btn, LV_STATE_CHECKED));
}

static void ir_btn_event_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_SHORT_CLICKED) return;
    // While charging: trigger an immediate live-IR dip instead of a standalone test
    if (s_state == BatState::CC_CV && s_liveIrPhase == 0) {
        if (measuredAmps() > 0.06) s_liveIrNextMs = 0; // dip now
        else s_faultMsg = "I too low for IR";
    } else
        startIrTest();
}

// Live settings: any spinbox change applies immediately to a running charge
static void sb_changed_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_VALUE_CHANGED) return;
    readSettings();
    if ((s_state == BatState::CC_CV || s_state == BatState::TRICKLE) && s_liveIrPhase == 0)
        setSetpoints(targetVolts(), s_state == BatState::TRICKLE ? trickleAmps() : chargeAmps());
}

// Writes the preset's V/cell into Target [V] and applies it the same way a
// manual spinbox edit would. Shared by touch selection and encoder stepping.
static void applyChemPreset(uint16_t sel)
{
    if (sel >= (uint16_t)s_numChemPresets || s_chemPresets[sel].mv < 0) return; // Custom: leave as-is
    lv_spinbox_set_value(s_target_sb, s_chemPresets[sel].mv);
    readSettings();
    if ((s_state == BatState::CC_CV || s_state == BatState::TRICKLE) && s_liveIrPhase == 0)
        setSetpoints(targetVolts(), s_state == BatState::TRICKLE ? trickleAmps() : chargeAmps());
}

// Chemistry preset selection via touch (list click)
static void chem_dd_event_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_VALUE_CHANGED) return;
    applyChemPreset(lv_dropdown_get_selected(s_chem_dd));
}

void createBatteryTab(lv_obj_t *parent)
{
    s_tab = parent;
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(parent, 0, LV_PART_MAIN);

    loadSettings();
    loadLeadR();

    // Spinboxes in a 2-column x 3-row grid. spinbox_pro puts its label
    // above-left of the box, so each column needs clear space to its left:
    //   col A boxes at x=60 (labels in 0..58), col B at x=215 (labels in 158..213).
    // Settings grid: 2 cols x 2 rows (single-cell: no Cells spinbox)
    const int colX[3]= {8, 112, 216}, rowY[3] = {16, 58, 90}, width=96; // rows 0-1: grid, row 2: buttons
    // Row 0: [chemistry preset] [Target V] [Charge A]
    // Row 1: [Cutoff A] [Timeout min] [Timer — live label, not editable]
    s_chem_dd = lv_dropdown_create(parent);
    lv_obj_set_size(s_chem_dd, width, 28); // graph_R_16 glyphs were clipped at the top in a 24px box
    lv_obj_set_style_pad_top(s_chem_dd, 2, LV_PART_MAIN);
    lv_obj_set_style_pad_bottom(s_chem_dd, 2, LV_PART_MAIN);
    lv_obj_align(s_chem_dd, LV_ALIGN_TOP_LEFT, colX[0], rowY[0]);
    {
        static char opts[96];
        opts[0] = '\0';
        for (int i = 0; i < s_numChemPresets; i++) {
            strcat(opts, s_chemPresets[i].name);
            if (i < s_numChemPresets - 1) strcat(opts, "\n");
        }
        lv_dropdown_set_options(s_chem_dd, opts);
    }
    lv_dropdown_set_selected(s_chem_dd, s_numChemPresets - 1); // "Custom" — don't clobber a loaded Target on boot
    lv_dropdown_set_symbol(s_chem_dd, NULL); // no arrow glyph: was overlapping the name in a 96px-wide box
    // Keep the selected-item highlight (default true): it only affects the
    // open list, not the closed box, and it's the only visual feedback while
    // stepping through the list with the encoder.
    lv_obj_set_style_text_font(s_chem_dd, &graph_R_16, LV_PART_MAIN);
    lv_obj_add_event_cb(s_chem_dd, chem_dd_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    {
        lv_obj_t *lbl = lv_label_create(parent);
        lv_label_set_recolor(lbl, true);
        lv_label_set_text(lbl, "#FFC107 Battery:#");
        lv_obj_set_style_text_font(lbl, &montserrat_b_12, 0); // matches spinbox_pro's own label style (s_style_spinbox_lbl)
        lv_obj_align(lbl, LV_ALIGN_TOP_LEFT, colX[0], rowY[0] - 18);
    }

    lv_obj_t *sbs[4];
    sbs[0] = spinbox_pro(parent, "#FFC107 Target [V]:#", 1000, 4400, 4, 1, LV_ALIGN_TOP_LEFT, colX[1], rowY[0], width, ID_VCELL, &graph_R_16); // 1.000V floor: room for NiMH/NiCd/LTO presets, not just Li chemistries
    lv_spinbox_set_value(sbs[0], s_mv_cell);
    s_target_sb = sbs[0];
    sbs[1] = spinbox_pro(parent, "#FFC107 Charge [A]:#", 10, 5000, 4, 1, LV_ALIGN_TOP_LEFT, colX[2], rowY[0], width, ID_ICHG, &graph_R_16);
    lv_spinbox_set_value(sbs[1], s_chg_mA);
    sbs[2] = spinbox_pro(parent, "#FFC107 Cutoff [A]:#", 5, 1000, 4, 1, LV_ALIGN_TOP_LEFT, colX[0], rowY[1], width, ID_ITERM, &graph_R_16);
    lv_spinbox_set_value(sbs[2], s_term_mA);
    sbs[3] = spinbox_pro(parent, "#FFC107 Timeout [min]:#", 0, 999, 3, 0, LV_ALIGN_TOP_LEFT, colX[1], rowY[1], width, ID_TMOUT, &graph_R_16);
    lv_spinbox_set_value(sbs[3], s_timeout_min);

    // Row 1, col 3: live elapsed-time readout (moved out of the bottom panel)
    {
        lv_obj_t *lbl = lv_label_create(parent);
        lv_label_set_recolor(lbl, true);
        lv_label_set_text(lbl, "#FFC107 Timer:#");
        lv_obj_set_style_text_font(lbl, &montserrat_b_12, 0); // matches spinbox_pro's own label style (s_style_spinbox_lbl)
        lv_obj_align(lbl, LV_ALIGN_TOP_LEFT, colX[2], rowY[1] - 18);
    }
    s_timer_label = lv_label_create(parent);
    lv_obj_set_style_text_font(s_timer_label, &graph_R_16, 0);
    lv_obj_set_style_text_color(s_timer_label, lv_color_hex(0xE0F0FF), 0);
    lv_label_set_text(s_timer_label, "0:00:00");
    lv_obj_align(s_timer_label, LV_ALIGN_TOP_LEFT, colX[2], rowY[1]);

    for (int i = 0; i < 4; i++) {
        lv_obj_add_event_cb(sbs[i], sb_changed_cb, LV_EVENT_VALUE_CHANGED, NULL);
        lv_obj_set_style_bg_color(sbs[i], lv_color_hex(0x000008), LV_PART_MAIN);
        // lv_obj_set_style_border_color(sbs[i], lv_color_hex(0x405060), LV_PART_MAIN);
        // lv_obj_set_style_border_width(sbs[i], 1, LV_PART_MAIN);
        lv_obj_set_style_radius(sbs[i], 3, LV_PART_MAIN);
        // lv_obj_set_height(sbs[i], 26);
        lv_obj_set_style_text_color(sbs[i], lv_color_hex(0xE0F0FF), LV_PART_MAIN);
    }

    // Row 3, col A zone: CHARGE (green -> red STOP) + IR, app-style dark buttons
    static lv_style_t style_chg, style_chg_checked, style_ir;
    lv_style_init(&style_chg);
    lv_style_set_bg_color(&style_chg, lv_color_hex(0x1B5E20));
    lv_style_set_text_color(&style_chg, lv_color_hex(0xC8FFC8));
    lv_style_init(&style_chg_checked);
    lv_style_set_bg_color(&style_chg_checked, lv_color_hex(0x9B1C1C));
    lv_style_set_text_color(&style_chg_checked, lv_color_hex(0xFFE0E0));
    lv_style_init(&style_ir);
    lv_style_set_bg_color(&style_ir, lv_palette_darken(LV_PALETTE_INDIGO, 2));
    lv_style_set_text_color(&style_ir, lv_palette_main(LV_PALETTE_AMBER));

    // Span the full grid width (colX[0] .. colX[2]+width) with a small gap
    // between the two buttons, instead of leaving column 3's width empty.
    const int rowSpan = colX[2] + width - colX[0], btnGap = 8;
    const int btnW = (rowSpan - btnGap) / 2;
    const int btnBx = colX[0] + btnW + btnGap;

    s_chg_btn = lv_btn_create(parent);
    lv_obj_add_flag(s_chg_btn, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_set_size(s_chg_btn, btnW, 26);
    lv_obj_set_style_radius(s_chg_btn, 3, LV_PART_MAIN);
    lv_obj_set_style_border_color(s_chg_btn, lv_color_hex(0x405060), LV_PART_MAIN);
    // lv_obj_set_style_border_width(s_chg_btn, 1, LV_PART_MAIN);
    lv_obj_add_style(s_chg_btn, &style_chg, LV_STATE_DEFAULT);
    lv_obj_add_style(s_chg_btn, &style_chg_checked, LV_STATE_CHECKED);
    lv_obj_align(s_chg_btn, LV_ALIGN_TOP_LEFT, colX[0], rowY[2]);
    s_chg_btn_label = lv_label_create(s_chg_btn);
    lv_label_set_text(s_chg_btn_label, "CHARGE");
    lv_obj_center(s_chg_btn_label);
    lv_obj_add_event_cb(s_chg_btn, chg_btn_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    s_ir_btn = lv_btn_create(parent);
    lv_obj_set_size(s_ir_btn, btnW, 26);
    lv_obj_set_style_radius(s_ir_btn, 3, LV_PART_MAIN);
    lv_obj_set_style_border_color(s_ir_btn, lv_color_hex(0x405060), LV_PART_MAIN);
    // lv_obj_set_style_border_width(s_ir_btn, 1, LV_PART_MAIN);
    lv_obj_add_style(s_ir_btn, &style_ir, LV_STATE_DEFAULT);
    lv_obj_align(s_ir_btn, LV_ALIGN_TOP_LEFT, btnBx, rowY[2]);
    lv_obj_t *l = lv_label_create(s_ir_btn);
    lv_label_set_text(l, "IR TEST");
    lv_obj_center(l);
    lv_obj_add_event_cb(s_ir_btn, ir_btn_event_cb, LV_EVENT_SHORT_CLICKED, NULL);

    // Bottom: instrument readout panel — state chip + big V/A + details line
    lv_obj_t *panel = lv_obj_create(parent);
    lv_obj_set_size(panel, 304, 58); // taller: chip/big row, bar+% row, details row
    lv_obj_align(panel, LV_ALIGN_BOTTOM_MID, 0, -5);
    lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(panel, lv_color_hex(0x05050A), LV_PART_MAIN);
    lv_obj_set_style_border_color(panel, lv_color_hex(0x405060), LV_PART_MAIN);
    // lv_obj_set_style_border_width(panel, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(panel, 6, LV_PART_MAIN);
    lv_obj_set_style_pad_hor(panel, 6, LV_PART_MAIN);
    lv_obj_set_style_pad_ver(panel, 3, LV_PART_MAIN);

    s_chip = lv_obj_create(panel);
    lv_obj_set_size(s_chip, 60, 18);
    lv_obj_align(s_chip, LV_ALIGN_TOP_LEFT, 0, 1);
    lv_obj_clear_flag(s_chip, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(s_chip, lv_color_hex(0x505050), LV_PART_MAIN);
    // lv_obj_set_style_border_width(s_chip, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(s_chip, 3, LV_PART_MAIN);
    lv_obj_set_style_pad_all(s_chip, 0, LV_PART_MAIN);
    s_chip_label = lv_label_create(s_chip);
    lv_obj_set_style_text_font(s_chip_label, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(s_chip_label, lv_color_hex(0xFFFFFF), 0);
    lv_label_set_text(s_chip_label, "IDLE");
    lv_obj_center(s_chip_label);

    s_big_label = lv_label_create(panel);
    lv_obj_set_style_text_font(s_big_label, &graph_R_18, 0);
    lv_obj_set_style_text_color(s_big_label, lv_color_hex(0x40E0FF), 0);
    lv_label_set_text(s_big_label, " -.---V -.---A");
    lv_obj_align(s_big_label, LV_ALIGN_TOP_RIGHT, 4, 2);

    // Charge progress bar (indicator color follows the state chip) + percent
    s_bar = lv_bar_create(panel);
    lv_obj_set_size(s_bar, 240, 8);
    lv_obj_align(s_bar, LV_ALIGN_LEFT_MID, 0, 2);
    lv_bar_set_range(s_bar, 0, 100);
    lv_bar_set_value(s_bar, 0, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(s_bar, lv_color_hex(0x2E3A46), LV_PART_MAIN); // visible track: shows distance to 100%
    lv_obj_set_style_border_color(s_bar, lv_color_hex(0x506070), LV_PART_MAIN);
    lv_obj_set_style_border_width(s_bar, 1, LV_PART_MAIN);
    lv_obj_set_style_bg_color(s_bar, lv_color_hex(0x00A048), LV_PART_INDICATOR);
    lv_obj_set_style_radius(s_bar, 2, LV_PART_MAIN);
    lv_obj_set_style_radius(s_bar, 2, LV_PART_INDICATOR);

    s_pct_label = lv_label_create(panel);
    lv_obj_set_style_text_font(s_pct_label, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(s_pct_label, lv_color_hex(0xB0B8C0), 0);
    lv_label_set_text(s_pct_label, "--%");
    lv_obj_align(s_pct_label, LV_ALIGN_TOP_RIGHT, 2, 18);

    // Details line split into 3 independent labels so each can be nudged by
    // exact pixels via its own lv_obj_align offset (a single label with a
    // format string can only be spaced in whole-glyph steps).
    s_mah_label = lv_label_create(panel);
    lv_obj_set_style_text_font(s_mah_label, &graph_R_16, 0);
    lv_obj_set_style_text_color(s_mah_label, lv_color_hex(0xB0B8C0), 0);
    lv_label_set_text(s_mah_label, "connect battery");
    lv_obj_align(s_mah_label, LV_ALIGN_BOTTOM_LEFT, 0, 1);

    s_wh_label = lv_label_create(panel);
    lv_obj_set_style_text_font(s_wh_label, &graph_R_16, 0);
    lv_obj_set_style_text_color(s_wh_label, lv_color_hex(0xB0B8C0), 0);
    lv_label_set_text(s_wh_label, "");
    lv_obj_align(s_wh_label, LV_ALIGN_BOTTOM_MID, -24, 1);

    s_ir_label = lv_label_create(panel);
    lv_obj_set_style_text_font(s_ir_label, &graph_R_16, 0);
    lv_obj_set_style_text_color(s_ir_label, lv_color_hex(0xB0B8C0), 0);
    lv_label_set_text(s_ir_label, "");
    lv_obj_align(s_ir_label, LV_ALIGN_BOTTOM_RIGHT, 4, 1);

    s_fault_label = lv_label_create(panel);
    lv_obj_set_style_text_font(s_fault_label, &graph_R_16, 0);
    lv_obj_set_style_text_color(s_fault_label, lv_color_hex(0xFF6060), 0);
    lv_label_set_long_mode(s_fault_label, LV_LABEL_LONG_CLIP); // never wraps and grows the panel
    lv_obj_set_width(s_fault_label, 296);
    lv_label_set_text(s_fault_label, "");
    lv_obj_align(s_fault_label, LV_ALIGN_BOTTOM_LEFT, 0, 1);
    lv_obj_add_flag(s_fault_label, LV_OBJ_FLAG_HIDDEN);
}

bool batteryChemDropdownOpen()
{
    return s_chem_dd && lv_dropdown_is_open(s_chem_dd);
}

void batteryChemDropdownStep(int dir)
{
    if (!batteryChemDropdownOpen()) return;
    int sel = (int)lv_dropdown_get_selected(s_chem_dd) + (dir > 0 ? 1 : -1);
    sel = constrain(sel, 0, s_numChemPresets - 1);
    lv_dropdown_set_selected(s_chem_dd, sel); // does not fire VALUE_CHANGED — apply explicitly
    lv_obj_t *list = lv_dropdown_get_list(s_chem_dd);
    if (list) {
        lv_obj_t *item = lv_obj_get_child(list, sel);
        if (item) lv_obj_scroll_to_view(item, LV_ANIM_OFF);
    }
    applyChemPreset((uint16_t)sel);
}

static volatile bool s_battChemClosePending = false;

void requestBattChemDropdownClose()
{
    s_battChemClosePending = true;
}

void drainBattChemDropdownClose()
{
    if (!s_battChemClosePending) return;
    s_battChemClosePending = false;
    if (batteryChemDropdownOpen())
        lv_dropdown_close(s_chem_dd);
}
