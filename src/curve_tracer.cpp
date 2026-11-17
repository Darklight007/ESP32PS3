// BJT curve tracer with two linked PSUs. See curve_tracer.h.
//
// Runs as a state machine from the main loop (Core 1): every wait is a
// millis() deadline, so the UI, keys and SCPI keep running during a sweep.
// The base drive is a voltage servo on the slave: it steps its V-Set until
// the current it measures equals the Ib target (secant method), then holds
// that voltage for the whole curve and records the Ib it reads per point.
#include "curve_tracer.h"
#include "psu_link.h"
#include "device.hpp"
#include <Preferences.h>
#include <esp_heap_caps.h>
#include <cmath>

extern Device PowerSupply;

namespace
{
    constexpr int kMaxCurves = 10, kMaxPts = 64;
    struct Pt { float vce, ic, ib; };
    Pt *s_pts = nullptr;               // [kMaxCurves][kMaxPts], PSRAM
    int s_nPts[kMaxCurves] = {};
    float s_ib[kMaxCurves] = {};       // mean measured Ib per curve
    int s_nCurves = 0;
    TracerCfg s_cfg;

    enum class St { IDLE, START, SERVO, SWEEP, DONE, DEMO };
    float s_demoBeta = 150;
    St s_st = St::IDLE;
    uint32_t s_until = 0;
    int s_curve = 0, s_pt = 0, s_iter = 0;
    float s_vb = 0, s_vbPrev = 0, s_ibPrev = 0;
    double s_accV = 0, s_accI = 0, s_accB = 0;
    int s_accN = 0;
    char s_msg[96] = "Link a second PSU (Settings > PSU Link), then Start.";
    float s_saveV = 0, s_saveI = 0; // master setpoints before the run

    // UI
    lv_obj_t *s_chart, *s_status, *s_btn, *s_btnLbl, *s_yLbl, *s_xLbl;
    lv_chart_series_t *s_ser[kMaxCurves] = {};
    lv_obj_t *s_curveLbl[kMaxCurves] = {};
    const uint32_t kColors[kMaxCurves] = {0xFFC107, 0x00BCD4, 0x8BC34A, 0xFF5722, 0xE040FB,
                                          0x03A9F4, 0xFFEB3B, 0x4CAF50, 0xFF9800, 0xF06292};

    Pt &pt(int c, int k) { return s_pts[c * kMaxPts + k]; }

    double masterV() { return PowerSupply.Voltage.measured.value; }
    double masterI()
    {
        double i = PowerSupply.Current.measured.value;
        return PowerSupply.mA_Active ? i / 1000.0 : i;
    }
    void setMasterV(double v)
    {
        auto &V = PowerSupply.Voltage;
        V.SetUpdate(v * V.adjFactor + V.adjOffset);
    }
    void setMasterI(double a)
    {
        auto &I = PowerSupply.Current;
        I.SetUpdate(a * I.adjFactor + I.adjOffset);
    }
    double vSetNow() { auto &V = PowerSupply.Voltage; return (V.adjValue - V.adjOffset) / V.adjFactor; }
    double iSetNow() { auto &I = PowerSupply.Current; return (I.adjValue - I.adjOffset) / I.adjFactor; }
    bool masterOn() { return PowerSupply.getStatus() != DEVICE::OFF; }

    float ibTarget(int c) { return s_cfg.ibStep * (c + 1); }
    float vcePoint(int k) { return s_cfg.vceMax * k / (float)(s_cfg.points - 1); }
    float servoVce() { return std::min(2.0f, s_cfg.vceMax * 0.5f); } // Ib set in the active region

    void fmtI(char *b, size_t n, float a)
    {
        if (fabsf(a) < 1e-3f)
            snprintf(b, n, "%.1fuA", a * 1e6f);
        else if (fabsf(a) < 1)
            snprintf(b, n, "%.2fmA", a * 1e3f);
        else
            snprintf(b, n, "%.3fA", a);
    }

    void plotReset()
    {
        if (!s_chart)
            return;
        for (int c = 0; c < kMaxCurves; c++)
        {
            lv_chart_set_all_value(s_chart, s_ser[c], LV_CHART_POINT_NONE);
            if (s_curveLbl[c])
                lv_obj_add_flag(s_curveLbl[c], LV_OBJ_FLAG_HIDDEN);
        }
        lv_chart_set_point_count(s_chart, s_cfg.points);
        char b[16];
        fmtI(b, sizeof(b), s_cfg.icMax);
        lv_label_set_text_fmt(s_yLbl, "Ic %s", b);
        lv_label_set_text_fmt(s_xLbl, "Vce %.0fV", s_cfg.vceMax);
        lv_chart_refresh(s_chart);
    }

    void plotPoint(int c, int k)
    {
        if (!s_chart || c < 0 || c >= kMaxCurves || k < 0 || k >= (int)lv_chart_get_point_count(s_chart))
            return;
        const Pt &p = pt(c, k);
        const lv_coord_t x = (lv_coord_t)lroundf(std::clamp(p.vce / s_cfg.vceMax, 0.0f, 1.0f) * 1000);
        const lv_coord_t y = (lv_coord_t)lroundf(std::clamp(p.ic / s_cfg.icMax, 0.0f, 1.0f) * 1000);
        s_ser[c]->x_points[k] = x;
        s_ser[c]->y_points[k] = y;
        lv_chart_refresh(s_chart);
    }

    // Ib label next to the curve's last point
    void plotCurveLabel(int c)
    {
        if (!s_chart || s_nPts[c] == 0)
            return;
        if (!s_curveLbl[c])
        {
            s_curveLbl[c] = lv_label_create(s_chart);
            lv_obj_set_style_text_font(s_curveLbl[c], &lv_font_montserrat_10, 0);
            lv_obj_set_style_text_color(s_curveLbl[c], lv_color_hex(kColors[c]), 0);
        }
        char b[16];
        fmtI(b, sizeof(b), s_ib[c]);
        lv_label_set_text(s_curveLbl[c], b);
        const Pt &p = pt(c, s_nPts[c] - 1);
        const int w = lv_obj_get_content_width(s_chart), h = lv_obj_get_content_height(s_chart);
        const int x = (int)(std::clamp(p.vce / s_cfg.vceMax, 0.0f, 1.0f) * w) - 34;
        int y = std::max(0, h - (int)(std::clamp(p.ic / s_cfg.icMax, 0.0f, 1.0f) * h) - 12);
        // don't cover an earlier curve's label: move up a line at a time
        for (bool moved = true; moved;)
        {
            moved = false;
            for (int o = 0; o < c; o++)
                if (s_curveLbl[o] && !lv_obj_has_flag(s_curveLbl[o], LV_OBJ_FLAG_HIDDEN) &&
                    abs(lv_obj_get_y(s_curveLbl[o]) - y) < 11)
                {
                    y = lv_obj_get_y(s_curveLbl[o]) - 11;
                    moved = true;
                }
            if (y < 0)
                break;
        }
        lv_obj_set_pos(s_curveLbl[c], std::max(0, x), std::max(0, y));
        lv_obj_clear_flag(s_curveLbl[c], LV_OBJ_FLAG_HIDDEN);
    }

    void finish(const char *why)
    {
        linkOverride(false); // back to follow: master off below -> slave off
        if (masterOn())
            PowerSupply.turn(SWITCH::OFF);
        setMasterV(s_saveV);
        setMasterI(s_saveI);
        snprintf(s_msg, sizeof(s_msg), "%s", why);
        s_st = St::DONE;
        if (s_btnLbl)
            lv_label_set_text(s_btnLbl, "Start");
    }

    void startCurve()
    {
        s_iter = 0;
        s_vbPrev = 0;
        s_ibPrev = 0;
        if (s_curve == 0)
            s_vb = 0.6f; // about one Vbe: little base current yet
        else // scale from the last curve: Ib ~ (Vb - 0.65) / Rb
            s_vb = 0.65f + (s_vb - 0.65f) * ibTarget(s_curve) / std::max(1e-9f, ibTarget(s_curve - 1));
        s_vb = std::clamp(s_vb, 0.0f, s_cfg.vbMax);
        setMasterV(servoVce());
        linkOverride(true, s_vb, std::max(2 * ibTarget(s_curve) + 1e-3f, 2e-3f), true);
        s_until = millis() + s_cfg.settleMs + 100;
        s_st = St::SERVO;
    }

    void step()
    {
        const uint32_t now = millis();
        if (s_st == St::IDLE || s_st == St::DONE)
            return;
        if (s_st == St::DEMO)
        {
            // simulated NPN: Ic = beta Ib tanh(Vce/0.12) (1 + Vce/VA), a
            // saturation knee, Early slope, a little noise; no output used
            if ((int32_t)(now - s_until) < 0)
                return;
            s_until = now + 15;
            const float vce = vcePoint(s_pt), ib = ibTarget(s_curve);
            const float noise = ((int)(esp_random() % 2001) - 1000) / 1000.0f;
            Pt &p = pt(s_curve, s_pt);
            p.vce = vce;
            p.ib = ib * (1 + 0.004f * noise);
            p.ic = std::min(s_cfg.icMax, s_demoBeta * p.ib * tanhf(vce / 0.12f) * (1 + vce / 60.0f) + 0.002f * s_cfg.icMax * noise);
            s_ib[s_curve] = (s_ib[s_curve] * s_pt + p.ib) / (s_pt + 1);
            s_nPts[s_curve] = s_pt + 1;
            plotPoint(s_curve, s_pt);
            snprintf(s_msg, sizeof(s_msg), "DEMO (simulated NPN, beta %.0f)  curve %d/%d", s_demoBeta, s_curve + 1, s_cfg.curves);
            if (++s_pt < s_cfg.points && p.vce * p.ic <= s_cfg.pMax)
                return;
            plotCurveLabel(s_curve);
            s_nCurves = s_curve + 1;
            s_pt = 0;
            if (++s_curve >= s_cfg.curves)
            {
                snprintf(s_msg, sizeof(s_msg), "DEMO done: simulated NPN, beta %.0f. Link a 2nd PSU for real parts.", s_demoBeta);
                s_st = St::DONE;
                if (s_btnLbl)
                    lv_label_set_text(s_btnLbl, "Start");
            }
            return;
        }
        if (!linkConnected())
        {
            finish("Stopped: link to the other PSU lost.");
            return;
        }
        if ((int32_t)(now - s_until) < 0)
            return;
        float sv, si;
        bool son;
        const bool haveSlave = linkSlaveReading(sv, si, son);

        switch (s_st)
        {
        case St::START:
            if (!masterOn())
            {
                if (now > s_until + 3000 + PowerSupply.settingParameters.outputDelayMs)
                    finish("Stopped: output did not turn on.");
                return;
            }
            s_curve = 0;
            startCurve();
            return;

        case St::SERVO:
        {
            if (!haveSlave)
                return;
            const float target = ibTarget(s_curve);
            const float ib = si;
            snprintf(s_msg, sizeof(s_msg), "Curve %d/%d: setting Ib %.1fuA (now %.1fuA, Vb %.2fV)",
                     s_curve + 1, s_cfg.curves, target * 1e6f, ib * 1e6f, s_vb);
            const bool ok = fabsf(ib - target) <= std::max(0.03f * target, 1e-6f);
            if (ok || s_iter >= 12)
            {
                s_pt = 0;
                s_nPts[s_curve] = 0;
                s_ib[s_curve] = 0;
                setMasterV(vcePoint(0));
                s_until = now + 3 * s_cfg.settleMs; // from the servo Vce down to 0: output cap discharges
                s_accN = 0;
                s_st = St::SWEEP;
                return;
            }
            float next;
            if (s_iter == 0 || fabsf(ib - s_ibPrev) < 2e-7f) // no slope yet: nudge up/down
                next = s_vb + (ib < target ? 0.15f : -0.1f);
            else // secant
                next = s_vb + (target - ib) * (s_vb - s_vbPrev) / (ib - s_ibPrev);
            next = std::clamp(next, std::max(0.0f, s_vb - 2.0f), std::min(s_cfg.vbMax, s_vb + 3.0f));
            if (s_vb >= s_cfg.vbMax - 1e-3f && ib < target)
            {
                finish("Stopped: base supply at its limit (Vb max). Use a smaller base resistor.");
                return;
            }
            s_vbPrev = s_vb;
            s_ibPrev = ib;
            s_vb = next;
            s_iter++;
            linkOverride(true, s_vb, std::max(2 * target + 1e-3f, 2e-3f), true);
            s_until = now + s_cfg.settleMs;
            return;
        }

        case St::SWEEP:
        {
            // average three readings 20 ms apart
            s_accV += masterV();
            s_accI += masterI();
            s_accB += haveSlave ? si : 0;
            if (++s_accN < 3)
            {
                s_until = now + 20;
                return;
            }
            Pt &p = pt(s_curve, s_pt);
            p.vce = (float)(s_accV / 3);
            p.ic = (float)(s_accI / 3);
            p.ib = (float)(s_accB / 3);
            s_accV = s_accI = s_accB = 0;
            s_accN = 0;
            s_ib[s_curve] = (s_ib[s_curve] * s_pt + p.ib) / (s_pt + 1);
            s_nPts[s_curve] = s_pt + 1;
            plotPoint(s_curve, s_pt);
            char b[16];
            fmtI(b, sizeof(b), p.ic);
            snprintf(s_msg, sizeof(s_msg), "Curve %d/%d  Vce %.2fV  Ic %s", s_curve + 1, s_cfg.curves, p.vce, b);
            const bool hot = p.vce * p.ic > s_cfg.pMax;
            if (++s_pt < s_cfg.points && !hot)
            {
                setMasterV(vcePoint(s_pt));
                s_until = now + s_cfg.settleMs;
                return;
            }
            plotCurveLabel(s_curve);
            s_nCurves = s_curve + 1;
            setMasterV(0);
            if (++s_curve >= s_cfg.curves)
            {
                char d[48];
                snprintf(d, sizeof(d), "Done: %d curves.", s_nCurves);
                finish(d);
                return;
            }
            startCurve();
            return;
        }
        default:
            return;
        }
    }

    void saveCfg()
    {
        Preferences p;
        p.begin("tracer", false);
        p.putBytes("cfg", &s_cfg, sizeof(s_cfg));
        p.end();
    }

    // dropdown helper: options text, values, current value -> index
    struct Choice { const char *opts; const float *vals; int n; float *target; int *itarget; };

    struct Dd { lv_obj_t *dd; const Choice *c; };
    Dd s_dds[5];
    int s_nDd = 0;
    int nearest(const Choice *c)
    {
        int sel = 0;
        const float cur = c->target ? *c->target : (float)*c->itarget;
        for (int k = 0; k < c->n; k++)
            if (fabsf(c->vals[k] - cur) < fabsf(c->vals[sel] - cur))
                sel = k;
        return sel;
    }

    lv_obj_t *row(lv_obj_t *par, const char *name, const Choice *c)
    {
        lv_obj_t *r = lv_obj_create(par);
        lv_obj_remove_style_all(r);
        lv_obj_set_size(r, LV_PCT(100), 21);
        lv_obj_clear_flag(r, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_t *l = lv_label_create(r);
        lv_obj_set_style_text_font(l, &lv_font_montserrat_10, 0);
        lv_obj_set_style_text_color(l, lv_color_hex(0x8A94A6), 0);
        lv_label_set_text(l, name);
        lv_obj_align(l, LV_ALIGN_LEFT_MID, 0, 0);
        lv_obj_t *dd = lv_dropdown_create(r);
        lv_obj_set_size(dd, 62, 20);
        lv_obj_align(dd, LV_ALIGN_RIGHT_MID, 0, 0);
        lv_obj_set_style_text_font(dd, &lv_font_montserrat_10, 0);
        lv_obj_set_style_pad_all(dd, 3, 0);
        lv_dropdown_set_options_static(dd, c->opts);
        int sel = 0;
        const float cur = c->target ? *c->target : (float)*c->itarget;
        for (int k = 0; k < c->n; k++)
            if (fabsf(c->vals[k] - cur) < fabsf(c->vals[sel] - cur))
                sel = k;
        lv_dropdown_set_selected(dd, sel);
        if (s_nDd < 5)
            s_dds[s_nDd++] = {dd, c};
        lv_obj_add_event_cb(dd, [](lv_event_t *e) {
            const Choice *c = (const Choice *)lv_event_get_user_data(e);
            if (tracerBusy()) // settings are fixed while tracing
            {
                lv_dropdown_set_selected(lv_event_get_target(e), nearest(c));
                return;
            }
            const float v = c->vals[lv_dropdown_get_selected(lv_event_get_target(e))];
            if (c->target)
                *c->target = v;
            else
                *c->itarget = (int)v;
            if (s_st == St::IDLE || s_st == St::DONE)
                plotReset();
        }, LV_EVENT_VALUE_CHANGED, (void *)c);
        return r;
    }
}

TracerCfg &tracerCfg() { return s_cfg; }
bool tracerBusy() { return s_st != St::IDLE && s_st != St::DONE; }

bool tracerDemo()
{
    if (tracerBusy())
        return false;
    if (!s_pts)
        s_pts = (Pt *)heap_caps_calloc(kMaxCurves * kMaxPts, sizeof(Pt), MALLOC_CAP_SPIRAM);
    if (!s_pts)
        return false;
    s_cfg.curves = std::clamp(s_cfg.curves, 1, kMaxCurves);
    s_cfg.points = std::clamp(s_cfg.points, 5, kMaxPts);
    // gain chosen so the top curve ends near 85% of Ic max
    s_demoBeta = std::clamp(0.85f * s_cfg.icMax / (s_cfg.ibStep * s_cfg.curves * 1.15f), 20.0f, 900.0f);
    s_nCurves = s_curve = s_pt = 0;
    memset(s_nPts, 0, sizeof(s_nPts));
    memset(s_ib, 0, sizeof(s_ib));
    plotReset();
    s_until = millis();
    s_st = St::DEMO;
    if (s_btnLbl)
        lv_label_set_text(s_btnLbl, "Stop");
    return true;
}

bool tracerStart()
{
    if (tracerBusy())
        return false;
    if (linkRole() != LinkRole::MASTER || !linkConnected())
    {
        snprintf(s_msg, sizeof(s_msg), "Needs a linked PSU: this one master (Settings > PSU Link).");
        return false;
    }
    if (!s_pts)
        s_pts = (Pt *)heap_caps_calloc(kMaxCurves * kMaxPts, sizeof(Pt), MALLOC_CAP_SPIRAM);
    if (!s_pts)
        return false;
    s_cfg.curves = std::clamp(s_cfg.curves, 1, kMaxCurves);
    s_cfg.points = std::clamp(s_cfg.points, 5, kMaxPts);
    s_cfg.vceMax = std::clamp(s_cfg.vceMax, 0.5f, 32.0f);
    saveCfg();
    s_nCurves = 0;
    memset(s_nPts, 0, sizeof(s_nPts));
    plotReset();
    s_saveV = (float)vSetNow();
    s_saveI = (float)iSetNow();
    setMasterV(0);
    setMasterI(s_cfg.icMax);
    if (!masterOn() && !lv_obj_has_state(PowerSupply.powerSwitch.btn, LV_STATE_CHECKED))
        PowerSupply.toggle();
    s_until = millis() + 300;
    s_st = St::START;
    snprintf(s_msg, sizeof(s_msg), "Starting...");
    if (s_btnLbl)
        lv_label_set_text(s_btnLbl, "Stop");
    return true;
}

void tracerStop()
{
    if (s_st == St::DEMO)
    {
        s_st = St::DONE;
        snprintf(s_msg, sizeof(s_msg), "Demo stopped.");
        if (s_btnLbl)
            lv_label_set_text(s_btnLbl, "Start");
    }
    else if (tracerBusy())
        finish("Stopped.");
}

void tracerLoop()
{
    step();
    static uint32_t last = 0;
    if (s_status && millis() - last > 250)
    {
        last = millis();
        for (int k = 0; k < s_nDd; k++) // values set over SCPI
        {
            const int sel = nearest(s_dds[k].c);
            if (!lv_dropdown_is_open(s_dds[k].dd) && lv_dropdown_get_selected(s_dds[k].dd) != sel)
                lv_dropdown_set_selected(s_dds[k].dd, sel);
        }
        char msg[140];
        if (tracerBusy() || s_st == St::DONE)
            snprintf(msg, sizeof(msg), "%s", s_msg);
        else if (!(linkRole() == LinkRole::MASTER && linkConnected()))
            snprintf(msg, sizeof(msg), "No 2nd PSU linked: Start shows a demo (simulated NPN).");
        else if (false)
            snprintf(msg, sizeof(msg), "%s", s_msg);
        else
            snprintf(msg, sizeof(msg), "Ready.%s", linkSlaveMilliamp() ? "" : " Tip: other PSU on mA range for small Ib.");
        if (strcmp(lv_label_get_text(s_status), msg) != 0)
            lv_label_set_text(s_status, msg);
    }
}

String tracerStatus()
{
    static const char *n[] = {"idle", "start", "base", "sweep", "done", "demo"};
    char b[160];
    snprintf(b, sizeof(b), "%s,curve %d/%d,pt %d,%s", n[(int)s_st], s_curve + (tracerBusy() ? 1 : 0),
             s_cfg.curves, s_pt, s_msg);
    return String(b);
}

// "%.4e" / "%.4f" without newlib's printf (~25 us a call: the CSV of 640
// points took 92 ms and stalled the display)
static char *putU(char *o, unsigned long v, int minDigits = 1)
{
    char t[12];
    int n = 0;
    do
        t[n++] = '0' + v % 10, v /= 10;
    while (v);
    while (n < minDigits)
        t[n++] = '0';
    while (n)
        *o++ = t[--n];
    return o;
}
static char *fmtE4(char *o, float v)
{
    if (!(v == v) || v > 3e38f || v < -3e38f)
        return (memcpy(o, "nan", 3), o + 3);
    if (v < 0)
        *o++ = '-', v = -v;
    int e = 0;
    long m = 0;
    if (v > 0)
    {
        e = (int)floorf(log10f(v));
        m = lroundf(v / powf(10.0f, (float)e) * 1e4f);
        if (m >= 100000)
            m /= 10, e++;
        else if (m < 10000)
            m *= 10, e--;
    }
    o = putU(o, m / 10000), *o++ = '.';
    o = putU(o, m % 10000, 4), *o++ = 'e', *o++ = e < 0 ? '-' : '+';
    return putU(o, e < 0 ? -e : e, 2);
}
static char *fmtF4(char *o, float v)
{
    if (v < 0)
        *o++ = '-', v = -v;
    const long m = lroundf(v * 1e4f);
    o = putU(o, m / 10000), *o++ = '.';
    return putU(o, m % 10000, 4);
}

String tracerData()
{
    String r = "curve,ib_A,vce_V,ic_A,ib_pt_A";
    if (!s_pts)
        return r;
    int n = 0;
    for (int c = 0; c < kMaxCurves; c++)
        n += s_nPts[c];
    r.reserve(32 + n * 46); // grown line by line it was copied ~600 times
    char b[80];
    for (int c = 0; c < kMaxCurves; c++)
        for (int k = 0; k < s_nPts[c]; k++)
        {
            char *o = b;
            *o++ = '\n', o = putU(o, c + 1), *o++ = ',';
            o = fmtE4(o, s_ib[c]), *o++ = ',';
            o = fmtF4(o, pt(c, k).vce), *o++ = ',';
            o = fmtE4(o, pt(c, k).ic), *o++ = ',';
            *fmtE4(o, pt(c, k).ib) = 0;
            r += b;
        }
    return r;
}

void createTracerTab(lv_obj_t *parent)
{
    Preferences p;
    p.begin("tracer", true);
    TracerCfg c;
    if (p.getBytes("cfg", &c, sizeof(c)) == sizeof(c))
        s_cfg = c;
    p.end();

    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(parent, 0, LV_PART_MAIN);

    s_chart = lv_chart_create(parent);
    lv_obj_set_size(s_chart, 198, 140);
    lv_obj_set_pos(s_chart, 3, 3);
    lv_obj_set_style_bg_color(s_chart, lv_color_hex(0x0d1117), 0);
    lv_obj_set_style_radius(s_chart, 6, 0);
    lv_obj_set_style_border_width(s_chart, 1, 0);
    lv_obj_set_style_border_color(s_chart, lv_color_hex(0x30363d), 0);
    lv_obj_set_style_pad_all(s_chart, 4, 0);
    lv_obj_set_style_line_color(s_chart, lv_color_hex(0x21262d), LV_PART_MAIN);
    lv_chart_set_div_line_count(s_chart, 5, 5);
    lv_obj_set_style_line_width(s_chart, 2, LV_PART_ITEMS);
    lv_obj_set_style_size(s_chart, 0, LV_PART_INDICATOR);
    lv_chart_set_type(s_chart, LV_CHART_TYPE_SCATTER);
    lv_chart_set_range(s_chart, LV_CHART_AXIS_PRIMARY_X, 0, 1000);
    lv_chart_set_range(s_chart, LV_CHART_AXIS_PRIMARY_Y, 0, 1000);
    lv_chart_set_point_count(s_chart, s_cfg.points);
    for (int k = 0; k < kMaxCurves; k++)
    {
        s_ser[k] = lv_chart_add_series(s_chart, lv_color_hex(kColors[k]), LV_CHART_AXIS_PRIMARY_Y);
        lv_chart_set_all_value(s_chart, s_ser[k], LV_CHART_POINT_NONE);
    }
    auto small = [](lv_obj_t *par, uint32_t col) {
        lv_obj_t *l = lv_label_create(par);
        lv_obj_set_style_text_font(l, &lv_font_montserrat_10, 0);
        lv_obj_set_style_text_color(l, lv_color_hex(col), 0);
        lv_label_set_text(l, "");
        return l;
    };
    s_yLbl = small(s_chart, 0x8A94A6);
    lv_obj_align(s_yLbl, LV_ALIGN_TOP_LEFT, 2, 0);
    s_xLbl = small(s_chart, 0x8A94A6);
    lv_obj_align(s_xLbl, LV_ALIGN_BOTTOM_RIGHT, -2, 0);

    s_status = small(parent, 0xC9D1D9);
    lv_label_set_long_mode(s_status, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(s_status, 198);
    lv_obj_set_pos(s_status, 4, 145);

    // settings card
    lv_obj_t *card = lv_obj_create(parent);
    lv_obj_remove_style_all(card);
    lv_obj_set_size(card, 115, 165);
    lv_obj_set_pos(card, 203, 3);
    lv_obj_set_style_bg_color(card, lv_color_hex(0x161b22), 0);
    lv_obj_set_style_bg_opa(card, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(card, 6, 0);
    lv_obj_set_style_pad_all(card, 4, 0);
    lv_obj_set_style_pad_row(card, 2, 0);
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);

    static const float vVce[] = {1, 2, 5, 10, 15, 20, 30};
    static const float vIc[] = {0.005f, 0.01f, 0.02f, 0.05f, 0.1f, 0.2f, 0.5f, 1, 2, 5};
    static const float vP[] = {0.25f, 0.5f, 1, 2, 5, 10, 20};
    static const float vIb[] = {1e-6f, 2e-6f, 5e-6f, 10e-6f, 20e-6f, 50e-6f, 100e-6f, 200e-6f, 500e-6f, 1e-3f, 2e-3f, 5e-3f, 10e-3f};
    static const float vN[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    static const Choice cVce = {"1V\n2V\n5V\n10V\n15V\n20V\n30V", vVce, 7, &s_cfg.vceMax, nullptr};
    static const Choice cIc = {"5mA\n10mA\n20mA\n50mA\n100mA\n200mA\n500mA\n1A\n2A\n5A", vIc, 10, &s_cfg.icMax, nullptr};
    static const Choice cP = {"0.25W\n0.5W\n1W\n2W\n5W\n10W\n20W", vP, 7, &s_cfg.pMax, nullptr};
    static const Choice cIb = {"1uA\n2uA\n5uA\n10uA\n20uA\n50uA\n100uA\n200uA\n500uA\n1mA\n2mA\n5mA\n10mA", vIb, 13, &s_cfg.ibStep, nullptr};
    static const Choice cN = {"1\n2\n3\n4\n5\n6\n7\n8\n9\n10", vN, 10, nullptr, &s_cfg.curves};
    row(card, "Vce max", &cVce);
    row(card, "Ic max", &cIc);
    row(card, "P max", &cP);
    row(card, "Ib step", &cIb);
    row(card, "Curves", &cN);

    s_btn = lv_btn_create(card);
    lv_obj_set_size(s_btn, LV_PCT(100), 26);
    s_btnLbl = lv_label_create(s_btn);
    lv_label_set_text(s_btnLbl, "Start");
    lv_obj_center(s_btnLbl);
    lv_obj_add_event_cb(s_btn, [](lv_event_t *) {
        if (tracerBusy())
            tracerStop();
        else if (linkRole() == LinkRole::MASTER && linkConnected())
            tracerStart();
        else
            tracerDemo(); // no second PSU: show a simulated transistor
    }, LV_EVENT_CLICKED, nullptr);

    plotReset();
}
