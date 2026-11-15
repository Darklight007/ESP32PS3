// Boot splash: drawn straight with TFT_eSPI right after the display starts
// (LVGL isn't running yet); LVGL's first frame paints over it. Night sky with
// Orion (the three belt stars in a line), a teal horizon, a perspective grid,
// a glowing title and a progress bar that setup() advances.
#include <Arduino.h>
#include "TFT_eSPI.h"
#include "version.h"
#include "boot_splash.h"

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

// A star: bright core, soft halo (radius from brightness)
static void star(int x, int y, float bright, uint32_t tint)
{
    const int halo = bright > 0.8f ? 4 : bright > 0.5f ? 3 : 2;
    for (int r = halo; r >= 1; r--)
        tft.fillCircle(x, y, r, mix(0x020A1A, tint, bright * (1.0f - (float)r / (halo + 1)) * 0.8f));
    tft.drawPixel(x, y, mix(tint, 0xFFFFFF, 0.6f));
    if (bright > 0.8f) // diffraction spikes on the brightest
    {
        const uint16_t c = mix(0x020A1A, tint, 0.45f);
        tft.drawFastHLine(x - 7, y, 15, c);
        tft.drawFastVLine(x, y - 7, 15, c);
        tft.drawPixel(x, y, TFT_WHITE);
    }
}

void splashBegin()
{
    // Sky: black -> deep navy -> dark teal at the horizon
    for (int y = 0; y < kHorizon; y++)
    {
        const float t = (float)y / kHorizon;
        tft.drawFastHLine(0, y, 320, t < 0.55f ? mix(0x000208, 0x02102A, t / 0.55f) : mix(0x02102A, 0x053A48, (t - 0.55f) / 0.45f));
    }
    // Faint background stars (fixed seed: the same sky every boot)
    uint32_t seed = 0x5EED1234;
    auto rnd = [&]() { seed = seed * 1664525u + 1013904223u; return seed >> 8; };
    for (int k = 0; k < 90; k++)
    {
        const int x = rnd() % 320, y = rnd() % (kHorizon - 12);
        const int b = 50 + rnd() % 120;
        tft.drawPixel(x, y, tft.color565(b * 3 / 4, b * 7 / 8, b));
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
    for (const S &s : orion)
        star(222 + (int)(84 * s.x), 70 + (int)(58 * s.y), s.b, s.tint);

    // Floor
    tft.fillRect(0, kHorizon, 320, 240 - kHorizon, mix(kFloor, kFloor, 0));
    // Horizon glow: green-teal line fading down
    for (int k = 0; k < 7; k++)
        tft.drawFastHLine(0, kHorizon + k, 320, mix(0x30FFB8, kFloor, k / 7.0f));
    for (int k = 1; k < 6; k++) // and up into the sky
        tft.drawFastHLine(0, kHorizon - k, 320, mix(0x0E7A78, 0x053A48, k / 6.0f));
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
}

void splashProgress(int pct, const char *what)
{
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
}
