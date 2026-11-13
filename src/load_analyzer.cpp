#include "load_analyzer.h"
#include "device.hpp"
#include "myFonts.h"
#include "esp_heap_caps.h"
#include <atomic>
#include <cmath>

extern Device PowerSupply;

// ─── Capture (Core 0 writes, Core 1 reads once s_capturing is false) ────────
static constexpr int kMaxSamples = 4096;
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
    if (!s_capturing.load(std::memory_order_relaxed))
        return;
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
static constexpr int kPlotPts = 160;
static String s_resultText = "No result yet";

static void show_result(const String &multi)
{
    if (s_result)
        lv_label_set_text(s_result, multi.c_str());
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
    if (!std::isfinite(ohm) || ohm <= 0 || ohm > 1e7)
        return "open";
    if (ohm >= 1e3)
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
    else
        snprintf(b, sizeof b, "%.2f mA", a * 1e3);
    return b;
}

static void plot_iv(int n)
{
    float imax = 1e-4f;
    for (int k = 0; k < n; k++)
        if (s_ph[k] != PH_SETTLE && s_i[k] > imax)
            imax = s_i[k];
    lv_chart_set_type(s_chart, LV_CHART_TYPE_SCATTER);
    lv_chart_set_point_count(s_chart, kPlotPts);
    lv_chart_set_range(s_chart, LV_CHART_AXIS_PRIMARY_X, 0, 1000);
    lv_chart_set_range(s_chart, LV_CHART_AXIS_PRIMARY_Y, 0, 1000);
    for (int sIdx = 0; sIdx < 2; sIdx++)
    {
        lv_chart_series_t *ser = sIdx ? s_ser2 : s_ser1;
        const uint8_t want = sIdx ? PH_DOWN : PH_UP;
        int cnt = 0;
        for (int k = 0; k < n; k++)
            if (s_ph[k] == want)
                cnt++;
        int j = 0, seen = 0;
        for (int k = 0; k < n && j < kPlotPts; k++)
        {
            if (s_ph[k] != want)
                continue;
            if ((long)seen++ * kPlotPts / std::max(cnt, 1) < j)
                continue;
            lv_chart_set_value_by_id2(s_chart, ser, j++,
                                      (lv_coord_t)lroundf(1000.0f * s_v[k] / s_vtop),
                                      (lv_coord_t)lroundf(1000.0f * s_i[k] / imax));
        }
        for (; j < kPlotPts; j++)
            lv_chart_set_value_by_id2(s_chart, ser, j, LV_CHART_POINT_NONE, LV_CHART_POINT_NONE);
    }
    lv_chart_refresh(s_chart);
    char b[64];
    snprintf(b, sizeof b, "I-V  x: 0-%.2fV  y: 0-%s", s_vtop, fmtA(imax).c_str());
    lv_label_set_text(s_axis, b);
}

static void analyse_sweep()
{
    const int n = s_n.load(std::memory_order_acquire);
    if (n < 50)
    {
        show_result("Too few samples");
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

    String type;
    if (knee)
        type = "Diode/LED/batt";
    else if (hasR && hasC)
        type = "R || C";
    else if (hasR)
        type = "Resistor";
    else if (hasC)
        type = "Capacitor";
    else
        type = "Open";

    String r = type + "\nR " + (hasR ? fmtR(1.0 / g) : String("open")) +
               "\nC " + (hasC ? fmtC(c) : String("< ") + fmtC(4 * se[2])) +
               "\nI0 " + fmtA(i0);
    if (knee)
    {
        char b[24];
        snprintf(b, sizeof b, "\nKnee %.2f V", vknee);
        r += b;
    }
    r += "\nNoise " + fmtA(rms);
    r += "\nL: too fast";
    if (s_zero.magic != kZeroMagic)
        r += "\n(no Zero)";
    show_result(r);
    plot_iv(n);
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
    char b[64];
    snprintf(b, sizeof b, "FFT I  x: 0-%.0fHz  y: 0-%s", fs / 2, fmtA(amax).c_str());
    lv_label_set_text(s_axis, b);

    char t[200];
    snprintf(t, sizeof t, "FFT of I\nfs %.0f Hz\nPeak %.1f Hz\n  %s\nI rms %s\nI dc %s\nV rms %.2f mV",
             fs, kpk * fs / kFftN, fmtA(amax).c_str(), fmtA(rms).c_str(), fmtA(mean).c_str(), vrms * 1e3);
    show_result(t);
}

// ─── Runner (LVGL timer, Core 1) ────────────────────────────────────────────
static void finish()
{
    if (s_timer)
    {
        lv_timer_del(s_timer);
        s_timer = nullptr;
    }
    s_capturing.store(false);
    if (!s_isFft)
        PowerSupply.Voltage.SetUpdate(s_savedAdj);
    if (s_isFft)
        analyse_fft();
    else
        analyse_sweep();
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
        show_result("Stopped:\noutput off");
        return;
    }
    s_vcmd = v;
    s_phase = ph;
    set_volts(v);
    if (!s_capturing.load() && s_n.load() >= s_cap)
        finish(); // buffer full: analyse what we have
}

static void fft_cb(lv_timer_t *)
{
    if (!s_capturing.load() || s_n.load() >= kFftN)
        finish();
    else if (millis() - s_startMs > 20000)
    {
        s_capturing.store(false);
        lv_timer_del(s_timer);
        s_timer = nullptr;
        show_result("FFT: no ADC data");
    }
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
    s_savedAdj = PowerSupply.Voltage.adjValue;
    s_vcmd = 0;
    s_phase = PH_SETTLE;
    s_cap = kMaxSamples;
    s_n.store(0);
    s_startMs = millis();
    s_capturing.store(true);
    set_volts(0);
    s_timer = lv_timer_create(sweep_cb, 10, nullptr);
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
    s_timer = lv_timer_create(fft_cb, 50, nullptr);
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

    s_chart = lv_chart_create(parent);
    lv_obj_set_size(s_chart, 200, 140);
    lv_obj_set_pos(s_chart, 4, 4);
    lv_obj_set_style_bg_color(s_chart, lv_color_hex(0x101418), 0);
    lv_obj_set_style_border_width(s_chart, 1, 0);
    lv_obj_set_style_border_color(s_chart, lv_palette_darken(LV_PALETTE_GREY, 2), 0);
    lv_obj_set_style_pad_all(s_chart, 2, 0);
    lv_obj_set_style_line_color(s_chart, lv_color_hex(0x2a3038), LV_PART_MAIN);
    lv_chart_set_div_line_count(s_chart, 5, 5);
    lv_obj_set_style_line_width(s_chart, 1, LV_PART_ITEMS);
    lv_obj_set_style_size(s_chart, 0, LV_PART_INDICATOR);
    lv_chart_set_update_mode(s_chart, LV_CHART_UPDATE_MODE_SHIFT);
    s_ser1 = lv_chart_add_series(s_chart, lv_palette_main(LV_PALETTE_AMBER), LV_CHART_AXIS_PRIMARY_Y);
    s_ser2 = lv_chart_add_series(s_chart, lv_palette_main(LV_PALETTE_CYAN), LV_CHART_AXIS_PRIMARY_Y);
    lv_chart_set_point_count(s_chart, kPlotPts);
    lv_chart_set_all_value(s_chart, s_ser1, LV_CHART_POINT_NONE);
    lv_chart_set_all_value(s_chart, s_ser2, LV_CHART_POINT_NONE);

    s_axis = lv_label_create(parent);
    lv_obj_set_style_text_font(s_axis, &lv_font_montserrat_10, 0);
    lv_obj_set_style_text_color(s_axis, lv_palette_main(LV_PALETTE_GREY), 0);
    lv_label_set_text(s_axis, "up: amber  down: cyan");
    lv_obj_set_pos(s_axis, 6, 146);

    s_result = lv_label_create(parent);
    lv_obj_set_style_text_font(s_result, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(s_result, lv_palette_lighten(LV_PALETTE_GREY, 3), 0);
    lv_obj_set_width(s_result, 108);
    lv_label_set_long_mode(s_result, LV_LABEL_LONG_CLIP);
    lv_obj_set_pos(s_result, 210, 4);
    lv_label_set_text(s_result, "Sweep: R, C,\nknee. Uses\nV-Set as top,\nI-Set limits.\n\nZero: run once\nwith nothing\nconnected.");

    static const char *names[3] = {"Sweep", "FFT", "Zero"};
    for (int b = 0; b < 3; b++)
    {
        lv_obj_t *btn = lv_btn_create(parent);
        lv_obj_set_size(btn, 62, 26);
        lv_obj_set_pos(btn, 4 + b * 68, 160);
        lv_obj_set_style_bg_color(btn, lv_palette_darken(LV_PALETTE_INDIGO, 2), 0);
        lv_obj_add_event_cb(btn, btn_cb, LV_EVENT_CLICKED, (void *)(intptr_t)b);
        lv_obj_t *l = lv_label_create(btn);
        lv_obj_set_style_text_font(l, &lv_font_montserrat_12, 0);
        lv_obj_set_style_text_color(l, lv_palette_main(LV_PALETTE_AMBER), 0);
        lv_label_set_text(l, names[b]);
        lv_obj_center(l);
    }
}
