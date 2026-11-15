// 9-point affine touch calibration. See touch_affine.h.
#include "touch_affine.h"
#include "TFT_eSPI.h"
#include <Preferences.h>
#include "esp_task_wdt.h"
#include <cmath>

extern TFT_eSPI tft;

struct AffCal
{
    uint32_t magic;
    float a, b, c, d, e, f;
};
static constexpr uint32_t kMagic = 0xAFF10001;
static AffCal s_cal{0, 0, 0, 0, 0, 0, 0};

void touchAffineLoad()
{
    Preferences p;
    p.begin("my-app", true);
    AffCal c{};
    const bool ok = p.getBytes("touch_aff", &c, sizeof c) == sizeof c && c.magic == kMagic &&
                    std::isfinite(c.a) && std::isfinite(c.e);
    p.end();
    s_cal = ok ? c : AffCal{0, 0, 0, 0, 0, 0, 0};
    Serial.printf("Touch mapping: %s\n", ok ? "9-point affine" : "4-corner (TFT_eSPI)");
}

bool touchAffineActive() { return s_cal.magic == kMagic; }

static void mapF(const AffCal &k, float rx, float ry, float &x, float &y)
{
    x = k.a * rx + k.b * ry + k.c;
    y = k.d * rx + k.e * ry + k.f;
}

bool touchAffineMap(uint16_t rx, uint16_t ry, uint16_t *x, uint16_t *y)
{
    if (!touchAffineActive())
        return false;
    float fx, fy;
    mapF(s_cal, rx, ry, fx, fy);
    if (fx < 0 || fy < 0 || fx >= tft.width() || fy >= tft.height())
        return false;
    *x = (uint16_t)lroundf(fx);
    *y = (uint16_t)lroundf(fy);
    return true;
}

// Screen position of a raw reading with whatever mapping is active
static bool mapActive(uint16_t rx, uint16_t ry, uint16_t *x, uint16_t *y)
{
    if (touchAffineActive())
        return touchAffineMap(rx, ry, x, y);
    uint16_t xx = rx, yy = ry;
    tft.convertRawXY(&xx, &yy);
    if (xx >= tft.width() || yy >= tft.height())
        return false;
    *x = xx, *y = yy;
    return true;
}

static constexpr uint16_t kZ = 600;

bool touchReadScreen(uint16_t *x, uint16_t *y)
{
    if (tft.getTouchRawZ() <= kZ)
        return false;
    uint16_t x1, y1, x2, y2;
    tft.getTouchRaw(&x1, &y1);
    if (tft.getTouchRawZ() <= kZ)
        return false;
    tft.getTouchRaw(&x2, &y2);
    if (abs((int)x1 - (int)x2) > 20 || abs((int)y1 - (int)y2) > 20)
        return false;
    return mapActive((x1 + x2) / 2, (y1 + y2) / 2, x, y);
}

// ─── Screens ────────────────────────────────────────────────────────────────
static int waitButtons(const char *const *labels, const uint16_t *colors, int n)
{
    const int y = 240 - 52, h = 42, gap = 10, w = (320 - gap * (n + 1)) / n;
    tft.setTextFont(2);
    for (int k = 0; k < n; k++)
    {
        const int x = gap + k * (w + gap);
        tft.fillRoundRect(x, y, w, h, 8, colors[k]);
        tft.setTextColor(TFT_BLACK, colors[k]);
        tft.setTextDatum(MC_DATUM);
        tft.drawString(labels[k], x + w / 2, y + h / 2);
    }
    tft.setTextDatum(TL_DATUM);
    uint16_t tx, ty;
    for (;;)
    {
        esp_task_wdt_reset();
        if (touchReadScreen(&tx, &ty))
            for (int k = 0; k < n; k++)
            {
                const int x = gap + k * (w + gap);
                if (tx >= x && tx < x + w && ty >= y - 6 && ty < y + h + 6)
                {
                    tft.drawRoundRect(x, y, w, h, 8, TFT_WHITE);
                    while (tft.getTouchRawZ() > kZ)
                        delay(10);
                    delay(150);
                    return k;
                }
            }
        delay(15);
    }
}

static void crosshair(int x, int y, uint16_t c)
{
    tft.drawCircle(x, y, 9, c);
    tft.drawCircle(x, y, 3, c);
    tft.drawFastHLine(x - 15, y, 31, c);
    tft.drawFastVLine(x, y - 15, 31, c);
}

// Hold on the target: median of the steady raw readings
static bool sampleRaw(uint16_t &rx, uint16_t &ry)
{
    // wait for a press
    while (tft.getTouchRawZ() <= kZ)
    {
        esp_task_wdt_reset();
        delay(5);
    }
    delay(60); // let the finger settle
    static uint16_t xs[64], ys[64];
    int n = 0;
    const uint32_t t0 = millis();
    while (millis() - t0 < 400 && n < 64)
    {
        if (tft.getTouchRawZ() > kZ)
        {
            uint16_t x, y;
            tft.getTouchRaw(&x, &y);
            if (tft.getTouchRawZ() > kZ) // still pressed after the read: valid
            {
                xs[n] = x;
                ys[n] = y;
                n++;
            }
        }
        delay(4);
    }
    // wait for the release (and a short quiet time)
    uint32_t up = millis();
    while (millis() - up < 120)
    {
        esp_task_wdt_reset();
        if (tft.getTouchRawZ() > kZ / 2)
            up = millis();
        delay(5);
    }
    if (n < 8)
        return false;
    std::sort(xs, xs + n);
    std::sort(ys, ys + n);
    rx = xs[n / 2];
    ry = ys[n / 2];
    return true;
}

// Least squares: [rx ry 1] * (a b c) = x, separately for x and y
static bool fit(const float *rx, const float *ry, const float *sx, const float *sy, int n, AffCal &k)
{
    double M[3][3] = {}, bx[3] = {}, by[3] = {};
    for (int i = 0; i < n; i++)
    {
        const double v[3] = {rx[i], ry[i], 1.0};
        for (int r = 0; r < 3; r++)
        {
            for (int c = 0; c < 3; c++)
                M[r][c] += v[r] * v[c];
            bx[r] += v[r] * sx[i];
            by[r] += v[r] * sy[i];
        }
    }
    // Cramer's rule
    auto det3 = [](double m[3][3]) {
        return m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1]) - m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0]) +
               m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
    };
    const double D = det3(M);
    if (fabs(D) < 1e-6)
        return false;
    double sol[2][3];
    for (int axis = 0; axis < 2; axis++)
    {
        const double *b = axis ? by : bx;
        for (int col = 0; col < 3; col++)
        {
            double T[3][3];
            memcpy(T, M, sizeof T);
            for (int r = 0; r < 3; r++)
                T[r][col] = b[r];
            sol[axis][col] = det3(T) / D;
        }
    }
    k = {kMagic, (float)sol[0][0], (float)sol[0][1], (float)sol[0][2], (float)sol[1][0], (float)sol[1][1], (float)sol[1][2]};
    return true;
}

bool touchCalibrateScreen()
{
    tft.fillScreen(TFT_BLACK);
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setCursor(0, 8);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.println("  Touch screen calibration");
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.printf("\n  In use: %s\n", touchAffineActive() ? "9-point fit" : "4-corner (basic)");
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.println("\n  9 targets appear one by one.");
    tft.println("  Press the centre of each with a");
    tft.println("  stylus and hold until it turns");
    tft.println("  green, then lift.");
    tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
    tft.println("\n  Corrects scale, offset, rotation");
    tft.println("  and skew of the touch panel.");
    {
        static const char *const lb[2] = {"Cancel", "Calibrate"};
        static const uint16_t col[2] = {TFT_LIGHTGREY, TFT_GREEN};
        if (waitButtons(lb, col, 2) == 0)
            return false;
    }

    static const int tx[3] = {20, 160, 300}, ty[3] = {20, 120, 220};
    float rx[9], ry[9], sx[9], sy[9];
    float oldErr = 0, oldMax = 0;
    int nOld = 0;
    for (int i = 0; i < 9; i++)
    {
        const int x = tx[i % 3], y = ty[i / 3];
        tft.fillScreen(TFT_BLACK);
        tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
        tft.setTextDatum(MC_DATUM);
        char b[24];
        snprintf(b, sizeof b, "Target %d of 9", i + 1);
        tft.drawString(b, 160, y == 120 ? 80 : 120);
        tft.setTextDatum(TL_DATUM);
        crosshair(x, y, TFT_CYAN);
        uint16_t r1, r2;
        while (!sampleRaw(r1, r2))
            crosshair(x, y, TFT_ORANGE); // too short a press: again
        crosshair(x, y, TFT_GREEN);
        rx[i] = r1, ry[i] = r2, sx[i] = x, sy[i] = y;
        // error of the mapping in use before, for the comparison
        uint16_t ox, oy;
        if (mapActive(r1, r2, &ox, &oy))
        {
            const float e = hypotf(ox - x, oy - y);
            oldErr += e, oldMax = std::max(oldMax, e), nOld++;
        }
        delay(250);
    }
    AffCal k;
    const bool ok = fit(rx, ry, sx, sy, 9, k);
    float err = 0, emax = 0;
    if (ok)
        for (int i = 0; i < 9; i++)
        {
            float fx, fy;
            mapF(k, rx[i], ry[i], fx, fy);
            const float e = hypotf(fx - sx[i], fy - sy[i]);
            err += e / 9, emax = std::max(emax, e);
        }
    if (nOld)
        oldErr /= nOld;

    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 8);
    tft.setTextFont(2);
    const bool good = ok && emax < 12;
    if (good)
    {
        s_cal = k;
        Preferences p;
        p.begin("my-app", false);
        p.putBytes("touch_aff", &s_cal, sizeof s_cal);
        p.end();
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        tft.println("  Calibration done - saved");
    }
    else
    {
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.println("  Not saved: points disagree");
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.println("  (a slipped press?) Old one kept.");
    }
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.println("\n  Error at the 9 targets:");
    if (nOld)
        tft.printf("  Before:  avg %4.1f px  worst %4.1f px\n", oldErr, oldMax);
    if (ok)
        tft.printf("  New fit: avg %4.1f px  worst %4.1f px\n", err, emax);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    if (good)
    {
        tft.println("\n  Used now and after restart.");
        if (nOld && oldMax > 2 * emax)
            tft.println("  Touch now follows your finger.");
    }
    {
        static const char *const lb[1] = {"Close"};
        static const uint16_t col[1] = {TFT_GREEN};
        waitButtons(lb, col, 1);
    }
    return good;
}
