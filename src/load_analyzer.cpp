#include "load_analyzer.h"
#include "device.hpp"
#include "myFonts.h"
#include "esp_heap_caps.h"
#include "esp_timer.h"
#include <atomic>
#include <cmath>
#include <complex>

extern Device PowerSupply;

// ─── Capture (Core 0 writes, Core 1 reads once s_capturing is false) ────────
static constexpr int kMaxSamples = 20000; // ~10 s sweep at ~800 pairs/s
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

// Core-0 sine generator for Z(f)/LCR: Task_ADC calls loadWaveTick() every
// pass (~1500-2000/s, one DAC write each) instead of the 2 ms setpoint timer
// - a capacitor's current is C*dV/dt, so coarse steps make current spikes
static volatile bool s_wOn = false;
static volatile float s_wBias = 0, s_wAmp = 0, s_wF = 1;
static volatile uint32_t s_wT0 = 0;
static volatile uint32_t s_wTicks = 0; // DAC writes, for the rate readout
static uint32_t s_wStartMs = 0;
static void wave_write()
{
    const float t = (micros() - s_wT0) * 1e-6f;
    const float v = s_wBias + s_wAmp * sinf(2 * (float)M_PI * s_wF * t);
    const int32_t code = (int32_t)lroundf(v * PowerSupply.Voltage.adjFactor + PowerSupply.Voltage.adjOffset);
    PowerSupply.DAC.writeAndPowerAll(DAC_VOLTAGE, (uint16_t)std::max<int32_t>(0, std::min<int32_t>(0xFFFF, code)));
    s_vcmd = v;
    s_wTicks++;
}
// 2 kHz from esp_timer (its own task): the DAC is on Wire, the ADC on Wire1,
// and while the sine runs nothing else uses Wire (key scan paused, regular
// DAC update skipped) - so these writes never delay a measurement
static esp_timer_handle_t s_wTimer = nullptr;
static void wave_timer_cb(void *)
{
    if (s_wOn)
        wave_write();
}
static void wave_timer_start()
{
    if (!s_wTimer)
    {
        esp_timer_create_args_t a = {};
        a.callback = wave_timer_cb;
        a.name = "zwave";
        a.dispatch_method = ESP_TIMER_TASK;
        esp_timer_create(&a, &s_wTimer);
    }
    if (s_wTimer)
        esp_timer_start_periodic(s_wTimer, 500); // us
}
static void wave_timer_stop()
{
    if (s_wTimer)
        esp_timer_stop(s_wTimer);
}
bool loadWaveActive() { return s_wOn; }

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

enum Sym { SYM_NONE, SYM_OPEN, SYM_R, SYM_C, SYM_RC, SYM_DIODE, SYM_LAMP, SYM_BATT, SYM_DIODE_R, SYM_ZENER, SYM_MOTOR };

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
    case SYM_DIODE_R:
    {
        // diode (x 8-36) then a resistor (x 52-96)
        static const lv_point_t a1[] = {{0, 20}, {8, 20}}, t1[] = {{8, 9}, {8, 31}, {28, 20}, {8, 9}}, k1[] = {{28, 9}, {28, 31}};
        static const lv_point_t m1[] = {{28, 20}, {48, 20}, {51, 13}, {57, 27}, {63, 13}, {69, 27}, {75, 13}, {81, 27}, {84, 20}, {104, 20}};
        line(a1, 2); line(t1, 4); line(k1, 2); line(m1, 10);
        break;
    }
    case SYM_MOTOR:
    {
        // circle with an M
        static lv_point_t circ[21];
        static bool init = false;
        if (!init)
        {
            for (int k = 0; k <= 20; k++)
                circ[k] = {(lv_coord_t)lroundf(52 + 15 * cosf(k * 2 * M_PI / 20)), (lv_coord_t)lroundf(20 + 15 * sinf(k * 2 * M_PI / 20))};
            init = true;
        }
        static const lv_point_t mA[] = {{0, 20}, {37, 20}}, mB[] = {{67, 20}, {104, 20}};
        static const lv_point_t mm[] = {{44, 28}, {44, 12}, {52, 22}, {60, 12}, {60, 28}};
        line(mA, 2); line(mB, 2); line(circ, 21); line(mm, 5);
        break;
    }
    case SYM_ZENER:
    {
        static const lv_point_t za[] = {{0, 20}, {40, 20}}, zt[] = {{40, 8}, {40, 32}, {62, 20}, {40, 8}};
        static const lv_point_t zk[] = {{56, 32}, {62, 32}, {62, 8}, {68, 8}}, zb[] = {{62, 20}, {104, 20}};
        line(za, 2); line(zt, 4); line(zk, 4); line(zb, 2);
        break;
    }
    case SYM_BATT:
    {
        static const lv_point_t bA[] = {{0, 20}, {46, 20}}, bL[] = {{46, 4}, {46, 36}}, bS[] = {{56, 12}, {56, 28}}, bB[] = {{56, 20}, {104, 20}};
        line(bA, 2); line(bL, 2); line(bS, 2); line(bB, 2);
        break;
    }
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
static lv_obj_t *s_btns[4] = {};

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

static String s_summary, s_details;
static float s_tau = NAN; // probe: let-go time constant, s
static float s_ccC = NAN; // probe: C from the 1 mA current step
static String s_diodeSummary; // probe found a diode: finish without the sweep
static float s_cLG = NAN; // probe: C from the let-go charge balance
static float s_rCC = NAN; // probe: R between the current steps (cold, ~no power)
static bool s_showDetails = false;
static lv_obj_t *s_hint = nullptr;

static void render_result()
{
    if (!s_result)
        return;
    const String &multi = (s_showDetails && s_details.length()) ? s_details : s_summary;
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
    if (s_hint)
    {
        if (s_details.length())
            lv_obj_clear_flag(s_hint, LV_OBJ_FLAG_HIDDEN);
        else
            lv_obj_add_flag(s_hint, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(s_hint, s_showDetails ? LV_SYMBOL_LEFT : LV_SYMBOL_LIST);
    }
    // the symbol would sit under a long details list
    if (s_symbol)
    {
        if (s_showDetails)
            lv_obj_add_flag(s_symbol, LV_OBJ_FLAG_HIDDEN);
        else
            lv_obj_clear_flag(s_symbol, LV_OBJ_FLAG_HIDDEN);
    }
}

static void show_result(const String &multi)
{
    s_summary = multi;
    render_result();
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
    lv_obj_set_style_size(s_chart, 0, LV_PART_INDICATOR);
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
    lv_obj_set_style_size(s_chart, 0, LV_PART_INDICATOR);
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
    // The probe saw the voltage decay on its own (let-go / discharge tau):
    // charge stored AND a path to drain it = R || C, whatever the sweep's
    // shape suggests (a capacitor's charging current bends it like a knee)
    const bool storage = std::isfinite(s_tau) && s_tau > 0.02f;
    s_det = {storage ? SYM_RC : knee ? SYM_DIODE : hasR && hasC ? SYM_RC : hasR ? SYM_R : hasC ? SYM_C : SYM_OPEN,
             g, c, i0, vknee, 4 * se[2], 4 * se[1]};
    draw_symbol(s_det.sym);
    plot_iv(n);

    // Resistor linearity: slope through the origin, lower vs upper half.
    // Also for R || C: a filament's thermal lag makes up and down differ
    // like a capacitor would, but its resistance rising with voltage gives
    // it away - checked first.
    if ((s_det.sym == SYM_R || s_det.sym == SYM_RC) && !storage)
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
        // Resistance falling with voltage and a straight upper part that
        // starts well above 0 V: a diode/LED in series with a resistor,
        // I = (V - Vf) / R
        if (s_det.nonlin < -0.10)
        {
            double sx = 0, sy = 0, sxx = 0, sxy = 0;
            int m = 0;
            for (int q = 0; q < n; q++)
                if ((s_ph[q] == PH_UP || s_ph[q] == PH_DOWN) && s_v[q] >= 0.6 * s_vtop)
                {
                    sx += s_v[q];
                    sy += s_i[q];
                    sxx += (double)s_v[q] * s_v[q];
                    sxy += (double)s_v[q] * s_i[q];
                    m++;
                }
            const double den = m * sxx - sx * sx;
            if (m > 30 && den > 0)
            {
                const double b = (m * sxy - sx * sy) / den, a = (sy - b * sx) / m;
                const double vf = b > 0 ? -a / b : NAN;
                if (vf > 0.3 && vf < 0.85 * s_vtop)
                {
                    const char *kind = vf < 0.4 ? "Schottky" : vf < 0.85 ? "Si diode" : vf < 1.4 ? "IR LED / 2 diodes" :
                                       vf < 2.15 ? "red/yel LED" : vf < 3.5 ? "grn/blu/wht LED" : "LED string";
                    char t[160];
                    snprintf(t, sizeof t, "Diode + R\nVf %.2f V\nR %s\n%s\nI %s\nat %.2f V", vf, fmtR(1.0 / b).c_str(),
                             kind, fmtA(a + b * s_vtop).c_str(), s_vtop);
                    draw_symbol(SYM_DIODE_R);
                    show_result(t);
                    return;
                }
            }
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
        // Big slope resistance above the knee = a resistor in series:
        // split it out, I = (V - Vf)/R, Vf where the fitted line meets 0 A
        if (g > 0 && 1.0 / g > 50.0 && -p[0] / g > 0.3)
        {
            const double vf = -p[0] / g;
            const char *kind = vf < 0.4 ? "Schottky" : vf < 0.85 ? "Si diode" : vf < 1.4 ? "IR LED / 2 diodes" :
                               vf < 2.15 ? "red/yel LED" : vf < 3.5 ? "grn/blu/wht LED" : "LED string";
            char t[160];
            snprintf(t, sizeof t, "Diode + R\nVf %.2f V\nR %s\n%s\nI %s\nat %.2f V", vf, fmtR(1.0 / g).c_str(), kind,
                     fmtA(p[0] + g * s_vtop).c_str(), s_vtop);
            draw_symbol(SYM_DIODE_R);
            show_result(t);
            return;
        }
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
    s_stage = 2; // (s_maSwitched kept: the probe may have chosen the mA range)
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
    if ((s_det.sym == SYM_RC || s_det.sym == SYM_R) && !(std::isfinite(s_tau) && s_tau > 0.02f))
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
            const double rc = std::isfinite(s_rCC) ? s_rCC : s_det.rCold; // probe = truly cold
            r = "Lamp / bulb\nRhot " + fmtR(r2) + "\nRcold " + fmtR(rc);
            snprintf(b, sizeof b, "\nRatio %.2fx", r2 / rc);
            r += b;
            snprintf(b, sizeof b, "\nP %.3f W", v2 * i2);
            r += b;
            snprintf(b, sizeof b, "\nWarming %+.1f%%", 100 * (r2 / r1 - 1));
            r += b;
            {
                // Filament heating time: the current's fall during the hold,
                // time for (I - I_end) to drop to 37% of its first value
                double vs0, is0, sd0;
                int m0;
                window(kS2RampS + 0.05f, kS2RampS + 0.15f, vs0, is0, sd0, m0);
                const double d0 = is0 - i2;
                float tth = NAN;
                if (m0 > 5 && d0 > 0.002 * i2)
                {
                    const int n2 = s_n.load();
                    for (int q = 0; q < n2; q++)
                        if (s_t[q] > kS2RampS + 0.1f && s_i[q] - i2 < 0.368 * d0)
                        {
                            // 25-sample mean guards against single noisy points
                            double sm = 0;
                            int c2 = 0;
                            for (int k = q; k < std::min(n2, q + 25); k++, c2++)
                                sm += s_i[k];
                            if (sm / c2 - i2 < 0.368 * d0)
                            {
                                tth = s_t[q] - kS2RampS - 0.1f;
                                break;
                            }
                        }
                }
                if (std::isfinite(tth))
                {
                    snprintf(b, sizeof b, "\nHeat tau %.2f s", tth);
                    s_details += b;
                }
                snprintf(b, sizeof b, "\nWarming %+.1f%%\nat %.2f V", 100 * (r2 / r1 - 1), v2);
                s_details += b;
            }
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
        // R || C: C from the probe's decay, tau = R*C, with R = V/I of the hold
        const double c = (s_det.sym == SYM_RC && std::isfinite(s_cLG) && s_cLG > 0) ? s_cLG
                         : (s_det.sym == SYM_RC && std::isfinite(s_tau) && il > 0) ? s_tau * il / v
                         : cq ? q / (vEnd - vStart) : s_det.c;
        // Step 2's leakage is far more exact than step 1's slope: it decides
        // whether there really is a resistor across the capacitor
        const bool realR = il > lim && v / il < 1e6;
        if (s_det.sym == SYM_RC && !realR)
            s_det.sym = SYM_C;
        else if (s_det.sym == SYM_C && realR && il > 0.001)
            s_det.sym = SYM_RC;
        draw_symbol(s_det.sym);
        if (s_det.sym == SYM_C && std::isfinite(s_tau) && c > 0)
            s_details += "\nR = tau/C " + fmtR(s_tau / c);
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
static constexpr float kImaxOpt[] = {0, 0.01f, 0.02f, 0.05f, 0.1f, 0.25f, 0.5f, 1, 2, 3, 5};
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
    s_savedIAdj = PowerSupply.Current.adjValue; // probe steps change the limit too
    if (s_imax > 0)
    {
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
    restore_range();
    if (s_savedIAdj >= 0)
        PowerSupply.Current.SetUpdate(s_savedIAdj); // back to the user's I-Set
    s_savedIAdj = -1;
    for (lv_obj_t *b : s_btns)
        if (b)
            lv_obj_clear_state(b, LV_STATE_DISABLED);
}

bool loadAnalyzerBusy() { return s_timer != nullptr; }
static bool s_impRunning = false;
bool loadImpBusy() { return s_impRunning && s_timer != nullptr; }
String loadAnalyzerResult() { return s_resultText; }
String loadAnalyzerDetails() { String d = s_details; d.replace("\n", ";"); return d; }

// ─── Probe: experiments before the sweep ────────────────────────────────────
// Gentle first, each step deciding whether the next is safe:
//   PRE    output at 0 V: any voltage = battery / charged cap / other source
//   TINY   10 mA limit, 0.2 V: short or not
//   CC1-4  1/5/10/20 mA current steps (V-Set as ceiling): diode law, R(I),
//          a capacitor's linear voltage rise (C = I / dV/dt)
//   HOLD   V-Set, user's limit: inrush of a cold filament / motor
//   LETGO  setpoint to 0: the PSU can't sink, so the load discharges itself:
//          instant = no storage, exponential tau = R*C, flat = holds charge
//   BATT   (source found) 20 mA charge pulse: internal R, or C if it ramps
enum : uint8_t { PR_PRE = 10, PR_TINY, PR_CC1, PR_CC2, PR_CC3, PR_CC4, PR_CC5, PR_CC6, PR_HOLD, PR_LETGO, PR_BATT, PR_WATCH };
static constexpr int kSteps = 6;
struct Seg { uint8_t ph; float dur, v, ilim; const char *name; };
static Seg s_segs[16];
static float s_segT0[16];
static int s_nseg = 0, s_seg = -1;
static uint32_t s_segStartMs = 0;
static float s_userImax = 0;
static void sweep_begin();

static float user_iset() { return (PowerSupply.Current.adjValue - PowerSupply.Current.adjOffset) / PowerSupply.Current.adjFactor; }
static void set_amps(double a) { PowerSupply.Current.SetUpdate(a * PowerSupply.Current.adjFactor + PowerSupply.Current.adjOffset); }

// Mean V, I of segment k between fractions [fa, fb] of its duration
static int seg_stats(int k, float fa, float fb, double &v, double &i)
{
    const int n = s_n.load(std::memory_order_acquire);
    const float t0 = s_segT0[k] + fa * s_segs[k].dur, t1 = s_segT0[k] + fb * s_segs[k].dur;
    double sv = 0, si = 0;
    int m = 0;
    for (int q = 0; q < n; q++)
        if (s_ph[q] == s_segs[k].ph && s_t[q] >= t0 && s_t[q] <= t1)
        {
            sv += s_v[q];
            si += s_i[q];
            m++;
        }
    v = m ? sv / m : NAN;
    i = m ? si / m : NAN;
    return m;
}
static int seg_index(uint8_t ph)
{
    for (int k = 0; k < s_nseg; k++)
        if (s_segs[k].ph == ph)
            return k;
    return -1;
}

static void probe_build()
{
    const float vt = s_vtop, im = s_userImax;
    s_nseg = 0;
    auto add = [&](uint8_t ph, float dur, float v, float il, const char *nm) { s_segs[s_nseg++] = {ph, dur, v, il, nm}; };
    add(PR_PRE, 0.6f, 0, std::min(0.02f, im), "Source check");
    add(PR_TINY, 0.5f, std::min(0.2f, vt), std::min(0.01f, im), "Short check");
    add(PR_WATCH, 3.0f, vt, im, "Watch (motor?)");
    static const float steps[kSteps] = {0.001f, 0.005f, 0.010f, 0.020f, 0.050f, 0.100f};
    for (int k = 0; k < kSteps; k++)
        if (steps[k] <= im)
            add(PR_CC1 + k, 0.8f, vt, steps[k], "Current steps");
    add(PR_HOLD, 1.2f, vt, im, "Hold (inrush)");
    add(PR_LETGO, 2.0f, 0, im, "Let go");
}

static void probe_end_with(const String &summary, Sym sym)
{
    // Stops here (short / source): no sweep
    if (s_timer)
    {
        lv_timer_del(s_timer);
        s_timer = nullptr;
    }
    s_capturing.store(false);
    PowerSupply.Voltage.SetUpdate(s_savedAdj);
    progress(-1);
    end_run();
    draw_symbol(sym);
    show_result(summary);
}

static void probe_analyse_and_sweep()
{
    char b[64];
    String d = "Probe details";
    // Current steps: settled V, I of each
    double vs[kSteps], is[kSteps];
    int nst = 0;
    bool top[kSteps];
    for (int k = 0; k < kSteps; k++)
    {
        const int si = seg_index(PR_CC1 + k);
        if (si < 0)
            break;
        seg_stats(si, 0.6f, 1.0f, vs[nst], is[nst]);
        top[nst] = vs[nst] > 0.95 * s_vtop;
        nst++;
    }
    for (int k = 0; k < nst; k++)
    {
        snprintf(b, sizeof b, "\n%s %.3f V%s", fmtA(is[k]).c_str(), vs[k], top[k] ? " (top)" : "");
        d += b;
    }
    // Capacitor: V rising in a straight line during the first step
    if (nst && seg_index(PR_CC1) >= 0)
    {
        const int si = seg_index(PR_CC1);
        double va, ia, vb, ib;
        seg_stats(si, 0.1f, 0.3f, va, ia);
        seg_stats(si, 0.7f, 0.9f, vb, ib);
        const double slope = (vb - va) / (0.6 * s_segs[si].dur);
        if (slope > 0.05 && vb < 0.95 * s_vtop && ia > 0.0003)
        {
            s_ccC = 0.5 * (ia + ib) / slope; // shown at the end if no R drains it
        }
    }
    // Diode law over the steps that settled below the top
    int lo = -1, hi = -1;
    for (int k = 0; k < nst; k++)
        if (!top[k] && is[k] > 0.0005)
        {
            if (lo < 0)
                lo = k;
            hi = k;
        }
    // Diode with a resistor in parallel: the lowest step sits well below the
    // knee (the resistor carries it), the rest follow the diode law. Use
    // that step for Rp and fit the diode to the others.
    double rPar = NAN;
    if (lo >= 0 && hi > lo + 1 && vs[lo] > 0.2 && !(vs[hi] / vs[lo] < 1.6) && vs[hi] / vs[lo + 1] < 1.25 &&
        is[hi] / is[lo + 1] > 2.5)
    {
        rPar = vs[lo] / is[lo];
        lo++;
    }
    // Voltage falling while the current rises: no diode or resistor does
    // that - the load changed during the steps (motor, electronics)
    bool unstable = false;
    for (int k = lo + 1; lo >= 0 && k <= hi; k++)
        if (vs[k] < vs[k - 1] - 0.05)
            unstable = true;
    if (unstable)
    {
        s_diodeSummary = "Unstable load\nV fell while I\nrose: it changes\nby itself (motor,\nelectronics)";
        d += "\n(unstable)";
    }
    else if (lo >= 0 && hi > lo && vs[lo] > 0.2 && vs[hi] / vs[lo] < 1.6 && is[hi] / is[lo] > 2.5)
    {
        const double n = (vs[hi] - vs[lo]) / (0.02585 * log(is[hi] / is[lo]));
        snprintf(b, sizeof b, "\nn (2-point) %.2f", n);
        d += b;
        const double vf = vs[lo];
        const char *kind = vf < 0.4 ? "Schottky/Ge" : vf < 0.85 ? "silicon" : vf < 1.4 ? "2 diodes / IR" :
                           vf < 2.15 ? "red/yel LED" : vf < 3.5 ? "grn/blue/wht LED" : "LED string";
        d += String("\nType ") + kind;
        // Shockley fit over every settled step: V = n*Vt*ln(I) - n*Vt*ln(Is) + I*Rs,
        // linear in (n*Vt, -n*Vt*ln Is, Rs)
        int cnt = 0;
        double A[3][3] = {}, bb[3] = {};
        for (int k = lo; k <= hi; k++)
            if (!top[k] && is[k] > 0.0005)
            {
                const double x[3] = {log(is[k]), 1.0, is[k]};
                for (int r = 0; r < 3; r++)
                {
                    for (int c = 0; c < 3; c++)
                        A[r][c] += x[r] * x[c];
                    bb[r] += x[r] * vs[k];
                }
                cnt++;
            }
        double pp[3];
        double nf = n, rs = NAN, isat = NAN;
        if (cnt >= 3 && solve3(A, bb, pp) && pp[0] > 0)
        {
            nf = pp[0] / 0.02585;
            rs = pp[2];
            isat = exp(-pp[1] / pp[0]);
        }
        char t[200];
        snprintf(t, sizeof t, "Diode\nVf %.3f V\n  at %s\nVf %.3f V\n  at %s\nn %.2f  %s", vs[lo], fmtA(is[lo]).c_str(),
                 vs[hi], fmtA(is[hi]).c_str(), nf, kind);
        if (std::isfinite(rPar))
        {
            // The parallel R also takes V/Rp at each step: show the diode's own share
            snprintf(t, sizeof t, "Diode || R\nVf %.3f V\n  at %s\nRp ~%s\nn %.2f  %s", vs[hi],
                     fmtA(is[hi] - vs[hi] / rPar).c_str(), fmtR(rPar).c_str(), nf, kind);
        }
        s_diodeSummary = t;
        // Zener / avalanche: a voltage this high that barely moves with
        // current (n < 1 is impossible for a forward-biased junction)
        if (vs[lo] > 1.8 && n < 1.3)
        {
            // Vz at the step nearest 5 mA (datasheets' usual test current)
            int k5 = lo;
            for (int k = lo; k <= hi; k++)
                if (fabs(is[k] - 0.005) < fabs(is[k5] - 0.005))
                    k5 = k;
            static const float e24[] = {2.4f, 2.7f, 3.0f, 3.3f, 3.6f, 3.9f, 4.3f, 4.7f, 5.1f, 5.6f, 6.2f, 6.8f, 7.5f, 8.2f, 9.1f,
                                        10, 11, 12, 13, 15, 16, 18, 20, 22, 24, 27, 30};
            float best = e24[0];
            for (float z : e24)
                if (fabs(z - vs[k5]) < fabs(best - vs[k5]))
                    best = z;
            const double rz = (vs[hi] - vs[k5]) / std::max(1e-6, is[hi] - is[k5]);
            snprintf(t, sizeof t, "Zener (reverse)\nVz %.3f V\n  at %s\nVz %.3f V\n  at %s\nrz %s  ~%gV", vs[k5],
                     fmtA(is[k5]).c_str(), vs[hi], fmtA(is[hi]).c_str(), fmtR(rz).c_str(), best);
            s_diodeSummary = t;
            rs = NAN; // the forward diode law doesn't apply
        }
        if (std::isfinite(rs))
        {
            snprintf(b, sizeof b, "\nRs %s", fmtR(std::max(0.0, rs)).c_str());
            d += b;
            snprintf(b, sizeof b, "\nIs %.3g A", isat);
            d += b;
        }
    }
    else if (lo >= 0 && hi > lo && vs[hi] - vs[lo] > 0.002)
    {
        // Difference between steps: the meter's offset and tiny voltages
        // at the lowest current cancel out
        s_rCC = (vs[hi] - vs[lo]) / (is[hi] - is[lo]); // shown at the end unless charging
    }
    // Inrush: first 60 ms of the hold against its settled end
    {
        const int si = seg_index(PR_HOLD);
        double vE, iE;
        seg_stats(si, 0.6f, 1.0f, vE, iE);
        const int n = s_n.load();
        float ipk = 0;
        for (int q = 0; q < n; q++)
            if (s_ph[q] == PR_HOLD && s_t[q] < s_segT0[si] + 0.06f && s_i[q] > ipk)
                ipk = s_i[q];
        if (iE > 0.002 && ipk / iE > 1.05)
        {
            snprintf(b, sizeof b, "\nInrush x%.2f", ipk / iE);
            d += b;
        }
        // Let go: how the voltage falls once the PSU stops driving
        const int sl = seg_index(PR_LETGO);
        double vh = vE;
        if (vh > 0.2)
        {
            float t37 = NAN, vEnd = NAN;
            for (int q = 0; q < n; q++)
                if (s_ph[q] == PR_LETGO)
                {
                    if (!std::isfinite(t37) && s_v[q] < 0.368 * vh)
                        t37 = s_t[q] - s_segT0[sl];
                    vEnd = s_v[q];
                }
            if (std::isfinite(t37) && t37 < 0.02f)
                d += "\nLet go: instant";
            else if (std::isfinite(t37))
            {
                if (!std::isfinite(s_tau))
                    s_tau = t37;
                snprintf(b, sizeof b, "\nLet-go tau %.3g s", t37);
                d += b;
            }
            else if (std::isfinite(vEnd) && vEnd > 0.9 * vh)
                d += "\nLet go: holds V";
            else if (std::isfinite(vEnd) && vEnd > 0)
            {
                s_tau = s_segs[sl].dur / log(vh / vEnd);
                snprintf(b, sizeof b, "\nLet-go tau ~%.3g s", s_tau);
                d += b;
            }
        }
    }
    // C from the let-go: the load's R (hold V/I) drains it and the PSU
    // sinks a few mA as well (measured: negative I), so
    //   C * dV/dt = -V/R + I_psu  ->  C = sum(x*y)/sum(x*x),
    //   x = -dV/dt, y = V/R - I_psu
    {
        const int sh = seg_index(PR_HOLD), sl = seg_index(PR_LETGO);
        double vH, iH;
        seg_stats(sh, 0.6f, 1.0f, vH, iH);
        const int n = s_n.load();
        if (sl >= 0 && iH > 0.0002 && vH > 0.2)
        {
            const double R = vH / iH;
            static constexpr int k = 8;
            double sxy = 0, sxx = 0;
            for (int q = k; q < n - k; q++)
            {
                if (s_ph[q] != PR_LETGO || s_ph[q - k] != PR_LETGO || s_ph[q + k] != PR_LETGO || s_v[q] < 0.1 * vH)
                    continue;
                const double dt = s_t[q + k] - s_t[q - k];
                if (dt <= 0)
                    continue;
                const double x = -(s_v[q + k] - s_v[q - k]) / dt;
                const double y = s_v[q] / R - s_i[q];
                sxy += x * y;
                sxx += x * x;
            }
            if (sxx > 0 && std::isfinite(s_tau))
            {
                s_cLG = sxy / sxx;
                d += "\nC (let-go) " + fmtC(s_cLG);
            }
        }
    }
    if (std::isfinite(s_tau))
    {
        snprintf(b, sizeof b, "\nDecay tau %.3g s", s_tau);
        d += b;
    }
    else
    {
        if (std::isfinite(s_ccC))
            d += "\nC (I/dVdt) " + fmtC(s_ccC);
        if (std::isfinite(s_rCC))
            d += "\nR (steps) " + fmtR(s_rCC);
    }
    // keep what earlier steps already wrote (watch, source) after the header
    if (s_details.startsWith("Probe details"))
        d += s_details.substring(13);
    s_details = d;
    if (s_diodeSummary.length())
    {
        // Diodes conduct hard below V-Set: a sweep would only hit the limit
        // and the steps already carry the whole diode law
        // Plot the current steps: I against V
        double vmx = 0, imx = 0;
        for (int k = 0; k < nst; k++)
        {
            vmx = std::max(vmx, vs[k]);
            imx = std::max(imx, is[k]);
        }
        lv_chart_set_type(s_chart, LV_CHART_TYPE_SCATTER);
        lv_chart_set_point_count(s_chart, kPlotPts);
        lv_chart_set_range(s_chart, LV_CHART_AXIS_PRIMARY_X, 0, 1000);
        lv_chart_set_range(s_chart, LV_CHART_AXIS_PRIMARY_Y, 0, 1000);
        lv_chart_set_all_value(s_chart, s_ser1, LV_CHART_POINT_NONE);
        lv_chart_set_all_value(s_chart, s_ser2, LV_CHART_POINT_NONE);
        lv_obj_set_style_size(s_chart, 6, LV_PART_INDICATOR); // dots for 4 points
        for (int k = 0; k < nst; k++)
            lv_chart_set_value_by_id2(s_chart, s_ser1, k, (lv_coord_t)(1000 * vs[k] / (1.1 * vmx)), (lv_coord_t)(1000 * is[k] / (1.1 * imx)));
        lv_chart_refresh(s_chart);
        char ax[16];
        snprintf(ax, sizeof ax, "%.2f V", 1.1 * vmx);
        set_axes("Current steps", ax, fmtA(1.1 * imx), false);
        probe_end_with(s_diodeSummary, s_diodeSummary.startsWith("Zener") ? SYM_ZENER : s_diodeSummary.startsWith("Unstable") ? SYM_NONE : SYM_DIODE);
        return;
    }
    // Small currents only (hold under 6 mA, the mA range reads to ~8): run
    // the sweep on the mA range, ~100x less noise
    {
        bool allTop = nst > 0;
        for (int k = 0; k < nst; k++)
            allTop = allTop && top[k];
        double vE2, iE2;
        seg_stats(seg_index(PR_HOLD), 0.6f, 1.0f, vE2, iE2);
        if (iE2 < 0.006 && !PowerSupply.mA_Active)
        {
            s_maPrev = false;
            s_maSwitched = true;
            setCurrentRange(true);
            s_details += "\n(mA range)";
        }
    }
    sweep_begin();
}

static void probe_cb(lv_timer_t *)
{
    if (!output_usable())
    {
        probe_end_with("Stopped:\noutput off", SYM_NONE);
        return;
    }
    const uint32_t now = millis();
    const bool advance = s_seg < 0 || (now - s_segStartMs) * 1e-3f >= s_segs[s_seg].dur;
    if (advance)
    {
        // Decisions at the end of a step
        if (s_seg >= 0 && s_segs[s_seg].ph == PR_PRE)
        {
            // Still falling (a capacitor left charged, discharging through
            // the load): wait for it - and its decay is a free R*C reading
            double va, ia, vb, ib;
            const float d = s_segs[s_seg].dur;
            seg_stats(s_seg, 1.0f - 0.4f / d, 1.0f - 0.2f / d, va, ia);
            seg_stats(s_seg, 1.0f - 0.15f / d, 1.0f, vb, ib);
            if (vb > 0.2 && va > vb * 1.01 && d < 20.0f)
            {
                if (!std::isfinite(s_tau))
                    s_tau = 0.275f / log(va / vb); // centres 0.275 s apart
                s_segs[s_seg].dur += 0.5f;
                s_segs[s_seg].name = "Discharging";
                goto ui;
            }
        }
        if (s_seg >= 0 && s_segs[s_seg].ph == PR_PRE)
        {
            double v, i;
            seg_stats(s_seg, 1.0f - 0.2f / s_segs[s_seg].dur, 1.0f, v, i); // last 0.2 s
            if (v > 0.2)
            {
                // Something already holds a voltage: battery or charged cap.
                // Short charge pulse (20 mA) a little above it tells which.
                const float vb = std::min((float)v + 0.5f, std::max(s_vtop, (float)v + 0.1f));
                s_nseg = s_seg + 1;
                s_segs[s_nseg++] = {PR_BATT, 1.5f, vb, std::min(0.02f, s_userImax), "Charge pulse"};
                s_details = "Probe details\nV0 " + String(v, 3) + " V";
            }
        }
        if (s_seg >= 0 && s_segs[s_seg].ph == PR_TINY)
        {
            double v, i;
            seg_stats(s_seg, 0.5f, 1.0f, v, i);
            if (i > 0.004 && v / i < 0.05)
            {
                char b[64];
                snprintf(b, sizeof b, "Short\nR %s\nat %s\nNothing else run", fmtR(v / i).c_str(), fmtA(i).c_str());
                probe_end_with(b, SYM_NONE);
                return;
            }
        }
        if (s_seg >= 0 && s_segs[s_seg].ph == PR_WATCH)
        {
            // Current over the watch in 50 ms bins: surge, settling, switching
            const int n = s_n.load(std::memory_order_acquire);
            const float t0 = s_segT0[s_seg], dur = s_segs[s_seg].dur;
            static constexpr int kB = 60;
            float bs[kB] = {}, bv[kB] = {};
            int bc[kB] = {};
            for (int q = 0; q < n; q++)
                if (s_ph[q] == PR_WATCH)
                {
                    const int b = (int)((s_t[q] - t0) / 0.05f);
                    if (b >= 0 && b < kB)
                    {
                        bs[b] += s_i[q];
                        bv[b] += s_v[q];
                        bc[b]++;
                    }
                }
            float ipk = 0, imin = 1e9f, imax = 0;
            int jumps = 0, lastLvl = -1;
            for (int b = 0; b < kB; b++)
                if (bc[b])
                {
                    const float m = bs[b] / bc[b];
                    if (b < 4)
                        ipk = std::max(ipk, m);
                    imin = std::min(imin, m);
                    imax = std::max(imax, m);
                }
            double vEnd, iEnd;
            seg_stats(s_seg, 1.0f - 0.5f / dur, 1.0f, vEnd, iEnd);
            // switching: bins alternating between two levels
            for (int b = 4; b < kB; b++)
                if (bc[b])
                {
                    const float m = bs[b] / bc[b];
                    const int lvl = m > 0.5f * (imin + imax) ? 1 : 0;
                    if (lastLvl >= 0 && lvl != lastLvl)
                        jumps++;
                    lastLvl = lvl;
                }
            // settling time: first bin within 20% of the end current
            float tSettle = NAN;
            for (int b = 0; b < kB; b++)
                if (bc[b] && fabs(bs[b] / bc[b] - iEnd) < 0.2 * iEnd)
                {
                    tSettle = b * 0.05f;
                    break;
                }
            // Ripple: FFT of the last 1024 samples of the watch
            float fpk = NAN, apk = 0, floor_ = 0;
            {
                int last = -1;
                for (int q = n - 1; q >= 0; q--)
                    if (s_ph[q] == PR_WATCH)
                    {
                        last = q;
                        break;
                    }
                const int N = 1024;
                if (last >= N)
                {
                    float *re = (float *)heap_caps_malloc(N * sizeof(float), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
                    float *im = (float *)heap_caps_malloc(N * sizeof(float), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
                    if (re && im)
                    {
                        const int a = last - N + 1;
                        double mean = 0;
                        for (int q = 0; q < N; q++)
                            mean += s_i[a + q];
                        mean /= N;
                        float wsum = 0;
                        for (int q = 0; q < N; q++)
                        {
                            const float w = 0.5f - 0.5f * cosf(2 * M_PI * q / (N - 1));
                            wsum += w;
                            re[q] = (s_i[a + q] - mean) * w;
                            im[q] = 0;
                        }
                        const double fs = (N - 1) / std::max(1e-6f, s_t[last] - s_t[a]);
                        fft(re, im, N);
                        std::vector<float> mag(N / 2);
                        for (int q = 1; q < N / 2; q++)
                            mag[q] = 2 * sqrtf(re[q] * re[q] + im[q] * im[q]) / wsum;
                        int kp = 0;
                        const int kmin = (int)(15.0 * N / fs); // above 15 Hz
                        for (int q = std::max(2, kmin); q < N / 2; q++)
                            if (mag[q] > apk)
                                apk = mag[q], kp = q;
                        std::vector<float> srt(mag.begin() + 2, mag.end());
                        std::nth_element(srt.begin(), srt.begin() + srt.size() / 2, srt.end());
                        floor_ = srt[srt.size() / 2];
                        fpk = kp * fs / N;
                    }
                    heap_caps_free(re);
                    heap_caps_free(im);
                }
            }
            // commutation ripple is a real share of the current (fan: 3%), noise is not
            const bool ripple = std::isfinite(fpk) && apk > 6 * floor_ && apk > 0.0005f && apk > 0.005 * iEnd && iEnd > 0.005;
            const bool spinUp = ipk > 1.3 * iEnd && iEnd > 0.005 && iEnd > 0.1 * ipk;
            char b[160];
            snprintf(b, sizeof b, "\nWatch: peak %s end %s", fmtA(ipk).c_str(), fmtA(iEnd).c_str());
            if (!s_details.length())
                s_details = "Probe details";
            s_details += b;
            if (std::isfinite(fpk))
            {
                snprintf(b, sizeof b, "\nRipple %.1f Hz %s (x%.0f floor)", fpk, fmtA(apk).c_str(), apk / std::max(floor_, 1e-9f));
                s_details += b;
            }
            if (jumps >= 1 && imax > 3 * std::max(imin, 0.0005f) && imax > 0.005)
            {
                snprintf(b, sizeof b, "Switching load\n%s / %s\n%d changes in 3 s\nStalled motor\nretrying? Raise V",
                         fmtA(imax).c_str(), fmtA(imin).c_str(), jumps);
                probe_end_with(b, SYM_MOTOR);
                return;
            }
            if (ripple && (spinUp || fpk > 20))
            {
                snprintf(b, sizeof b, "Motor / fan\nRun %s\nSurge %s\nSpin-up %.1f s\nRipple %.0f Hz\n~%.0f RPM (4/rev)",
                         fmtA(iEnd).c_str(), fmtA(ipk).c_str(), std::isfinite(tSettle) ? tSettle : 0.0f, fpk, fpk * 60 / 4);
                snprintf(b + strlen(b), sizeof b - strlen(b), "");
                s_details += "\nP " + String(vEnd * iEnd, 3) + " W";
                probe_end_with(b, SYM_MOTOR);
                return;
            }
        }
        if (s_seg >= 0 && s_segs[s_seg].ph == PR_BATT)
        {
            const int si = s_seg;
            double v0, i0, va, ia, vb, ib;
            seg_stats(seg_index(PR_PRE), 1.0f - 0.2f / s_segs[seg_index(PR_PRE)].dur, 1.0f, v0, i0);
            seg_stats(si, 0.15f, 0.35f, va, ia);
            seg_stats(si, 0.75f, 0.95f, vb, ib);
            const double slope = (vb - va) / (0.6 * s_segs[si].dur);
            char b[120];
            String sum;
            Sym sym;
            if (slope > 0.02 && ib > 0.001)
            {
                const double c = 0.5 * (ia + ib) / slope;
                snprintf(b, sizeof b, "Charged cap\nV0 %.3f V\nC %s\n(from charge rate)", v0, fmtC(c).c_str());
                sym = SYM_C;
            }
            else
            {
                // HPPC-style: the step from the PRE current (the PSU sinks a
                // few mA from a source at 0 V) to the charge current.
                // R0 = instant (ohmic) part, R = after the pulse (+ polarization)
                const double dI = ib - i0;
                double v1, i1;
                seg_stats(si, 0.0f, 0.04f / s_segs[si].dur, v1, i1); // first 40 ms
                const double r0 = (std::isfinite(v1) && i1 - i0 > 0.002) ? (v1 - v0) / (i1 - i0) : NAN;
                const double rt = dI > 0.002 ? (vb - v0) / dI : NAN;
                const double ocv = std::isfinite(rt) ? v0 - i0 * rt : v0;
                // Rough state of charge from the open-circuit voltage
                String kind;
                if (ocv > 3.0 && ocv < 4.3)
                {
                    static const float tv[] = {3.0f, 3.3f, 3.5f, 3.6f, 3.7f, 3.8f, 3.9f, 4.0f, 4.1f, 4.2f};
                    static const float tp[] = {0, 2, 7, 15, 30, 50, 65, 78, 90, 100};
                    float pc = 100;
                    for (int k = 1; k < 10; k++)
                        if (ocv < tv[k])
                        {
                            pc = tp[k - 1] + (tp[k] - tp[k - 1]) * (ocv - tv[k - 1]) / (tv[k] - tv[k - 1]);
                            break;
                        }
                    char kb[24];
                    snprintf(kb, sizeof kb, "Li-ion ~%.0f%%", pc);
                    kind = kb;
                }
                else if (ocv > 1.0 && ocv < 1.7)
                    kind = ocv > 1.45 ? "alkaline/primary" : "NiMH / NiCd";
                else if (ocv > 1.8 && ocv < 2.2)
                    kind = "lead-acid cell";
                else
                    kind = "source";
                snprintf(b, sizeof b, "Battery\nOCV %.3f V\nR0 %s\nR %s\n%s", ocv,
                         std::isfinite(r0) ? fmtR(r0).c_str() : "-", std::isfinite(rt) ? fmtR(rt).c_str() : "-", kind.c_str());
                char dd[160];
                snprintf(dd, sizeof dd, "\nI0 %s (sink)\nV %.3f V\nI %s\nR0 = 40 ms\nR = 1.5 s",
                         fmtA(i0).c_str(), vb, fmtA(ib).c_str());
                s_details += dd;
                sym = SYM_BATT;
            }
            probe_end_with(b, sym);
            return;
        }
        if (s_seg + 1 >= s_nseg)
        {
            if (s_timer)
            {
                lv_timer_del(s_timer);
                s_timer = nullptr;
            }
            probe_analyse_and_sweep();
            return;
        }
        s_seg++;
        s_segStartMs = now;
        s_segT0[s_seg] = (now - s_startMs) * 1e-3f;
        const Seg &sg = s_segs[s_seg];
        set_amps(sg.ilim);
        set_volts(sg.v);
        s_vcmd = sg.v;
        s_phase = sg.ph;
    }
ui:
    static uint32_t lastUi = 0;
    if (now - lastUi > 150)
    {
        lastUi = now;
        float total = 0, done = 0;
        for (int k = 0; k < s_nseg; k++)
        {
            total += s_segs[k].dur;
            if (k < s_seg)
                done += s_segs[k].dur;
        }
        done += (now - s_segStartMs) * 1e-3f;
        char b[120];
        snprintf(b, sizeof b, "Probe %d/%d\n%s\nlimit %s\n%.3f V\n%s", s_seg + 1, s_nseg, s_segs[s_seg].name,
                 fmtA(s_segs[s_seg].ilim).c_str(), PowerSupply.Voltage.oldValue,
                 fmtA(PowerSupply.Current.oldValue * (PowerSupply.mA_Active ? 0.001 : 1.0)).c_str());
        show_result(b);
        progress((int)(40 * done / std::max(total, 0.1f))); // probe = first 40%
    }
}

static void probe_begin()
{
    s_tau = NAN;
    s_ccC = NAN;
    s_rCC = NAN;
    s_cLG = NAN;
    s_diodeSummary = "";
    s_details = "";
    s_seg = -1;
    s_vcmd = 0;
    s_phase = PR_PRE;
    s_cap = kMaxSamples;
    s_n.store(0);
    s_startMs = millis();
    s_capturing.store(true);
    probe_build();
    s_timer = lv_timer_create(probe_cb, 10, nullptr);
}

bool loadSweepStart(bool zero)
{
    s_impRunning = false;
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
    s_userImax = s_imax > 0 ? s_imax : user_iset();
    s_showDetails = false;
    s_details = "";
    begin_run();
    draw_symbol(SYM_NONE);
    progress(0);
    if (zero)
        sweep_begin();
    else
    {
        show_result("Probe\nstarting");
        probe_begin();
    }
    return true;
}

static void sweep_begin()
{
    s_stage = 1;
    s_vcmd = 0;
    s_phase = PH_SETTLE;
    s_cap = kMaxSamples;
    s_n.store(0);
    s_startMs = millis();
    s_capturing.store(true);
    set_amps(s_userImax);
    set_volts(0);
    s_timer = lv_timer_create(sweep_cb, 10, nullptr);
    show_result(s_isZero ? "Zero sweep...\n(nothing\nconnected)" : "Sweeping\n0 -> V-Set -> 0");
}

bool loadFftStart()
{
    s_impRunning = false;
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

// ─── Impedance spectroscopy + LCR (lock-in) ─────────────────────────────────
// Pass 1: DC bias V-Set/2 plus small sines at 0.5-10 Hz -> |Z(f)|, Bode plot.
// Pass 2 (LCR): 10 Hz x30 cycles and 1 Hz x6, each sine sized from pass 1
// so the current swings ~+-2 mA (inside what the PSU can sink), DAC at
// 500 updates/s. Readout like an LCR meter: Cs, ESR, D, Cp, Rp (or Ls, Q).
// V and I are read ~1-3 ms apart (+ filter delays): modelled as one delay
// tau_d, phase error = omega*tau_d. A run on a plain resistor (DC R ==
// |Z|, phase near 0) measures tau_d and stores it (NVS "imp_td").
static constexpr int kImpMax = 8;
static constexpr uint8_t PH_IMP0 = 40; // + segment index
static float s_impF[kImpMax], s_impA[kImpMax], s_impT0[kImpMax], s_impDur[kImpMax];
static int s_impNseg = 0, s_impPass = 1;
static float s_impBias = 0, s_impTotal = 0;
static float s_impTd = 0;            // V/I timing skew, s (fallback model)
// Measured phase of the whole chain on a resistive load, per frequency
// (NVS "imp_pc"); subtracted when present - the skew is not one fixed delay
static constexpr float kCalF[7] = {0.5f, 1, 2, 5, 10, 20, 50};
struct PhCal { uint32_t magic; float ph[7]; };
static PhCal s_phCal{0, {}};
static constexpr uint32_t kPhMagic = 0x9AC40001;
static bool phase_cal_for(float f, double &ph)
{
    if (s_phCal.magic != kPhMagic)
        return false;
    for (int k = 0; k < 7; k++)
        if (kCalF[k] == f)
        {
            ph = s_phCal.ph[k];
            return true;
        }
    return false;
}
static bool s_impTdLoaded = false;
static std::complex<double> s_z1[kImpMax]; // pass-1 results
static float s_z1f[kImpMax];
static std::complex<double> s_rawZ[kImpMax]; // uncorrected, for calibration
static int s_z1n = 0;
static constexpr float kImpSettleS = 2.0f;
static constexpr float kItarget = 0.001f; // A, AC amplitude in pass 2 (the sink limits the negative half: ~3 mA minus DC)

static void imp_load_td()
{
    if (s_impTdLoaded)
        return;
    s_impTdLoaded = true;
    PowerSupply.StoreMem.begin("my-app", true);
    s_impTd = PowerSupply.StoreMem.isKey("imp_td") ? PowerSupply.StoreMem.getFloat("imp_td", 0.0f) : 0.0f;
    if (PowerSupply.StoreMem.isKey("imp_pc"))
        PowerSupply.StoreMem.getBytes("imp_pc", &s_phCal, sizeof s_phCal);
    PowerSupply.StoreMem.end();
    if (!std::isfinite(s_impTd) || fabsf(s_impTd) > 0.02f)
        s_impTd = 0;
}

// Lock-in over segment k (skipping its first cycle): Z = V/I, delay-corrected
static bool imp_phasor(int k, std::complex<double> &Z, double &vdc, double &idc, double &iac)
{
    const int n = s_n.load(std::memory_order_acquire);
    const double w = 2 * M_PI * s_impF[k];
    const float t0 = s_impT0[k] + 1.0f / s_impF[k], t1 = s_impT0[k] + s_impDur[k];
    double vm = 0, im = 0;
    int m = 0;
    for (int q = 0; q < n; q++)
        if (s_ph[q] == PH_IMP0 + k && s_t[q] >= t0 && s_t[q] < t1)
        {
            vm += s_v[q];
            im += s_i[q];
            m++;
        }
    if (m < 20)
        return false;
    vm /= m;
    im /= m;
    double vc = 0, vs = 0, ic = 0, is = 0;
    for (int q = 0; q < n; q++)
        if (s_ph[q] == PH_IMP0 + k && s_t[q] >= t0 && s_t[q] < t1)
        {
            const double tt = s_t[q] - s_impT0[k];
            vc += (s_v[q] - vm) * cos(w * tt);
            vs += (s_v[q] - vm) * sin(w * tt);
            ic += (s_i[q] - im) * cos(w * tt);
            is += (s_i[q] - im) * sin(w * tt);
        }
    const std::complex<double> V(vs, vc), I(is, ic);
    if (std::abs(I) < 1e-12)
        return false;
    // remove the chain's phase: per-frequency table, else the delay model
    double pc;
    Z = V / I;
    s_rawZ[k] = Z;
    if (phase_cal_for(s_impF[k], pc))
        Z *= std::polar(1.0, -pc);
    else
        Z *= std::polar(1.0, w * s_impTd);
    vdc = vm;
    idc = im;
    iac = 2 * std::abs(I) / m;
    return true;
}

static void imp_plan(int pass)
{
    s_impPass = pass;
    s_impNseg = 0;
    float t = kImpSettleS;
    auto add = [&](float f, float cyc, float a) {
        s_impF[s_impNseg] = f;
        s_impA[s_impNseg] = a;
        s_impT0[s_impNseg] = t;
        s_impDur[s_impNseg] = cyc / f;
        t += cyc / f;
        s_impNseg++;
    };
    const float a0 = std::min(0.2f, s_impBias * 0.4f);
    if (pass == 1)
    {
        static const float f1[] = {0.5f, 1, 2, 5, 10, 20, 50}, c1[] = {2, 3, 4, 6, 10, 20, 50};
        for (int k = 0; k < 7; k++)
            add(f1[k], c1[k], a0 * std::min(1.0f, 1.0f / f1[k]));
    }
    else
    {
        // Size each sine from pass 1: amplitude = Itarget * |Z(f)|
        auto zAt = [&](float f) {
            for (int k = 0; k < s_z1n; k++)
                if (s_z1f[k] == f)
                    return std::abs(s_z1[k]);
            return 0.0;
        };
        const float amax = std::min(0.4f, s_impBias * 0.4f);
        add(10, 100, std::max(0.005f, std::min(amax, (float)(kItarget * zAt(10)))));
        add(1, 6, std::max(0.005f, std::min(amax, (float)(kItarget * zAt(1)))));
        // 20 Hz, not 50: at 50 Hz capacitors still read ~10 deg off (ADC re-used
        // samples suspected); 20 Hz and below are within +-1.5 deg
        add(20, 80, std::max(0.005f, std::min(amax, (float)(kItarget * zAt(20)))));
    }
    s_impTotal = t;
}

static void imp_start_capture()
{
    s_vcmd = s_impBias;
    s_phase = PH_SETTLE;
    s_cap = kMaxSamples;
    s_n.store(0);
    s_startMs = millis();
    s_capturing.store(true);
    set_volts(s_impBias);
}

static String fmtEsr(double r)
{
    if (r <= 0.005)
        return "~0 Ohm";
    return fmtR(r);
}

static String lcrText(const std::complex<double> &Z, float f, bool full)
{
    const double w = 2 * M_PI * f;
    char b[200];
    if (Z.imag() < 0)
    {
        const double cs = -1.0 / (w * Z.imag()), esr = Z.real(), D = esr / -Z.imag();
        const std::complex<double> Y = 1.0 / Z;
        const double cp = Y.imag() / w, rp = Y.real() > 0 ? 1.0 / Y.real() : INFINITY;
        if (full)
            snprintf(b, sizeof b, "LCR @ %g Hz\nCs %s\nESR %s\nD %.3f\nCp %s\nRp %s", f, fmtC(cs).c_str(),
                     fmtEsr(esr).c_str(), D, fmtC(cp).c_str(), fmtR(rp).c_str());
        else
            snprintf(b, sizeof b, "\n@%gHz Cs %s D %.3f ESR %s", f, fmtC(cs).c_str(), D, fmtEsr(esr).c_str());
    }
    else
    {
        const double ls = Z.imag() / w, q = Z.real() > 0 ? Z.imag() / Z.real() : INFINITY;
        char lb[24];
        snprintf(lb, sizeof lb, ls >= 1 ? "%.3g H" : "%.3g mH", ls >= 1 ? ls : ls * 1e3);
        if (full)
            snprintf(b, sizeof b, "LCR @ %g Hz\nLs %s\nRs %s\nQ %.2f\n|Z| %s", f, lb, fmtR(std::max(0.0, Z.real())).c_str(), q,
                     fmtR(std::abs(Z)).c_str());
        else
            snprintf(b, sizeof b, "\n@%gHz Ls %s Rs %s", f, lb, fmtR(std::max(0.0, Z.real())).c_str());
    }
    return b;
}

static bool s_calDone = false;
static void analyse_imp_pass1()
{
    s_z1n = 0;
    s_calDone = false;
    for (int k = 0; k < s_impNseg; k++)
    {
        std::complex<double> Z;
        double vd, id, ia;
        if (imp_phasor(k, Z, vd, id, ia))
        {
            s_z1[s_z1n] = Z;
            s_z1f[s_z1n] = s_impF[k];
            s_z1n++;
        }
    }
    // Resistive load (|Z| flat within 3%, raw phase small everywhere):
    // record the chain's phase per frequency
    if (s_z1n == 7)
    {
        double zmin = 1e30, zmax = 0, pmax = 0;
        for (int k = 0; k < 7; k++)
        {
            zmin = std::min(zmin, std::abs(s_rawZ[k]));
            zmax = std::max(zmax, std::abs(s_rawZ[k]));
            pmax = std::max(pmax, fabs(std::arg(s_rawZ[k])));
        }
        if (zmax / zmin < 1.03 && pmax < 0.6)
        {
            s_phCal.magic = kPhMagic;
            for (int k = 0; k < 7; k++)
                s_phCal.ph[k] = std::arg(s_rawZ[k]);
            PowerSupply.StoreMem.begin("my-app", false);
            PowerSupply.StoreMem.putBytes("imp_pc", &s_phCal, sizeof s_phCal);
            PowerSupply.StoreMem.end();
            s_calDone = true;
            for (int k = 0; k < 7; k++)
                s_z1[k] = std::abs(s_z1[k]); // resistive: phase 0 after the table
        }
    }
    // Bode plot of pass 1: |Z| log (amber), phase (cyan, 0 top .. -90 bottom)
    double lmin = 1e9, lmax = -1e9;
    for (int k = 0; k < s_z1n; k++)
    {
        lmin = std::min(lmin, log10(std::abs(s_z1[k])));
        lmax = std::max(lmax, log10(std::abs(s_z1[k])));
    }
    if (lmax - lmin < 0.5)
    {
        const double c0 = 0.5 * (lmax + lmin);
        lmin = c0 - 0.25, lmax = c0 + 0.25;
    }
    lv_obj_set_style_size(s_chart, 6, LV_PART_INDICATOR);
    lv_chart_set_type(s_chart, LV_CHART_TYPE_LINE);
    lv_chart_set_point_count(s_chart, std::max(2, s_z1n));
    lv_chart_set_range(s_chart, LV_CHART_AXIS_PRIMARY_Y, 0, 1000);
    for (int k = 0; k < s_z1n; k++)
    {
        const double ph = std::arg(s_z1[k]) * 180 / M_PI;
        lv_chart_set_value_by_id(s_chart, s_ser1, k, (lv_coord_t)(1000 * (log10(std::abs(s_z1[k])) - lmin) / (lmax - lmin)));
        lv_chart_set_value_by_id(s_chart, s_ser2, k, (lv_coord_t)(1000 * (1 + std::max(-90.0, std::min(0.0, ph)) / 90)));
    }
    lv_chart_refresh(s_chart);
    set_axes("#FFC107 |Z| log#  #00BCD4 phase#  0.5-50 Hz", "50 Hz", fmtR(pow(10, lmax)), false);
}

static void analyse_imp_pass2()
{
    char b[96];
    String d = "Impedance\nf: |Z| phase";
    for (int k = 0; k < s_z1n; k++)
    {
        snprintf(b, sizeof b, "\n%gHz %s %.1f", s_z1f[k], fmtR(std::abs(s_z1[k])).c_str(), std::arg(s_z1[k]) * 180 / M_PI);
        d += b;
    }
    std::complex<double> Z10, Z1;
    double vd = 0, id = 0, ia10 = 0, ia1 = 0, vd2, id2;
    const bool ok10 = imp_phasor(0, Z10, vd, id, ia10);
    const bool ok1 = imp_phasor(1, Z1, vd2, id2, ia1);
    if (!ok10 && !ok1)
    {
        show_result("LCR\nno signal");
        return;
    }
    // DC part: bias over mean current = the parallel (leak) R, exact
    const double Rdc = id > 0.0002 ? vd / id : INFINITY;
    if (s_calDone)
    {
        String c = "Phase calibrated\n(resistive load)\nR " + fmtR(std::abs(s_z1[4])) + "\nsaved for 0.5-50 Hz";
        String dd = "Phase cal (deg):";
        for (int k = 0; k < 7; k++)
        {
            snprintf(b, sizeof b, "\n%gHz %+.2f", kCalF[k], s_phCal.ph[k] * 180 / M_PI);
            dd += b;
        }
        show_result(c);
        draw_symbol(SYM_R);
        s_details = dd;
        render_result();
        return;
    }
    // Self-calibration on a plain resistor: |Z| == DC R and phase small
    // at both frequencies -> the remaining phase is the V/I skew
    if (ok10 && ok1 && std::isfinite(Rdc) && fabs(std::abs(Z10) / Rdc - 1) < 0.03 && fabs(std::abs(Z1) / Rdc - 1) < 0.03 &&
        fabs(std::arg(Z10)) < 0.35)
    {
        // residual phase at 10 Hz (corrected with the old tau) -> new tau
        const double extra = std::arg(Z10) / (2 * M_PI * 10); // s; Z rotated by +w*td
        s_impTd -= extra;
        PowerSupply.StoreMem.begin("my-app", false);
        PowerSupply.StoreMem.putFloat("imp_td", s_impTd);
        PowerSupply.StoreMem.end();
        snprintf(b, sizeof b, "\nPhase cal: skew %.2f ms (saved)", s_impTd * 1e3);
        d += b;
        snprintf(b, sizeof b, "Resistor\nR %s\n(phase calib.\nsaved: %.2f ms)", fmtR(Rdc).c_str(), s_impTd * 1e3);
        show_result(b);
        draw_symbol(SYM_R);
        s_details = d;
        render_result();
        return;
    }
    String r = lcrText(ok10 ? Z10 : Z1, ok10 ? 10 : 1, true);
    if (std::isfinite(Rdc) && Z10.imag() < 0)
        r += "\nRdc " + fmtR(Rdc);
    show_result(r);
    draw_symbol(Z10.imag() < 0 ? (std::isfinite(Rdc) ? SYM_RC : SYM_C) : SYM_NONE);
    if (ok1)
        d += lcrText(Z1, 1, false);
    {
        std::complex<double> Z50;
        double v5, i5, a5;
        if (imp_phasor(2, Z50, v5, i5, a5))
            d += lcrText(Z50, 20, false);
    }
    snprintf(b, sizeof b, "\nIac %s @10Hz\nskew corr %.2f ms%s", fmtA(ia10).c_str(), s_impTd * 1e3,
             s_impTd == 0 ? " (uncal: run on a resistor)" : "");
    d += b;
    snprintf(b, sizeof b, "\nbias %.2f V\nDAC %.0f/s", s_impBias, s_wTicks * 1000.0 / std::max<uint32_t>(1, millis() - s_wStartMs));
    d += b;
    s_details = d;
    render_result();
}

static void imp_cb(lv_timer_t *)
{
    if (!output_usable())
    {
        s_wOn = false;
        wave_timer_stop();
        s_capturing.store(false);
        lv_timer_del(s_timer);
        s_timer = nullptr;
        PowerSupply.Voltage.SetUpdate(s_savedAdj);
        progress(-1);
        end_run();
        show_result("Stopped:\noutput off");
        return;
    }
    const float t = (millis() - s_startMs) * 1e-3f;
    if (t >= s_impTotal)
    {
        if (s_impPass == 1)
        {
            s_capturing.store(false);
            analyse_imp_pass1();
            imp_plan(2);
            imp_start_capture();
            return;
        }
        s_wOn = false;
        wave_timer_stop();
        lv_timer_del(s_timer);
        s_timer = nullptr;
        s_capturing.store(false);
        PowerSupply.Voltage.SetUpdate(s_savedAdj);
        progress(-1);
        end_run();
        analyse_imp_pass2();
        return;
    }
    int seg = -1;
    for (int k = 0; k < s_impNseg; k++)
        if (t >= s_impT0[k] && t < s_impT0[k] + s_impDur[k])
            seg = k;
    // Segment change: hand the sine to the Core-0 generator (loadWaveTick)
    static int lastSeg = -2;
    static int lastPass = 0;
    if (seg != lastSeg || s_impPass != lastPass)
    {
        lastSeg = seg;
        lastPass = s_impPass;
        if (seg >= 0)
        {
            s_wF = s_impF[seg];
            s_wT0 = micros() - (uint32_t)((t - s_impT0[seg]) * 1e6f);
            s_wAmp = s_impA[seg];
        }
        else
            s_wAmp = 0;
        s_wBias = s_impBias;
        s_phase = seg >= 0 ? PH_IMP0 + seg : PH_SETTLE;
    }
    static uint32_t lastUi = 0;
    if (millis() - lastUi > 200)
    {
        lastUi = millis();
        char b[110];
        if (seg >= 0)
            snprintf(b, sizeof b, "%s\n%g Hz sine\n+-%.0f mV on %.2f V\n%.0f s left", s_impPass == 1 ? "Z(f) sweep" : "LCR measure",
                     s_impF[seg], 1000 * s_impA[seg], s_impBias, s_impTotal - t);
        else
            snprintf(b, sizeof b, "%s\nsettling at\n%.2f V bias", s_impPass == 1 ? "Z(f) sweep" : "LCR measure", s_impBias);
        show_result(b);
        progress((s_impPass == 1 ? 0 : 50) + (int)(50 * t / s_impTotal));
    }
}

bool loadImpStart()
{
    if (s_timer || !output_usable() || !alloc_buffers())
        return false;
    s_vtop = user_vset();
    if (s_vtop < 0.5f)
    {
        show_result("Set V-Set >= 0.5 V\n(bias = V-Set/2)");
        return false;
    }
    imp_load_td();
    s_isFft = false;
    s_isZero = false;
    s_savedAdj = PowerSupply.Voltage.adjValue;
    s_showDetails = false;
    s_details = "";
    s_impBias = s_vtop / 2;
    imp_plan(1);
    begin_run();
    imp_start_capture();
    draw_symbol(SYM_NONE);
    progress(0);
    s_impRunning = true;
    s_wBias = s_impBias;
    s_wAmp = 0;
    s_wTicks = 0;
    s_wStartMs = millis();
    s_wOn = true; // the 2 kHz generator now drives the voltage DAC
    wave_timer_start();
    s_timer = lv_timer_create(imp_cb, 2, nullptr);
    return true;
}

static void btn_cb(lv_event_t *e)
{
    const int which = (int)(intptr_t)lv_event_get_user_data(e);
    bool ok = which == 1 ? loadImpStart() : which == 2 ? loadFftStart() : loadSweepStart(which == 3);
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
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(card, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(card, [](lv_event_t *) {
        if (!s_details.length())
            return;
        s_showDetails = !s_showDetails;
        render_result();
    }, LV_EVENT_CLICKED, nullptr);

    s_result = lv_label_create(card);
    lv_obj_set_style_text_font(s_result, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(s_result, lv_color_hex(0xE6EDF3), 0);
    lv_obj_set_style_text_line_space(s_result, 1, 0);
    lv_label_set_recolor(s_result, true);
    lv_obj_set_width(s_result, 108);
    lv_label_set_long_mode(s_result, LV_LABEL_LONG_CLIP);
    lv_obj_set_pos(s_result, 5, 4);
    show_result("Ready\nSweep finds\nR, C, diode,\nthen details.\nTop = V-Set\nZero: once,\nno load");

    s_hint = small(card, lv_color_hex(0x8A94A6));
    lv_obj_align(s_hint, LV_ALIGN_TOP_RIGHT, -4, 4);
    lv_obj_add_flag(s_hint, LV_OBJ_FLAG_HIDDEN);

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
    lv_dropdown_set_options(s_imaxDd, "Imax: I-Set\nImax: 10 mA\nImax: 20 mA\nImax: 50 mA\nImax: 100 mA\nImax: 250 mA\nImax: 500 mA\nImax: 1 A\nImax: 2 A\nImax: 3 A\nImax: 5 A");
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

    static const char *names[4] = {LV_SYMBOL_PLAY " Probe", "Z(f)", "FFT", "Zero"};
    for (int b = 0; b < 4; b++)
    {
        lv_obj_t *btn = lv_btn_create(parent);
        s_btns[b] = btn;
        lv_obj_set_size(btn, b == 0 ? 58 : 44, 22);
        lv_obj_set_pos(btn, b == 0 ? 3 : 64 + (b - 1) * 46, 160);
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
