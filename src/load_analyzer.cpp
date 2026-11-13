#include "load_analyzer.h"
#include "device.hpp"
#include "myFonts.h"
#include "esp_heap_caps.h"
#include <atomic>
#include <cmath>

extern Device PowerSupply;

// ─── Capture (Core 0 writes, Core 1 reads once s_capturing is false) ────────
static constexpr int kMaxSamples = 10000; // ~10 s sweep at ~800 pairs/s
static float *s_t = nullptr;   // s since capture start
static float *s_v = nullptr;   // V measured
static float *s_i = nullptr;   // A measured
static float *s_cmd = nullptr; // V commanded at that moment
static uint8_t *s_ph = nullptr; // sweep phase
static std::atomic<bool> s_capturing{false};
static std::atomic<int> s_n{0};
static int s_cap = 0;          // samples wanted
static uint32_t s_t0 = 0;
static volatile float s_vcmd = 0;
static volatile uint8_t s_phase = 0;

enum Phase : uint8_t { PH_SETTLE = 0, PH_UP = 1, PH_HOLD = 2, PH_DOWN = 3, PH_END = 4 };

void loadAnalyzerAdcHook(double volts, double amps)
{
    if (!s_capturing.load(std::memory_order_relaxed) || !std::isfinite(amps))
        return; // (NAN right after a current-range switch)
    const int n = s_n.load(std::memory_order_relaxed);
    if (n >= s_cap)
    {
        s_capturing.store(false);
        return;
    }
    const uint32_t now = micros();
    if (n == 0)
        s_t0 = now;
    s_t[n] = (now - s_t0) * 1e-6f;
    s_v[n] = volts;
    s_i[n] = amps;
    s_cmd[n] = s_vcmd;
    s_ph[n] = s_phase;
    s_n.store(n + 1, std::memory_order_release);
}

static bool alloc_buffers()
{
    if (s_t)
        return true;
    auto ps = [](size_t b) { return heap_caps_malloc(b, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT); };
    s_t = (float *)ps(kMaxSamples * sizeof(float));
    s_v = (float *)ps(kMaxSamples * sizeof(float));
    s_i = (float *)ps(kMaxSamples * sizeof(float));
    s_cmd = (float *)ps(kMaxSamples * sizeof(float));
    s_ph = (uint8_t *)ps(kMaxSamples);
    return s_t && s_v && s_i && s_cmd && s_ph;
}

// ─── UI ─────────────────────────────────────────────────────────────────────
static lv_obj_t *s_chart = nullptr;
static lv_chart_series_t *s_ser1 = nullptr, *s_ser2 = nullptr;
static lv_obj_t *s_result = nullptr;
static lv_obj_t *s_axis = nullptr;
static lv_obj_t *s_bar = nullptr;    // progress
static lv_obj_t *s_symbol = nullptr; // drawn component of the result

enum Sym { SYM_NONE, SYM_OPEN, SYM_R, SYM_C, SYM_RC, SYM_DIODE, SYM_LAMP };

static void line(const lv_point_t *pts, int n)
{
    lv_obj_t *l = lv_line_create(s_symbol);
    lv_line_set_points(l, pts, n);
    lv_obj_set_style_line_width(l, 2, 0);
    lv_obj_set_style_line_color(l, lv_palette_main(LV_PALETTE_AMBER), 0);
    lv_obj_set_style_line_rounded(l, true, 0);
}

// Schematic symbol, 104 x 40 box, leads left and right at mid height
static void draw_symbol(Sym sym)
{
    if (!s_symbol)
        return;
    lv_obj_clean(s_symbol);
    static const lv_point_t rL[] = {{0, 20}, {28, 20}, {32, 13}, {40, 27}, {48, 13}, {56, 27}, {64, 13}, {72, 27}, {76, 20}, {104, 20}};
    static const lv_point_t cA[] = {{0, 20}, {47, 20}}, cP1[] = {{47, 6}, {47, 34}}, cP2[] = {{57, 6}, {57, 34}}, cB[] = {{57, 20}, {104, 20}};
    // R || C: resistor on top (y 9), capacitor below (y 32), joined at x 14 / 90
    static const lv_point_t pr[] = {{14, 9}, {34, 9}, {37, 4}, {43, 14}, {49, 4}, {55, 14}, {61, 4}, {67, 14}, {70, 9}, {90, 9}};
    static const lv_point_t pcA[] = {{14, 32}, {48, 32}}, pc1[] = {{48, 24}, {48, 40}}, pc2[] = {{56, 24}, {56, 40}}, pcB[] = {{56, 32}, {90, 32}};
    static const lv_point_t pL[] = {{0, 20}, {14, 20}}, pR[] = {{90, 20}, {104, 20}}, pv1[] = {{14, 9}, {14, 32}}, pv2[] = {{90, 9}, {90, 32}};
    static const lv_point_t dA[] = {{0, 20}, {40, 20}}, dT[] = {{40, 8}, {40, 32}, {62, 20}, {40, 8}}, dK[] = {{62, 8}, {62, 32}}, dB[] = {{62, 20}, {104, 20}};
    static const lv_point_t oA[] = {{0, 20}, {40, 20}}, oB[] = {{64, 20}, {104, 20}};
    switch (sym)
    {
    case SYM_R: line(rL, 10); break;
    case SYM_C: line(cA, 2); line(cP1, 2); line(cP2, 2); line(cB, 2); break;
    case SYM_RC:
        line(pr, 10); line(pcA, 2); line(pc1, 2); line(pc2, 2); line(pcB, 2);
        line(pL, 2); line(pR, 2); line(pv1, 2); line(pv2, 2);
        break;
    case SYM_DIODE: line(dA, 2); line(dT, 4); line(dK, 2); line(dB, 2); break;
    case SYM_OPEN: line(oA, 2); line(oB, 2); break;
    case SYM_LAMP:
    {
        // Circle (r 14 around 52,20) with an X: the lamp symbol
        static lv_point_t circ[21];
        static bool init = false;
        if (!init)
        {
            for (int k = 0; k <= 20; k++)
                circ[k] = {(lv_coord_t)lroundf(52 + 14 * cosf(k * 2 * M_PI / 20)), (lv_coord_t)lroundf(20 + 14 * sinf(k * 2 * M_PI / 20))};
            init = true;
        }
        static const lv_point_t lA[] = {{0, 20}, {38, 20}}, lB[] = {{66, 20}, {104, 20}};
        static const lv_point_t x1[] = {{42, 10}, {62, 30}}, x2[] = {{62, 10}, {42, 30}};
        line(lA, 2); line(lB, 2); line(circ, 21); line(x1, 2); line(x2, 2);
        break;
    }
    default: break;
    }
}

static lv_obj_t *s_pct = nullptr;
static void progress(int pct)
{
    if (s_pct)
    {
        if (pct < 0)
            lv_label_set_text(s_pct, "");
        else
            lv_label_set_text_fmt(s_pct, "%d%%", std::min(pct, 100));
    }
    if (!s_bar)
        return;
    if (pct < 0)
        lv_obj_add_flag(s_bar, LV_OBJ_FLAG_HIDDEN);
    else
    {
        lv_obj_clear_flag(s_bar, LV_OBJ_FLAG_HIDDEN);
        lv_bar_set_value(s_bar, pct, LV_ANIM_OFF);
    }
}
static constexpr int kPlotPts = 160;
static String s_resultText = "No result yet";

static lv_obj_t *s_ytop = nullptr, *s_xright = nullptr;
static lv_obj_t *s_btns[3] = {};

// Result card: first line = title (amber), then "key value" rows with the
// key dimmed - LVGL recolor markup, built from the plain text
static void set_axes(const char *title, const String &xmax, const String &ymax, bool legend)
{
    if (!s_axis)
        return;
    String t = title;
    if (legend)
        t += "   #FFC107 up#  #00BCD4 down#";
    lv_label_set_text(s_axis, t.c_str());
    lv_label_set_text(s_xright, xmax.c_str());
    lv_label_set_text(s_ytop, ymax.c_str());
}

static void show_result(const String &multi)
{
    if (s_result)
    {
        String out;
        int start = 0, line = 0;
        while (start <= (int)multi.length())
        {
            int nl = multi.indexOf('\n', start);
            if (nl < 0)
                nl = multi.length();
            String ln = multi.substring(start, nl);
            if (line)
                out += "\n";
            if (line == 0)
                out += "#FFC107 " + ln + "#";
            else
            {
                const int sp = ln.indexOf(' ');
                const char nx = sp > 0 && sp + 1 < (int)ln.length() ? ln[sp + 1] : 0;
                if (sp > 0 && sp < 7 && (isdigit((unsigned char)nx) || strchr("<>+-~", nx)) && nx)
                    out += "#8A94A6 " + ln.substring(0, sp) + "#" + ln.substring(sp);
                else
                    out += ln;
            }
            start = nl + 1;
            line++;
        }
        lv_label_set_text(s_result, out.c_str());
    }
    String one = multi;
    one.replace("\n", ";");
    s_resultText = one;
}

// ─── Sweep ──────────────────────────────────────────────────────────────────
static constexpr float kSettleS = 1.0f, kRampS = 4.0f, kHoldS = 1.0f;
static lv_timer_t *s_timer = nullptr;
static uint32_t s_startMs = 0;
static float s_vtop = 0;
static int s_savedAdj = 0;
static bool s_isZero = false;
static bool s_isFft = false;

struct ZeroCal { uint32_t magic; float i0, g, c; };

// Step 1 result (detect), refined by step 2
struct Detect { Sym sym; double g, c, i0, vknee, cLim, gLim; double nonlin = NAN; double rCold = NAN; };
static Detect s_det{};
static bool s_needStage2 = false;
static bool s_limited = false; // output never reached the top: I-Set (CC) held it
static int s_stage = 1;
static constexpr float kS2RampS = 1.5f, kS2HoldS = 6.0f;
static constexpr float kS2SwitchS = kS2RampS + 1.0f;  // to the mA range (leakage)
static constexpr float kS2LeakFromS = kS2RampS + 2.5f; // leakage window start
static bool s_maSwitched = false, s_maPrev = false;
void setCurrentRange(bool mA);
static void restore_range()
{
    if (s_maSwitched)
        setCurrentRange(s_maPrev);
    s_maSwitched = false;
}
static ZeroCal s_zero{0, 0, 0, 0};
static constexpr uint32_t kZeroMagic = 0x10AD0001;

static float user_vset() { return (PowerSupply.Voltage.adjValue - PowerSupply.Voltage.adjOffset) / PowerSupply.Voltage.adjFactor; }
static void set_volts(double v) { PowerSupply.Voltage.SetUpdate(v * PowerSupply.Voltage.adjFactor + PowerSupply.Voltage.adjOffset); }

static bool output_usable()
{
    const DEVICE st = PowerSupply.getStatus();
    return st != DEVICE::OFF && st != DEVICE::FUN;
}

// Solve 3x3 A x = b (Gaussian elimination, partial pivot)
static bool solve3(double A[3][3], double b[3], double x[3])
{
    for (int c = 0; c < 3; c++)
    {
        int p = c;
        for (int r = c + 1; r < 3; r++)
            if (fabs(A[r][c]) > fabs(A[p][c]))
                p = r;
        if (fabs(A[p][c]) < 1e-30)
            return false;
        if (p != c)
        {
            for (int k = 0; k < 3; k++)
                std::swap(A[c][k], A[p][k]);
            std::swap(b[c], b[p]);
        }
        for (int r = c + 1; r < 3; r++)
        {
            const double f = A[r][c] / A[c][c];
            for (int k = c; k < 3; k++)
                A[r][k] -= f * A[c][k];
            b[r] -= f * b[c];
        }
    }
    for (int r = 2; r >= 0; r--)
    {
        double s = b[r];
        for (int k = r + 1; k < 3; k++)
            s -= A[r][k] * x[k];
        x[r] = s / A[r][r];
    }
    return true;
}

static String fmtR(double ohm)
{
    char b[24];
    if (!std::isfinite(ohm) || ohm <= 0 || ohm > 1e10)
        return "open";
    if (ohm >= 1e6)
        snprintf(b, sizeof b, "%.3g MOhm", ohm / 1e6);
    else if (ohm >= 1e3)
        snprintf(b, sizeof b, "%.3g kOhm", ohm / 1e3);
    else
        snprintf(b, sizeof b, "%.3g Ohm", ohm);
    return b;
}

static String fmtC(double f)
{
    char b[24];
    const double a = fabs(f);
    if (a >= 1e-3)
        snprintf(b, sizeof b, "%.3g mF", f * 1e3);
    else if (a >= 1e-6)
        snprintf(b, sizeof b, "%.3g uF", f * 1e6);
    else
        snprintf(b, sizeof b, "%.3g nF", f * 1e9);
    return b;
}

static String fmtA(double a)
{
    char b[24];
    if (fabs(a) >= 1.0)
        snprintf(b, sizeof b, "%.3f A", a);
    else if (fabs(a) >= 1e-3)
        snprintf(b, sizeof b, "%.2f mA", a * 1e3);
    else
        snprintf(b, sizeof b, "%.1f uA", a * 1e6);
    return b;
}

// I-V: mean current per voltage bin (noise averages out), up and down apart
static void plot_iv(int n)
{
    static float sum[2][kPlotPts];
    static uint16_t cnt[2][kPlotPts];
    memset(sum, 0, sizeof sum);
    memset(cnt, 0, sizeof cnt);
    for (int q = 0; q < n; q++)
    {
        const int sIdx = s_ph[q] == PH_UP ? 0 : s_ph[q] == PH_DOWN ? 1 : -1;
        if (sIdx < 0)
            continue;
        const int b = std::min(kPlotPts - 1, std::max(0, (int)(kPlotPts * s_v[q] / s_vtop)));
        sum[sIdx][b] += s_i[q];
        cnt[sIdx][b]++;
    }
    float imax = 1e-4f;
    for (int sIdx = 0; sIdx < 2; sIdx++)
        for (int b = 0; b < kPlotPts; b++)
            if (cnt[sIdx][b] && sum[sIdx][b] / cnt[sIdx][b] > imax)
                imax = sum[sIdx][b] / cnt[sIdx][b];
    lv_chart_set_type(s_chart, LV_CHART_TYPE_SCATTER);
    lv_chart_set_point_count(s_chart, kPlotPts);
    lv_chart_set_range(s_chart, LV_CHART_AXIS_PRIMARY_X, 0, 1000);
    lv_chart_set_range(s_chart, LV_CHART_AXIS_PRIMARY_Y, 0, 1000);
    for (int sIdx = 0; sIdx < 2; sIdx++)
    {
        lv_chart_series_t *ser = sIdx ? s_ser2 : s_ser1;
        int j = 0;
        for (int b = 0; b < kPlotPts; b++)
            if (cnt[sIdx][b])
                lv_chart_set_value_by_id2(s_chart, ser, j++, (lv_coord_t)(1000 * b / (kPlotPts - 1)),
                                          (lv_coord_t)lroundf(1000.0f * sum[sIdx][b] / cnt[sIdx][b] / imax));
        for (; j < kPlotPts; j++)
            lv_chart_set_value_by_id2(s_chart, ser, j, LV_CHART_POINT_NONE, LV_CHART_POINT_NONE);
    }
    lv_chart_refresh(s_chart);
    char b[16];
    snprintf(b, sizeof b, "%.2f V", s_vtop);
    set_axes("Current vs voltage", b, fmtA(imax), true);
}

// I against time, mean per time bin (step 2 of a capacitor: charge, then leak)
static void plot_it(int n, float t0)
{
    static float sum[kPlotPts];
    static uint16_t cnt[kPlotPts];
    memset(sum, 0, sizeof sum);
    memset(cnt, 0, sizeof cnt);
    const float tEnd = s_t[n - 1];
    for (int q = 0; q < n; q++)
    {
        if (s_t[q] < t0)
            continue;
        const int b = std::min(kPlotPts - 1, (int)(kPlotPts * (s_t[q] - t0) / std::max(1e-3f, tEnd - t0)));
        sum[b] += s_i[q];
        cnt[b]++;
    }
    float imax = 1e-5f;
    for (int b = 0; b < kPlotPts; b++)
        if (cnt[b] && sum[b] / cnt[b] > imax)
            imax = sum[b] / cnt[b];
    lv_chart_set_type(s_chart, LV_CHART_TYPE_LINE);
    lv_chart_set_point_count(s_chart, kPlotPts);
    lv_chart_set_range(s_chart, LV_CHART_AXIS_PRIMARY_Y, 0, 1000);
    for (int b = 0; b < kPlotPts; b++)
    {
        lv_chart_set_value_by_id(s_chart, s_ser1, b, cnt[b] ? (lv_coord_t)lroundf(1000.0f * std::max(0.0f, sum[b] / cnt[b]) / imax) : LV_CHART_POINT_NONE);
        lv_chart_set_value_by_id(s_chart, s_ser2, b, LV_CHART_POINT_NONE);
    }
    lv_chart_refresh(s_chart);
    char b[16];
    snprintf(b, sizeof b, "%.0f s", tEnd - t0);
    set_axes("Current vs time", b, fmtA(imax), false);
}

static void analyse_sweep()
{
    const int n = s_n.load(std::memory_order_acquire);
    if (n < 50)
    {
        show_result("Too few samples");
        return;
    }
    {
        // Highest voltage actually reached while holding at the top
        double vmax = 0;
        for (int q = 0; q < n; q++)
            if (s_ph[q] == PH_HOLD && s_v[q] > vmax)
                vmax = s_v[q];
        s_limited = vmax < 0.9 * s_vtop;
    }
    if (s_limited && !s_isZero)
    {
        char b[96];
        snprintf(b, sizeof b, "Current limit\nI-Set held the\noutput below\n%.2f V.\nRaise I-Set\nand sweep again", s_vtop);
        draw_symbol(SYM_NONE);
        plot_iv(n);
        show_result(b);
        return;
    }
    double imaxUp = 0;
    for (int q = 0; q < n; q++)
        if (s_ph[q] == PH_UP && s_i[q] > imaxUp)
            imaxUp = s_i[q];
    // Knee: highest up-ramp V with I under 3% of the max (and at least 0.5 mA)
    const double thr = std::max(0.03 * imaxUp, 0.0005);
    double vknee = 0;
    for (int q = 0; q < n; q++)
        if (s_ph[q] == PH_UP && s_i[q] < thr && s_v[q] > vknee)
            vknee = s_v[q];
    const bool knee = vknee > 0.3 && vknee < 0.9 * s_vtop && imaxUp > 0.002;

    // dV/dt per sample: least-squares slope over +-k samples of the same
    // phase; NAN = not used in the fit. s_cmd is reused for it after the
    // follow check (the command is no longer needed).
    static constexpr int k = 6;
    int used = 0;
    for (int q = 0; q < n; q++)
    {
        float d = NAN;
        const uint8_t ph = s_ph[q];
        const bool ok = q >= k && q < n - k && (ph == PH_UP || ph == PH_DOWN || ph == PH_HOLD) &&
                        s_ph[q - k] == ph && s_ph[q + k] == ph &&
                        // the output follows the command (down ramp: a capacitor can
                        // hold the voltage up - the PSU can't sink)
                        fabs(s_v[q] - s_cmd[q]) <= 0.03 * s_vtop + 0.05 &&
                        !(knee && s_v[q] < vknee + 0.2);
        if (ok)
        {
            double st = 0, sv = 0, stt = 0, stv = 0;
            for (int m = q - k; m <= q + k; m++)
            {
                st += s_t[m];
                sv += s_v[m];
                stt += (double)s_t[m] * s_t[m];
                stv += (double)s_t[m] * s_v[m];
            }
            const int M = 2 * k + 1;
            const double den = M * stt - st * st;
            if (den > 0)
                d = (M * stv - st * sv) / den;
        }
        s_ph[q] = std::isfinite(d) ? (ph | 0x80) : ph; // mark used
        s_cmd[q] = d; // safe: s_cmd[q] was read above, before this write
        if (std::isfinite(d))
            used++;
    }
    double A[3][3] = {}, bvec[3] = {};
    for (int q = 0; q < n; q++)
    {
        if (!(s_ph[q] & 0x80))
            continue;
        const double x[3] = {1.0, s_v[q], s_cmd[q]};
        for (int r = 0; r < 3; r++)
        {
            for (int c = 0; c < 3; c++)
                A[r][c] += x[r] * x[c];
            bvec[r] += x[r] * s_i[q];
        }
    }
    double A2[3][3];
    memcpy(A2, A, sizeof A);
    double p[3] = {0, 0, 0};
    if (used < 30 || !solve3(A2, bvec, p))
    {
        for (int q = 0; q < n; q++)
            s_ph[q] &= 0x7f;
        show_result("Fit failed\n(too few points)");
        plot_iv(n);
        return;
    }
    // Residual and the standard error of each parameter: se_j = rms*sqrt((A^-1)_jj)
    double ss = 0;
    for (int q = 0; q < n; q++)
        if (s_ph[q] & 0x80)
        {
            const double e = s_i[q] - (p[0] + p[1] * s_v[q] + p[2] * s_cmd[q]);
            ss += e * e;
        }
    for (int q = 0; q < n; q++)
        s_ph[q] &= 0x7f;
    const double rms = sqrt(ss / std::max(1, used - 3));
    double se[3];
    for (int j = 0; j < 3; j++)
    {
        double M[3][3], e[3] = {0, 0, 0}, col[3];
        memcpy(M, A, sizeof M);
        e[j] = 1;
        se[j] = solve3(M, e, col) && col[j] > 0 ? rms * sqrt(col[j]) : INFINITY;
    }

    double i0 = p[0], g = p[1], c = p[2];
    if (s_isZero)
    {
        s_zero = {kZeroMagic, (float)i0, (float)g, (float)c};
        PowerSupply.StoreMem.begin("my-app", false);
        PowerSupply.StoreMem.putBytes("load_zero", &s_zero, sizeof(s_zero));
        PowerSupply.StoreMem.end();
        String r = "Zero stored\nC0 " + fmtC(c) + "\nR0 " + fmtR(1.0 / g) + "\nI0 " + fmtA(i0);
        show_result(r);
        plot_iv(n);
        return;
    }
    if (s_zero.magic == kZeroMagic)
    {
        i0 -= s_zero.i0;
        g -= s_zero.g;
        c -= s_zero.c;
    }
    // Significant = 4 standard errors, and R must carry > 0.2 mA at the top
    const bool hasC = c > 4 * se[2];
    const bool hasR = g > 4 * se[1] && g * s_vtop > 0.0002;
    s_det = {knee ? SYM_DIODE : hasR && hasC ? SYM_RC : hasR ? SYM_R : hasC ? SYM_C : SYM_OPEN,
             g, c, i0, vknee, 4 * se[2], 4 * se[1]};
    draw_symbol(s_det.sym);
    plot_iv(n);

    // Resistor linearity: slope through the origin, lower vs upper half.
    // Also for R || C: a filament's thermal lag makes up and down differ
    // like a capacitor would, but its resistance rising with voltage gives
    // it away - checked first.
    if (s_det.sym == SYM_R || s_det.sym == SYM_RC)
    {
        double a[2] = {0, 0}, b2[2] = {0, 0};
        for (int q = 0; q < n; q++)
        {
            if (s_ph[q] != PH_UP || s_v[q] < 0.1 * s_vtop)
                continue;
            const int h = s_v[q] >= 0.55 * s_vtop;
            a[h] += (double)s_v[q] * (s_i[q] - p[0]);
            b2[h] += (double)s_v[q] * s_v[q];
        }
        s_det.nonlin = (a[0] > 0 && a[1] > 0) ? (a[0] / b2[0]) / (a[1] / b2[1]) - 1.0 : NAN; // R_hi/R_lo - 1
        // Cold resistance: slope through the origin over the lowest fifth
        double al = 0, bl = 0;
        for (int q = 0; q < n; q++)
            if (s_ph[q] == PH_UP && s_v[q] >= 0.05 * s_vtop && s_v[q] <= 0.25 * s_vtop)
            {
                al += (double)s_v[q] * (s_i[q] - p[0]);
                bl += (double)s_v[q] * s_v[q];
            }
        s_det.rCold = al > 0 ? bl / al : NAN;
        // Resistance rising >10% with voltage: a filament (lamp / bulb)
        if (!(s_det.nonlin > 0.10) && s_det.sym == SYM_RC)
            ; // real R || C: keep
        else if (s_det.nonlin > 0.10)
        {
            s_det.sym = SYM_LAMP;
            draw_symbol(SYM_LAMP);
        }
    }
    if (s_det.sym == SYM_DIODE)
    {
        // Forward voltage where the up-ramp current (bin means) first passes 1 / 10 mA
        static float sum[kPlotPts];
        static uint16_t cnt[kPlotPts];
        memset(sum, 0, sizeof sum);
        memset(cnt, 0, sizeof cnt);
        for (int q = 0; q < n; q++)
            if (s_ph[q] == PH_UP)
            {
                const int bb = std::min(kPlotPts - 1, std::max(0, (int)(kPlotPts * s_v[q] / s_vtop)));
                sum[bb] += s_i[q];
                cnt[bb]++;
            }
        auto vAt = [&](double ia) -> double
        {
            float prevV = 0, prevI = 0;
            bool have = false;
            for (int bb = 0; bb < kPlotPts; bb++)
            {
                if (!cnt[bb])
                    continue;
                const float v = s_vtop * (bb + 0.5f) / kPlotPts, i = sum[bb] / cnt[bb];
                if (have && prevI < ia && i >= ia)
                    return prevV + (v - prevV) * (ia - prevI) / (i - prevI);
                prevV = v, prevI = i, have = true;
            }
            return NAN;
        };
        String r = "Diode / LED";
        char b[40];
        const double v1 = vAt(0.001), v10 = vAt(0.010);
        if (std::isfinite(v1)) { snprintf(b, sizeof b, "\nVf %.3f V @1mA", v1); r += b; }
        if (std::isfinite(v10)) { snprintf(b, sizeof b, "\nVf %.3f V @10mA", v10); r += b; }
        r += "\nrd " + fmtR(1.0 / g) + "\n(above knee)";
        snprintf(b, sizeof b, "\nKnee ~%.2f V", vknee);
        r += b;
        show_result(r);
        return;
    }
    if (s_det.sym == SYM_OPEN)
    {
        show_result("Open\nnothing found\nC < " + fmtC(s_det.cLim) + "\nR > " + fmtR(1.0 / std::max(s_det.gLim, 1e-9)));
        return;
    }
    s_needStage2 = true; // R, C, R||C: detail step next (finish())
}

// ─── FFT ────────────────────────────────────────────────────────────────────
static constexpr int kFftN = 1024;

static void fft(float *re, float *im, int n)
{
    for (int i = 1, j = 0; i < n; i++)
    {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1)
            j ^= bit;
        j ^= bit;
        if (i < j)
        {
            std::swap(re[i], re[j]);
            std::swap(im[i], im[j]);
        }
    }
    for (int len = 2; len <= n; len <<= 1)
    {
        const float ang = -2 * M_PI / len;
        const float wr = cosf(ang), wi = sinf(ang);
        for (int i = 0; i < n; i += len)
        {
            float cr = 1, ci = 0;
            for (int j = 0; j < len / 2; j++)
            {
                const float ur = re[i + j], ui = im[i + j];
                const float vr = re[i + j + len / 2] * cr - im[i + j + len / 2] * ci;
                const float vi = re[i + j + len / 2] * ci + im[i + j + len / 2] * cr;
                re[i + j] = ur + vr;
                im[i + j] = ui + vi;
                re[i + j + len / 2] = ur - vr;
                im[i + j + len / 2] = ui - vi;
                const float nr = cr * wr - ci * wi;
                ci = cr * wi + ci * wr;
                cr = nr;
            }
        }
    }
}

static void analyse_fft()
{
    const int n = s_n.load(std::memory_order_acquire);
    if (n < kFftN)
    {
        show_result("FFT: capture short");
        return;
    }
    // Reuse the V / cmd buffers as FFT work space (cmd holds V for the text)
    float *re = s_cmd, *im = (float *)heap_caps_malloc(kFftN * sizeof(float), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!im)
    {
        show_result("FFT: no memory");
        return;
    }
    const double fs = (kFftN - 1) / std::max(1e-6, (double)(s_t[kFftN - 1] - s_t[0]));
    double mean = 0, vmean = 0;
    for (int q = 0; q < kFftN; q++)
    {
        mean += s_i[q];
        vmean += s_v[q];
    }
    mean /= kFftN;
    vmean /= kFftN;
    double rms = 0, vrms = 0, wsum = 0;
    for (int q = 0; q < kFftN; q++)
    {
        const double d = s_i[q] - mean;
        rms += d * d;
        vrms += (s_v[q] - vmean) * (s_v[q] - vmean);
        const float w = 0.5f - 0.5f * cosf(2 * M_PI * q / (kFftN - 1)); // Hann
        wsum += w;
        re[q] = d * w;
        im[q] = 0;
    }
    rms = sqrt(rms / kFftN);
    vrms = sqrt(vrms / kFftN);
    fft(re, im, kFftN);
    const int half = kFftN / 2;
    float amax = 1e-9f;
    int kpk = 1;
    for (int q = 1; q < half; q++)
    {
        re[q] = 2.0f * sqrtf(re[q] * re[q] + im[q] * im[q]) / wsum; // peak amplitude, A
        if (q > 1 && re[q] > amax)
        {
            amax = re[q];
            kpk = q;
        }
    }
    heap_caps_free(im);

    // Plot: bins 1..half-1 grouped to kPlotPts, max per group
    lv_chart_set_type(s_chart, LV_CHART_TYPE_LINE);
    lv_chart_set_point_count(s_chart, kPlotPts);
    lv_chart_set_range(s_chart, LV_CHART_AXIS_PRIMARY_Y, 0, 1000);
    for (int j = 0; j < kPlotPts; j++)
    {
        const int a = 1 + (long)j * (half - 1) / kPlotPts, b = 1 + (long)(j + 1) * (half - 1) / kPlotPts;
        float m = 0;
        for (int q = a; q < std::max(b, a + 1); q++)
            m = std::max(m, re[q]);
        lv_chart_set_value_by_id(s_chart, s_ser1, j, (lv_coord_t)lroundf(1000.0f * m / amax));
        lv_chart_set_value_by_id(s_chart, s_ser2, j, LV_CHART_POINT_NONE);
    }
    lv_chart_refresh(s_chart);
    char b[16];
    snprintf(b, sizeof b, "%.0f Hz", fs / 2);
    set_axes("Current spectrum", b, fmtA(amax), false);

    char t[200];
    snprintf(t, sizeof t, "FFT of I\nfs %.0f Hz\nPeak %.1f Hz\n  %s\nI rms %s\nI dc %s\nV rms %.2f mV",
             fs, kpk * fs / kFftN, fmtA(amax).c_str(), fmtA(rms).c_str(), fmtA(mean).c_str(), vrms * 1e3);
    show_result(t);
}

// ─── Runner (LVGL timer, Core 1) ────────────────────────────────────────────
static void stage2_start();
static void end_run_impl();
static void end_run() { end_run_impl(); }
static void analyse_stage2();

static void finish()
{
    progress(-1);
    if (s_timer)
    {
        lv_timer_del(s_timer);
        s_timer = nullptr;
    }
    s_capturing.store(false);
    if (s_isFft)
    {
        end_run();
        analyse_fft();
        return;
    }
    if (s_stage == 2)
    {
        PowerSupply.Voltage.SetUpdate(s_savedAdj);
        restore_range();
        end_run();
        analyse_stage2();
        return;
    }
    s_needStage2 = false;
    analyse_sweep();
    if (s_needStage2 && !s_isZero && output_usable())
        stage2_start();
    else
    {
        PowerSupply.Voltage.SetUpdate(s_savedAdj);
        end_run();
    }
}

// ─── Step 2: ramp back to the top and hold (leakage / self-heating) ─────────
static void stage2_cb(lv_timer_t *)
{
    const float t = (millis() - s_startMs) * 1e-3f;
    if (t >= kS2RampS + kS2HoldS || !output_usable())
    {
        if (!output_usable())
        {
            s_capturing.store(false);
            lv_timer_del(s_timer);
            s_timer = nullptr;
            PowerSupply.Voltage.SetUpdate(s_savedAdj);
            restore_range();
            progress(-1);
            end_run();
            show_result("Stopped:\noutput off");
            return;
        }
        s_phase = PH_END;
        finish();
        return;
    }
    const bool ramp = t < kS2RampS;
    // Capacitor leakage is uA: read it on the mA range (far less noise), if the
    // current already fits there
    if (s_det.sym != SYM_R && s_det.sym != SYM_LAMP && !s_maSwitched && t >= kS2SwitchS &&
        fabs(PowerSupply.Current.oldValue * (PowerSupply.mA_Active ? 0.001 : 1.0)) < 0.005)
    {
        s_maPrev = PowerSupply.mA_Active;
        s_maSwitched = true;
        setCurrentRange(true);
    }
    const float v = ramp ? s_vtop * t / kS2RampS : s_vtop;
    static uint32_t lastUi = 0;
    if (millis() - lastUi > 150)
    {
        lastUi = millis();
        const bool cap = s_det.sym != SYM_R && s_det.sym != SYM_LAMP;
        char b[120];
        snprintf(b, sizeof b, "Found: %s\nStep 2: %s\n%s\n%.2f V  %s\n%.0f s left",
                 cap ? (s_det.sym == SYM_RC ? "R || C" : "capacitor") : s_det.sym == SYM_LAMP ? "lamp" : "resistor",
                 cap ? "leakage" : "heating",
                 ramp ? "ramp to top" : "holding",
                 v, fmtA(PowerSupply.Current.oldValue * (PowerSupply.mA_Active ? 0.001 : 1.0)).c_str(),
                 kS2RampS + kS2HoldS - t);
        show_result(b);
        progress((int)(100 * t / (kS2RampS + kS2HoldS)));
    }
    s_vcmd = v;
    s_phase = ramp ? PH_UP : PH_HOLD;
    set_volts(v);
}

static void stage2_start()
{
    s_stage = 2;
    s_maSwitched = false;
    s_vcmd = 0;
    s_phase = PH_UP;
    s_cap = kMaxSamples;
    s_n.store(0);
    s_startMs = millis();
    s_capturing.store(true);
    progress(0);
    s_timer = lv_timer_create(stage2_cb, 10, nullptr);
}

static void analyse_stage2()
{
    const int n = s_n.load(std::memory_order_acquire);
    if (n < 50)
    {
        show_result("Step 2: too few\nsamples");
        return;
    }
    // Mean and spread of V and I over a time window of the hold
    auto window = [&](float ta, float tb, double &v, double &i, double &sdI, int &m)
    {
        double sv = 0, si = 0, sii = 0;
        m = 0;
        for (int q = 0; q < n; q++)
            if (s_t[q] >= ta && s_t[q] < tb)
            {
                sv += s_v[q];
                si += s_i[q];
                sii += (double)s_i[q] * s_i[q];
                m++;
            }
        v = m ? sv / m : NAN;
        i = m ? si / m : NAN;
        sdI = m > 1 ? sqrt(std::max(0.0, sii / m - i * i)) : NAN;
    };
    const float tEnd = s_t[n - 1];
    auto zeroI = [&](double v) { return s_zero.magic == kZeroMagic ? s_zero.i0 + s_zero.g * v : 0.0; };
    String r;
    // Filament check: at a constant voltage its current keeps falling as it
    // heats (resistance rises). A real R || C or a resistor stays flat.
    if (s_det.sym == SYM_RC || s_det.sym == SYM_R)
    {
        double ve, ie, sde, vl, il2, sdl;
        int me, ml;
        window(kS2RampS + 0.3f, kS2RampS + 1.0f, ve, ie, sde, me);
        window(tEnd - 0.7f, tEnd + 1, vl, il2, sdl, ml);
        const double fall = (ie / ve) / (il2 / vl) - 1.0; // R_late/R_early - 1
        if (me > 20 && ml > 20 && fall > 0.01 && ie > 0.005)
        {
            s_det.sym = SYM_LAMP;
            draw_symbol(SYM_LAMP);
        }
    }
    if (s_det.sym == SYM_R || s_det.sym == SYM_LAMP)
    {
        double v1, i1, sd1, v2, i2, sd2;
        int m1, m2;
        window(kS2RampS + 0.2f, kS2RampS + 1.2f, v1, i1, sd1, m1);
        window(tEnd - 1.0f, tEnd + 1, v2, i2, sd2, m2);
        const double r1 = v1 / (i1 - zeroI(v1)), r2 = v2 / (i2 - zeroI(v2));
        char b[48];
        if (s_det.sym == SYM_LAMP)
        {
            r = "Lamp / bulb\nRhot " + fmtR(r2) + "\nRcold " + fmtR(s_det.rCold);
            snprintf(b, sizeof b, "\nRatio %.1fx", r2 / s_det.rCold);
            r += b;
            snprintf(b, sizeof b, "\nP %.3f W", v2 * i2);
            r += b;
            snprintf(b, sizeof b, "\nWarming %+.1f%%", 100 * (r2 / r1 - 1));
            r += b;
            snprintf(b, sizeof b, "\nat %.2f V", v2);
            r += b;
            show_result(r);
            return;
        }
        r = "Resistor\nR " + fmtR(r2);
        if (std::isfinite(s_det.nonlin))
        {
            snprintf(b, sizeof b, "\nNon-linear %+.2f%%", 100 * s_det.nonlin);
            r += b;
        }
        snprintf(b, sizeof b, "\nDrift %+.2f%%", 100 * (r2 / r1 - 1));
        r += b;
        snprintf(b, sizeof b, "\nP %.3f W", v2 * i2);
        r += b;
        if (s_det.nonlin < -0.10)
            r += "\n(NTC-like)";
    }
    else
    {
        // Leakage: from 2.5 s into the hold (on the mA range if switched),
        // the PSU's own share subtracted
        double v, i, sd;
        int m;
        window(kS2LeakFromS, tEnd + 1, v, i, sd, m);
        const double il = i - zeroI(v);
        const double lim = 3 * sd / sqrt(std::max(1, m / 4)); // samples partly correlated
        // C again from the charge: Q = integral of (I - leak) dt over the ramp
        // and first second, divided by the voltage it moved
        double q = 0, vStart = s_v[0], vEnd = NAN;
        for (int k2 = 1; k2 < n && s_t[k2] < kS2SwitchS; k2++)
        {
            q += (s_i[k2] - std::max(0.0, il)) * (s_t[k2] - s_t[k2 - 1]);
            vEnd = s_v[k2];
        }
        const bool cq = s_det.sym == SYM_C && std::isfinite(vEnd) && vEnd - vStart > 0.3 * s_vtop;
        const double c = cq ? q / (vEnd - vStart) : s_det.c;
        // Step 2's leakage is far more exact than step 1's slope: it decides
        // whether there really is a resistor across the capacitor
        const bool realR = il > lim && v / il < 1e6;
        if (s_det.sym == SYM_RC && !realR)
            s_det.sym = SYM_C;
        else if (s_det.sym == SYM_C && realR && il > 0.001)
            s_det.sym = SYM_RC;
        draw_symbol(s_det.sym);
        r = s_det.sym == SYM_RC ? "R || C" : "Capacitor";
        r += "\nC " + fmtC(c);
        if (s_det.sym == SYM_RC)
            r += "\nR " + fmtR(v / il) + "\nI " + fmtA(il);
        else if (il > lim)
            r += "\nRleak " + fmtR(v / il) + "\nIleak " + fmtA(il);
        else
            r += "\nRleak >" + fmtR(v / lim) + "\nIleak <" + fmtA(lim);
        char b[32];
        snprintf(b, sizeof b, "\nat %.2f V%s", v, s_maSwitched ? "" : "");
        r += b;
        r += "\nESR: too fast";
        plot_it(n, 0);
    }
    show_result(r);
}

static void sweep_cb(lv_timer_t *)
{
    const float t = (millis() - s_startMs) * 1e-3f;
    float v;
    uint8_t ph;
    if (t < kSettleS)
        v = 0, ph = PH_SETTLE;
    else if (t < kSettleS + kRampS)
        v = s_vtop * (t - kSettleS) / kRampS, ph = PH_UP;
    else if (t < kSettleS + kRampS + kHoldS)
        v = s_vtop, ph = PH_HOLD;
    else if (t < kSettleS + 2 * kRampS + kHoldS)
        v = s_vtop * (1 - (t - kSettleS - kRampS - kHoldS) / kRampS), ph = PH_DOWN;
    else
    {
        s_phase = PH_END;
        finish();
        return;
    }
    if (!output_usable())
    {
        s_capturing.store(false);
        if (s_timer)
        {
            lv_timer_del(s_timer);
            s_timer = nullptr;
        }
        PowerSupply.Voltage.SetUpdate(s_savedAdj);
        progress(-1);
        end_run();
        show_result("Stopped:\noutput off");
        return;
    }
    static uint32_t lastUi = 0;
    if (millis() - lastUi > 150)
    {
        lastUi = millis();
        static const char *names[] = {"Settling at 0 V", "Ramp up", "Hold at top", "Ramp down"};
        const float total = kSettleS + 2 * kRampS + kHoldS;
        char b[96];
        snprintf(b, sizeof b, "%s\n%s\n%.2f V\n%s\n%.0f s left", s_isZero ? "Zero sweep" : "Sweeping",
                 names[ph], v, fmtA(PowerSupply.Current.oldValue * (PowerSupply.mA_Active ? 0.001 : 1.0)).c_str(), total - t);
        show_result(b);
        progress((int)(100 * t / total));
    }
    s_vcmd = v;
    s_phase = ph;
    set_volts(v);
    if (!s_capturing.load() && s_n.load() >= s_cap)
        finish(); // buffer full: analyse what we have
}

static void fft_cb(lv_timer_t *)
{
    {
        char b[48];
        snprintf(b, sizeof b, "FFT capture\n%d / %d samples", s_n.load(), kFftN);
        show_result(b);
        progress(100 * s_n.load() / kFftN);
    }
    if (!s_capturing.load() || s_n.load() >= kFftN)
        finish();
    else if (millis() - s_startMs > 20000)
    {
        s_capturing.store(false);
        lv_timer_del(s_timer);
        s_timer = nullptr;
        progress(-1);
        end_run();
        show_result("FFT: no ADC data");
    }
}

static int s_savedRate = -1;
static float s_imax = 0;        // analysis current limit, A (0 = keep I-Set)
static int s_savedIAdj = -1;
static lv_obj_t *s_imaxDd = nullptr;
static constexpr float kImaxOpt[] = {0, 0.01f, 0.05f, 0.1f, 0.25f, 0.5f, 1, 2, 3, 5};
void loadAnalyzerSetImax(float a)
{
    s_imax = a;
    if (s_imaxDd)
        for (int k = 0; k < (int)(sizeof kImaxOpt / sizeof kImaxOpt[0]); k++)
            if (fabsf(kImaxOpt[k] - a) < 1e-4f)
                lv_dropdown_set_selected(s_imaxDd, k);
}
static void begin_run()
{
    s_savedIAdj = -1;
    if (s_imax > 0)
    {
        s_savedIAdj = PowerSupply.Current.adjValue;
        PowerSupply.Current.SetUpdate(s_imax * PowerSupply.Current.adjFactor + PowerSupply.Current.adjOffset);
    }
    auto &sp = PowerSupply.settingParameters;
    s_savedRate = sp.adcRate;
    if (sp.adcRate != 4)
    {
        sp.adcRate = 4; // Turbo: most samples (not saved; restored by end_run)
        PowerSupply.applyAdcRate();
    }
    for (lv_obj_t *b : s_btns)
        if (b)
            lv_obj_add_state(b, LV_STATE_DISABLED);
}
static void end_run_impl()
{
    auto &sp = PowerSupply.settingParameters;
    if (s_savedRate >= 0 && sp.adcRate != s_savedRate)
    {
        sp.adcRate = s_savedRate;
        PowerSupply.applyAdcRate();
    }
    s_savedRate = -1;
    if (s_savedIAdj >= 0)
        PowerSupply.Current.SetUpdate(s_savedIAdj); // back to the user's I-Set
    s_savedIAdj = -1;
    for (lv_obj_t *b : s_btns)
        if (b)
            lv_obj_clear_state(b, LV_STATE_DISABLED);
}

bool loadAnalyzerBusy() { return s_timer != nullptr; }
String loadAnalyzerResult() { return s_resultText; }

bool loadSweepStart(bool zero)
{
    if (s_timer || !output_usable() || !alloc_buffers())
        return false;
    s_vtop = user_vset();
    if (s_vtop < 0.5f)
    {
        show_result("Set V-Set >= 0.5 V\n(sweep top)");
        return false;
    }
    s_isZero = zero;
    s_isFft = false;
    s_stage = 1;
    s_savedAdj = PowerSupply.Voltage.adjValue;
    s_vcmd = 0;
    s_phase = PH_SETTLE;
    s_cap = kMaxSamples;
    s_n.store(0);
    s_startMs = millis();
    s_capturing.store(true);
    set_volts(0);
    begin_run();
    s_timer = lv_timer_create(sweep_cb, 10, nullptr);
    draw_symbol(SYM_NONE);
    progress(0);
    show_result(zero ? "Zero sweep...\n(nothing\nconnected)" : "Sweeping\n0 -> V-Set -> 0\n(~11 s)");
    return true;
}

bool loadFftStart()
{
    if (s_timer || !output_usable() || !alloc_buffers())
        return false;
    s_isFft = true;
    s_isZero = false;
    s_vcmd = user_vset();
    s_phase = PH_HOLD;
    s_cap = kFftN;
    s_n.store(0);
    s_startMs = millis();
    s_capturing.store(true);
    begin_run();
    s_timer = lv_timer_create(fft_cb, 50, nullptr);
    draw_symbol(SYM_NONE);
    progress(0);
    show_result("FFT capture...");
    return true;
}

static void btn_cb(lv_event_t *e)
{
    const int which = (int)(intptr_t)lv_event_get_user_data(e);
    bool ok = which == 1 ? loadFftStart() : loadSweepStart(which == 2);
    if (!ok && !s_timer && !output_usable())
        show_result("Turn the\noutput ON");
}

void createLoadTab(lv_obj_t *parent)
{
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(parent, 0, LV_PART_MAIN);

    PowerSupply.StoreMem.begin("my-app", true);
    ZeroCal z{};
    if (PowerSupply.StoreMem.getBytes("load_zero", &z, sizeof(z)) == sizeof(z) && z.magic == kZeroMagic)
        s_zero = z;
    PowerSupply.StoreMem.end();

    // Plot: dark panel, 4x4 grid, value labels inside the corners
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
    lv_chart_set_update_mode(s_chart, LV_CHART_UPDATE_MODE_SHIFT);
    s_ser1 = lv_chart_add_series(s_chart, lv_color_hex(0xFFC107), LV_CHART_AXIS_PRIMARY_Y);
    s_ser2 = lv_chart_add_series(s_chart, lv_color_hex(0x00BCD4), LV_CHART_AXIS_PRIMARY_Y);
    lv_chart_set_point_count(s_chart, kPlotPts);
    lv_chart_set_all_value(s_chart, s_ser1, LV_CHART_POINT_NONE);
    lv_chart_set_all_value(s_chart, s_ser2, LV_CHART_POINT_NONE);

    auto small = [&](lv_obj_t *par, lv_color_t col) {
        lv_obj_t *l = lv_label_create(par);
        lv_obj_set_style_text_font(l, &lv_font_montserrat_10, 0);
        lv_obj_set_style_text_color(l, col, 0);
        lv_label_set_recolor(l, true);
        lv_label_set_text(l, "");
        return l;
    };
    s_ytop = small(s_chart, lv_color_hex(0x8A94A6));
    lv_obj_align(s_ytop, LV_ALIGN_TOP_LEFT, 2, 0);
    s_xright = small(s_chart, lv_color_hex(0x8A94A6));
    lv_obj_align(s_xright, LV_ALIGN_BOTTOM_RIGHT, -2, 0);
    lv_obj_t *x0 = small(s_chart, lv_color_hex(0x8A94A6));
    lv_label_set_text(x0, "0");
    lv_obj_align(x0, LV_ALIGN_BOTTOM_LEFT, 2, 0);

    s_axis = small(parent, lv_color_hex(0xC9D1D9));
    lv_label_set_text(s_axis, "Load analyser");
    lv_obj_set_pos(s_axis, 6, 145);

    // Result card: title + key/value rows, component symbol, progress
    lv_obj_t *card = lv_obj_create(parent);
    lv_obj_remove_style_all(card);
    lv_obj_set_size(card, 115, 142);
    lv_obj_set_pos(card, 203, 3);
    lv_obj_set_style_bg_color(card, lv_color_hex(0x161b22), 0);
    lv_obj_set_style_bg_opa(card, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(card, 6, 0);
    lv_obj_set_style_border_width(card, 1, 0);
    lv_obj_set_style_border_color(card, lv_color_hex(0x30363d), 0);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);

    s_result = lv_label_create(card);
    lv_obj_set_style_text_font(s_result, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(s_result, lv_color_hex(0xE6EDF3), 0);
    lv_obj_set_style_text_line_space(s_result, 1, 0);
    lv_label_set_recolor(s_result, true);
    lv_obj_set_width(s_result, 108);
    lv_label_set_long_mode(s_result, LV_LABEL_LONG_CLIP);
    lv_obj_set_pos(s_result, 5, 4);
    show_result("Ready\nSweep finds\nR, C, diode,\nthen details.\nTop = V-Set\nZero: once,\nno load");

    s_symbol = lv_obj_create(card);
    lv_obj_remove_style_all(s_symbol);
    lv_obj_set_size(s_symbol, 104, 42);
    lv_obj_set_pos(s_symbol, 5, 97);
    lv_obj_clear_flag(s_symbol, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);

    s_bar = lv_bar_create(parent);
    lv_obj_set_size(s_bar, 80, 6);
    lv_obj_set_pos(s_bar, 205, 150);
    s_pct = small(parent, lv_color_hex(0xC9D1D9));
    lv_obj_set_pos(s_pct, 290, 146);

    // Current limit for the analysis (restored after)
    s_imaxDd = lv_dropdown_create(parent);
    lv_dropdown_set_options(s_imaxDd, "Imax: I-Set\nImax: 10 mA\nImax: 50 mA\nImax: 100 mA\nImax: 250 mA\nImax: 500 mA\nImax: 1 A\nImax: 2 A\nImax: 3 A\nImax: 5 A");
    lv_dropdown_set_dir(s_imaxDd, LV_DIR_TOP);
    lv_dropdown_set_symbol(s_imaxDd, LV_SYMBOL_UP);
    lv_obj_set_size(s_imaxDd, 115, 22);
    lv_obj_set_pos(s_imaxDd, 203, 160);
    lv_obj_set_style_text_font(s_imaxDd, &lv_font_montserrat_12, 0);
    lv_obj_set_style_pad_ver(s_imaxDd, 3, 0);
    lv_obj_set_style_pad_hor(s_imaxDd, 6, 0);
    lv_obj_set_style_radius(s_imaxDd, 6, 0);
    lv_obj_set_style_bg_color(s_imaxDd, lv_color_hex(0x21262d), 0);
    lv_obj_set_style_border_color(s_imaxDd, lv_color_hex(0x30363d), 0);
    lv_obj_set_style_text_color(s_imaxDd, lv_color_hex(0xE6EDF3), 0);
    lv_obj_add_event_cb(s_imaxDd, [](lv_event_t *e) {
        s_imax = kImaxOpt[lv_dropdown_get_selected(lv_event_get_target(e))];
    }, LV_EVENT_VALUE_CHANGED, nullptr);
    if (lv_obj_t *list = lv_dropdown_get_list(s_imaxDd))
        lv_obj_set_style_text_font(list, &lv_font_montserrat_12, 0);
    lv_bar_set_range(s_bar, 0, 100);
    lv_obj_set_style_bg_color(s_bar, lv_color_hex(0x30363d), LV_PART_MAIN);
    lv_obj_set_style_bg_color(s_bar, lv_color_hex(0xFFC107), LV_PART_INDICATOR);
    lv_obj_add_flag(s_bar, LV_OBJ_FLAG_HIDDEN);

    static const char *names[3] = {LV_SYMBOL_PLAY " Sweep", LV_SYMBOL_AUDIO " FFT", LV_SYMBOL_REFRESH " Zero"};
    for (int b = 0; b < 3; b++)
    {
        lv_obj_t *btn = lv_btn_create(parent);
        s_btns[b] = btn;
        lv_obj_set_size(btn, 63, 22);
        lv_obj_set_pos(btn, 3 + b * 68, 160);
        lv_obj_set_style_radius(btn, 6, 0);
        lv_obj_set_style_shadow_width(btn, 0, 0);
        lv_obj_set_style_bg_color(btn, b == 0 ? lv_color_hex(0x1f6feb) : lv_color_hex(0x21262d), 0);
        lv_obj_set_style_border_width(btn, 1, 0);
        lv_obj_set_style_border_color(btn, lv_color_hex(0x30363d), 0);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x161b22), LV_STATE_DISABLED);
        lv_obj_set_style_text_color(btn, lv_color_hex(0x484f58), LV_STATE_DISABLED);
        lv_obj_add_event_cb(btn, btn_cb, LV_EVENT_CLICKED, (void *)(intptr_t)b);
        lv_obj_t *l = lv_label_create(btn);
        lv_obj_set_style_text_font(l, &lv_font_montserrat_12, 0);
        lv_obj_set_style_text_color(l, lv_color_hex(0xE6EDF3), 0);
        lv_label_set_text(l, names[b]);
        lv_obj_center(l);
    }
}
