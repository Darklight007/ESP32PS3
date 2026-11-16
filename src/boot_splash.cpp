// Boot splash: drawn straight with TFT_eSPI right after the display starts
// (LVGL isn't running yet); LVGL's first frame paints over it. ARASH 3206:
// named for Arash the Archer, who gave his life to the arrow that set the
// border of Iran. Night sky with
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
// Sky plus nebula at a pixel, 0xRRGGBB (stars and their redraws use this)
static uint32_t skyAt(int x, int y)
{
    float r, g, b;
    nebula(x, y, r, g, b);
    const uint32_t s = skyRgb(y);
    auto add = [](int c, float v) { return (uint32_t)std::min(255, c + (int)v); };
    return add((s >> 16) & 255, r) << 16 | add((s >> 8) & 255, g) << 8 | add(s & 255, b);
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
static void pushRow(int y)
{
    TFT_eSPI *t = g_fwd();
    if (t != &tft) // sprite: plain stores
    {
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
static TFT_eSprite *s_bgSpr = nullptr;
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
static Wave s_wave[kWaves];
static int16_t s_waveDrawn[kWaves];
static bool inBoatBox(int x0, int x1, int y, int bx, int by)
{
    return y >= by - 12 && y <= by + 8 && x1 >= bx - 9 && x0 <= bx + 10;
}

// ─── Weather ────────────────────────────────────────────────────────────────
static WxKind s_wx = WX_CLEAR;
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
}
// Clouds on the scene: dark, a faint lit rim on their tops
static void cloudsDraw()
{
    if (!s_nBlob || !s_bgSpr)
        return;
    const uint32_t core = s_wx == WX_CLOUD ? 0x0E1626 : 0x0C1220, rim = s_wx == WX_CLOUD ? 0x2E4262 : 0x223048;
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
    for (int y = kHorizon - 46; y < 214; y++)
        for (int x = 0; x < 320; x++)
        {
            const float d = (y - kHorizon) / (y < kHorizon ? 20.0f : 34.0f);
            const float drift = 0.75f + 0.25f * sinf(x * 0.035f + y * 0.11f) * sinf(x * 0.011f - y * 0.05f);
            blendPix(x, y, 0x1C2A3C, 0.85f * expf(-d * d) * drift);
        }
}

// Rain, snow, lightning: drawn over the screen, erased from the scene sprite
struct Drop { float x, y, vx, vy; int16_t yEnd, lx, ly; uint8_t len, splash; };
static constexpr int kDrops = 90;
static Drop *s_drop = nullptr; // PSRAM
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
template <typename R> static void dropSpawn(Drop &d, R &rnd, bool first)
{
    const bool snow = s_wx == WX_SNOW;
    d.x = rnd() % 360 - 20;
    d.y = first ? (float)(rnd() % 240) : -(float)(rnd() % 40);
    d.vy = snow ? 0.5f + (rnd() % 60) / 100.0f : 6.5f + (rnd() % 30) / 10.0f;
    d.vx = snow ? 0 : -1.4f;
    d.len = snow ? 1 + rnd() % 3 : 4 + rnd() % 3; // snow: 1 dot, 2 wide, 2x2
    d.yEnd = snow ? 240 : kHorizon + 2 + rnd() % 80;
    d.lx = -100, d.splash = 0;
}
static void dropErase(const Drop &d)
{
    if (d.lx <= -100)
        return;
    if (s_wx == WX_SNOW)
    {
        unPix(d.lx, d.ly);
        if (d.len >= 2)
            unPix(d.lx + 1, d.ly);
        if (d.len == 3)
            unPix(d.lx, d.ly + 1), unPix(d.lx + 1, d.ly + 1);
        return;
    }
    for (int i = 0; i < d.len; i++)
        unPix(d.lx + (i * 14) / 65, d.ly - i);
}
template <typename R> static void weatherFrame(R &rnd)
{
    static uint32_t t = 0;
    t++;
    if ((s_wx == WX_RAIN || s_wx == WX_THUNDER || s_wx == WX_SNOW) && s_drop)
    {
        const bool snow = s_wx == WX_SNOW;
        for (int k = 0; k < kDrops; k++)
        {
            Drop &d = s_drop[k];
            if (d.splash) // ripple of the last frame
            {
                for (int i = -2; i <= 2; i++)
                    unPix(d.lx + i, d.ly);
                dropSpawn(d, rnd, false);
                continue;
            }
            dropErase(d);
            d.y += d.vy;
            d.x += snow ? 0.35f * sinf(t * 0.07f + k) : d.vx;
            if (d.y >= d.yEnd)
            {
                if (!snow && d.yEnd > kHorizon) // a ring on the water
                {
                    d.lx = (int)d.x, d.ly = d.yEnd, d.splash = 1;
                    for (int i = -2; i <= 2; i++)
                        putPix(d.lx + i, d.ly, 0x6A8CB8, abs(i) == 2 ? 0.2f : 0.4f);
                    continue;
                }
                dropSpawn(d, rnd, false);
                continue;
            }
            d.lx = (int)d.x, d.ly = (int)d.y;
            if (snow)
            {
                const float a = 0.6f + 0.12f * d.len;
                putPix(d.lx, d.ly, 0xC8D2E2, a);
                if (d.len >= 2)
                    putPix(d.lx + 1, d.ly, 0xC8D2E2, a * 0.8f);
                if (d.len == 3)
                    putPix(d.lx, d.ly + 1, 0xC8D2E2, a * 0.8f), putPix(d.lx + 1, d.ly + 1, 0xC8D2E2, a * 0.6f);
            }
            else
                for (int i = 0; i < d.len; i++) // streak, fainter tail
                    putPix(d.lx + (i * 14) / 65, d.ly - i, 0x8CA8D8, 0.6f - 0.07f * i);
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
struct PixList { int16_t *x = nullptr, *y = nullptr; int n = 0; }; // lists in PSRAM (internal RAM is short)
static PixList s_life, s_traffic;
static PixList *s_pl = &s_life;
static bool s_waterOnly = true;
static float s_sc = 1; // size by distance
static void lifePix(int x, int y, uint32_t c, float a)
{
    if (s_waterOnly && (y <= kHorizon || (y > 150 && (x < 42 || x > 278))))
        return; // sky or the wooded shores
    if (skipPix(x, y) || s_pl->n >= 900)
        return;
    if (!s_pl->x)
    {
        s_pl->x = (int16_t *)heap_caps_malloc(900 * 2 * sizeof(int16_t), MALLOC_CAP_SPIRAM);
        if (!s_pl->x)
            return;
        s_pl->y = s_pl->x + 900;
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
template <typename R> static void seaLifeFrame(R &rnd)
{
    enum { DOLPHINS, WHALE, OCTOPUS, SERPENT, JELLY, SHARK, FISHES, TURTLE, MANTA, SEAL, GULLS, FLYFISH, KRAKEN, KINDS };
    static const uint32_t durs[KINDS] = {1900, 5200, 4200, 9000, 4500, 7000, 2600, 5000, 1700, 4500, 22000, 2200, 4800};
    static int kind = -1, dir = 1, x0 = 0, y0 = 0;
    static uint32_t t0 = 0, next = 0, dur = 0, showSeen = 0, seed = 1;
    const uint32_t now = millis();
    s_pl = &s_life, s_waterOnly = true;
    if (showSeen != s_showMs)
        showSeen = s_showMs, kind = -1, s_life.n = 0, next = now + 2000 + rnd() % 3000;
    listErase(s_life);
    if (kind < 0)
    {
        if ((int32_t)(now - next) < 0)
            return;
        kind = rnd() % KINDS;
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
    switch (kind)
    {
    case DOLPHINS: // two or three leaps
        for (int d = 0; d < 3; d++)
        {
            const float q = (p * dur - d * 280) / 1300.0f;
            const float bx0 = x0 - d * 11 * s * dir, by0 = y0 + d * 2;
            if (q < 0 || q > 1)
                continue;
            const float w = 44 * s, hh = 15 * s;
            const float x = bx0 + dir * (q * w - w / 2), y = by0 - hh * sinf(PI_ * q);
            float ux = w, uy = -hh * PI_ * cosf(PI_ * q);
            const float l = sqrtf(ux * ux + uy * uy);
            dolphin(x, y, ux / l, uy / l * dir, dir);
            if (q < 0.18f)
                splash(bx0 - dir * w / 2, by0, 1 - q / 0.18f, 77 + d);
            if (q > 0.82f)
                splash(bx0 + dir * w / 2, by0, (q - 0.82f) / 0.18f, 99 + d);
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
        if (p > 0.3f && p < 0.7f)
        {
            const float sp = sinf(PI_ * (p - 0.3f) / 0.4f);
            const int H = (int)(20 * s * sp);
            const float sx = bxw + dir * 9 * s;
            for (int y = 0; y < H; y++)
                lp(sx + ((y * 7 + (int)(now / 60)) % 3) - 1, y0 - h - y, 0xB8D0EA, 0.55f * sp);
            for (int i = 0; i < 10; i++)
            {
                const float a = i / 9.0f * PI_;
                lp(sx + cosf(a) * 7 * s * sp, y0 - h - H + fabsf(cosf(a)) * 6 * s * sp, 0xC8DCF0, 0.6f * sp);
            }
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
    case JELLY: // a glowing bell pulsing under the surface, trailing threads
    {
        const float pulse = 0.75f + 0.25f * sinf(now * 0.008f), jy = y0 + 4 - 4 * p;
        for (int a = 0; a < 24; a++)
        {
            const float t = a / 24.0f * 6.28318f;
            if (sinf(t) <= 0.2f)
                lp(x0 + cosf(t) * 6 * s * pulse, jy + sinf(t) * 4 * s * pulse, 0x40FFD0, 0.6f * fade);
        }
        for (int k = -2; k <= 2; k++)
            for (int y = 1; y < (int)(8 * s); y++)
                lp(x0 + k * 2 * s + sinf(y * 0.6f + now * 0.006f + k), jy + y, 0x20C0A0, 0.4f * fade);
        break;
    }
    case SHARK: // a fin cutting along, a thin wake behind it
    {
        const float fx = x0 + dir * (p * 90 - 45) * s, h = 6 * s * std::min(1.0f, fade * 3);
        for (int y = 0; y < (int)h; y++)
            for (int x = 0; x <= (int)((h - y) * 0.7f); x++)
                lp(fx - dir * x + dir * y * 0.3f, y0 - y, 0x1A2230, 0.95f);
        for (int i = 2; i < (int)(16 * s); i += 2)
            lp(fx - dir * i, y0 + (i / 2 % 2), 0x8CA8C8, 0.45f * fade * (1 - i / (16 * s)));
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
            const float x = fx0 + dir * (q * 10 - 5) * s, y = fy0 - 5 * s * sinf(PI_ * q);
            lp(x, y, 0xD0E0F0, 0.8f), lp(x - dir, y + (q < 0.5f ? 1 : -1) * 0.5f, 0x8098B0, 0.7f);
        }
        break;
    }
    case TURTLE: // a shell and a small head, drifting, then down again
    {
        const float tx = x0 + dir * (p * 12 - 6) * s, h = 2.5f * s * fade;
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
        const float x = x0 + dir * (p * 30 - 15) * s, y = y0 - 14 * s * sinf(PI_ * p), w = 8 * s;
        for (int i = -(int)w; i <= (int)w; i++)
        {
            const float t = 1 - fabsf(i / w);
            lp(x + i, y - t * 1.5f * s * sinf(now * 0.02f), 0x202838, 0.95f);
            if (t > 0.5f)
                lp(x + i, y + 1, 0x101620, 0.95f);
        }
        lp(x - dir * (w + 2), y + 1, 0x202838, 0.8f), lp(x - dir * (w + 3), y + 2, 0x202838, 0.6f); // tail
        if (p < 0.12f)
            splash(x0 - dir * 15 * s, y0, 1 - p / 0.12f, seed);
        if (p > 0.88f)
            splash(x0 + dir * 15 * s, y0, (p - 0.88f) / 0.12f, seed + 1);
        break;
    }
    case SEAL: // a round head bobs up, looks around
    {
        const float hy = y0 - 3 * s * fade + sinf(now * 0.006f), r = 2.5f * s;
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
    case GULLS: // fly in from off screen, maybe land and float a while,
                // then take off and fly out past the other edge
    {
        s_waterOnly = false;
        const bool lands = seed & 1;
        const float sx = dir > 0 ? -15 : 335, ex = dir > 0 ? 335 : -15;
        for (int b = 0; b < 3; b++)
        {
            const float lx = x0 - dir * b * 8 * s, ly = y0 + b * 2; // landing spot on the water
            float gx, gy;
            bool flying = true;
            if (!lands)
                gx = sx + (ex - sx) * p, gy = y0 - 20 * s - b * 4;
            else if (p < 0.3f) // glide in, descending
            {
                const float q = p / 0.3f;
                gx = sx + (lx - sx) * q, gy = ly - (1 - q) * (24 * s + b * 4) - 1;
            }
            else if (p < 0.68f) // floating, bobbing
                gx = lx + dir * (p - 0.3f) * 6, gy = ly + 0.5f * sinf(now * 0.004f + b), flying = false;
            else // take off, climbing out
            {
                const float q = (p - 0.68f) / 0.32f;
                gx = lx + dir * 2.3f + (ex - lx) * q, gy = ly - q * (30 * s + b * 4);
            }
            gx -= dir * b * 9 * s * (flying ? 1 : 0);
            if (gy < 2 || gx < -12 || gx > 332)
                continue;
            if (!flying)
            {
                lp(gx, gy - 1, 0xD8DEE6, 0.85f), lp(gx + dir, gy - 1, 0xC0C8D0, 0.8f), lp(gx - dir, gy, 0xA0A8B0, 0.7f);
                lp(gx + 2 * dir, gy - 2, 0xE0E4E8, 0.85f); // head
                continue;
            }
            const float fl = sinf(now * 0.02f + b * 2); // wing beat
            const int w = std::max(2, (int)(4 * s));
            for (int i = 1; i <= w; i++)
            {
                const float dy = -fl * i * 0.6f;
                lp(gx - i, gy + dy, 0xC0C8D0, 0.75f), lp(gx + i, gy + dy, 0xC0C8D0, 0.75f);
            }
            lp(gx, gy, 0xE0E4E8, 0.85f);
        }
        s_waterOnly = true;
        break;
    }
    case FLYFISH: // glides low over the surface, fins shimmering
    {
        const float x = x0 + dir * (p * 70 - 35) * s, y = y0 - 4 * s * sinf(PI_ * p);
        for (int i = -2; i <= 2; i++)
            lp(x + dir * i * s, y, 0x90A8C8, 0.85f);
        const float sh = 0.5f + 0.5f * sinf(now * 0.05f);
        lp(x, y - 1, 0xC0F0FF, 0.6f * sh), lp(x - dir, y - 2, 0xC0F0FF, 0.5f * sh);
        if (p < 0.1f || p > 0.9f)
            splash(x, y0, 0.6f, seed);
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
    case SATELLITE: // a faint steady point drifting across
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
    const uint16_t land = 0x0000, snow = mix(0x3A4860, 0, 0);
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
                        tft.drawPixel(nx + i, w.y, mix(0x03183A, 0x3A70A8, (0.25f + 0.3f * near) * (0.4f + 0.6f * e)));
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
// where Arash shot from, lower peaks fading out before Orion
static float ridgeBase(int x);
static int ridgeSmooth(int x) { return std::min(kHorizon, (int)lroundf(ridgeBase(x))); }
static int ridgeY(int x)
{
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
    const int d = y - kHorizon;
    const float f = std::min(1.0f, d / 70.0f);
    float ra, ga, ba, rb, gb, bb;
    rgbOf(0x020C1C, ra, ga, ba);
    rgbOf(kFloor, rb, gb, bb);
    const int shift = (int)(2.0f * sinf(d * 0.9f));
    const int sx = std::max(0, std::min(319, x + shift));
    const float k = (kHorizon - ridgeY(sx)) * 0.9f > d ? 0.35f : 1.0f; // mirrored mountain
    return dith((ra + f * (rb - ra)) * k, (ga + f * (gb - ga)) * k, (ba + f * (bb - ba)) * k, x, y);
}

void splashBegin()
{
    s_wx = weatherKind();
    cloudsSetup(s_wx);
    s_shootOk = s_wx == WX_CLEAR || s_wx == WX_CLOUD;
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
    g = s_bgSpr ? (TFT_eSPI *)s_bgSpr : &tft;
    // Sky: black, then deep navy, a dark teal glow only near the horizon
    for (int y = 0; y < kHorizon; y++)
    {
        float r, g, b;
        rgbOf(skyRgb(y), r, g, b);
        // skyRgb is integer per row; smooth it with the next row
        float r2, g2, b2;
        rgbOf(skyRgb(y + 1), r2, g2, b2);
        for (int x = 0; x < 320; x++)
        {
            float nr, ng, nb;
            nebula(x, y, nr, ng, nb);
            s_row[x] = dith((r + r2) / 2 + nr, (g + g2) / 2 + ng, (b + b2) / 2 + nb, x, y);
        }
        pushRow(y);
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
        const bool hidden = overcast || cloudA(x, y) > 0.3f;
        s_bg[k++] = {(int16_t)x, (int16_t)y, (uint8_t)(hidden ? 255 : 0), b, tint};
        if (!hidden)
            g->drawPixel(x, y, mix(skyAt(x, y), tint, b));
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
        star(s_orion[k].x, s_orion[k].y, s_orion[k].halo, o.b, o.tint);
        if (cloudA(s_orion[k].x, s_orion[k].y) > 0.35f)
            s_orion[k].halo = 0; // behind a cloud: no twinkle
    }
    cloudsDraw();

    // Mountains: dark silhouettes, a faint rim of starlight, Damavand's snow
    for (int x = 0; x < 320; x++)
    {
        const int r = ridgeY(x);
        if (r >= kHorizon)
            continue;
        g->drawFastVLine(x, r, kHorizon - r, mix(0x010306, 0, 0)); // near black
        g->drawPixel(x, r, mix(0x0A1220, 0, 0)); // faint rim
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
            g->drawPixel(x, y, mix(0x010306, 0x6E7C96, v));
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
            const uint16_t sea = mix(0x020C1C, kFloor, f);
            for (int x = 0; x < 320; x++)
                s_row[x] = oceanPix(x, y);
            pushRow(y);
            // glints: short dim dashes, longer nearer the viewer
            if (rn() % 3 == 0)
            {
                const int len = 2 + d / 12 + rn() % 4;
                g->drawFastHLine(rn() % 320, y, len, mix(sea, 0x1E4A78, 0.35f + 0.3f * (1 - f)));
            }
        }
        // Rigel's reflection: a broken line under it
        for (int y = kHorizon + 2; y < kHorizon + 40; y += 3)
            g->drawFastHLine(297 - 1 + (int)(1.5f * sinf(y)), y, 3, mix(0x02101E, 0x5070A0, 0.5f * (1.0f - (y - kHorizon) / 40.0f)));
    }
    fogDraw();
    // Shores: dark land, then pines (drawn by treesCompose, which the
    // wind animation redraws; the tree-free shore is kept for it)
    {
        const uint16_t land = 0x0000;
        g->fillTriangle(0, 186, 0, 240, 32, 240, land);
        g->fillTriangle(0, 186, 30, 205, 32, 240, land);
        g->fillTriangle(320, 180, 320, 240, 288, 240, land);
        g->fillTriangle(320, 180, 292, 200, 288, 240, land);
        treesInit();
    }
    // Name with a glow (dim offset copies, then the bright core)
    g->setTextFont(4);
    g->setTextDatum(MC_DATUM);
    const char *title = "A R A S H";
    const uint16_t glow = g->color565(4, 34, 70);
    for (int dx = -2; dx <= 2; dx += 2)
        for (int dy = -2; dy <= 2; dy += 2)
            if (dx || dy)
            {
                g->setTextColor(glow);
                g->drawString(title, 112 + dx, 22 + dy);
            }
    g->setTextColor(g->color565(70, 175, 235));
    g->drawString(title, 112, 22);
    g->setTextFont(2);
    g->setTextColor(g->color565(70, 230, 170));
    g->drawString("PRECISION POWER SUPPLY", 112, 46);
    g->setTextFont(1);
    g->setTextColor(g->color565(60, 140, 160));
    g->drawString("MODEL 3206 - 0-32V 0-6A 24-bit", 112, 62);
    g->setTextColor(g->color565(90, 115, 140));
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
        g->setTextColor(mix(0x30506A, 0, 0));
        g->drawString(b, 3, 230);
        // a small degree ring before the C
        const int cx = 3 + g->textWidth(b) - g->textWidth(String(" C  ") + nm[s_wx]) + 2;
        g->drawCircle(cx, 231, 1, mix(0x30506A, 0, 0));
    }
    if (s_bgSpr)
        s_bgSpr->pushSprite(0, 0);
    g = &tft;
    // Progress bar on a clear strip (grid lines showing through its empty
    // part looked like stripes)
    s_barShown = !s_noBar;
    if (!s_noBar)
    {
        tft.fillRect(kBarX - 6, kBarY - 15, kBarW + 12, 26, mix(kFloor, kFloor, 0));
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
    if (s_lock && xTaskCreatePinnedToCore(twinkleTask, "twinkle", 4096, nullptr, 1, &s_task, 0) != pdPASS)
        s_task = nullptr;
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

// Settings > About > Software information: the splash again, until a tap
// or any key (no timeout). Caller holds the display (spiQuietAcquire) and
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
    if (s_bgSpr)
    {
        s_bgSpr->deleteSprite();
        delete s_bgSpr;
        s_bgSpr = nullptr;
    }
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
void splashShow()
{
    s_noBar = true;
    splashBegin();
    s_noBar = false;
    // the tap that opened it: wait for its release first
    const uint32_t t0 = millis();
    while (touchedNow() && millis() - t0 < 3000)
        delay(20);
    delay(150);
    const uint32_t keys = g_keyPressCount;
    while (!touchedNow() && g_keyPressCount == keys)
    {
        esp_task_wdt_reset();
        delay(20);
    }
    const uint32_t t1 = millis();
    while (touchedNow() && millis() - t1 < 3000)
        delay(20);
    splashStop();
}

// DISP:SPLASH:SHOT? [ms]: show the splash, wait, read the LCD back (MISO)
// and send it like DISP:SHOT?: 240 lines of 320 RGB565 hex, then END.
void splashShot(uint32_t waitMs, bool show)
{
    if (show)
    {
        s_noBar = true;
        splashBegin();
        s_noBar = false;
        delay(waitMs);
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
            const uint16_t v = row[x];
            line[4 * x] = hex[v >> 12];
            line[4 * x + 1] = hex[(v >> 8) & 15];
            line[4 * x + 2] = hex[(v >> 4) & 15];
            line[4 * x + 3] = hex[v & 15];
        }
        line[1280] = '\n';
        Serial.write((const uint8_t *)line, sizeof line);
        Serial.flush();
        esp_task_wdt_reset();
    }
    Serial.println("END");
    free(fb);
    if (show)
        splashStop();
}
