#pragma once
// Display driver seam for the TFT_eSPI -> LovyanGFX port.
//
// The default env (`pio run`) builds TFT_eSPI exactly as before. The `lgfx`
// env (`pio run -e lgfx`) defines USE_LGFX and compiles display_lgfx.cpp, which
// holds the LovyanGFX panel/bus/touch configuration.
//
// This header deliberately exposes no library types, so it can be included
// next to either TFT_eSPI or LovyanGFX without their macros colliding.
//
// Port status: step 1 (skeleton). Nothing calls into this yet - the existing
// TFT_eSPI call sites are untouched. Next steps: init + flush without DMA,
// then DMA with the spi_quiet slots, then touch, then touch calibration.

#include <stdint.h>

#ifdef USE_LGFX
// Bring up the panel and touch controller. Returns false if init failed.
bool lgfxInit(uint8_t rotation);
#endif
