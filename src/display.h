#pragma once
// Display driver seam for the TFT_eSPI -> LovyanGFX port.
//
// The default env (`pio run`) builds TFT_eSPI exactly as before. The `lgfx`
// env (`pio run -e lgfx`) lib_ignores TFT_eSPI and puts lgfx_shim/ on the
// include path: lgfx_shim/TFT_eSPI.h implements the TFT_eSPI API the project
// uses on top of LovyanGFX (device config in lgfx_shim/lgfx_ps.hpp), so the
// call sites in src/ are shared by both builds.
