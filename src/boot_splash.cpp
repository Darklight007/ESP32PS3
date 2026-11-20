#include "scpi_parser.h" // splashShow(): answer queries while shown
#include "psu_link.h"
// Boot splash: drawn straight with TFT_eSPI right after the display starts
// (LVGL isn't running yet); LVGL's first frame paints over it. FANOOS 3206:
// "lantern", built from the names of girls executed in Iran's prisons in the
// 1980s, in letter order FA + N + O(O) + S: FAtemeh (Mesbah, Sajedi,
// Jabbarzadeh Ansari, reported 13), Nasrin (Nouri Mani, reported 15), mOna
// (Mahmudnizhad, 17, Shiraz 1983) and Shahla (Ghorbani, reported 13), and
// every girl whose name was never recorded (README: "The name"). Night sky with
// Orion (the three belt stars in a line), Damavand over a dark sea, a
// fishing boat, the local weather (weather.cpp) and a progress bar that
// setup() advances. The still scene is drawn once into a PSRAM sprite; the
// moving things restore what is behind them from it.
#include <Arduino.h>
#include "weather.h"
#include "TFT_eSPI.h"
#include "version.h"
#include "boot_splash.h"
#include "Keypad_MC17.h"
#include "buzzer.h"
#include "esp_task_wdt.h"
extern Keypad_MC17 kpd;

extern TFT_eSPI tft;

static uint16_t mix(uint32_t a, uint32_t b, float t)
{
    t = std::max(0.0f, std::min(1.0f, t));
    auto ch = [&](int sh) {
        const int ca = (a >> sh) & 255, cb = (b >> sh) & 255; // signed: a fade to darker must not wrap
        return std::max(0, std::min(255, (int)lroundf(ca + t * (cb - ca))));
    };
    const int r = ch(16), g = ch(8), bl = ch(0);
    return tft.color565(r, g, bl);
}

static constexpr int kHorizon = 132;
static constexpr int kBarX = 40, kBarY = 226, kBarW = 240;
static constexpr uint32_t kFloor = 0x01070F; // ocean colour near the bottom (bar strip)

// Sky colour at row y: black, then deep navy, a dark teal only near the horizon
static uint32_t skyRgb(int y)
{
    const float t = (float)y / kHorizon;
    auto lerp = [](uint32_t a, uint32_t b, float t) {
        t = std::max(0.0f, std::min(1.0f, t));
        uint32_t r = 0;
        for (int sh = 0; sh <= 16; sh += 8)
            r |= (uint32_t)(((a >> sh) & 255) + t * ((int)((b >> sh) & 255) - (int)((a >> sh) & 255))) << sh;
        return r;
    };
    return t < 0.75f ? lerp(0x000000, 0x000106, t / 0.75f) : lerp(0x000106, 0x01040B, (t - 0.75f) / 0.25f); // near black, faint at the horizon
}

// Orion's nebulae, faint and added to the sky (long-exposure look): Barnard's
// Loop (red arc around the left side), the Lambda Orionis bubble around the
// head, M42 (pink, blue-white core), the Flame near Alnitak, Betelgeuse and
// Rigel glows, the Witch Head (blue wisp right of Rigel). Orion's box: x 222-306, y 66-128.
static float hnoise(float x, float y, uint32_t seed) // 2-D value noise, 0..1
{
    auto h = [seed](int i, int j) {
        uint32_t v = (uint32_t)i * 374761393u + (uint32_t)j * 668265263u + seed * 2246822519u;
        v = (v ^ (v >> 13)) * 1274126177u;
        return ((v ^ (v >> 16)) & 1023) / 1023.0f;
    };
    const int i = (int)floorf(x), j = (int)floorf(y);
    float fx = x - i, fy = y - j;
    fx = fx * fx * (3 - 2 * fx);
    fy = fy * fy * (3 - 2 * fy);
    const float a = h(i, j) + fx * (h(i + 1, j) - h(i, j));
    const float b = h(i, j + 1) + fx * (h(i + 1, j + 1) - h(i, j + 1));
    return a + fy * (b - a);
}
static float fbm(float x, float y, uint32_t seed)
{
    return 0.55f * hnoise(x, y, seed) + 0.3f * hnoise(x * 2.1f, y * 2.1f, seed + 7) + 0.15f * hnoise(x * 4.3f, y * 4.3f, seed + 13);
}
static void nebula(int x, int y, float &r, float &g, float &b)
{
    r = g = b = 0;
    if (x < 180 || y > 140)
        return;
    auto gauss = [](float d2, float s) { return expf(-d2 / (2 * s * s)); };
    const float n = fbm(x * 0.09f, y * 0.09f, 3), n2 = fbm(x * 0.21f, y * 0.21f, 9);
    // Barnard's Loop: an elliptic band, strongest on the left, broken by noise
    {
        const float dx = (x - 266) / 44.0f, dy = (y - 104) / 46.0f;
        const float rr = sqrtf(dx * dx + dy * dy);
        const float side = std::max(0.0f, std::min(1.0f, (0.35f - dx / (rr + 1e-3f)) * 1.4f));
        const float band = gauss((rr - 1) * (rr - 1), 0.07f + 0.05f * n);
        const float a = band * side * (0.35f + 1.1f * n2);
        r += 70 * a, g += 8 * a, b += 22 * a;
        const float in = (rr < 1 ? 1.0f : 0.0f) * side * 0.18f * n; // faint red haze inside
        r += 30 * in, g += 6 * in, b += 8 * in;
    }
    // Lambda Orionis: a red bubble round Meissa, rim a little brighter
    {
        const float d = sqrtf((float)((x - 259) * (x - 259) + (y - 63) * (y - 63))) / 13.0f;
        const float a = (d < 1.15f ? 0.35f + 0.5f * gauss((d - 0.95f) * (d - 0.95f), 0.12f) : gauss((d - 1.15f) * (d - 1.15f), 0.08f) * 0.6f) * (0.4f + 0.9f * n);
        r += 55 * a, g += 7 * a, b += 16 * a;
    }
    // M42: pink cloud, blue-white core
    {
        const float d2 = (x - 261) * (x - 261) + (y - 111) * (y - 111) * 0.8f;
        const float a = gauss(d2, 4.5f) * (0.6f + 0.6f * n2);
        r += 90 * a, g += 25 * a, b += 70 * a;
        const float c = gauss(d2, 1.4f);
        r += 40 * c, g += 55 * c, b += 80 * c;
    }
    // Flame / Horsehead region below Alnitak
    {
        const float a = gauss((x - 249) * (x - 249) + (y - 104) * (y - 104), 3.0f) * (0.5f + n2);
        r += 45 * a, g += 12 * a, b += 6 * a;
    }
    // star glows
    {
        const float a = gauss((x - 232) * (x - 232) + (y - 73) * (y - 73), 6.0f);
        r += 40 * a, g += 18 * a, b += 4 * a; // Betelgeuse
        const float c = gauss((x - 297) * (x - 297) + (y - 119) * (y - 119), 7.0f);
        r += 8 * c, g += 18 * c, b += 45 * c; // Rigel
    }
    // Witch Head: a thin blue wisp down-right of Rigel
    {
        const float t = std::max(0.0f, std::min(1.0f, ((x - 302) * 0.6f + (y - 117) * 0.8f) / 18.0f));
        const float px = 302 + t * 18 * 0.6f, py = 117 + t * 18 * 0.8f;
        const float a = gauss((x - px) * (x - px) + (y - py) * (y - py), 1.6f) * (1 - t * 0.6f) * (0.4f + n2);
        r += 6 * a, g += 16 * a, b += 38 * a;
    }
    // dark dust lanes cut through it all
    const float dust = std::max(0.0f, fbm(x * 0.14f + 5, y * 0.14f, 21) - 0.55f) * 2.2f;
    const float k = std::max(0.0f, 1 - dust);
    r *= k, g *= k, b *= k;
}
// ─── Daylight ───────────────────────────────────────────────────────────────
// The real sun (weather.cpp: clock + place) sets the scene: night (as it
// was), twilight (indigo to orange, the sun on the horizon) or day (blue sky,
// sunlit sea and snow). Weights from the sun's altitude; the sun's azimuth
// puts it on screen, looking toward the equator (east left in the north).
static WxKind s_wx = WX_CLEAR;
struct Light
{
    float night = 1, twi = 0, day = 0, alt = -30, sx = -999, sy = 999, over = 0, low = 1;
    bool known = false;
    float farR, farG, farB, nearR, nearG, nearB, mirror; // sea colours
};
static Light s_L;
static float smooth01(float e0, float e1, float v)
{
    const float t = std::max(0.0f, std::min(1.0f, (v - e0) / (e1 - e0)));
    return t * t * (3 - 2 * t);
}
static uint32_t lerpC(uint32_t a, uint32_t b, float t)
{
    t = std::max(0.0f, std::min(1.0f, t));
    uint32_t r = 0;
    for (int sh = 0; sh <= 16; sh += 8)
        r |= (uint32_t)lroundf(((a >> sh) & 255) + t * ((int)((b >> sh) & 255) - (int)((a >> sh) & 255))) << sh;
    return r;
}
// a colour for night / twilight / day, mixed by the light now
static uint32_t pal3(uint32_t n, uint32_t tw, uint32_t d)
{
    const Light &L = s_L;
    float r = 0, g = 0, b = 0;
    auto acc = [&](uint32_t c, float w) { r += w * ((c >> 16) & 255), g += w * ((c >> 8) & 255), b += w * (c & 255); };
    acc(n, L.night), acc(tw, L.twi), acc(d, L.day);
    return (uint32_t)std::min(255L, lroundf(r)) << 16 | (uint32_t)std::min(255L, lroundf(g)) << 8 | (uint32_t)std::min(255L, lroundf(b));
}
static bool isDay() { return s_L.day + 0.5f * s_L.twi > 0.55f; }
static float starVis() { return s_L.known ? s_L.night : 1.0f; }

// Sky colour of row y without nebula and sun glow (floats 0..255)
static void skyRowBase(int y, float &r, float &g, float &b)
{
    const Light &L = s_L;
    r = g = b = 0;
    const float t = std::max(0.0f, std::min(1.0f, (float)y / kHorizon));
    auto acc = [&](uint32_t c, float w) { r += w * ((c >> 16) & 255), g += w * ((c >> 8) & 255), b += w * (c & 255); };
    if (L.night > 0.001f)
    {
        const uint32_t s0 = skyRgb(y), s1 = skyRgb(y + 1); // integer per row: smooth with the next
        r += L.night * (((s0 >> 16) & 255) + ((s1 >> 16) & 255)) / 2.0f;
        g += L.night * (((s0 >> 8) & 255) + ((s1 >> 8) & 255)) / 2.0f;
        b += L.night * ((s0 & 255) + (s1 & 255)) / 2.0f;
    }
    const bool storm = s_wx == WX_THUNDER, snow = s_wx == WX_SNOW;
    if (L.twi > 0.001f) // indigo above, mauve, orange at the horizon
    {
        uint32_t c = t < 0.6f ? lerpC(0x161E48, 0x6A4E80, t / 0.6f) : lerpC(0x6A4E80, 0xEE9050, (t - 0.6f) / 0.4f);
        c = lerpC(c, lerpC(0x2A2834, 0x5A4C56, t), L.over * 0.85f);
        acc(c, L.twi);
    }
    if (L.day > 0.001f) // deep blue overhead, pale at the horizon; grey when overcast
    {
        uint32_t c = lerpC(0x2A62C2, 0xB0D2EE, powf(t, 1.6f));
        const uint32_t grey = storm ? lerpC(0x4C545E, 0x707882, t) : snow ? lerpC(0xA4ACB6, 0xCED4DA, t) : lerpC(0x828C98, 0xAAB2BC, t);
        c = lerpC(c, grey, L.over);
        acc(c, L.day);
    }
}
// Sky colour at a pixel, floats 0..255 (+ nebula at night, sun glow by day)
static void skyF(int x, int y, float &r, float &g, float &b)
{
    const Light &L = s_L;
    skyRowBase(y, r, g, b);
    if (L.night > 0.001f)
    {
        float nr, ng, nb;
        nebula(x, y, nr, ng, nb);
        r += L.night * nr, g += L.night * ng, b += L.night * nb;
    }
    if (L.known && L.twi + L.day > 0.02f) // glow round the sun, and along the horizon at dusk
    {
        const float dx = x - L.sx, dy = y - L.sy, k = 1 - 0.85f * L.over;
        const float halo = expf(-(dx * dx + dy * dy) / (2 * 38.0f * 38.0f)) * (0.45f * L.day + 0.6f * L.twi) * k;
        const uint32_t warm = lerpC(0xFFF0D0, 0xFF9440, L.low);
        r += halo * ((warm >> 16) & 255) * 0.8f, g += halo * ((warm >> 8) & 255) * 0.8f, b += halo * (warm & 255) * 0.8f;
        const float band = expf(-dx * dx / (2 * 110.0f * 110.0f)) * expf(-(kHorizon - y) / 24.0f) * L.twi * k * 0.55f;
        r += band * 255, g += band * 120, b += band * 50;
    }
}
// Sky at a pixel, 0xRRGGBB (stars and their redraws use this)
static uint32_t skyAt(int x, int y)
{
    float r, g, b;
    skyF(x, y, r, g, b);
    auto c = [](float v) { return (uint32_t)std::max(0, std::min(255, (int)v)); };
    return c(r) << 16 | c(g) << 8 | c(b);
}

// Ordered (Bayer 4x4) dither to RGB565: dark gradients have only a few
// 565 levels, which showed as steps; dithering mixes neighbouring levels.
static uint16_t dith(float r, float g, float b, int x, int y)
{
    static const uint8_t B[4][4] = {{0, 8, 2, 10}, {12, 4, 14, 6}, {3, 11, 1, 9}, {15, 7, 13, 5}};
    const float t = (B[y & 3][x & 3] + 0.5f) / 16.0f;
    auto q = [t](float c, int levels) {
        const float v = std::max(0.0f, std::min(255.0f, c)) * levels / 255.0f;
        int n = (int)v;
        if (v - n > t && n < levels)
            n++;
        return n;
    };
    return (q(r, 31) << 11) | (q(g, 63) << 5) | q(b, 31);
}
static void rgbOf(uint32_t c, float &r, float &g, float &b) { r = (c >> 16) & 255, g = (c >> 8) & 255, b = c & 255; }
static uint16_t s_row[320];
static TFT_eSPI *g_fwd();
static TFT_eSprite *s_bgSpr = nullptr; // the still scene (below)
static void pushRow(int y)
{
    TFT_eSPI *t = g_fwd();
    if (t != &tft) // sprite: straight into its buffer (stored byte-swapped, as drawPixel does)
    {
        uint16_t *buf = s_bgSpr ? (uint16_t *)s_bgSpr->getPointer() : nullptr;
        if (buf)
            for (int x = 0; x < 320; x++)
                buf[y * 320 + x] = (uint16_t)((s_row[x] >> 8) | (s_row[x] << 8));
        else
            for (int x = 0; x < 320; x++)
                t->drawPixel(x, y, s_row[x]);
        return;
    }
    const bool sw = tft.getSwapBytes();
    tft.setSwapBytes(true);
    tft.pushImage(0, y, 320, 1, s_row);
    tft.setSwapBytes(sw);
}
static bool s_noBar = false; // About page: the scene only
static bool s_barShown = false;

// Still scene: built in a PSRAM sprite (g points at it), pushed once, kept
// as the source for restoring pixels behind moving things.
static TFT_eSPI *g = &tft;
static TFT_eSPI *g_fwd() { return g; }
static uint16_t oceanPix(int x, int y);
static uint16_t bgPix(int x, int y)
{
    if (x < 0 || x >= 320 || y < 0 || y >= 240)
        return 0;
    if (s_bgSpr)
        return s_bgSpr->readPixel(x, y);
    return y < kHorizon ? mix(skyAt(x, y), 0, 0) : oceanPix(x, y);
}
static void restorePix(int x, int y)
{
    if (x >= 0 && x < 320 && y >= 0 && y < 240)
        tft.drawPixel(x, y, bgPix(x, y));
}
static void restoreRect(int x0, int y0, int w, int h)
{
    for (int y = std::max(0, y0); y < std::min(240, y0 + h); y++)
    {
        const int xa = std::max(0, x0), xb = std::min(320, x0 + w);
        if (xb <= xa)
            continue;
        for (int x = xa; x < xb; x++)
            s_row[x - xa] = bgPix(x, y);
        const bool sw = tft.getSwapBytes();
        tft.setSwapBytes(true);
        tft.pushImage(xa, y, xb - xa, 1, s_row);
        tft.setSwapBytes(sw);
    }
}
static void rgb565(uint16_t v, float &r, float &gg, float &b)
{
    r = ((v >> 11) & 31) * 255.0f / 31, gg = ((v >> 5) & 63) * 255.0f / 63, b = (v & 31) * 255.0f / 31;
}
// blend the scene pixel toward an RGB colour (building only)
static void blendPix(int x, int y, uint32_t to, float a)
{
    if (!s_bgSpr || a <= 0.01f)
        return;
    float r, gg, b, tr, tg, tb;
    rgb565(s_bgSpr->readPixel(x, y), r, gg, b);
    rgbOf(to, tr, tg, tb);
    a = std::min(1.0f, a);
    s_bgSpr->drawPixel(x, y, dith(r + a * (tr - r), gg + a * (tg - gg), b + a * (tb - b), x, y));
}

// A star: bright core, soft halo of a fixed radius (so a dimmer redraw
// covers the brighter one)
static void star(int x, int y, int halo, float bright, uint32_t tint)
{
    const uint32_t bg = skyAt(x, y);
    for (int r = halo; r >= 1; r--)
        g->fillCircle(x, y, r, mix(bg, tint, bright * (1.0f - (float)r / (halo + 1)) * 0.8f));
    g->drawPixel(x, y, mix(bg, 0xF0F4FF, std::min(1.0f, bright * 1.1f)));
    if (halo >= 4) // diffraction spikes on the brightest
    {
        const uint16_t c = mix(bg, tint, 0.5f * bright);
        g->drawFastHLine(x - 7, y, 6, c);
        g->drawFastHLine(x + 2, y, 6, c);
        g->drawFastVLine(x, y - 7, 6, c);
        g->drawFastVLine(x, y + 2, 6, c);
    }
}

// Shooting star lane (upper right, clear of the title and Orion) and the UFO
static float kSx0 = 312, kSy0 = 6, kSx1 = 236, kSy1 = 46; // re-rolled for each streak
static bool nearShoot(int x, int y)
{
    for (int k = 0; k <= 30; k++)
    {
        const float t = k / 30.0f;
        if (fabsf(kSx0 + t * (kSx1 - kSx0) - x) < 4 && fabsf(kSy0 + t * (kSy1 - kSy0) - y) < 4)
            return true;
    }
    return false;
}
// UFO: one fast pass (~0.5 s) in the clear sky above the title, then
// angling down the right side; the sky is restored behind it
static int ufoY(int x) { return x < 220 ? 3 : 3 + (x - 220) / 4; }
static bool nearUfo(int x, int y) { return abs(y - ufoY(x)) < 5; }
static void ufoErase(int x)
{
    restoreRect(x - 6, ufoY(x) - 3, 13, 7);
}
static void ufoDraw(int x, bool blink)
{
    const int y = ufoY(x);
    tft.drawFastHLine(x - 3, y, 7, tft.color565(40, 70, 120));   // hull
    tft.drawFastHLine(x - 2, y + 1, 5, tft.color565(22, 40, 70));
    tft.drawFastHLine(x - 1, y - 1, 3, tft.color565(30, 130, 150)); // dome
    tft.drawPixel(x + (blink ? 2 : -2), y + 1, blink ? 0x07E0 : 0x041F); // light
}

// Twinkling: a task redraws a few stars at a time while setup() runs; one
// mutex keeps it and splashProgress() off the SPI bus at the same moment.
struct Star { int16_t x, y; uint8_t halo; float b; uint32_t tint; };
static constexpr int kBg = 110;
static Star s_bg[kBg];
static Star s_orion[9];
static SemaphoreHandle_t s_lock = nullptr;
static TaskHandle_t s_task = nullptr;
static volatile bool s_run = false;
static uint32_t s_showMs = 0;
static bool s_ufoOn = false;

static int ridgeY(int x);
static uint16_t oceanPix(int x, int y);

// Fishing boat drifting across the sea, a warm lamp on board
static constexpr int kBoatY = 150; // waterline: far out, near the horizon
static int s_bx = -1000, s_by = 0; // boat now (rain/snow keep off it)
static void boatErase(int bx, int by) { restoreRect(bx - 9, by - 12, 20, 21); }
static void boatDraw(int bx, int by, int flick)
{
    if (isDay()) // by day: red hull, white gunwale and wheelhouse, lamps off
    {
        const uint16_t hull = mix(0x7A2018, 0, 0), white = mix(0xE4E8EC, 0, 0), dark = mix(0x2A3038, 0, 0);
        tft.drawFastHLine(bx - 7, by - 2, 15, white);
        tft.drawFastHLine(bx + 7, by - 3, 2, white);
        tft.drawFastHLine(bx - 7, by - 1, 15, hull);
        tft.drawFastHLine(bx - 5, by, 12, hull);
        tft.fillRect(bx - 5, by - 5, 4, 3, white);
        tft.drawFastHLine(bx - 6, by - 6, 6, dark);
        tft.drawPixel(bx - 4, by - 4, mix(0x283848, 0, 0));
        tft.drawFastVLine(bx + 2, by - 11, 9, dark);
        tft.drawLine(bx + 2, by - 10, bx + 8, by - 3, mix(0x606870, 0, 0));
        tft.drawLine(bx + 2, by - 10, bx - 6, by - 3, mix(0x606870, 0, 0));
        tft.drawFastHLine(bx - 4, by + 1, 10, mix(lerpC(pal3(0, 0, 0x1C4F80), 0, 0.3f), 0, 0)); // its shadow on the water
        return;
    }
    const uint16_t hull = 0x0000, rim = mix(0x10243C, 0, 0), rig = mix(0x0A1828, 0, 0);
    // lamp reflection: short broken warm dashes
    for (int k = 0; k < 3; k++)
    {
        const int y = by + 2 + k * 2, w = 1 + ((k + flick) % 2);
        tft.drawFastHLine(bx - 3 + ((k * 5 + flick) % 3) - 1, y, w, mix(0x020C1C, 0xC07020, 0.5f - 0.13f * k));
    }
    // hull, bow raised to the right
    tft.drawFastHLine(bx - 7, by - 2, 15, rim);
    tft.drawFastHLine(bx + 7, by - 3, 2, rim);
    tft.drawFastHLine(bx - 7, by - 1, 15, hull);
    tft.drawFastHLine(bx - 5, by, 12, hull);
    // wheelhouse with a lit window
    tft.fillRect(bx - 5, by - 5, 4, 3, hull);
    tft.drawFastHLine(bx - 6, by - 6, 6, rim);
    tft.drawPixel(bx - 4, by - 4, mix(0x000000, 0xFFA040, 0.7f + 0.05f * flick));
    // mast, rigging, masthead light
    tft.drawFastVLine(bx + 2, by - 11, 9, rig);
    tft.drawLine(bx + 2, by - 10, bx + 8, by - 3, rig);
    tft.drawLine(bx + 2, by - 10, bx - 6, by - 3, rig);
    tft.drawPixel(bx + 2, by - 12, flick & 1 ? mix(0, 0xFFD070, 0.85f) : mix(0, 0xFFB050, 0.55f));
}

// Wave crests: short light dashes drifting slowly
struct Wave { float x; int16_t y, len; float v; };
static constexpr int kWaves = 34;
static uint32_t s_waveLo = 0x03183A, s_waveHi = 0x3A70A8; // crest colours (light)
static Wave s_wave[kWaves];
static int16_t s_waveDrawn[kWaves];
static bool inBoatBox(int x0, int x1, int y, int bx, int by)
{
    return y >= by - 12 && y <= by + 8 && x1 >= bx - 9 && x0 <= bx + 10;
}

// ─── Weather ────────────────────────────────────────────────────────────────
static bool s_shootOk = true;
struct Blob { int16_t cx, cy, rx, ry; float w; };
static constexpr int kMaxBlobs = 22;
static Blob s_blob[kMaxBlobs];
static int s_nBlob = 0;
static float cloudA(int x, int y)
{
    float a = 0;
    for (int i = 0; i < s_nBlob; i++)
    {
        const Blob &b = s_blob[i];
        const float dx = (float)(x - b.cx) / b.rx, dy = (float)(y - b.cy) / b.ry;
        const float d2 = dx * dx + dy * dy;
        if (d2 < 1)
        {
            const float f = 1 - d2;
            a += b.w * f * sqrtf(f);
        }
    }
    return std::min(1.0f, a);
}
static void cloudsSetup(WxKind k)
{
    s_nBlob = 0;
    auto add = [](int cx, int cy, int rx, int ry, float w) {
        if (s_nBlob < kMaxBlobs)
            s_blob[s_nBlob++] = {(int16_t)cx, (int16_t)cy, (int16_t)rx, (int16_t)ry, w};
    };
    if (k == WX_CLOUD) // a few banks, Orion mostly clear
    {
        add(40, 26, 44, 11, 0.9f), add(84, 18, 38, 9, 0.8f), add(112, 30, 30, 8, 0.6f);
        add(140, 104, 44, 7, 0.75f), add(186, 98, 32, 6, 0.6f), add(104, 110, 26, 5, 0.5f);
        add(304, 34, 30, 7, 0.55f), add(270, 18, 22, 5, 0.4f);
    }
    else if (k == WX_RAIN || k == WX_THUNDER || k == WX_SNOW) // overcast
    {
        uint32_t sd = k == WX_SNOW ? 0x5A0C : 0xA1A;
        auto rn = [&]() { sd = sd * 1664525u + 1013904223u; return sd >> 8; };
        const int n = k == WX_SNOW ? 14 : 20;
        for (int i = 0; i < n;)
        {
            const int cx = rn() % 340 - 10, cy = 4 + rn() % 110, rx = 34 + rn() % 34, ry = 8 + rn() % 9;
            const float w = 0.55f + (rn() % 40) / 100.0f;
            if (cx + rx * 0.6f > 222 && cx - rx * 0.6f < 306 && cy + ry > 60 && cy - ry < 130)
                continue; // keep a gap around Orion
            add(cx, cy, rx, ry, w), i++;
        }
    }
    // a thin veil over Orion: long, flat wisps it shows through
    if (k == WX_CLOUD || k == WX_RAIN || k == WX_THUNDER)
    {
        const float f = k == WX_CLOUD ? 1.0f : 1.3f;
        add(252, 82, 38, 4, 0.30f * f), add(288, 104, 30, 3, 0.26f * f);
        add(238, 116, 28, 3, 0.24f * f), add(274, 64, 24, 3, 0.20f * f);
    }
}
// Clouds on the scene: dark, a faint lit rim on their tops
static void cloudsDraw()
{
    if (!s_nBlob || !s_bgSpr)
        return;
    const bool cl = s_wx == WX_CLOUD, storm = s_wx == WX_THUNDER;
    const uint32_t core = pal3(cl ? 0x0E1626 : 0x0C1220, cl ? 0x3A3048 : 0x2E2A36, cl ? 0xC2CCD8 : storm ? 0x4A525C : 0x707A86);
    const uint32_t rim = pal3(cl ? 0x2E4262 : 0x223048, cl ? 0xF2A070 : 0x8A6A68, cl ? 0xFFFFFF : storm ? 0x7A828C : 0xA6AEB8);
    float cr, cg, cb, rr, rg, rb;
    rgbOf(core, cr, cg, cb);
    rgbOf(rim, rr, rg, rb);
    // alpha of each row once; the row 4 above (for the lit top edge) from a ring
    float *ring = (float *)heap_caps_malloc(5 * 320 * sizeof(float), MALLOC_CAP_SPIRAM);
    if (!ring)
        return;
    for (int y = -4; y < kHorizon; y++)
    {
        float *row = ring + ((y + 5) % 5) * 320;
        for (int x = 0; x < 320; x++)
            row[x] = cloudA(x, y);
        if (y < 0)
            continue;
        const float *above = ring + ((y + 1) % 5) * 320; // row y-4
        for (int x = 0; x < 320; x++)
        {
            const float a = row[x];
            if (a < 0.02f)
                continue;
            const float lit = std::max(0.0f, std::min(1.0f, (a - above[x]) * 3.0f)); // top edge
            const uint32_t c = ((uint32_t)(cr + lit * (rr - cr)) << 16) | ((uint32_t)(cg + lit * (rg - cg)) << 8) | (uint32_t)(cb + lit * (rb - cb));
            blendPix(x, y, c, a * 0.95f);
        }
    }
    free(ring);
}
static void fogDraw()
{
    if (s_wx != WX_FOG || !s_bgSpr)
        return;
    const uint32_t fogC = pal3(0x1C2A3C, 0x6A5A68, 0xC8D0D8);
    for (int y = kHorizon - 46; y < 214; y++)
        for (int x = 0; x < 320; x++)
        {
            const float d = (y - kHorizon) / (y < kHorizon ? 20.0f : 34.0f);
            const float drift = 0.75f + 0.25f * sinf(x * 0.035f + y * 0.11f) * sinf(x * 0.011f - y * 0.05f);
            blendPix(x, y, fogC, 0.85f * expf(-d * d) * drift);
        }
}

// Rain, snow, lightning: drawn over the screen, erased from the scene sprite
// Rain: three depth layers of thin, faint motion-blurred streaks (anti-aliased
// over two columns, tapered at both ends), slanted by a gusting wind and
// thinned in passing curtains. Far drops ring the sea near the horizon,
// nearer ones further down, the nearest fall past the camera.
struct Drop { float x, y, vy, lx, ly, ls, a; int16_t yEnd; uint8_t len, layer, dark; };
static constexpr int kDrops = 200;
static Drop *s_drop = nullptr; // PSRAM
static float s_wind = -0.2f;   // streak slope dx/dy now
static uint32_t s_wxT = 0;     // weather frame count
static bool skipPix(int x, int y)
{
    if (x < 0 || x >= 320 || y < 0 || y >= 240)
        return true;
    if (s_barShown && y >= kBarY - 15)
        return true;
    return s_bx > -1000 && x >= s_bx - 9 && x < s_bx + 11 && y >= s_by - 12 && y <= s_by + 8;
}
static void putPix(int x, int y, uint32_t to, float a)
{
    if (skipPix(x, y))
        return;
    float r, gg, b, tr, tg, tb;
    rgb565(bgPix(x, y), r, gg, b);
    rgbOf(to, tr, tg, tb);
    tft.drawPixel(x, y, tft.color565(r + a * (tr - r), gg + a * (tg - gg), b + a * (tb - b)));
}
static void unPix(int x, int y)
{
    if (!skipPix(x, y))
        tft.drawPixel(x, y, bgPix(x, y));
}
// a rain pixel: lighter than a dark background, darker than a bright one
// (and darker for the "shadow" streaks)
static void rainPix(int x, int y, float a, bool dark)
{
    if (a < 0.012f || skipPix(x, y))
        return;
    float r, gg, b;
    rgb565(bgPix(x, y), r, gg, b);
    const float lum = 0.3f * r + 0.59f * gg + 0.11f * b;
    float tr, tg, tb;
    if (dark || lum > 120)
        tr = r * 0.45f, tg = gg * 0.47f, tb = b * 0.52f;
    else
        tr = 165, tg = 180, tb = 204;
    tft.drawPixel(x, y, tft.color565(r + a * (tr - r), gg + a * (tg - gg), b + a * (tb - b)));
}
static void streak(const Drop &d, float x, float y, float slope, bool draw, float curtain)
{
    const int n = d.len, y0 = (int)y;
    for (int i = 0; i < n; i++)
    {
        const float px = x - slope * i;
        const int ix = (int)floorf(px), py = y0 - i;
        if (!draw)
        {
            unPix(ix, py), unPix(ix + 1, py);
            continue;
        }
        const float u = (i + 0.5f) / n; // head 0 .. tail 1
        const float f = px - ix, a = d.a * curtain * sinf(3.14159f * (0.15f + 0.85f * u));
        rainPix(ix, py, a * (1 - f), d.dark), rainPix(ix + 1, py, a * f, d.dark);
    }
}
static float rainCurtain(float x)
{
    const float t = (float)s_wxT;
    return 0.5f + 0.5f * sinf(x * 0.021f - t * 0.045f + 0.8f * sinf(t * 0.013f + x * 0.007f));
}
template <typename R> static void dropSpawn(Drop &d, R &rnd, bool first)
{
    const bool snow = s_wx == WX_SNOW;
    auto fr = [&]() { return (rnd() % 1000) / 1000.0f; };
    d.lx = -1000;
    if (snow)
    {
        d.x = rnd() % 360 - 20;
        d.y = first ? (float)(rnd() % 240) : -(float)(rnd() % 40);
        d.vy = 0.5f + (rnd() % 60) / 100.0f;
        d.len = 1 + rnd() % 3; // 1 dot, 2 wide, 2x2
        d.yEnd = 240, d.layer = 0, d.dark = 0, d.a = 1;
        return;
    }
    const int r = rnd() % 100;
    d.layer = r < 45 ? 0 : r < 83 ? 1 : 2;
    switch (d.layer)
    {
    case 0: // far: short, slow, faint, rings near the horizon
        d.vy = 5.5f + 1.5f * fr(), d.len = 3 + rnd() % 3, d.a = 0.10f + 0.08f * fr();
        d.yEnd = kHorizon + 2 + rnd() % 13;
        break;
    case 1:
        d.vy = 9 + 3 * fr(), d.len = 7 + rnd() % 5, d.a = 0.13f + 0.09f * fr();
        d.yEnd = kHorizon + 14 + rnd() % 50;
        break;
    default: // near the lens: long, quick, barely there, off the bottom
        d.vy = 17 + 7 * fr(), d.len = 16 + rnd() % 11, d.a = 0.06f + 0.07f * fr();
        d.yEnd = 270;
        break;
    }
    d.dark = rnd() % 100 < (d.layer == 2 ? 30 : 12);
    d.x = -10 + fr() * 420; // the wind carries them left
    d.y = first ? fr() * 240 : -(float)(rnd() % 50);
}
static void dropErase(const Drop &d)
{
    if (d.lx <= -1000)
        return;
    if (s_wx == WX_SNOW)
    {
        const int lx = (int)d.lx, ly = (int)d.ly;
        unPix(lx, ly);
        if (d.len >= 2)
            unPix(lx + 1, ly);
        if (d.len == 3)
            unPix(lx, ly + 1), unPix(lx + 1, ly + 1);
        return;
    }
    streak(d, d.lx, d.ly, d.ls, false, 0);
}
static void rainHit(float x, int y, bool droplet); // after the water physics
template <typename R> static void weatherFrame(R &rnd)
{
    const uint32_t t = ++s_wxT;
    if ((s_wx == WX_RAIN || s_wx == WX_THUNDER || s_wx == WX_SNOW) && s_drop)
    {
        const bool snow = s_wx == WX_SNOW, storm = s_wx == WX_THUNDER;
        const int active = snow ? 90 : storm ? kDrops : 150;
        // gusting wind: slant swings slowly, with quicker flurries
        s_wind = (storm ? -0.30f : -0.18f) - 0.09f * sinf(t * 0.011f) - 0.04f * sinf(t * 0.047f + 1.3f);
        for (int k = 0; k < active; k++) // erase all first: no drop cuts another
            dropErase(s_drop[k]);
        for (int k = 0; k < active; k++)
        {
            Drop &d = s_drop[k];
            d.lx = -1000;
            if (snow)
            {
                d.y += d.vy;
                d.x += 0.35f * sinf(t * 0.07f + k);
                if (d.y >= d.yEnd)
                {
                    dropSpawn(d, rnd, false);
                    continue;
                }
                const int lx = (int)d.x, ly = (int)d.y;
                d.lx = lx, d.ly = ly;
                const float a = 0.6f + 0.12f * d.len;
                const uint32_t fc = isDay() ? 0xFFFFFF : 0xC8D2E2;
                putPix(lx, ly, fc, a);
                if (d.len >= 2)
                    putPix(lx + 1, ly, fc, a * 0.8f);
                if (d.len == 3)
                    putPix(lx, ly + 1, fc, a * 0.8f), putPix(lx + 1, ly + 1, fc, a * 0.6f);
                continue;
            }
            const float slope = s_wind * (0.85f + 0.12f * d.layer);
            d.y += d.vy;
            d.x += slope * d.vy;
            if (d.y >= d.yEnd)
            {
                if (d.layer < 2 && d.x >= 0 && d.x < 320) // a tiny ring, now and then a rebound droplet
                    rainHit(d.x, d.yEnd, d.layer == 1 && rnd() % 6 == 0);
                dropSpawn(d, rnd, false);
                continue;
            }
            if (d.x < -40 || d.x > 440)
            {
                dropSpawn(d, rnd, false);
                continue;
            }
            d.lx = d.x, d.ly = d.y, d.ls = slope;
            streak(d, d.x, d.y, slope, true, 0.35f + 0.65f * rainCurtain(d.x));
        }
    }
    if (s_wx == WX_THUNDER) // a bolt every few seconds: on, on, off, on, gone
    {
        static int16_t bx[24], by[24];
        static int n = 0, phase = -1, wait = 40;
        if (phase < 0 && --wait <= 0)
        {
            int x = 40 + rnd() % 240, y = 14 + rnd() % 20;
            n = 0;
            while (n < 24 && y < kHorizon - 4 && y < ridgeY(std::max(0, std::min(319, x))) - 1)
            {
                bx[n] = x, by[n] = y, n++;
                x += (int)(rnd() % 13) - 6;
                y += 5 + rnd() % 6;
            }
            phase = 0;
        }
        if (phase >= 0)
        {
            auto bolt = [&](bool on) {
                for (int i = 1; i < n; i++)
                {
                    const int x0 = bx[i - 1], y0 = by[i - 1], x1 = bx[i], y1 = by[i];
                    const int m = std::max(1, std::max(abs(x1 - x0), abs(y1 - y0)));
                    for (int j = 0; j <= m; j++)
                    {
                        const int px = x0 + (x1 - x0) * j / m, py = y0 + (y1 - y0) * j / m;
                        if (on)
                        {
                            putPix(px - 1, py, 0x3050A0, 0.5f), putPix(px + 1, py, 0x3050A0, 0.5f);
                            putPix(px, py, 0xC8DCFF, 0.9f);
                        }
                        else
                            unPix(px - 1, py), unPix(px, py), unPix(px + 1, py);
                    }
                }
            };
            static const bool seq[] = {true, true, false, true, false};
            bolt(seq[phase]);
            if (++phase >= 5)
                phase = -1, wait = 50 + rnd() % 80;
        }
    }
}

// Sea life and traffic. Now and then something surfaces (dolphins, whale,
// octopus, sea serpent, jellyfish, shark, fish, turtle, manta, seal, gulls,
// flying fish, a kraken's eye), sized by distance (smaller near the
// horizon); ships, tankers, planes and satellites pass. Drawn pixels are
// listed and put back from the scene, so nothing else is touched.
static constexpr int kPixCap = 1600;
struct PixList { int16_t *x = nullptr, *y = nullptr; int n = 0; }; // lists in PSRAM (internal RAM is short)
static PixList s_life, s_traffic;
static PixList *s_pl = &s_life;
static bool s_waterOnly = true;
static float s_sc = 1; // size by distance
static void lifePix(int x, int y, uint32_t c, float a)
{
    if (s_waterOnly && (y <= kHorizon || (y > 150 && (x < 42 || x > 278))))
        return; // sky or the wooded shores
    if (skipPix(x, y) || s_pl->n >= kPixCap)
        return;
    if (!s_pl->x)
    {
        s_pl->x = (int16_t *)heap_caps_malloc(kPixCap * 2 * sizeof(int16_t), MALLOC_CAP_SPIRAM);
        if (!s_pl->x)
            return;
        s_pl->y = s_pl->x + kPixCap;
    }
    putPix(x, y, c, a);
    s_pl->x[s_pl->n] = x, s_pl->y[s_pl->n++] = y;
}
static void lp(float x, float y, uint32_t c, float a) { lifePix(lroundf(x), lroundf(y), c, a); }
static void listErase(PixList &l)
{
    for (int i = 0; i < l.n; i++)
        if (!skipPix(l.x[i], l.y[i]))
            restorePix(l.x[i], l.y[i]);
    l.n = 0;
}
static void dolphin(float x, float y, float ux, float uy, int dir)
{
    const uint32_t body = 0x26364C, rim = 0x7090B8;
    const float s = s_sc, nx = uy * dir, ny = -ux * dir; // "up" side of the body
    const int n = std::max(2, (int)(5 * s));
    for (int i = -n; i <= n; i++)
    {
        const float px = x + i * ux * dir, py = y + i * uy * dir;
        lp(px, py, body, 0.95f);
        if (abs(i) < n - 1 && s > 0.7f)
            lp(px + nx, py + ny, rim, 0.55f), lp(px - nx, py - ny, body, 0.8f);
    }
    lp(x + 2 * s * nx, y + 2 * s * ny, body, 0.9f); // dorsal fin
    const float tx = x - (n + 1) * ux * dir, ty = y - (n + 1) * uy * dir; // flukes
    lp(tx + nx, ty + ny, body, 0.9f), lp(tx - nx, ty - ny, body, 0.9f);
}
static void splash(float x, float y, float k, uint32_t seed)
{
    for (int i = 0; i < 6; i++)
    {
        seed = seed * 1664525u + 1013904223u;
        const int dx = (int)(seed >> 28) - 8, dy = (int)((seed >> 24) & 7);
        lp(x + dx / 2 * s_sc, y - dy * k * s_sc, 0xA8C4E0, 0.7f * k);
    }
}
static void ring(float x, float y, float r, uint32_t c, float a) // a ripple on the water
{
    for (int i = 0; i < 16; i++)
    {
        const float t = i / 16.0f * 6.28318f;
        lp(x + cosf(t) * r, y + sinf(t) * r * 0.3f, c, a);
    }
}
// ---- water physics: droplets under gravity, spreading ripples ----
// Units: px and seconds; s_sc scales with distance (far = small and slow).
struct Particle { float x, y, vx, vy, sy, life, a; uint32_t c; };
struct Ripple { float x, y, r0, rMax, life, age, a; };
static constexpr int kParts = 72, kRips = 96;
static Particle *s_part = nullptr; // PSRAM
static Ripple *s_rip = nullptr;
static uint32_t s_physSeed = 0x9E3779B9;
static float frand() { s_physSeed = s_physSeed * 1664525u + 1013904223u; return (s_physSeed >> 8) / 16777216.0f; }
static bool physInit()
{
    if (!s_part)
        s_part = (Particle *)heap_caps_calloc(kParts, sizeof(Particle), MALLOC_CAP_SPIRAM);
    if (!s_rip)
        s_rip = (Ripple *)heap_caps_calloc(kRips, sizeof(Ripple), MALLOC_CAP_SPIRAM);
    return s_part && s_rip;
}
// a ring that spreads like a capillary wave: fast at first, slowing (r ~ sqrt t), fading
static void rippleAdd(float x, float y, float a, float rMax, float life, bool onlyFree = false)
{
    if (!physInit())
        return;
    if (onlyFree) // rain: never steals a creature's ring
    {
        for (int i = 0; i < kRips; i++)
            if (s_rip[i].age >= s_rip[i].life)
            {
                s_rip[i] = {x, y, 0.4f, rMax, life, 0, a};
                return;
            }
        return;
    }
    int k = 0;
    for (int i = 0; i < kRips; i++)
        if (s_rip[i].age >= s_rip[i].life) { k = i; break; }
        else if (s_rip[i].age / s_rip[i].life > s_rip[k].age / s_rip[k].life) k = i; // replace the oldest
    s_rip[k] = {x, y, 0.6f, rMax, life, 0, a};
}
// droplets thrown up where something breaks the surface; vxBias carries them along
static void splashAdd(float x, float y, float strength, float sc, float vxBias = 0, int n = 8, uint32_t c = 0xB8D0EA, float g = 1)
{
    if (!physInit())
        return;
    for (int i = 0, k = 0; i < n && k < kParts; k++)
        if (s_part[k].life <= 0)
        {
            const float up = (28 + 34 * frand()) * strength * sc;
            s_part[k] = {x + (frand() - 0.5f) * 3 * sc, y - 0.5f, vxBias + (frand() - 0.5f) * 30 * sc * strength, -up,
                         y, 1.5f, 0.75f, c};
            s_part[k].a = g; // gravity factor stored in a for mist (<1 = light)
            i++;
        }
    rippleAdd(x, y + 0.5f, 0.45f * std::min(1.0f, strength), (6 + 8 * strength) * sc, 0.9f + 0.4f * strength);
}
static void physStep(float dt)
{
    if (!s_part || !s_rip)
        return;
    const float G = 160; // px/s^2 at scale 1
    for (int k = 0; k < kParts; k++)
    {
        Particle &q = s_part[k];
        if (q.life <= 0)
            continue;
        const float gk = q.a;
        q.vy += G * gk * s_sc * dt;
        q.vx *= 1 - 0.8f * dt;                  // air drag
        if (gk < 0.5f)
            q.vx += 6 * dt, q.vy *= 1 - 1.5f * dt; // mist: wind and strong drag
        q.x += q.vx * dt, q.y += q.vy * dt;
        q.life -= dt;
        if (q.y >= q.sy && q.vy > 0) // back in the water: a tiny ring
        {
            if (gk >= 0.5f && frand() < 0.35f)
                rippleAdd(q.x, q.sy + 0.5f, 0.25f, 2.5f, 0.4f);
            q.life = 0;
            continue;
        }
        lp(q.x, q.y, q.c, (gk < 0.5f ? 0.45f : 0.7f) * std::min(1.0f, q.life * 3));
    }
    for (int i = 0; i < kRips; i++)
    {
        Ripple &r = s_rip[i];
        if (r.age >= r.life)
            continue;
        r.age += dt;
        const float u = std::min(1.0f, r.age / r.life);
        const float rad = r.r0 + (r.rMax - r.r0) * sqrtf(u);
        const float a = r.a * (1 - u) * (1 - u);
        const int n = std::clamp((int)(rad * 2.2f), 6, 20);
        for (int j = 0; j < n; j++)
        {
            const float t = j / (float)n * 6.28318f;
            lp(r.x + cosf(t) * rad, r.y + sinf(t) * rad * 0.28f, 0x7C9CC4, a);
        }
    }
}
static void rainHit(float x, int y, bool droplet)
{
    const float sc = 0.45f + (y - kHorizon) / 85.0f; // nearer = bigger
    rippleAdd(x, y, 0.16f + 0.1f * sc, 2.2f * sc + 0.8f, 0.3f + 0.15f * sc, true);
    if (!droplet || !s_part)
        return;
    for (int k = 0; k < kParts; k++)
        if (s_part[k].life <= 0)
        {
            s_part[k] = {x, (float)y, (frand() - 0.5f) * 8, -(10 + 8 * frand()) * sc, (float)y, 0.45f, 1, 0x8CA4C4};
            break;
        }
}
// damped float: rises past its rest line, settles (buoyancy + water drag)
static float bobUp(float t, float h, float w = 7.5f, float tau = 0.6f)
{
    return h * (1 - expf(-t / tau) * cosf(w * t));
}

static volatile int s_lifeAsk = -1;         // DISP:SPLASH:LIFE: this creature next
static volatile uint32_t s_frMax = 0, s_frSum = 0, s_frN = 0; // frame period, ms
template <typename R> static void seaLifeFrame(R &rnd)
{
    enum { DOLPHINS, WHALE, OCTOPUS, SERPENT, JELLY, SHARK, FISHES, TURTLE, MANTA, SEAL, GULLS, FLYFISH, KRAKEN, KINDS };
    static const uint32_t durs[KINDS] = {1900, 5200, 4200, 9000, 6000, 11000, 2600, 5000, 1700, 4500, 22000, 2200, 4800};
    static int kind = -1, dir = 1, x0 = 0, y0 = 0;
    static uint32_t t0 = 0, next = 0, dur = 0, showSeen = 0, seed = 1;
    const uint32_t now = millis();
    s_pl = &s_life, s_waterOnly = true;
    if (showSeen != s_showMs)
        showSeen = s_showMs, kind = -1, s_life.n = 0, next = now + 2000 + rnd() % 3000;
    listErase(s_life);
    static uint32_t lastT = 0;
    if (lastT && now - lastT < 1000)
    {
        const uint32_t d = now - lastT;
        s_frSum += d, s_frN++;
        if (d > s_frMax)
            s_frMax = d;
    }
    const float dt = lastT ? std::min(0.15f, (now - lastT) / 1000.0f) : 0.05f;
    lastT = now;
    {
        const float sc = s_sc;
        physStep(dt); // droplets and ripples, also after the creature is gone
        s_sc = sc;
    }
    static float prevP = -1;
    if (s_lifeAsk >= 0 && s_lifeAsk < KINDS)
        kind = -1, next = now;
    if (kind < 0)
    {
        if ((int32_t)(now - next) < 0)
            return;
        prevP = -1;
        kind = rnd() % KINDS;
        if (s_lifeAsk >= 0)
            kind = s_lifeAsk % KINDS, s_lifeAsk = -1;
        dur = durs[kind];
        y0 = kind == SERPENT || kind == KRAKEN ? kHorizon + 8 + rnd() % 26 : kHorizon + 10 + rnd() % 66;
        s_sc = 0.45f + (y0 - kHorizon) / 85.0f; // far = small
        const int span = kind == SERPENT || kind == SHARK || kind == GULLS || kind == FLYFISH ? (int)(45 * s_sc) : (int)(25 * s_sc);
        for (int tries = 0; tries < 10; tries++) // clear of the boat and the shores
        {
            x0 = 50 + span + rnd() % std::max(1, 220 - 2 * span);
            if (abs(x0 - s_bx) > 30 + span)
                break;
        }
        dir = rnd() & 1 ? 1 : -1;
        seed = rnd();
        t0 = now;
    }
    const float p = (now - t0) / (float)dur;
    if (p >= 1)
    {
        kind = -1, next = now + 4000 + rnd() % 8000;
        return;
    }
    s_sc = 0.45f + (y0 - kHorizon) / 85.0f;
    const float s = s_sc, fade = sinf(3.14159f * p), PI_ = 3.14159f;
    const float tS = p * dur / 1000.0f;                       // seconds since it started
    auto crossed = [&](float e) { return prevP < e && p >= e; }; // an event this frame
    struct PrevSet { float &r; float v; ~PrevSet() { r = v; } } prevSet{prevP, p};
    switch (kind)
    {
    case DOLPHINS: // three leaps, each a true projectile: x = vx t, y = vy t - g t^2 / 2
        for (int d = 0; d < 3; d++)
        {
            const float T = 1.05f, tl = tS - d * 0.28f;          // airtime, this dolphin's clock
            const float vx = dir * 40 * s, vy = 46 * s, g = 2 * vy / T; // apex vy^2/2g ~ 12 s px
            const float bx0 = x0 - d * 11 * s * dir - dir * 20 * s, by0 = y0 + d * 2;
            const float e0 = (d * 0.28f) * 1000 / dur, e1 = (d * 0.28f + T) * 1000 / dur;
            if (crossed(e0))
                splashAdd(bx0, by0, 0.9f, s, vx * 0.3f, 9);
            if (crossed(e1))
                splashAdd(bx0 + vx * T, by0, 1.2f, s, vx * 0.4f, 12);
            if (tl < 0 || tl > T)
                continue;
            const float x = bx0 + vx * tl, y = by0 - (vy * tl - g * tl * tl / 2);
            const float ux = vx, uy = -(vy - g * tl); // velocity: the body points along it
            const float l = sqrtf(ux * ux + uy * uy);
            dolphin(x, y, ux / l * dir, uy / l * dir, dir);
        }
        break;
    case WHALE: // a long back rolls up, blows, then the flukes
    {
        const float h = 4 * s * fade, bxw = x0 + dir * (p * 16 - 8) * s;
        for (int dy = 0; dy <= (int)h; dy++)
        {
            const int w = (int)(16 * s * sqrtf(std::max(0.0f, 1 - (dy / (h + 0.5f)) * (dy / (h + 0.5f)))));
            for (int dx = -w; dx <= w; dx++)
                lp(bxw + dx, y0 - dy, dy == (int)h ? 0x405876 : 0x182434, 0.95f);
        }
        if (p > 0.3f && p < 0.42f) // the blow: a column of mist, light, drifting downwind
        {
            const float sx = bxw + dir * 9 * s;
            for (int i = 0; i < 4; i++)
            {
                for (int k = 0; k < kParts && s_part; k++)
                    if (s_part[k].life <= 0)
                    {
                        s_part[k] = {sx + (frand() - 0.5f) * 2 * s, y0 - h, (frand() - 0.5f) * 14 * s, -(55 + 25 * frand()) * s,
                                     (float)y0, 1.6f, 0.15f, 0xC8DCF0};
                        break;
                    }
            }
            physInit();
        }
        if (p > 0.72f)
        {
            const float f = sinf(PI_ * (p - 0.72f) / 0.28f), fx = bxw - dir * 18 * s;
            const int fh = (int)(9 * s * f);
            for (int y = 0; y < fh; y++)
                lp(fx, y0 - y, 0x182434, 0.95f);
            for (int i = 0; i <= (int)(5 * s); i++)
                lp(fx - i, y0 - fh - i / 2 + 2, 0x2A3C54, 0.95f * f), lp(fx + i, y0 - fh - i / 2 + 2, 0x2A3C54, 0.95f * f);
        }
        break;
    }
    case OCTOPUS: // tentacles curl out of the water, two lamp eyes between
    {
        const float L = 18 * s * fade;
        for (int i = 0; i < 6; i++)
        {
            const float base = x0 + (i - 2.5f) * 6 * s;
            for (int k = 0; k < (int)L; k++)
            {
                const float u = k / std::max(1.0f, L);
                const float cx = base + (4 + 3 * u) * s * sinf(k * 0.32f / s + now * 0.004f + i * 1.3f) * u;
                lp(cx, y0 - k, 0x4A2C5C, 0.95f);
                if (u < 0.6f && s > 0.7f)
                    lp(cx + 1, y0 - k, 0x2E1A3A, 0.95f);
                if (k % 4 == 2)
                    lp(cx - 1, y0 - k, 0xC08AD0, 0.5f);
            }
        }
        const float e = fade * (0.6f + 0.4f * sinf(now * 0.01f));
        lp(x0 - 4 * s, y0 - 2, 0xFFE070, e), lp(x0 + 4 * s, y0 - 2, 0xFFE070, e);
        break;
    }
    case SERPENT: // far out: humps and a neck gliding along, one red eye
    {
        const float hx = x0 + dir * (p * 90 - 45) * s;
        for (int k = 0; k < 4; k++)
        {
            const float cx = hx - dir * (10 + k * 11) * s, r = (3.5f - k * 0.5f) * s * fade * (0.8f + 0.2f * sinf(now * 0.005f + k));
            for (int a = 0; a <= 12; a++)
            {
                const float t = a / 12.0f * PI_;
                lp(cx + cosf(t) * (r + 1), y0 - sinf(t) * r, 0x1C3A2C, 0.95f);
            }
        }
        const int nh = (int)(12 * s * fade);
        for (int y = 0; y < nh; y++)
            lp(hx + dir * (y > nh - 4 ? (y - nh + 4) : 0) * 0.8f, y0 - y, 0x1C3A2C, 0.95f);
        if (nh > 3)
        {
            const float hx2 = hx + dir * 3 * s;
            for (int i = 0; i < (int)(4 * s) + 1; i++)
                lp(hx2 + dir * i, y0 - nh, 0x2A5440, 0.95f);
            lp(hx2 + dir, y0 - nh, 0xFF3020, fade);
        }
        break;
    }
    case JELLY: // a faint bell pulsing below the surface, trailing threads
    {
        // seen through the water: dim, sea-tinted, never above the surface
        // line; a soft glow at night, a pale shape by day
        const float pulse = 0.85f + 0.15f * sinf(now * 0.004f), top = y0 + 2 + 5 * s;
        const float jy = top + 4 * s * pulse + 2 * s * sinf(PI_ * p);
        const uint32_t bell = isDay() ? 0xB8D8E0 : 0x3CC8B0, thread = isDay() ? 0x90B8C4 : 0x2A9A88;
        const float a0 = (isDay() ? 0.22f : 0.30f) * fade;
        for (int a = 0; a < 24; a++)
        {
            const float t = a / 24.0f * 6.28318f;
            const float y = jy + sinf(t) * 4 * s * pulse;
            if (sinf(t) <= 0.2f && y > y0 + 1)
                lp(x0 + cosf(t) * 6 * s * pulse, y, bell, a0);
        }
        for (int k = -2; k <= 2; k++)
            for (int y = 1; y < (int)(8 * s); y++)
                lp(x0 + k * 2 * s + 0.8f * sinf(y * 0.6f + now * 0.003f + k), jy + y, thread, a0 * 0.6f * (1 - y / (8 * s)));
        break;
    }
    case SHARK: // fin speed pulses with each tail beat; a V (Kelvin) wake trails it
    {
        // slow cruise: about a pixel per frame up close; the tail beat only
        // nudges the speed (+-15%), the fin edges are drawn with sub-pixel
        // coverage so it glides instead of hopping a pixel at a time
        const float v = 11 * s, beat = 0.9f;                          // px/s, tail beats/s
        const float xs = v * (tS + 0.15f / (2 * PI_ * beat) * sinf(2 * PI_ * beat * tS));
        const float fx = x0 + dir * (xs - v * dur / 2000.0f), h = 6 * s * std::min(1.0f, fade * 3);
        const float yaw = 0.6f * sinf(2 * PI_ * beat * tS);           // the fin sways a little
        // dorsal fin swept back: the leading edge (at fx, the front) slopes up
        // and backwards, the tip points behind, the trailing edge is steeper
        for (int y = 0; y < (int)h; y++)
        {
            const float front = 0.75f * y + 0.05f * yaw * y, back = 0.9f * h - 0.15f * y;
            if (back < front)
                continue;
            // the row spans [a, b] on screen; each pixel gets its coverage
            const float a = dir > 0 ? fx - back : fx + front, b = dir > 0 ? fx - front : fx + back;
            for (int px = (int)floorf(a - 0.5f); px <= (int)ceilf(b + 0.5f); px++)
            {
                const float cov = std::min(b + 0.5f, px + 0.5f) - std::max(a - 0.5f, px - 0.5f);
                if (cov > 0.05f)
                    lifePix(px, (int)(y0 - y), 0x1A2230, 0.95f * std::min(1.0f, cov));
            }
        }
        const float L = 22 * s; // wake arms at ~19.5 deg, flattened by the view
        for (int i = 2; i < (int)L; i++)
        {
            const float a = 0.5f * fade * (1 - i / L);
            const float spread = i * 0.354f * 0.35f;
            lp(fx - dir * i, y0 + 0.5f - spread, 0x8CA8C8, a * 0.45f);
            lp(fx - dir * i, y0 + 0.5f + spread, 0x8CA8C8, a * 0.75f);
        }
        break;
    }
    case FISHES: // a little school skipping out of the water
    {
        uint32_t sd = seed;
        for (int f = 0; f < 9; f++)
        {
            sd = sd * 1664525u + 1013904223u;
            const float off = (sd >> 24) / 255.0f * 0.5f, q = (p - off) / 0.45f;
            sd = sd * 1664525u + 1013904223u;
            const float fx0 = x0 + ((int)(sd >> 27) - 16) * s, fy0 = y0 + ((sd >> 24) & 7) - 3;
            if (q < 0 || q > 1)
                continue;
            const float x = fx0 + dir * (q * 10 - 5) * s, y = fy0 - 20 * s * q * (1 - q); // parabola (gravity)
            if (q < 0.08f && ((sd >> 9) & 3) == 0)
                splashAdd(fx0 - dir * 5 * s, fy0, 0.3f, s, 0, 2);
            lp(x, y, 0xD0E0F0, 0.8f), lp(x - dir, y + (q < 0.5f ? 1 : -1) * 0.5f, 0x8098B0, 0.7f);
        }
        break;
    }
    case TURTLE: // a shell and a small head, drifting, then down again
    {
        const float sink = p > 0.8f ? (p - 0.8f) / 0.2f : 0;
        const float tx = x0 + dir * (p * 12 - 6) * s, h = std::max(0.0f, bobUp(tS, 2.5f * s) * (1 - sink));
        for (int dy = 0; dy <= (int)h; dy++)
        {
            const int w = (int)(5 * s * sqrtf(std::max(0.0f, 1 - (dy / (h + 0.5f)) * (dy / (h + 0.5f)))));
            for (int dx = -w; dx <= w; dx++)
                lp(tx + dx, y0 - dy, (dx + dy) % 3 ? 0x2C4A30 : 0x46684A, 0.95f);
        }
        if (fade > 0.4f)
            lp(tx + dir * 6 * s, y0 - 1, 0x3E5E42, 0.95f), lp(tx + dir * 7 * s, y0 - 2, 0x3E5E42, 0.95f);
        ring(tx, y0 + 1, 7 * s * (0.5f + p), 0x6A88AA, 0.3f * fade);
        break;
    }
    case MANTA: // a flat ray leaps and slaps back down
    {
        const float x = x0 + dir * (p * 30 - 15) * s, y = y0 - 56 * s * p * (1 - p), w = 8 * s; // parabola
        for (int i = -(int)w; i <= (int)w; i++)
        {
            const float t = 1 - fabsf(i / w);
            lp(x + i, y - t * 1.5f * s * sinf(now * 0.02f), 0x202838, 0.95f);
            if (t > 0.5f)
                lp(x + i, y + 1, 0x101620, 0.95f);
        }
        lp(x - dir * (w + 2), y + 1, 0x202838, 0.8f), lp(x - dir * (w + 3), y + 2, 0x202838, 0.6f); // tail
        if (crossed(0.0f))
            splashAdd(x0 - dir * 15 * s, y0, 0.8f, s, dir * 6 * s, 8);
        if (crossed(0.99f)) // the belly slap
            splashAdd(x0 + dir * 15 * s, y0, 1.5f, s, dir * 10 * s, 16);
        break;
    }
    case SEAL: // a round head bobs up, looks around
    {
        const float sink = p > 0.8f ? (p - 0.8f) / 0.2f : 0;
        const float hy = y0 - bobUp(tS, 3 * s, 6.0f, 0.8f) * (1 - sink) + 0.4f * sinf(now * 0.004f), r = 2.5f * s;
        for (int dy = -(int)r; dy <= 0; dy++)
            for (int dx = -(int)r; dx <= (int)r; dx++)
                if (dx * dx + dy * dy <= r * r)
                    lp(x0 + dx, hy + dy, 0x24282E, 0.95f * std::min(1.0f, fade * 2));
        const float look = sinf(now * 0.002f) > 0 ? 1 : -1;
        lp(x0 + look * r * 0.6f, hy - r * 0.5f, 0xE0E8F0, 0.8f * fade);
        lp(x0 + look * (r + 1), hy - 0.5f, 0x50565E, 0.7f * fade); // nose
        ring(x0, y0 + 1, 5 * s, 0x6A88AA, 0.3f * fade);
        break;
    }
    case GULLS: // flap-flap-glide flight, eased speed changes; landing flare,
                // floating, a running take-off; in and out past the edges
    {
        s_waterOnly = false;
        const bool lands = seed & 1;
        const float sx = dir > 0 ? -15 : 335, ex = dir > 0 ? 335 : -15;
        auto ease = [](float u) { u = std::clamp(u, 0.0f, 1.0f); return u * u * (3 - 2 * u); };
        for (int b = 0; b < 3; b++)
        {
            const float lx = x0 - dir * b * 8 * s, ly = y0 + b * 2;
            const float bt = tS + b * 0.37f; // own wingbeat clock
            float gx, gy, flapHz = 3.2f;
            bool flying = true, flare = false, run = false;
            if (!lands)
            {
                gx = sx + (ex - sx) * p - dir * b * 9 * s;
                gy = y0 - 20 * s - b * 4 + 2.0f * sinf(0.9f * tS + b);
            }
            else if (p < 0.3f) // glide in: decelerating, descending, a flare at the end
            {
                const float q = p / 0.3f, e = 1 - (1 - q) * (1 - q); // fast then slowing
                gx = sx + (lx - sx) * e - dir * b * 9 * s * (1 - q);
                gy = ly - (1 - ease(q)) * (24 * s + b * 4) - 1;
                flare = q > 0.85f;
                if (crossed(0.3f) && b == 0)
                    for (int k = 0; k < 3; k++)
                        splashAdd(lx - dir * k * 8 * s, ly + k * 2, 0.35f, s, dir * 8 * s, 3);
            }
            else if (p < 0.68f) // floating: drifting, rocked by the swell
            {
                gx = lx + dir * (p - 0.3f) * 6;
                gy = ly + 0.6f * sinf(now * 0.0035f + b * 0.8f);
                flying = false;
            }
            else // take-off: running on the water with fast beats, then climbing
            {
                const float q = (p - 0.68f) / 0.32f, e = q * q;    // accelerating
                gx = lx + dir * 2.3f + (ex - lx) * e;
                gy = ly - ease((q - 0.12f) / 0.88f) * (30 * s + b * 4);
                run = q < 0.15f;
                flapHz = run ? 5.5f : 4.0f;
                if (run && ((now / 70 + b) & 1))
                    splashAdd(gx - dir * 2, ly, 0.15f, s, -dir * 10 * s, 1);
            }
            if (gy < 2 || gx < -12 || gx > 332)
                continue;
            if (!flying)
            {
                lp(gx, gy - 1, 0xD8DEE6, 0.85f), lp(gx + dir, gy - 1, 0xC0C8D0, 0.8f), lp(gx - dir, gy, 0xA0A8B0, 0.7f);
                lp(gx + 2 * dir, gy - 2, 0xE0E4E8, 0.85f); // head
                continue;
            }
            // 1.4 s of beats then 0.9 s gliding (wings held, slightly raised)
            const float cyc = fmodf(bt, 2.3f);
            const bool glide = !lands ? cyc > 1.4f : (!run && p > 0.12f && p < 0.25f);
            float fl = glide ? -0.25f : sinf(2 * PI_ * flapHz * bt);
            if (flare)
                fl = -1; // wings up, braking
            const float lift = glide ? 0 : 0.5f * cosf(2 * PI_ * flapHz * bt); // body rises on the downstroke
            const int w = std::max(2, (int)(4 * s));
            for (int i = 1; i <= w; i++)
            {
                const float tip = (float)i / w, dy = -fl * i * 0.6f * (0.6f + 0.4f * tip) + lift;
                lp(gx - i, gy + dy, 0xC0C8D0, 0.75f), lp(gx + i, gy + dy, 0xC0C8D0, 0.75f);
            }
            lp(gx, gy + lift, 0xE0E4E8, 0.85f);
        }
        s_waterOnly = true;
        break;
    }
    case FLYFISH: // launch, glide sinking slowly (drag), a tail tap that
                  // re-boosts it, then it drops in
    {
        const float T1 = 0.48f; // first glide ends here (fraction), the tap
        float h, x;
        const float v = 70 * s;
        x = x0 + dir * (tS * v - dur / 2000.0f * v);
        if (p < 0.1f)
            h = 9 * s * (p / 0.1f) * (2 - p / 0.1f);                    // launch arc
        else if (p < T1)
            h = 9 * s * expf(-(p - 0.1f) * 4.2f);                         // gliding down
        else if (p < T1 + 0.08f)
            h = 9 * s * expf(-(T1 - 0.1f) * 4.2f) * (1 - (p - T1) / 0.08f) + 7 * s * ((p - T1) / 0.08f) * ((p - T1) / 0.08f);
        else
            h = 7 * s * expf(-(p - T1 - 0.08f) * 5.5f) * (1 - std::max(0.0f, (p - 0.9f) / 0.1f));
        if (crossed(0.0f))
            splashAdd(x, y0, 0.5f, s, dir * 20 * s, 5);
        if (crossed(T1))
            splashAdd(x, y0, 0.35f, s, dir * 25 * s, 4);
        if (crossed(0.97f))
            splashAdd(x, y0, 0.6f, s, dir * 25 * s, 6);
        const float y = y0 - h;
        for (int i = -2; i <= 2; i++)
            lp(x + dir * i * s, y, 0x90A8C8, 0.85f);
        const float sh = 0.5f + 0.5f * sinf(now * 0.05f);
        lp(x, y - 1, 0xC0F0FF, 0.6f * sh), lp(x - dir, y - 2, 0xC0F0FF, 0.5f * sh);
        break;
    }
    case KRAKEN: // far out, a vast eye opens on the water, looks, blinks
    {
        const float open = std::min(1.0f, fade * 2.5f) * (fabsf(p - 0.6f) < 0.03f ? 0.1f : 1); // a blink
        const float w = 7 * s, h = 2.5f * s * open;
        for (int dx = -(int)w; dx <= (int)w; dx++)
        {
            const float hh = h * sqrtf(std::max(0.0f, 1 - (dx / w) * (dx / w)));
            for (int dy = -(int)hh; dy <= (int)hh; dy++)
                lp(x0 + dx, y0 + dy, abs(dx - (int)(2 * s * sinf(now * 0.001f))) < std::max(1.0f, s) ? 0x100800 : 0xE0B020, 0.9f);
        }
        for (int a = 0; a < 6; a++) // dark coils around it
        {
            const float t = now * 0.0015f + a * 1.05f;
            lp(x0 + cosf(t) * (w + 6 * s), y0 + sinf(t) * 2 * s, 0x301828, 0.8f * fade);
        }
        break;
    }
    }
}

// Ships, tankers, planes and satellites crossing now and then
template <typename R> static void trafficFrame(R &rnd)
{
    enum { TRAWLER, TANKER, LINER, PLANE, PLANE2, SATELLITE, KINDS };
    static int kind = -1, dir = 1, y0 = 0;
    static float speed = 0;
    static uint32_t t0 = 0, next = 0, showSeen = 0;
    const uint32_t now = millis();
    s_pl = &s_traffic, s_waterOnly = false;
    if (showSeen != s_showMs)
        showSeen = s_showMs, kind = -1, s_traffic.n = 0, next = now + 4000 + rnd() % 6000;
    listErase(s_traffic);
    if (kind < 0)
    {
        if ((int32_t)(now - next) < 0)
            return;
        kind = rnd() % KINDS;
        dir = rnd() & 1 ? 1 : -1;
        const bool ship = kind <= LINER;
        y0 = ship ? kHorizon + 2 + rnd() % 3 : kind == SATELLITE ? 6 + rnd() % 30 : 10 + rnd() % 70;
        speed = ship ? 0.8f + (rnd() % 100) / 70.0f : kind == SATELLITE ? 9 + rnd() % 6 : 22 + rnd() % 20; // ships far out: ~1-2 px/s
        t0 = now;
    }
    const float x = (dir > 0 ? -20 : 340) + dir * speed * (now - t0) / 1000.0f;
    if (x < -50 || x > 370)
    {
        kind = -1, next = now + 8000 + rnd() % 14000;
        return;
    }
    const int X = (int)x, Y = y0;
    const bool blink = (now / 500) % 2, strobe = (now % 1200) < 90;
    switch (kind)
    {
    case TRAWLER: // small hull, wheelhouse, a mast with a work light
        for (int i = -5; i <= 5; i++)
            lifePix(X + i, Y, 0x05080C, 1);
        for (int i = -3; i <= 4; i++)
            lifePix(X + i, Y + 1, 0x05080C, 1);
        lifePix(X - 2, Y - 1, 0x05080C, 1), lifePix(X - 1, Y - 1, 0x05080C, 1), lifePix(X - 1, Y - 2, 0x05080C, 1);
        lifePix(X - 2, Y - 1, 0xFFC060, 0.6f);
        for (int y = 1; y <= 5; y++)
            lifePix(X + 2, Y - y, 0x0A1018, 1);
        lifePix(X + 2, Y - 6, 0xFFFFFF, blink ? 0.9f : 0.5f);
        break;
    case TANKER: // long low hull, bridge aft, red and white lights
        for (int i = -16; i <= 16; i++)
            lifePix(X + i, Y, 0x05080C, 1), lifePix(X + i, Y + 1, 0x05080C, 1);
        for (int y = 1; y <= 4; y++)
            for (int i = 0; i < 4; i++)
                lifePix(X - dir * (12 + i), Y - y, 0x080C12, 1);
        lifePix(X - dir * 13, Y - 2, 0xFFD080, 0.5f), lifePix(X - dir * 14, Y - 3, 0xFFD080, 0.4f);
        lifePix(X - dir * 13, Y - 6, 0xFF2010, blink ? 0.9f : 0.3f);
        lifePix(X + dir * 15, Y - 2, 0xFFFFFF, 0.6f);
        for (int i = -10; i < 10; i += 5)
            lifePix(X + i, Y - 1, 0x0A1018, 1); // deck pipework
        break;
    case LINER: // a cruise ship: rows of lit windows
        for (int i = -13; i <= 13; i++)
            lifePix(X + i, Y, 0x06090E, 1), lifePix(X + i, Y + 1, 0x06090E, 1);
        for (int i = -11; i <= 10; i++)
            lifePix(X + i, Y - 1, 0x0A0E14, 1), lifePix(X + i, Y - 2, 0x0A0E14, 1);
        for (int i = -10; i <= 9; i += 2)
            lifePix(X + i, Y - 1, 0xFFE0A0, ((i * 7 + now / 900) % 5) ? 0.6f : 0.2f);
        for (int i = -6; i <= 5; i += 2)
            lifePix(X + i, Y - 2, 0xFFE8B0, 0.5f);
        lifePix(X - dir * 3, Y - 3, 0x0A0E14, 1), lifePix(X - dir * 3, Y - 4, 0x0A0E14, 1); // funnel
        break;
    case PLANE:
    case PLANE2: // tiny dark body, red/green wing lights, white strobe
    {
        const int Yp = Y + (int)((x - 160) * (kind == PLANE2 ? 0.06f : -0.03f)); // gently climbing/descending
        for (int i = -2; i <= 2; i++)
            lifePix(X + i, Yp, 0x101418, 0.9f);
        lifePix(X, Yp - 1, 0xFF2020, blink ? 0.9f : 0.4f);
        lifePix(X, Yp + 1, 0x20FF40, blink ? 0.4f : 0.9f);
        if (strobe)
            lifePix(X - dir * 2, Yp, 0xFFFFFF, 1);
        break;
    }
    case SATELLITE: // a faint steady point drifting across (not in daylight)
        if (starVis() < 0.5f)
            break;
        lifePix(X, Y + (int)(x * 0.15f), 0xD0D8E8, 0.55f + 0.3f * ((now / 2500) % 2));
        break;
    }
}

// Pines on the shores, swaying in the wind. Each shore strip (40 x 90)
// is composed from a tree-free copy plus the pines at the current sway and
// pushed to the LCD; the rows below kTreeY1 never move (trunks, caption).
static constexpr int kTreeY0 = 150, kTreeY1 = 222, kTreeH = kTreeY1 - kTreeY0, kTreeW = 40;
static constexpr int kTreeX[2] = {0, 280};
static uint16_t *s_treeBase = nullptr, *s_treeBuf = nullptr; // PSRAM: [2][kTreeH*kTreeW], one strip
struct Pine { int16_t x, base, h; };
static const Pine kPines[] = {{6, 200, 40}, {17, 210, 28}, {27, 222, 18}, {314, 196, 44}, {302, 206, 32}, {291, 220, 20}};
static float s_sway[6] = {0};
template <typename Plot> static void pineDraw(const Pine &t, int ti, float sway, Plot plot)
{
    const uint16_t land = mix(pal3(0x000000, 0x06080A, 0x14261A), 0, 0), snow = mix(pal3(0x3A4860, 0x8C7C88, 0xE8EEF4), 0, 0);
    const bool snowy = s_wx == WX_SNOW;
    const int h = t.h, trunk = std::max(2, h / 8), crown = h - trunk;
    for (int y = t.base - trunk; y <= t.base; y++)
        for (int x = t.x - (h > 30 ? 1 : 0); x <= t.x + 1; x++)
            plot(x, y, land);
    auto hash = [&](int x, int y) { uint32_t v = x * 73856093u ^ y * 19349663u ^ ti * 83492791u; v ^= v >> 13; v *= 0x5bd1e995; return (v >> 8) & 255; };
    const float A = h / 13.0f; // tip sway in px at full wind
    for (int y = 0; y < crown; y++)
    {
        const float f = (float)y / crown;                 // 0 top .. 1 bottom
        const float tf = fmodf(f * 5, 1.0f);              // place in its tier
        const float maxW = (0.07f + 0.31f * f) * h;
        const float w = maxW * (0.3f + 0.7f * tf);
        const int yy = t.base - trunk - crown + y;
        const float bend = sway * A * powf(1 - f, 1.6f);  // top bends most
        const int cx = t.x + (int)lroundf(bend);
        const int core = (int)(w * 0.5f);
        for (int dx = -(int)w - 1; dx <= (int)w + 1; dx++)
        {
            const int ad = abs(dx);
            bool on = ad <= core;
            if (!on && ad <= w + 1) // needles thin out toward the edge
                on = hash(cx + dx, yy) < 255 * (1.15f - (ad - core) / (w - core + 1.5f));
            if (on)
            {
                const int tip = ad > core ? (int)lroundf(sway * 0.6f * tf) : 0; // outer needles flutter
                const bool top = snowy && tf < 0.35f && ad > core / 2;
                plot(cx + dx + tip, yy, top ? snow : land);
            }
        }
        if (tf > 0.82f && w > 3) // a drooping branch at the bottom of each tier
            for (int side = -1; side <= 1; side += 2)
                for (int k = (int)w; k <= (int)w + 3; k++)
                    plot(cx + side * k + (int)lroundf(sway * 0.8f), yy + (k - (int)w) / 2, land);
    }
}
static void treesCompose(int side)
{
    uint16_t *buf = s_treeBuf;
    memcpy(buf, s_treeBase + side * kTreeH * kTreeW, kTreeH * kTreeW * 2);
    const int x0 = kTreeX[side];
    for (int i = 0; i < 6; i++)
        if ((kPines[i].x < 160) == (side == 0))
            pineDraw(kPines[i], i, s_sway[i], [&](int x, int y, uint16_t c) {
                if (x >= x0 && x < x0 + kTreeW && y >= kTreeY0 && y < kTreeY1)
                    buf[(y - kTreeY0) * kTreeW + x - x0] = c;
            });
}
static void treesInit()
{
    if (!s_treeBase)
        s_treeBase = (uint16_t *)heap_caps_malloc(3 * kTreeH * kTreeW * 2, MALLOC_CAP_SPIRAM);
    if (s_treeBase && s_bgSpr)
    {
        s_treeBuf = s_treeBase + 2 * kTreeH * kTreeW;
        for (int side = 0; side < 2; side++)
            for (int y = 0; y < kTreeH; y++)
                for (int x = 0; x < kTreeW; x++)
                    s_treeBase[side * kTreeH * kTreeW + y * kTreeW + x] = s_bgSpr->readPixel(kTreeX[side] + x, kTreeY0 + y);
    }
    for (int i = 0; i < 6; i++) // the still picture: trees at rest, full height
        s_sway[i] = 0, pineDraw(kPines[i], i, 0, [&](int x, int y, uint16_t c) { if (x >= 0 && x < 320) g->drawPixel(x, y, c); });
}
// Wind: a slow swell plus gusts; each tree has its own phase and stiffness
// Date and time, small, bottom right (not during the boot progress bar).
// Drawn into the scene sprite too, so rain or snow passing over it puts it
// back; the clean box under it is kept to redraw it each minute.
static constexpr int kClkW = 110, kClkH = 9, kClkX = 320 - 3 - kClkW, kClkY = 230;
static uint16_t *s_clkBg = nullptr; // PSRAM, the scene under the box
static bool s_clkBgOk = false;      // false after a scene build
static void clockFrame()
{
    static int lastMin = -1;
    static uint32_t showSeen = 0;
    if (s_barShown || !s_bgSpr)
        return;
    struct tm t;
    if (!weatherLocalTime(t))
        return;
    const int m = t.tm_yday * 1440 + t.tm_hour * 60 + t.tm_min;
    if (m == lastMin && showSeen == s_showMs && s_clkBgOk)
        return;
    lastMin = m, showSeen = s_showMs;
    if (!s_clkBg)
        s_clkBg = (uint16_t *)heap_caps_malloc(kClkW * kClkH * 2, MALLOC_CAP_SPIRAM);
    if (!s_clkBg)
        return;
    if (!s_clkBgOk) // first time on a fresh scene: keep what is under the box
        for (int y = 0; y < kClkH; y++)
            for (int x = 0; x < kClkW; x++)
                s_clkBg[y * kClkW + x] = s_bgSpr->readPixel(kClkX + x, kClkY + y);
    s_clkBgOk = true;
    for (int y = 0; y < kClkH; y++)
        for (int x = 0; x < kClkW; x++)
            s_bgSpr->drawPixel(kClkX + x, kClkY + y, s_clkBg[y * kClkW + x]);
    static const char *const wd[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    static const char *const mo[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    char b[24];
    snprintf(b, sizeof b, "%s %d %s  %02d:%02d", wd[t.tm_wday % 7], t.tm_mday, mo[t.tm_mon % 12], t.tm_hour, t.tm_min);
    s_bgSpr->setTextFont(1);
    s_bgSpr->setTextDatum(TL_DATUM);
    const uint32_t wc = isDay() ? 0xD8E6F0 : 0x30506A; // like the weather line
    s_bgSpr->setTextColor(mix(wc, 0, 0));
    s_bgSpr->drawString(b, 320 - 3 - s_bgSpr->textWidth(b), kClkY);
    restoreRect(kClkX, kClkY, kClkW, kClkH);
}

static void treesFrame()
{
    static uint32_t last = 0;
    const uint32_t now = millis();
    if (!s_treeBase || !s_bgSpr || now - last < 110)
        return;
    last = now;
    const float t = now / 1000.0f;
    const float gust = 0.5f + 0.5f * sinf(t * 0.23f) * sinf(t * 0.61f + 1);
    for (int i = 0; i < 6; i++)
        s_sway[i] = (0.35f + 0.65f * gust) * (0.6f * sinf(t * (1.1f + 0.13f * i) + i * 1.7f) + 0.4f * sinf(t * 2.3f + i)) + 0.5f * gust;
    const bool sw = tft.getSwapBytes();
    tft.setSwapBytes(true);
    for (int side = 0; side < 2; side++)
    {
        treesCompose(side);
        const int rows = s_barShown ? std::min(kTreeH, kBarY - 15 - kTreeY0) : kTreeH;
        tft.pushImage(kTreeX[side], kTreeY0, kTreeW, rows, s_treeBuf);
    }
    tft.setSwapBytes(sw);
}

static void twinkleTask(void *)
{
    uint32_t seed = 0xC0FFEE;
    auto rnd = [&]() { seed = seed * 1664525u + 1013904223u; return seed >> 8; };
    while (s_run)
    {
        if (xSemaphoreTake(s_lock, pdMS_TO_TICKS(50)) == pdTRUE)
        {
            for (int k = 0; k < 8; k++) // faint stars: on/off flicker
            {
                const Star &st = s_bg[rnd() % kBg];
                if (st.halo == 255)
                    continue; // behind a cloud
                const float f = 0.15f + (rnd() % 100) / 70.0f;
                tft.drawPixel(st.x, st.y, mix(skyAt(st.x, st.y), st.tint, std::min(1.0f, st.b * f)));
            }
            // Boat: ~6 px/s left to right, steady keel (no bob); waves drift
            {
                static int lastBx = -1000, lastBy = 0, lastFl = -1;
                const uint32_t el = millis() - s_showMs;
                // speed drifts between stop and ~1.4 px/s: a new target every
                // 3-9 s, eased toward it, so the boat idles and gets under way
                static float pos = 0, v = 0, vt = 0;
                static uint32_t lastMs = 0, nextRoll = 0, showSeen = 0;
                const uint32_t now = millis();
                if (showSeen != s_showMs)
                    showSeen = s_showMs, pos = 20 + rnd() % 300, v = vt = 0.7f, lastMs = now, nextRoll = now + 2000;
                const float dt = (now - lastMs) / 1000.0f;
                lastMs = now;
                if ((int32_t)(now - nextRoll) >= 0)
                    vt = (rnd() % 1000) / 1000.0f * 1.4f, nextRoll = now + 3000 + rnd() % 6000;
                v += (vt - v) * std::min(1.0f, dt * 0.4f);
                pos += v * dt;
                if (pos >= 360)
                    pos -= 360; // sails off the right edge, comes back in from the left
                const int bx = -20 + (int)pos;
                const int by = kBoatY;
                const int fl = (el / 260) % 3;
                if (bx != lastBx || by != lastBy || fl != lastFl)
                {
                    if (lastBx > -1000)
                        boatErase(lastBx, lastBy);
                    boatDraw(bx, by, fl);
                    lastBx = bx, lastBy = by, lastFl = fl;
                    s_bx = bx, s_by = by;
                }
                for (int k = 0; k < kWaves; k++)
                {
                    Wave &w = s_wave[k];
                    w.x += w.v;
                    if (w.x < 34)
                        w.x = 282 - w.len;
                    const int nx = (int)w.x;
                    if (nx == s_waveDrawn[k])
                        continue;
                    if (s_waveDrawn[k] >= 0) // restore the old dash
                        for (int x = s_waveDrawn[k]; x < s_waveDrawn[k] + w.len; x++)
                            if (!inBoatBox(x, x, w.y, bx, by))
                                tft.drawPixel(x, w.y, bgPix(x, w.y));
                    s_waveDrawn[k] = nx;
                    if (inBoatBox(nx, nx + w.len, w.y, bx, by))
                        continue;
                    const float near = (w.y - kHorizon) / 76.0f;
                    for (int i = 0; i < w.len; i++)
                    {
                        const float e = 1.0f - fabsf(2.0f * i / (w.len - 1) - 1.0f); // bright middle
                        tft.drawPixel(nx + i, w.y, mix(s_waveLo, s_waveHi, (0.25f + 0.3f * near) * (0.4f + 0.6f * e)));
                    }
                }
            }
            // UFO: once, starting 0.6 s in, ~650 px/s left to right
            {
                const uint32_t el = millis() - s_showMs;
                static int lastX = -100;
                if (el > 600 && s_ufoOn)
                {
                    const int x = -8 + (int)((el - 600) * 0.65f);
                    if (lastX > -100)
                        ufoErase(lastX);
                    if (x > 330)
                        s_ufoOn = false, lastX = -100;
                    else
                    {
                        ufoDraw(x, (el / 90) & 1);
                        lastX = x;
                    }
                }
            }
            // Shooting star: now and then a streak runs down the lane
            static float st = -1, lastH = -1, sv = 0.09f;
            const float L = 0.22f; // trail length, fraction of the lane
            auto lane = [](float t, int &x, int &y) {
                x = (int)(kSx0 + t * (kSx1 - kSx0));
                y = (int)(kSy0 + t * (kSy1 - kSy0));
            };
            if (st < 0 && s_shootOk && rnd() % 25 == 0)
            {
                // random start, direction (either way, 10-55 deg down), length, speed
                kSx0 = 60 + rnd() % 256, kSy0 = 2 + rnd() % 50;
                const float ang = (10 + rnd() % 46) * 0.01745f, len = 45 + rnd() % 80;
                const float dir = (kSx0 > 200 || (kSx0 > 110 && rnd() & 1)) ? -1.0f : 1.0f;
                kSx1 = kSx0 + dir * len * cosf(ang), kSy1 = std::min(kSy0 + len * sinf(ang), 100.0f);
                kSx1 = std::max(2.0f, std::min(317.0f, kSx1));
                sv = 0.05f + (rnd() % 100) / 800.0f; // 0.05-0.175 lane per frame
                st = 0, lastH = -1;
            }
            if (st >= 0)
            {
                int x0, y0, x1, y1;
                if (lastH >= 0) // erase the old streak
                {
                    lane(std::max(0.0f, lastH - L), x0, y0);
                    lane(std::min(1.0f, lastH), x1, y1);
                    const int n = std::max(abs(x1 - x0), abs(y1 - y0)) + 1;
                    for (int i = 0; i <= n; i++)
                    {
                        const int px = x0 + (x1 - x0) * i / n, py = y0 + (y1 - y0) * i / n;
                        restorePix(px, py - 1), restorePix(px, py), restorePix(px, py + 1);
                    }
                }
                st += sv;
                if (st - L > 1.0f)
                    st = -1;
                else
                {
                    const float h = std::min(1.0f, st), tl = std::max(0.0f, st - L);
                    for (int seg = 0; seg < 3; seg++) // dim tail -> bright head
                    {
                        lane(tl + (h - tl) * seg / 3, x0, y0);
                        lane(tl + (h - tl) * (seg + 1) / 3, x1, y1);
                        tft.drawLine(x0, y0, x1, y1, mix(skyRgb(y1), 0xDDEBFF, 0.25f + 0.3f * seg));
                    }
                    if (st <= 1.0f)
                        tft.drawPixel(x1, y1, TFT_WHITE);
                    lastH = h;
                }
            }
            const Star &o = s_orion[rnd() % 9]; // and one Orion star
            if (o.halo)
                star(o.x, o.y, o.halo, o.b * (0.55f + (rnd() % 100) / 160.0f), o.tint);
            treesFrame();
            clockFrame();
            trafficFrame(rnd);
            seaLifeFrame(rnd);
            weatherFrame(rnd);
            xSemaphoreGive(s_lock);
        }
        vTaskDelay(pdMS_TO_TICKS(45));
    }
    s_task = nullptr;
    vTaskDelete(nullptr);
}

// Any keypad key or the touch screen held down: keep the splash up
static bool anyHeld()
{
    kpd.getKeys();
    toneOff(); // a key press starts its beep; Task_ADC (which ends beeps) isn't running yet
    for (int i = 0; i < LIST_MAX; i++)
        if (kpd.key[i].kchar && (kpd.key[i].kstate == PRESSED || kpd.key[i].kstate == HOLD))
            return true;
    bool touch = false;
    if (s_lock && xSemaphoreTake(s_lock, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        touch = tft.getTouchRawZ() > 600 && tft.getTouchRawZ() > 600;
        xSemaphoreGive(s_lock);
    }
    return touch;
}

static void freeScene();
void splashEnd()
{
    // Held key or touch: stay (stars keep twinkling) until it is released
    bool shown = false;
    for (int quiet = 0; quiet < 3;) // 3 polls in a row without a press
    {
        if (anyHeld())
        {
            quiet = 0;
            if (!shown)
            {
                splashProgress(100, "Paused - release to continue");
                shown = true;
            }
        }
        else
            quiet++;
        delay(20);
        if (!shown && quiet >= 1)
            break; // nothing held at the end of boot: go on at once
    }
    s_run = false;
    // the twinkle task must be gone before LVGL draws: a frame still in
    // flight would interleave its SPI writes with LVGL's (corrupt GRAM)
    for (int k = 0; k < 600 && s_task; k++)
        delay(5);
    freeScene();
}

// Mountain ridge (top y at column x; kHorizon = none): Damavand on the left,
// the old legend's mountain, lower peaks fading out before Orion
static float ridgeBase(int x);
static int ridgeSmooth(int x) { return std::min(kHorizon, (int)lroundf(ridgeBase(x))); }
static int16_t *s_ridge = nullptr; // ridgeY per column (the sea's mirror asks per pixel), PSRAM
static bool s_ridgeOk = false;
static int ridgeY(int x)
{
    if (s_ridgeOk && x >= 0 && x < 320)
        return s_ridge[x];
    const float best = ridgeBase(x);
    // a little roughness on the slopes
    const int j = ((x * 37) ^ (x * 11 >> 2)) % 3;
    return std::min(kHorizon, (int)best + (best < kHorizon - 1 ? j - 1 : 0));
}
static float ridgeBase(int x)
{
    struct P { int x, y, w; };
    static const P pk[] = {{84, 117, 24}, {110, 111, 30}, {146, 123, 20}, {174, 116, 28}, {206, 125, 18}}; // uneven, not sorted by height
    // Damavand: a broad volcanic cone - small flat summit, concave flanks
    // (steep near the top, easing out), the right flank longer, a shoulder
    // part way down the left
    const float dx = x - 40.0f;
    const float u = std::max(0.0f, fabsf(dx) - 2.5f) / (dx < 0 ? 42.0f : 58.0f);
    float dam = 99 + (kHorizon - 99) * std::min(1.0f, powf(u, 0.68f));
    if (dx < 0)
        dam -= 3.0f * expf(-powf((u - 0.52f) / 0.09f, 2));
    else
        dam -= 1.5f * expf(-powf((u - 0.66f) / 0.07f, 2));
    float best = std::min((float)kHorizon, dam);
    for (const P &p : pk)
    {
        const float d = fabsf((float)(x - p.x)) / p.w;
        best = std::min(best, p.y + (kHorizon - p.y) * std::min(1.0f, d * (0.85f + 0.15f * d)));
    }
    return best;
}
static bool behindMountain(int x, int y) { return x >= 0 && x < 320 && y >= ridgeY(x) - 2; }
static uint16_t oceanPix(int x, int y)
{
    const Light &L = s_L;
    const int d = y - kHorizon;
    const float f = std::min(1.0f, d / 70.0f);
    const int shift = (int)(2.0f * sinf(d * 0.9f));
    const int sx = std::max(0, std::min(319, x + shift));
    const float k = (kHorizon - ridgeY(sx)) * 0.9f > d ? L.mirror : 1.0f; // mirrored mountain
    return dith((L.farR + f * (L.nearR - L.farR)) * k, (L.farG + f * (L.nearG - L.farG)) * k, (L.farB + f * (L.nearB - L.farB)) * k, x, y);
}
// the light now: weights, the sun on screen, sea colours
static void lightSetup()
{
    Light L;
    float alt, az;
    if (weatherSun(alt, az))
    {
        L.known = true;
        L.alt = alt;
        L.day = smooth01(2, 12, alt);
        L.night = 1 - smooth01(-14, -3, alt);
        L.twi = std::max(0.0f, 1 - L.day - L.night);
        L.low = 1 - smooth01(2, 22, alt);
        const float facing = weatherLat() < 0 ? 0 : 180; // toward the equator
        const float rel = fmodf(az - facing + 540, 360) - 180;
        L.sx = 160 + rel * 1.9f, L.sy = kHorizon - alt * 1.1f; // a flat arc: high sun stays below the title
    }
    L.over = s_wx == WX_THUNDER ? 1.0f : s_wx == WX_RAIN || s_wx == WX_SNOW ? 0.85f : s_wx == WX_FOG ? 0.6f : s_wx == WX_CLOUD ? 0.2f : 0.0f;
    s_L = L;
    const uint32_t far = pal3(0x020C1C, lerpC(0xC07458, 0x4E4450, L.over), lerpC(0x5A92C4, 0x84909A, L.over));
    const uint32_t near = pal3(kFloor, lerpC(0x1A2042, 0x1E1E28, L.over), lerpC(0x1A4C80, 0x3A4856, L.over));
    rgbOf(far, s_L.farR, s_L.farG, s_L.farB);
    rgbOf(near, s_L.nearR, s_L.nearG, s_L.nearB);
    s_L.mirror = 0.35f * L.night + 0.6f * L.twi + 0.78f * L.day;
    s_waveLo = pal3(0x03183A, 0x1E2440, lerpC(0x2A5E8C, 0x4A5866, L.over));
    s_waveHi = pal3(0x3A70A8, 0xD89070, 0xE6F0F8);
    if (!s_ridgeOk)
    {
        int16_t *t = (int16_t *)heap_caps_malloc(320 * sizeof(int16_t), MALLOC_CAP_SPIRAM);
        if (t)
        {
            for (int x = 0; x < 320; x++)
                t[x] = ridgeY(x);
            s_ridge = t;
            s_ridgeOk = true;
        }
    }
}

void splashBegin()
{
    const uint32_t tBegin = millis();
    s_wx = weatherKind();
    cloudsSetup(s_wx);
    lightSetup();
    s_shootOk = (s_wx == WX_CLEAR || s_wx == WX_CLOUD) && starVis() > 0.6f;
    s_bx = -1000;
    if (!s_bgSpr) // the still scene, in PSRAM (TFT_eSprite picks internal RAM while DMA is on)
    {
        s_bgSpr = new TFT_eSprite(&tft);
        s_bgSpr->setColorDepth(16);
        const bool dma = tft.DMA_Enabled;
        tft.DMA_Enabled = false;
        void *ok = s_bgSpr->createSprite(320, 240);
        tft.DMA_Enabled = dma;
        if (!ok)
        {
            delete s_bgSpr;
            s_bgSpr = nullptr;
        }
    }
    // The still scene is kept (PSRAM) and shown again at once while nothing
    // it shows has changed: weather, sun (0.5 deg steps), the weather line
    char key[96];
    snprintf(key, sizeof key, "%d|%d|%d|%d|%.0f|%s", (int)s_wx, s_L.known ? (int)lroundf(s_L.alt * 2) : -999,
             (int)lroundf(s_L.sx / 3), (int)weatherHasData(), weatherTemp(), weatherCity());
    static char s_key[96] = "";
    const bool reuse = s_bgSpr && !strcmp(key, s_key);
    strlcpy(s_key, s_bgSpr ? key : "", sizeof s_key);
    g = s_bgSpr ? (TFT_eSPI *)s_bgSpr : &tft;
    if (!reuse)
        s_clkBgOk = false; // the box under the clock is clean again
    if (!reuse)
    {
    // Sky: night (black to deep navy, nebulae), twilight or day (sun glow).
    // Same sum as skyF(), fast: the gradient once per row, the nebula from a
    // cache (it never changes), the glow as x-table times y-factor.
    {
        static uint8_t *s_neb = nullptr; // nebula, x 180..319, y 0..139, RGB
        static bool s_nebOk = false;
        const bool night = s_L.night > 0.001f;
        if (night && !s_nebOk)
        {
            if (!s_neb)
                s_neb = (uint8_t *)heap_caps_malloc(140 * 140 * 3, MALLOC_CAP_SPIRAM);
            if (s_neb)
            {
                for (int y = 0; y < 140; y++)
                    for (int x = 180; x < 320; x++)
                    {
                        float nr, ng, nb;
                        nebula(x, y, nr, ng, nb);
                        uint8_t *q = s_neb + (y * 140 + x - 180) * 3;
                        q[0] = (uint8_t)std::min(255.0f, nr), q[1] = (uint8_t)std::min(255.0f, ng), q[2] = (uint8_t)std::min(255.0f, nb);
                    }
                s_nebOk = true;
            }
        }
        const Light &L = s_L;
        const bool glow = L.known && L.twi + L.day > 0.02f;
        float *gx = (float *)heap_caps_malloc(2 * 320 * sizeof(float), MALLOC_CAP_SPIRAM), *bx = gx ? gx + 320 : nullptr;
        const bool glow2 = glow && gx; // (internal RAM is short: tables in PSRAM)
        const float k = 1 - 0.85f * L.over, hAmp = (0.45f * L.day + 0.6f * L.twi) * k * 0.8f;
        const uint32_t warm = lerpC(0xFFF0D0, 0xFF9440, L.low);
        const float wr = ((warm >> 16) & 255) * hAmp, wg = ((warm >> 8) & 255) * hAmp, wb = (warm & 255) * hAmp;
        if (glow2)
            for (int x = 0; x < 320; x++)
            {
                const float dx = x - L.sx;
                gx[x] = expf(-dx * dx / (2 * 38.0f * 38.0f));
                bx[x] = expf(-dx * dx / (2 * 110.0f * 110.0f)) * L.twi * k * 0.55f;
            }
        for (int y = 0; y < kHorizon; y++)
        {
            float r0, g0, b0;
            skyRowBase(y, r0, g0, b0);
            const float dy = y - L.sy, gy = glow2 ? expf(-dy * dy / (2 * 38.0f * 38.0f)) : 0, by = glow2 ? expf(-(kHorizon - y) / 24.0f) : 0;
            for (int x = 0; x < 320; x++)
            {
                float r = r0, gg = g0, b = b0;
                if (night && s_neb && x >= 180 && y < 140)
                {
                    const uint8_t *q = s_neb + (y * 140 + x - 180) * 3;
                    r += L.night * q[0], gg += L.night * q[1], b += L.night * q[2];
                }
                if (glow2)
                {
                    const float h = gx[x] * gy, bd = bx[x] * by;
                    r += h * wr + bd * 255, gg += h * wg + bd * 120, b += h * wb + bd * 50;
                }
                s_row[x] = dith(r, gg, b, x, y);
            }
            pushRow(y);
        }
        free(gx);
    }
    // The sun: a soft-edged disk, white-gold high up, orange-red low
    if (s_L.known && s_L.twi + s_L.day > 0.05f && s_L.sy < kHorizon + 8 && s_L.over < 0.95f)
    {
        const uint32_t core = lerpC(0xFFFCF0, 0xFF8C40, s_L.low);
        const float rad = 6.5f, vis = 1 - 0.8f * s_L.over;
        for (int y = (int)s_L.sy - 10; y <= (int)s_L.sy + 10; y++)
            for (int x = (int)s_L.sx - 10; x <= (int)s_L.sx + 10; x++)
            {
                if (x < 0 || x >= 320 || y < 0 || y >= kHorizon)
                    continue;
                const float d = sqrtf((x - s_L.sx) * (x - s_L.sx) + (y - s_L.sy) * (y - s_L.sy));
                const float a = std::max(0.0f, std::min(1.0f, rad + 0.5f - d)) + 0.3f * std::max(0.0f, 1 - (d - rad) / 4);
                blendPix(x, y, core, std::min(1.0f, a) * vis);
            }
    }
    // Faint background stars (fixed seed: the same sky every boot), not
    // behind the title, which a twinkle redraw would chip
    uint32_t seed = 0x5EED1234;
    auto rnd = [&]() { seed = seed * 1664525u + 1013904223u; return seed >> 8; };
    for (int k = 0; k < kBg;)
    {
        const int x = rnd() % 320, y = rnd() % (kHorizon - 10);
        const float b = 0.25f + (rnd() % 100) / 150.0f;
        const uint32_t tint = rnd() % 5 ? 0xC8DCFF : 0xFFE6C0;
        if (nearUfo(x, y) || behindMountain(x, y))
            continue;
        const bool overcast = s_wx == WX_RAIN || s_wx == WX_SNOW || s_wx == WX_THUNDER;
        const float sv = starVis(); // fade out in twilight, none by day
        const bool hidden = overcast || cloudA(x, y) > 0.12f || sv < 0.25f;
        s_bg[k++] = {(int16_t)x, (int16_t)y, (uint8_t)(hidden ? 255 : 0), b * sv, tint};
        if (!hidden)
            g->drawPixel(x, y, mix(skyAt(x, y), tint, b * sv));
    }
    // Orion, as seen in the sky: Betelgeuse (orange) top-left, Bellatrix
    // top-right, the belt Alnitak-Alnilam-Mintaka in a line, Saiph and Rigel
    // (blue-white) below, Meissa the head. Box at x 222-306, y 66-128.
    struct S { float x, y, b; uint32_t tint; };
    static const S orion[] = {
        {0.12f, 0.06f, 0.95f, 0xFFB070}, // Betelgeuse
        {0.86f, 0.13f, 0.65f, 0xBFD8FF}, // Bellatrix
        {0.46f, -0.06f, 0.35f, 0xCFE0FF}, // Meissa
        {0.37f, 0.53f, 0.60f, 0xCFE0FF}, // Alnitak
        {0.49f, 0.48f, 0.65f, 0xCFE0FF}, // Alnilam
        {0.61f, 0.43f, 0.58f, 0xCFE0FF}, // Mintaka
        {0.22f, 0.95f, 0.55f, 0xCFE0FF}, // Saiph
        {0.90f, 0.86f, 1.00f, 0xA8C8FF}, // Rigel
        {0.47f, 0.70f, 0.30f, 0xB0C8E0}, // M42 (sword)
    };
    for (int k = 0; k < 9; k++)
    {
        const S &o = orion[k];
        s_orion[k] = {(int16_t)(222 + (int)(84 * o.x)), (int16_t)(70 + (int)(58 * o.y)),
                      (uint8_t)(o.b > 0.8f ? 4 : o.b > 0.5f ? 3 : 2), o.b, o.tint};
        if (starVis() < 0.5f)
        {
            s_orion[k].halo = 0; // daylight: not drawn, no twinkle
            continue;
        }
        star(s_orion[k].x, s_orion[k].y, s_orion[k].halo, o.b * starVis(), o.tint);
        if (cloudA(s_orion[k].x, s_orion[k].y) > 0.08f)
            s_orion[k].halo = 0; // behind a cloud or veil: no twinkle (its redraw would cut a hole)
    }
    cloudsDraw();

    // Mountains: dark silhouettes, a faint rim of starlight, Damavand's snow
    for (int x = 0; x < 320; x++)
    {
        const int r = ridgeY(x);
        if (r >= kHorizon)
            continue;
        // near black at night, dusk-purple, hazy blue by day (farther peaks paler)
        g->drawFastVLine(x, r, kHorizon - r, mix(pal3(0x010306, 0x2A2236, x < 76 ? 0x4E5E76 : 0x6A7C96), 0, 0));
        g->drawPixel(x, r, mix(pal3(0x0A1220, 0xD08A6A, x < 76 ? 0x8494AA : 0x92A2B8), 0, 0)); // lit rim
        // snow cap: on the smooth ridge (no rock roughness), down to a gently
        // ragged line, fading with height; left (moonlit) slope lighter
        const int base = s_wx == WX_SNOW ? 123 : 114; // snow over the upper third of the cone
        const int rs = ridgeSmooth(x);
        // Texture from the summit down: irregular ribs of snow and rock
        // gullies (smooth value noise over the direction from the summit,
        // so they vary in width and spacing), snow running further down the
        // ribs, scattered rock showing through, a soft skyline edge
        const float sdx = x - 40.0f;
        auto vnoise = [](float t, uint32_t seed) { // smooth 1-D value noise, -1..1
            const int i = (int)floorf(t);
            const float f = t - i, u = f * f * (3 - 2 * f);
            auto h = [&](int k) { uint32_t v = (uint32_t)k * 2654435761u ^ seed; v ^= v >> 15; v *= 2246822519u; v ^= v >> 13; return (v & 1023) / 511.5f - 1; };
            return h(i) + (h(i + 1) - h(i)) * u;
        };
        for (int y = rs; x < 96 && y < base + 10; y++)
        {
            const float a = sdx / (y - 99 + 4.0f);                       // direction from the summit
            const float rib = 0.65f * vnoise(a * 4.2f, 11) + 0.35f * vnoise(a * 9.5f + 0.02f * (y - 99), 23);
            const int line = base + (int)lroundf(1.0f * sinf(x * 0.19f) + 6.0f * std::max(0.0f, rib) - 1.5f * std::max(0.0f, -rib));
            if (y >= line)
                continue;
            const float k = std::min(1.0f, (line - y) / 12.0f);          // brighter higher up
            const int slope = ridgeSmooth(x + 5) - ridgeSmooth(x - 5);
            const float lit = std::max(0.5f, std::min(1.0f, 0.8f - 0.05f * slope)) * (0.88f + 0.18f * rib);
            float v = lit * (0.35f + 0.45f * k);
            if (rib < -0.3f && y > rs + 2 && y > 103)
                v *= 0.55f + 0.3f * (rib + 1);                            // rock showing in the gullies
            const uint32_t hh = (uint32_t)(x * 73856093u ^ y * 19349663u);
            if (y > rs + 3 && ((hh >> 7) & 63) == 0)
                v *= 0.6f;                                                // a scattered dark rock
            if (y == rs)
                v = std::min(1.0f, v * 1.2f + 0.06f);                      // soft skyline edge
            g->drawPixel(x, y, mix(pal3(0x010306, 0x2A2236, 0x56606C), pal3(0x6E7C96, 0xF4B4A0, 0xF6F8FC), v)); // alpenglow at dusk
        }
    }
    // Ocean: deep navy at the horizon to near black, with the mountains
    // mirrored (rippled) and starlight glints
    {
        uint32_t sd = 0x0CEA11;
        auto rn = [&]() { sd = sd * 1664525u + 1013904223u; return sd >> 8; };
        for (int y = kHorizon; y < 240; y++)
        {
            const int d = y - kHorizon;
            const float f = std::min(1.0f, d / 70.0f);
            const uint32_t seaC = (uint32_t)s_L.farR << 16 | (uint32_t)s_L.farG << 8 | (uint32_t)s_L.farB;
            const uint32_t seaN = (uint32_t)s_L.nearR << 16 | (uint32_t)s_L.nearG << 8 | (uint32_t)s_L.nearB;
            const uint16_t sea = mix(seaC, seaN, f);
            { // = oceanPix(), with the row's colours and ripple shift worked out once
                const Light &L = s_L;
                const float r1 = L.farR + f * (L.nearR - L.farR), g1 = L.farG + f * (L.nearG - L.farG), b1 = L.farB + f * (L.nearB - L.farB);
                const int shift = (int)(2.0f * sinf(d * 0.9f));
                for (int x = 0; x < 320; x++)
                {
                    const int sx = std::max(0, std::min(319, x + shift));
                    const float m = (kHorizon - ridgeY(sx)) * 0.9f > d ? L.mirror : 1.0f;
                    s_row[x] = dith(r1 * m, g1 * m, b1 * m, x, y);
                }
            }
            // the sun's glitter path: sparkles, wider and sparser nearer
            if (s_L.known && s_L.alt > -3 && s_L.over < 0.9f && s_L.sx > -60 && s_L.sx < 380)
            {
                const float half = 3 + d * (0.25f + 0.25f * s_L.low), dens = std::min(1.0f, (s_L.alt + 3) / 4) * (1 - s_L.over) * (0.55f + 0.45f * s_L.low); // high sun: narrower, softer
                const uint32_t gl = lerpC(0xFFF8E8, 0xFF9C50, s_L.low);
                for (int x = std::max(0, (int)(s_L.sx - 2 * half)); x < std::min(320, (int)(s_L.sx + 2 * half) + 1); x++)
                {
                    const float u = (x - s_L.sx) / half, w = expf(-u * u * 1.5f);
                    const uint32_t hh = (uint32_t)(x * 2654435761u ^ y * 40503u) >> 9;
                    if ((hh & 255) < 255 * w * 0.6f * dens)
                    {
                        float r, gg, b, tr, tg, tb;
                        rgb565(s_row[x], r, gg, b);
                        rgbOf(gl, tr, tg, tb);
                        const float a = (0.35f + 0.55f * w) * dens;
                        s_row[x] = dith(r + a * (tr - r), gg + a * (tg - gg), b + a * (tb - b), x, y);
                    }
                }
            }
            pushRow(y);
            // glints: short dim dashes, longer nearer the viewer
            if (rn() % 3 == 0)
            {
                const int len = 2 + d / 12 + rn() % 4;
                g->drawFastHLine(rn() % 320, y, len, mix(sea, pal3(0x1E4A78, 0xE0A070, 0xD8E8F4), (0.35f + 0.3f * (1 - f)) * (isDay() ? 0.6f : 1)));
            }
        }
        // Rigel's reflection: a broken line under it
        for (int y = kHorizon + 2; starVis() >= 0.5f && y < kHorizon + 40; y += 3)
            g->drawFastHLine(297 - 1 + (int)(1.5f * sinf(y)), y, 3, mix(0x02101E, 0x5070A0, 0.5f * (1.0f - (y - kHorizon) / 40.0f)));
    }
    fogDraw();
    // Shores: dark land, then pines (drawn by treesCompose, which the
    // wind animation redraws; the tree-free shore is kept for it)
    {
        const uint16_t land = mix(pal3(0x000000, 0x06080A, 0x1A2416), 0, 0);
        g->fillTriangle(0, 186, 0, 240, 32, 240, land);
        g->fillTriangle(0, 186, 30, 205, 32, 240, land);
        g->fillTriangle(320, 180, 320, 240, 288, 240, land);
        g->fillTriangle(320, 180, 292, 200, 288, 240, land);
        treesInit();
    }
    // Name with a glow (dim offset copies, then the bright core)
    g->setTextFont(4);
    g->setTextDatum(MC_DATUM);
    const char *title = "F A N O O S";
    const bool dayText = isDay(); // bright sky: white letters with a navy glow, dark small lines
    const uint16_t glow = dayText ? g->color565(8, 30, 70) : g->color565(4, 34, 70);
    for (int dx = -2; dx <= 2; dx += 2)
        for (int dy = -2; dy <= 2; dy += 2)
            if (dx || dy)
            {
                g->setTextColor(glow);
                g->drawString(title, 112 + dx, 22 + dy);
            }
    g->setTextColor(dayText ? g->color565(250, 252, 255) : g->color565(70, 175, 235));
    g->drawString(title, 112, 22);
    g->setTextFont(2);
    g->setTextColor(dayText ? g->color565(255, 240, 170) : g->color565(70, 230, 170));
    g->drawString("PRECISION POWER SUPPLY", 112, 46);
    g->setTextFont(1);
    g->setTextColor(dayText ? g->color565(10, 34, 70) : g->color565(60, 140, 160));
    g->drawString("MODEL 3206 - 0-32V 0-6A 24-bit", 112, 62);
    g->setTextColor(dayText ? g->color565(20, 44, 80) : g->color565(90, 115, 140));
    g->drawString(SOFTWARE_VERSION, 112, 74);
    g->setTextDatum(TL_DATUM);
    for (int k = 0; k < kBg; k++)
    {
        Star &st = s_bg[k];
        if (st.halo != 255 && g->readPixel(st.x, st.y) != mix(skyAt(st.x, st.y), st.tint, st.b))
            st.halo = 255; // under a letter or its glow: stays put, never twinkles
    }
    // Local weather, small, top left
    if (weatherHasData())
    {
        static const char *const nm[WX_COUNT] = {"clear", "cloudy", "rain", "snow", "fog", "storm"};
        char b[48];
        snprintf(b, sizeof b, "%s  %.0f C  %s", weatherCity(), weatherTemp(), nm[s_wx]);
        g->setTextFont(1);
        const uint32_t wc = isDay() ? 0xD8E6F0 : 0x30506A;
        g->setTextColor(mix(wc, 0, 0));
        g->drawString(b, 3, 230);
        // a small degree ring before the C
        const int cx = 3 + g->textWidth(b) - g->textWidth(String(" C  ") + nm[s_wx]) + 2;
        g->drawCircle(cx, 231, 1, mix(wc, 0, 0));
    }
    } // !reuse
    if (s_bgSpr)
        s_bgSpr->pushSprite(0, 0);
    g = &tft;
    // Progress bar on a clear strip (grid lines showing through its empty
    // part looked like stripes)
    s_barShown = !s_noBar;
    if (!s_noBar)
    {
        tft.fillRect(kBarX - 6, kBarY - 15, kBarW + 12, 26, tft.color565(s_L.nearR, s_L.nearG, s_L.nearB));
        tft.fillRect(kBarX + 2, kBarY + 2, kBarW - 4, 4, tft.color565(2, 22, 32));
        tft.drawRoundRect(kBarX, kBarY, kBarW, 8, 4, tft.color565(0, 140, 170));
        splashProgress(2, "Starting");
    }
    if (!s_lock)
        s_lock = xSemaphoreCreateMutex();
    s_run = true;
    s_showMs = millis();
    s_ufoOn = true;
    {
        uint32_t sd = 0x5EA5EA;
        auto rn = [&]() { sd = sd * 1664525u + 1013904223u; return sd >> 8; };
        for (int k = 0; k < kWaves; k++)
        {
            const int y = kHorizon + 6 + (int)(70 * powf((rn() % 1000) / 1000.0f, 0.8f));
            const int len = 3 + (y - kHorizon) / 9 + rn() % 4;
            s_wave[k] = {(float)(34 + rn() % (248 - len)), (int16_t)y, (int16_t)len, -(0.04f + (rn() % 100) / 1500.0f)};
            s_waveDrawn[k] = -1;
        }
    }
    if (!s_drop)
        s_drop = (Drop *)heap_caps_malloc(kDrops * sizeof(Drop), MALLOC_CAP_SPIRAM);
    if (s_drop)
    {
        uint32_t sd = millis() | 1;
        auto rn = [&]() { sd = sd * 1664525u + 1013904223u; return sd >> 8; };
        for (int k = 0; k < kDrops; k++)
            dropSpawn(s_drop[k], rn, true);
    }
    // Static stack (.bss): at runtime (About) internal RAM is in small pieces
    // and a 4 KB heap stack failed, so nothing on the splash moved. (A PSRAM
    // stack is refused by FreeRTOS here.)
    static StackType_t s_stackBuf[4096];
    static StaticTask_t s_tcbBuf;
    StackType_t *s_stack = s_stackBuf;
    StaticTask_t *s_tcb = &s_tcbBuf;
    // the stack/TCB are reused: the last run's task must be fully gone (it
    // deletes itself; the idle task finishes that)
    static TaskHandle_t s_prev = nullptr;
    for (int k = 0; s_prev && k < 50 && eTaskGetState(s_prev) != eDeleted; k++)
        delay(10);
    if (s_prev)
        delay(30);
    s_task = nullptr;
    if (s_lock && s_stack && s_tcb)
        // Core 0 at boot (setup() runs on Core 1). Opened later (About, DISP:PAGE 5)
        // Core 1: the main loop only waits there, while on Core 0 Task_ADC (same
        // priority) took most of the time and frames came 120-250 ms apart
        s_task = s_prev = xTaskCreateStaticPinnedToCore(twinkleTask, "twinkle", 4096, nullptr, 1, s_stack, s_tcb, s_noBar ? 1 : 0);
    if (s_lock && !s_task)
        Serial.println("[SPLASH] animation task not started");
    ESP_LOGI("SPLASH", "scene built in %lu ms", (unsigned long)(millis() - tBegin));
}

void splashProgress(int pct, const char *what)
{
    if (s_noBar)
        return;
    const bool locked = s_lock && xSemaphoreTake(s_lock, pdMS_TO_TICKS(200)) == pdTRUE;
    pct = std::max(0, std::min(100, pct));
    // One gradient fill of the whole done part, every time (was one line per
    // pixel column: banded where updates overlapped)
    const int inner = kBarW - 4, w = inner * pct / 100;
    if (w > 0)
        tft.fillRectHGradient(kBarX + 2, kBarY + 2, w, 4, tft.color565(0, 110, 255), mix(0x0070FF, 0x00FFA0, (float)pct / 100));
    // status text above the bar
    tft.fillRect(kBarX, kBarY - 13, kBarW, 11, mix(kFloor, kFloor, 0));
    tft.setTextFont(1);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(tft.color565(80, 230, 200));
    char b[48];
    snprintf(b, sizeof b, "%s  %d%%", what, pct);
    tft.drawString(b, 160, kBarY - 7);
    tft.setTextDatum(TL_DATUM);
    if (locked)
        xSemaphoreGive(s_lock);
}

// Settings > About > Software information: the splash again, until a tap,
// any key or a serial command (no timeout). Caller holds the display (spiQuietAcquire) and
// repaints LVGL afterwards. Keys come from Task_ADC's scan (g_keyPressCount).
extern volatile uint32_t g_keyPressCount;
static bool touchedNow()
{
    bool t = false;
    if (s_lock && xSemaphoreTake(s_lock, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        t = tft.getTouchRawZ() > 600 && tft.getTouchRawZ() > 600;
        xSemaphoreGive(s_lock);
    }
    return t;
}
// Hold the splash's drawing (twinkle task) while setup reconfigures the SPI bus
void splashHold(bool hold)
{
    if (!s_lock)
        return;
    if (hold)
        xSemaphoreTake(s_lock, portMAX_DELAY);
    else
        xSemaphoreGive(s_lock);
}
static void freeScene()
{
    if (s_task)
        return; // still drawing
    // the still scene stays (PSRAM, 150 KB): the next splash reuses it
    s_bx = -1000;
}
static void splashStop()
{
    s_run = false;
    // the twinkle task must be gone before LVGL draws: a frame still in
    // flight would interleave its SPI writes with LVGL's (corrupt GRAM)
    for (int k = 0; k < 600 && s_task; k++)
        delay(5);
    freeScene();
}
extern volatile bool g_splashUp;
void splashShow()
{
    g_splashUp = true; // keys only close it (input_device.cpp)
    linkHold(true);
    s_noBar = true;
    splashBegin();
    s_noBar = false;
    // the tap that opened it: wait for its release first
    // linkService(): a linked partner must keep hearing from this unit
    const uint32_t t0 = millis();
    while (touchedNow() && millis() - t0 < 3000)
        delay(20), linkService();
    for (int k = 0; k < 8; k++)
        delay(20), linkService();
    // SCPI queries (a polling phone bridge) are answered meanwhile; any other
    // command closes it and then runs, so a remote client is never locked out
    const uint32_t keys = g_keyPressCount;
    while (!touchedNow() && g_keyPressCount == keys && !scpiParser.serviceWhileHeld())
    {
        esp_task_wdt_reset();
        delay(20);
        linkService();
    }
    const uint32_t t1 = millis();
    while (touchedNow() && millis() - t1 < 3000)
        delay(20), linkService();
    splashStop();
    linkHold(false);
    g_splashUp = false;
}

// DISP:SPLASH:LIFE? [n] (a query, so the splash stays): creature n next (0 dolphins ... 4 jelly, 5 shark);
// returns the frame period since the last call: "avg_ms,max_ms,frames"
String splashLife(int kind)
{
    char b[40];
    snprintf(b, sizeof b, "%.1f,%lu,%lu", s_frN ? (float)s_frSum / s_frN : 0.0f, (unsigned long)s_frMax, (unsigned long)s_frN);
    s_frSum = s_frN = s_frMax = 0;
    s_lifeAsk = kind;
    return b;
}

// DISP:SPLASH:SHOT? [ms]: show the splash, wait, read the LCD back (MISO)
// and send it like DISP:SHOT?: 240 lines of 320 RGB565 hex, then END.
void splashShot(uint32_t waitMs, bool show, bool hiFirst)
{
    linkHold(true);
    if (show)
    {
        s_noBar = true;
        splashBegin();
        s_noBar = false;
        for (const uint32_t t0 = millis(); millis() - t0 < waitMs;)
            delay(10), linkService();
    }
    else
        tft.dmaWait(); // DISP:LCD?: what the panel holds now
    // whole frame first (one lock, ~0.1 s), so moving things aren't smeared
    uint16_t *fb = (uint16_t *)heap_caps_malloc(320 * 240 * 2, MALLOC_CAP_SPIRAM);
    if (s_lock)
        xSemaphoreTake(s_lock, portMAX_DELAY);
    if (fb)
        for (int y = 0; y < 240; y++)
            tft.readRect(0, y, 320, 1, fb + y * 320);
    if (s_lock)
        xSemaphoreGive(s_lock);
    static const char hex[] = "0123456789abcdef";
    static char line[320 * 4 + 1];
    for (int y = 0; y < 240 && fb; y++)
    {
        const uint16_t *row = fb + y * 320;
        for (int x = 0; x < 320; x++)
        {
            uint16_t v = row[x];
            if (hiFirst)
                v = (uint16_t)(v << 8 | v >> 8); // readRect() gives it byte-swapped
            line[4 * x] = hex[v >> 12];
            line[4 * x + 1] = hex[(v >> 8) & 15];
            line[4 * x + 2] = hex[(v >> 4) & 15];
            line[4 * x + 3] = hex[v & 15];
        }
        line[1280] = '\n';
        Serial.write((const uint8_t *)line, sizeof line);
        Serial.flush();
        esp_task_wdt_reset();
        linkService();
    }
    Serial.println("END");
    free(fb);
    if (show)
        splashStop();
    linkHold(false);
}
