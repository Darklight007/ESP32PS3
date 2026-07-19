// Li-ion battery CC/CV charger + tester
// - CC/CV charge with pre-charge (trickle) for deep-discharged cells
// - Charge counter (mAh / Wh) and elapsed time
// - Internal resistance (IR) test: two current levels, IR = dV/dI
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
enum class BatState { IDLE, TRICKLE, CC_CV, IR_LOW, IR_HIGH, DONE, FAULT };
static BatState s_state = BatState::IDLE;
static const char *stateName(BatState s)
{
    switch (s) {
    case BatState::IDLE: return "IDLE";
    case BatState::TRICKLE: return "PRE-CHG";
    case BatState::CC_CV: return "CC/CV";
    case BatState::IR_LOW: return "IR lo";
    case BatState::IR_HIGH: return "IR hi";
    case BatState::DONE: return "DONE";
    default: return "FAULT";
    }
}

static double s_mAh = 0, s_Wh = 0;
static unsigned long s_startMs = 0, s_lastIntMs = 0, s_phaseMs = 0;
static int s_termCount = 0;
static double s_ir_V1 = 0, s_ir_I1 = 0, s_ir_mOhm = NAN;
static const char *s_faultMsg = "";

// Live IR during charge: every 10 s dip the current setpoint to the low level
// for ~1.2 s and compute IR from the two (V, I) points.
static unsigned long s_liveIrNextMs = 0, s_liveIrPhaseMs = 0, s_liveIrCooldownMs = 0;
static int s_liveIrPhase = 0; // 0 = normal charging, 1 = low-current dip active
static double s_liveIrV1 = 0, s_liveIrI1 = 0;

// ---------- UI ----------
static lv_obj_t *s_status_label = nullptr; // details line (mono)
static lv_obj_t *s_big_label = nullptr;    // large V/A readout
static lv_obj_t *s_chip = nullptr;         // colored state badge
static lv_obj_t *s_chip_label = nullptr;
static lv_obj_t *s_bar = nullptr;       // charge progress bar (pro-charger style)
static lv_obj_t *s_pct_label = nullptr; // percent estimate
static lv_obj_t *s_chg_btn = nullptr;
static lv_obj_t *s_chg_btn_label = nullptr;
static lv_obj_t *s_ir_btn = nullptr;
static lv_obj_t *s_tab = nullptr;

static lv_color_t stateColor(const char *nm)
{
    if (!strcmp(nm, "CC")) return lv_color_hex(0xFFA000);      // amber
    if (!strcmp(nm, "CV")) return lv_color_hex(0x0090B0);      // cyan
    if (!strcmp(nm, "IR")) return lv_color_hex(0x7040C0);      // purple dip
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
static int chargePct(double vbat, double amps)
{
    if (s_state == BatState::DONE) return 100;
    double vt = targetVolts();
    double pct;
    if (vbat < vt * 0.985)
        pct = 80.0 * (vbat - 3.0) / fmax(0.1, vt - 3.0);
    else {
        double hi = chargeAmps(), lo = s_term_mA / 1000.0;
        pct = 80.0 + 20.0 * (1.0 - (amps - lo) / fmax(0.001, hi - lo));
    }
    return (int)fmin(100.0, fmax(0.0, pct));
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
    PowerSupply.Voltage.SetUpdate(vc);
    PowerSupply.Current.SetUpdate(ic);
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
           s_state == BatState::IR_LOW || s_state == BatState::IR_HIGH;
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

static void startIrTest()
{
    readSettings();
    saveSettings();
    if (!prechecksOk()) { s_state = BatState::FAULT; return; }
    if (batteryChargerActive()) return; // not while charging

    s_ir_mOhm = NAN;
    s_faultMsg = "";
    setSetpoints(targetVolts(), fmax(0.05, chargeAmps() * 0.2)); // low level: 20% of Ichg
    ensureOutput(true);
    s_phaseMs = millis();
    s_state = BatState::IR_LOW;
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

        // Live IR: periodic low-current dip while still in real CC (skip when
        // the taper current is already near the dip level — no usable dI).
        // Dip to half the ACTUAL current (not the charge setting) so IR still
        // works deep into CV taper; below ~60 mA there is too little dI.
        double dipAmps = fmax(0.02, amps * 0.5);
        if (s_liveIrPhase == 0 && now >= s_liveIrNextMs && amps > 0.06) {
            s_liveIrV1 = vbat; s_liveIrI1 = amps;
            setSetpoints(vt, dipAmps);
            s_liveIrPhase = 1;
            s_liveIrPhaseMs = now;
        } else if (s_liveIrPhase == 1 && now - s_liveIrPhaseMs >= 1200) {
            double dI = s_liveIrI1 - amps;
            if (fabs(dI) > 0.02)
                s_ir_mOhm = (s_liveIrV1 - vbat) / dI * 1000.0;
            setSetpoints(vt, chargeAmps()); // restore full current
            s_liveIrPhase = 0;
            s_liveIrCooldownMs = now + 300; // let the setpoint settle before the bar reacts again
            // Space dips out as the pack fills: frequent early on, rare near full
            // (avoids interrupting the charge every 10 s once it barely matters).
            int pct = chargePct(vbat, amps);
            s_liveIrNextMs = now + (pct < 50 ? 10000 : 60000);
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

    case BatState::IR_LOW:
        if (now - s_phaseMs >= 2500) { // settled
            s_ir_V1 = vbat; s_ir_I1 = amps;
            setSetpoints(vt, chargeAmps()); // high level
            s_phaseMs = now;
            s_state = BatState::IR_HIGH;
        }
        break;

    case BatState::IR_HIGH:
        if (now - s_phaseMs >= 2500) {
            double dI = amps - s_ir_I1;
            s_ir_mOhm = (fabs(dI) > 0.010) ? (vbat - s_ir_V1) / dI * 1000.0 : NAN;
            ensureOutput(false);
            s_state = BatState::IDLE;
            myTone(NOTE_A3, 100, true);
        }
        break;

    default:
        break;
    }

    // Status label (only when the Utility page objects exist)
    if (s_status_label && lv_obj_is_valid(s_status_label) && !blockAll) {
        char tstr[16] = "0:00:00";
        if (batteryChargerActive() || s_state == BatState::DONE) {
            unsigned long secs = (now - s_startMs) / 1000;
            snprintf(tstr, sizeof(tstr), "%lu:%02lu:%02lu", secs / 3600, (secs % 3600) / 60, secs % 60);
        }
        char mah[16]; // "001mAh" while <1000, collapses to e.g. "1.2Ah" beyond
        if (s_mAh > 999.0) snprintf(mah, sizeof(mah), "%.1fAh", s_mAh / 1000.0);
        else snprintf(mah, sizeof(mah), "%03.0fmAh", s_mAh);

        char ir[16]; // "---mΩ" or e.g. "1.2Ω" — collapses to Ω once IR exceeds 999 mΩ
        if (std::isnan(s_ir_mOhm)) snprintf(ir, sizeof(ir), "---mΩ");
        else if (s_ir_mOhm > 999.0) snprintf(ir, sizeof(ir), "%.1fΩ", s_ir_mOhm / 1000.0);
        else snprintf(ir, sizeof(ir), "%03.0fmΩ", s_ir_mOhm);

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
            if (st == DEVICE::CC) nm = s_liveIrPhase ? "IR" : "CC";
            else if (st == DEVICE::VC || st == DEVICE::ON) nm = "CV";
        }

        lv_label_set_text(s_chip_label, nm);
        lv_obj_set_style_bg_color(s_chip, stateColor(nm), LV_PART_MAIN);
        lv_label_set_text_fmt(s_big_label, "%6.3fV %6.3fA", vbat, amps); // %6.3f reserves the minus-sign slot: width constant, V never shifts
        // Progress bar + % — only meaningful while supervising (or done)
        // Progress bar + % — only updated while supervising (or done) AND NOT during live IR measurement dips
        if ((batteryChargerActive() || s_state == BatState::DONE) &&
            s_liveIrPhase == 0 && now >= s_liveIrCooldownMs) {
            int pct = chargePct(vbat, amps);
            lv_bar_set_value(s_bar, pct, LV_ANIM_OFF);
            lv_obj_set_style_bg_color(s_bar, stateColor(nm), LV_PART_INDICATOR);
            lv_label_set_text_fmt(s_pct_label, "%d%%", pct);
        }
        lv_label_set_text_fmt(s_status_label,
            "%s  %s  IR:%s%s",
            mah, tstr, ir, s_faultMsg);
    }
    
}

void BatteryChargerInterval(unsigned long interval)
{
    static unsigned long timer_ = {0};
    schedule(&batteryTick, interval, timer_);
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

void createBatteryTab(lv_obj_t *parent)
{
    s_tab = parent;
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(parent, 0, LV_PART_MAIN);

    loadSettings();

    // Spinboxes in a 2-column x 3-row grid. spinbox_pro puts its label
    // above-left of the box, so each column needs clear space to its left:
    //   col A boxes at x=60 (labels in 0..58), col B at x=215 (labels in 158..213).
    // Settings grid: 2 cols x 2 rows (single-cell: no Cells spinbox)
    const int colA = 30, colB = 185, rowY[3] = {16, 58, 90}; // rows 0-1: spinboxes, row 2: buttons
    lv_obj_t *sbs[4];
    sbs[0] = spinbox_pro(parent, "#FFC107 Target [V]:#", 3000, 4400, 4, 1, LV_ALIGN_TOP_LEFT, colA, rowY[0], 98, ID_VCELL, &graph_R_16);
    lv_spinbox_set_value(sbs[0], s_mv_cell);
    sbs[1] = spinbox_pro(parent, "#FFC107 Charge [A]:#", 10, 5000, 4, 1, LV_ALIGN_TOP_LEFT, colB, rowY[0], 98, ID_ICHG, &graph_R_16);
    lv_spinbox_set_value(sbs[1], s_chg_mA);
    sbs[2] = spinbox_pro(parent, "#FFC107 Cutoff [A]:#", 5, 1000, 4, 1, LV_ALIGN_TOP_LEFT, colA, rowY[1], 98, ID_ITERM, &graph_R_16);
    lv_spinbox_set_value(sbs[2], s_term_mA);
    sbs[3] = spinbox_pro(parent, "#FFC107 Timeout [min]:#", 0, 999, 3, 0, LV_ALIGN_TOP_LEFT, colB, rowY[1], 98, ID_TMOUT, &graph_R_16);
    lv_spinbox_set_value(sbs[3], s_timeout_min);

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

    s_chg_btn = lv_btn_create(parent);
    lv_obj_add_flag(s_chg_btn, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_set_size(s_chg_btn, 98, 26); // same width as spinboxes, aligned under col A
    lv_obj_set_style_radius(s_chg_btn, 3, LV_PART_MAIN);
    lv_obj_set_style_border_color(s_chg_btn, lv_color_hex(0x405060), LV_PART_MAIN);
    // lv_obj_set_style_border_width(s_chg_btn, 1, LV_PART_MAIN);
    lv_obj_add_style(s_chg_btn, &style_chg, LV_STATE_DEFAULT);
    lv_obj_add_style(s_chg_btn, &style_chg_checked, LV_STATE_CHECKED);
    lv_obj_align(s_chg_btn, LV_ALIGN_TOP_LEFT, colA, rowY[2]);
    s_chg_btn_label = lv_label_create(s_chg_btn);
    lv_label_set_text(s_chg_btn_label, "CHARGE");
    lv_obj_center(s_chg_btn_label);
    lv_obj_add_event_cb(s_chg_btn, chg_btn_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    s_ir_btn = lv_btn_create(parent);
    lv_obj_set_size(s_ir_btn, 98, 26); // aligned under col B
    lv_obj_set_style_radius(s_ir_btn, 3, LV_PART_MAIN);
    lv_obj_set_style_border_color(s_ir_btn, lv_color_hex(0x405060), LV_PART_MAIN);
    // lv_obj_set_style_border_width(s_ir_btn, 1, LV_PART_MAIN);
    lv_obj_add_style(s_ir_btn, &style_ir, LV_STATE_DEFAULT);
    lv_obj_align(s_ir_btn, LV_ALIGN_TOP_LEFT, colB, rowY[2]);
    lv_obj_t *l = lv_label_create(s_ir_btn);
    lv_label_set_text(l, "IR");
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

    s_status_label = lv_label_create(panel);
    lv_obj_set_style_text_font(s_status_label, &graph_R_16, 0); // clearer glyphs than monofont, still monospace
    lv_obj_set_style_text_color(s_status_label, lv_color_hex(0xB0B8C0), 0);
    lv_label_set_text(s_status_label, "connect battery, A range, out off");
    lv_obj_align(s_status_label, LV_ALIGN_BOTTOM_LEFT, -6, 1);
}
