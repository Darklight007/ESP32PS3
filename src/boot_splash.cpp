// Boot splash: drawn straight with TFT_eSPI right after the display starts
// (LVGL isn't running yet); LVGL's first frame paints over it. Night sky with
// Orion (the three belt stars in a line), a teal horizon, a perspective grid,
// a glowing title and a progress bar that setup() advances.
#include <Arduino.h>
#include "TFT_eSPI.h"
#include "version.h"
#include "boot_splash.h"
#include "Keypad_MC17.h"
extern Keypad_MC17 kpd;

extern TFT_eSPI tft;

static uint16_t mix(uint32_t a, uint32_t b, float t)
{
    t = std::max(0.0f, std::min(1.0f, t));
    const int r = ((a >> 16) & 255) + t * (((b >> 16) & 255) - ((a >> 16) & 255));
    const int g = ((a >> 8) & 255) + t * (((b >> 8) & 255) - ((a >> 8) & 255));
    const int bl = (a & 255) + t * ((b & 255) - (a & 255));
    return tft.color565(r, g, bl);
}

static constexpr int kHorizon = 132;
static constexpr int kBarX = 40, kBarY = 226, kBarW = 240;
static constexpr uint32_t kFloor = 0x010810;

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
    return t < 0.7f ? lerp(0x000000, 0x00040E, t / 0.7f) : lerp(0x00040E, 0x01202A, (t - 0.7f) / 0.3f);
}

// A star: bright core, soft halo of a fixed radius (so a dimmer redraw
// covers the brighter one)
static void star(int x, int y, int halo, float bright, uint32_t tint)
{
    const uint32_t bg = skyRgb(y);
    for (int r = halo; r >= 1; r--)
        tft.fillCircle(x, y, r, mix(bg, tint, bright * (1.0f - (float)r / (halo + 1)) * 0.8f));
    tft.drawPixel(x, y, mix(bg, 0xF0F4FF, std::min(1.0f, bright * 1.1f)));
    if (halo >= 4) // diffraction spikes on the brightest
    {
        const uint16_t c = mix(bg, tint, 0.5f * bright);
        tft.drawFastHLine(x - 7, y, 6, c);
        tft.drawFastHLine(x + 2, y, 6, c);
        tft.drawFastVLine(x, y - 7, 6, c);
        tft.drawFastVLine(x, y + 2, 6, c);
    }
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
                const float f = 0.15f + (rnd() % 100) / 70.0f;
                tft.drawPixel(st.x, st.y, mix(skyRgb(st.y), st.tint, std::min(1.0f, st.b * f)));
            }
            const Star &o = s_orion[rnd() % 9]; // and one Orion star
            star(o.x, o.y, o.halo, o.b * (0.55f + (rnd() % 100) / 160.0f), o.tint);
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
    if (!s_task)
        return;
    s_run = false;
    for (int k = 0; k < 40 && s_task; k++)
        delay(5);
}

void splashBegin()
{
    // Sky: black, then deep navy, a dark teal glow only near the horizon
    for (int y = 0; y < kHorizon; y++)
        tft.drawFastHLine(0, y, 320, mix(skyRgb(y), skyRgb(y), 0));
    // Faint background stars (fixed seed: the same sky every boot), not
    // behind the title, which a twinkle redraw would chip
    uint32_t seed = 0x5EED1234;
    auto rnd = [&]() { seed = seed * 1664525u + 1013904223u; return seed >> 8; };
    for (int k = 0; k < kBg;)
    {
        const int x = rnd() % 320, y = rnd() % (kHorizon - 10);
        const float b = 0.25f + (rnd() % 100) / 150.0f;
        const uint32_t tint = rnd() % 5 ? 0xC8DCFF : 0xFFE6C0;
        if (x > 6 && x < 222 && y > 12 && y < 82)
            continue;
        s_bg[k++] = {(int16_t)x, (int16_t)y, 0, b, tint};
        tft.drawPixel(x, y, mix(skyRgb(y), tint, b));
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
    }

    // Floor
    tft.fillRect(0, kHorizon, 320, 240 - kHorizon, mix(kFloor, kFloor, 0));
    // Horizon glow: green-teal line fading down
    for (int k = 0; k < 7; k++)
        tft.drawFastHLine(0, kHorizon + k, 320, mix(0x30FFB8, kFloor, k / 7.0f));
    for (int k = 1; k < 6; k++) // and up into the sky
        tft.drawFastHLine(0, kHorizon - k, 320, mix(0x0A5E60, skyRgb(kHorizon - k), k / 6.0f));
    // Perspective grid: rays from the vanishing point, rows closer together far away
    for (int k = -11; k <= 11; k++)
        tft.drawLine(160 + k * 6, kHorizon + 1, 160 + k * 60, 240, mix(0x0A3A4A, 0x00A890, 1.0f - fabsf(k) / 14.0f));
    for (int k = 1; k <= 8; k++)
    {
        const float f = k / 8.0f;
        tft.drawFastHLine(0, kHorizon + (int)((240 - kHorizon) * f * f), 320, mix(0x06303C, 0x00C8A0, f));
    }
    // Title with a glow (dim offset copies, then the bright core)
    tft.setTextFont(4);
    tft.setTextDatum(MC_DATUM);
    const char *title = "PRECISION  PSU";
    const uint16_t glow = tft.color565(0, 70, 90);
    for (int dx = -2; dx <= 2; dx += 2)
        for (int dy = -2; dy <= 2; dy += 2)
            if (dx || dy)
            {
                tft.setTextColor(glow);
                tft.drawString(title, 112 + dx, 30 + dy);
            }
    tft.setTextColor(tft.color565(210, 255, 250));
    tft.drawString(title, 112, 30);
    tft.setTextFont(2);
    tft.setTextColor(tft.color565(70, 230, 170));
    tft.drawString("0-32 V   0-6 A   24-bit", 112, 56);
    tft.setTextFont(1);
    tft.setTextColor(tft.color565(110, 140, 170));
    tft.drawString(SOFTWARE_VERSION, 112, 72);
    tft.setTextDatum(TL_DATUM);
    // Progress bar on a clear strip (grid lines showing through its empty
    // part looked like stripes)
    tft.fillRect(kBarX - 6, kBarY - 15, kBarW + 12, 26, mix(kFloor, kFloor, 0));
    tft.fillRect(kBarX + 2, kBarY + 2, kBarW - 4, 4, tft.color565(2, 22, 32));
    tft.drawRoundRect(kBarX, kBarY, kBarW, 8, 4, tft.color565(0, 140, 170));
    splashProgress(2, "Starting");
    s_lock = xSemaphoreCreateMutex();
    s_run = true;
    if (s_lock && xTaskCreatePinnedToCore(twinkleTask, "twinkle", 3072, nullptr, 1, &s_task, 0) != pdPASS)
        s_task = nullptr;
}

void splashProgress(int pct, const char *what)
{
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
