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
static unsigned long s_liveIrNextMs = 0, s_liveIrPhaseMs = 0;
static int s_liveIrPhase = 0; // 0 = normal charging, 1 = low-current dip active
static double s_liveIrV1 = 0, s_liveIrI1 = 0;

// ---------- UI ----------
static lv_obj_t *s_status_label = nullptr;
static lv_obj_t *s_chg_btn = nullptr;
static lv_obj_t *s_chg_btn_label = nullptr;
static lv_obj_t *s_ir_btn = nullptr;
static lv_obj_t *s_tab = nullptr;

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
    s_cells = get_spinbox_data_by_id(s_tab, ID_CELLS);
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
        double dipAmps = fmax(0.05, chargeAmps() * 0.2);
        if (s_liveIrPhase == 0 && now >= s_liveIrNextMs && amps > dipAmps * 2.0) {
            s_liveIrV1 = vbat; s_liveIrI1 = amps;
            setSetpoints(vt, dipAmps);
            s_liveIrPhase = 1;
            s_liveIrPhaseMs = now;
        } else if (s_liveIrPhase == 1 && now - s_liveIrPhaseMs >= 1200) {
            double dI = s_liveIrI1 - amps;
            if (fabs(dI) > 0.05)
                s_ir_mOhm = (s_liveIrV1 - vbat) / dI * 1000.0;
            setSetpoints(vt, chargeAmps()); // restore full current
            s_liveIrPhase = 0;
            s_liveIrNextMs = now + 10000;
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
            myTone(NOTE_A5, 100, true);
        }
        break;

    default:
        break;
    }

    // Status label (only when the Utility page objects exist)
    if (s_status_label && lv_obj_is_valid(s_status_label) && !blockAll) {
        char tstr[16] = "00:00:00";
        if (batteryChargerActive() || s_state == BatState::DONE) {
            unsigned long secs = (now - s_startMs) / 1000;
            snprintf(tstr, sizeof(tstr), "%02lu:%02lu:%02lu", secs / 3600, (secs % 3600) / 60, secs % 60);
        }
        char ir[16];
        if (std::isnan(s_ir_mOhm)) snprintf(ir, sizeof(ir), " ---");
        else snprintf(ir, sizeof(ir), "%4.0f", s_ir_mOhm);

        // Show the actual regulation mode while charging (device knows:
        // CC = current loop active, VC = voltage loop active → CV)
        const char *nm = stateName(s_state);
        if (s_state == BatState::CC_CV) {
            DEVICE st = PowerSupply.getStatus();
            if (st == DEVICE::CC) nm = s_liveIrPhase ? "IR" : "CC";
            else if (st == DEVICE::VC || st == DEVICE::ON) nm = "CV";
        }

        lv_label_set_text_fmt(s_status_label,
            "#FFFF00 %-7s#%6.3fV %5.3fA %s\n"
            "%7.1fmAh %5.2fWh %s IR:%smO",
            nm, vbat, amps, s_faultMsg,
            s_mAh, s_Wh, tstr, ir);
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
    if (lv_event_get_code(e) == LV_EVENT_SHORT_CLICKED)
        startIrTest();
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
    const int colA = 60, colB = 215, rowY[3] = {14, 56, 98};
    lv_obj_t *sb;
    sb = spinbox_pro(parent, "#FFFFF7 Cells:#", 1, 8, 1, 0, LV_ALIGN_TOP_LEFT, colA, rowY[0], 98, ID_CELLS, &graph_R_16);
    lv_spinbox_set_value(sb, s_cells);
    sb = spinbox_pro(parent, "#FFFFF7 V/cell:#", 3000, 4400, 4, 1, LV_ALIGN_TOP_LEFT, colB, rowY[0], 98, ID_VCELL, &graph_R_16);
    lv_spinbox_set_value(sb, s_mv_cell);
    sb = spinbox_pro(parent, "#FFFFF7 I chg A:#", 10, 5000, 4, 1, LV_ALIGN_TOP_LEFT, colA, rowY[1], 98, ID_ICHG, &graph_R_16);
    lv_spinbox_set_value(sb, s_chg_mA);
    sb = spinbox_pro(parent, "#FFFFF7 I term A:#", 5, 1000, 4, 1, LV_ALIGN_TOP_LEFT, colB, rowY[1], 98, ID_ITERM, &graph_R_16);
    lv_spinbox_set_value(sb, s_term_mA);
    sb = spinbox_pro(parent, "#FFFFF7 T.out min:#", 0, 999, 3, 0, LV_ALIGN_TOP_LEFT, colB, rowY[2], 98, ID_TMOUT, &graph_R_16);
    lv_spinbox_set_value(sb, s_timeout_min);

    // Row 3, col A zone: CHARGE (green, turns red STOP when running) + IR
    static lv_style_t style_chg, style_chg_checked, style_ir;
    lv_style_init(&style_chg);
    lv_style_set_bg_color(&style_chg, lv_palette_darken(LV_PALETTE_GREEN, 2));
    lv_style_init(&style_chg_checked);
    lv_style_set_bg_color(&style_chg_checked, lv_palette_darken(LV_PALETTE_RED, 1));
    lv_style_init(&style_ir);
    lv_style_set_bg_color(&style_ir, lv_palette_darken(LV_PALETTE_INDIGO, 2));

    s_chg_btn = lv_btn_create(parent);
    lv_obj_add_flag(s_chg_btn, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_set_size(s_chg_btn, 82, 34);
    lv_obj_set_style_radius(s_chg_btn, 4, LV_PART_MAIN);
    lv_obj_add_style(s_chg_btn, &style_chg, LV_STATE_DEFAULT);
    lv_obj_add_style(s_chg_btn, &style_chg_checked, LV_STATE_CHECKED);
    lv_obj_align(s_chg_btn, LV_ALIGN_TOP_LEFT, 8, rowY[2] - 3);
    s_chg_btn_label = lv_label_create(s_chg_btn);
    lv_label_set_text(s_chg_btn_label, "CHARGE");
    lv_obj_center(s_chg_btn_label);
    lv_obj_add_event_cb(s_chg_btn, chg_btn_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    s_ir_btn = lv_btn_create(parent);
    lv_obj_set_size(s_ir_btn, 50, 34);
    lv_obj_set_style_radius(s_ir_btn, 4, LV_PART_MAIN);
    lv_obj_add_style(s_ir_btn, &style_ir, LV_STATE_DEFAULT);
    lv_obj_align(s_ir_btn, LV_ALIGN_TOP_LEFT, 98, rowY[2] - 3);
    lv_obj_t *l = lv_label_create(s_ir_btn);
    lv_label_set_text(l, "IR");
    lv_obj_center(l);
    lv_obj_add_event_cb(s_ir_btn, ir_btn_event_cb, LV_EVENT_SHORT_CLICKED, NULL);

    // Bottom: framed status panel across the full width
    lv_obj_t *panel = lv_obj_create(parent);
    lv_obj_set_size(panel, 308, 46);
    lv_obj_align(panel, LV_ALIGN_BOTTOM_MID, 0, -1);
    lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(panel, lv_color_hex(0x0A0A12), LV_PART_MAIN);
    lv_obj_set_style_border_color(panel, lv_palette_darken(LV_PALETTE_GREY, 2), LV_PART_MAIN);
    lv_obj_set_style_border_width(panel, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(panel, 4, LV_PART_MAIN);
    lv_obj_set_style_pad_all(panel, 3, LV_PART_MAIN);

    s_status_label = lv_label_create(panel);
    lv_label_set_recolor(s_status_label, true);
    lv_obj_set_style_text_font(s_status_label, &monofont_R_16, 0);
    lv_label_set_text(s_status_label, "#FFFF00 IDLE#   connect battery\nA range, output off");
    lv_obj_align(s_status_label, LV_ALIGN_LEFT_MID, 2, 0);
}
