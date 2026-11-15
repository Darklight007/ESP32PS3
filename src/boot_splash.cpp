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
static constexpr uint32_t kFloor = 0x010B1A; // ocean colour near the bottom (bar strip)

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
    return t < 0.7f ? lerp(0x000000, 0x00040E, t / 0.7f) : lerp(0x00040E, 0x000A16, (t - 0.7f) / 0.3f);
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
    return y < kHorizon ? mix(skyRgb(y), 0, 0) : oceanPix(x, y);
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
    const uint32_t bg = skyRgb(y);
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
        tft.drawFastHLine(bx - 3 + ((k * 5 + flick) % 3) - 1, y, w, mix(0x02132C, 0xC07020, 0.5f - 0.13f * k));
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
                tft.drawPixel(st.x, st.y, mix(skyRgb(st.y), st.tint, std::min(1.0f, st.b * f)));
            }
            // Boat: ~6 px/s left to right, steady keel (no bob); waves drift
            {
                static int lastBx = -1000, lastBy = 0, lastFl = -1;
                const uint32_t el = millis() - s_showMs;
                const int bx = 30 + (int)((el / 700) % 260); // ~1.4 px/s
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
static int ridgeY(int x)
{
    struct P { int x, y, w; };
    static const P pk[] = {{38, 100, 44}, {92, 114, 34}, {128, 120, 26}, {168, 124, 30}};
    float best = kHorizon;
    for (const P &p : pk)
    {
        const float d = fabsf((float)(x - p.x)) / p.w;
        best = std::min(best, p.y + (kHorizon - p.y) * std::min(1.0f, d * (0.85f + 0.15f * d)));
    }
    // a little roughness on the slopes
    const int j = ((x * 37) ^ (x * 11 >> 2)) % 3;
    return std::min(kHorizon, (int)best + (best < kHorizon - 1 ? j - 1 : 0));
}
static bool behindMountain(int x, int y) { return x >= 0 && x < 320 && y >= ridgeY(x) - 2; }
static uint16_t oceanPix(int x, int y)
{
    const int d = y - kHorizon;
    const float f = std::min(1.0f, d / 70.0f);
    float ra, ga, ba, rb, gb, bb;
    rgbOf(0x02132C, ra, ga, ba);
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
            s_row[x] = dith((r + r2) / 2, (g + g2) / 2, (b + b2) / 2, x, y);
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
            g->drawPixel(x, y, mix(skyRgb(y), tint, b));
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
        g->drawPixel(x, r, mix(0x081830, 0, 0));
        const int line = s_wx == WX_SNOW ? 122 : 108; // snow line, lower when it snows
        if (r < line) // snow cap, dim blue-grey
            g->drawFastVLine(x, r + 1, std::min(s_wx == WX_SNOW ? 7 : 4, line - r), mix(0x0C1828, 0x22324C, (line - r) / 10.0f));
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
            const uint16_t sea = mix(0x02132C, kFloor, f);
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
    // Shores with pine trees, left and right of the progress bar
    {
        const uint16_t land = 0x0000;
        auto pine = [&](int x, int base, int h) {
            g->drawFastVLine(x, base - h, h, land);
            for (int k = 0; k < 4; k++) // four stacked tiers
            {
                const int top = base - h + k * h / 5, w = 2 + k * h / 9;
                g->fillTriangle(x, top, x - w, top + h / 3, x + w, top + h / 3, land);
            }
        };
        g->fillTriangle(0, 186, 0, 240, 32, 240, land);
        g->fillTriangle(0, 186, 30, 205, 32, 240, land);
        g->fillTriangle(320, 180, 320, 240, 288, 240, land);
        g->fillTriangle(320, 180, 292, 200, 288, 240, land);
        pine(6, 200, 40);
        pine(17, 210, 28);
        pine(27, 222, 18);
        pine(314, 196, 44);
        pine(302, 206, 32);
        pine(291, 220, 20);
        if (s_wx == WX_SNOW) // snow on the branches
        {
            static const int16_t tr[6][3] = {{6, 200, 40}, {17, 210, 28}, {27, 222, 18}, {314, 196, 44}, {302, 206, 32}, {291, 220, 20}};
            const uint16_t sn = mix(0x2A3850, 0, 0);
            for (auto &p : tr)
                for (int k = 0; k < 4; k++)
                {
                    const int top = p[1] - p[2] + k * p[2] / 5, w = 2 + k * p[2] / 9;
                    g->drawLine(p[0] - w, top + p[2] / 3, p[0] - w / 2, top + p[2] / 3 - 2, sn);
                    g->drawLine(p[0] + w / 2, top + p[2] / 3 - 2, p[0] + w, top + p[2] / 3, sn);
                }
        }
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
        if (st.halo != 255 && g->readPixel(st.x, st.y) != mix(skyRgb(st.y), st.tint, st.b))
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
