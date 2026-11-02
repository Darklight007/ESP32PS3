#pragma once
// LovyanGFX device config for the ILI9341 + XPT2046 panel (lgfx env only).
// Pins and clocks come from the same TFT_* build flags TFT_eSPI uses
// (platformio.ini), so both drivers always agree on the wiring.
#define LGFX_USE_V1
#include <LovyanGFX.hpp>

class LGFX_PS : public lgfx::LGFX_Device
{
    lgfx::Panel_ILI9341 _panel;
    lgfx::Bus_SPI _bus;
    lgfx::Touch_XPT2046 _touch;

public:
    LGFX_PS()
    {
        {
            auto cfg = _bus.config();
            cfg.spi_host = SPI2_HOST; // FSPI - what TFT_eSPI uses on the S3 without USE_HSPI_PORT
            cfg.spi_mode = 0;
            cfg.freq_write = SPI_FREQUENCY;     // 80 MHz
            cfg.freq_read = SPI_READ_FREQUENCY; // 20 MHz
            cfg.spi_3wire = false;
            cfg.use_lock = true;
            cfg.dma_channel = SPI_DMA_CH_AUTO;
            cfg.pin_sclk = TFT_SCLK;
            cfg.pin_mosi = TFT_MOSI;
            cfg.pin_miso = TFT_MISO;
            cfg.pin_dc = TFT_DC;
            _bus.config(cfg);
            _panel.setBus(&_bus);
        }
        {
            auto cfg = _panel.config();
            cfg.pin_cs = TFT_CS;
            cfg.pin_rst = TFT_RST;
            cfg.pin_busy = -1;
            cfg.memory_width = 240;
            cfg.memory_height = 320;
            cfg.panel_width = 240;
            cfg.panel_height = 320;
            cfg.offset_x = 0;
            cfg.offset_y = 0;
            // LovyanGFX's ILI9341 rotation table sits 180 deg from TFT_eSPI's:
            // setRotation(3) drew the UI upside down. Touch is unaffected - the
            // shim maps raw XPT2046 coordinates with the TFT_eSPI calibration.
            cfg.offset_rotation = 2;
            cfg.readable = true;
            cfg.invert = false;
            cfg.rgb_order = false;
            cfg.dlen_16bit = false;
            cfg.bus_shared = true; // touch controller sits on the same SPI bus
            _panel.config(cfg);
        }
        {
            auto cfg = _touch.config();
            // Placeholder raw range until touch is ported and recalibrated:
            // TFT_eSPI calibration (uint16_t[5]) is not compatible with
            // LovyanGFX's (uint16_t[8]), so it gets its own NVS key later.
            cfg.x_min = 0;
            cfg.x_max = 319;
            cfg.y_min = 0;
            cfg.y_max = 239;
            cfg.pin_int = -1;
            cfg.bus_shared = true;
            cfg.offset_rotation = 0;
            cfg.spi_host = SPI2_HOST;
            cfg.freq = SPI_TOUCH_FREQUENCY; // 2.5 MHz
            cfg.pin_sclk = TFT_SCLK;
            cfg.pin_mosi = TFT_MOSI;
            cfg.pin_miso = TFT_MISO;
            cfg.pin_cs = TOUCH_CS;
            _touch.config(cfg);
            _panel.setTouch(&_touch);
        }
        setPanel(&_panel);
    }
};
