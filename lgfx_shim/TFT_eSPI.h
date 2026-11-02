#pragma once
// TFT_eSPI-compatible facade over LovyanGFX, used only by the `lgfx` env.
//
// That env lib_ignores the real TFT_eSPI and puts this directory on the include
// path, so every existing `#include <TFT_eSPI.h>` and `tft.` / `spr[]` call in
// src/ compiles unchanged against LovyanGFX. Only the methods the project
// actually uses are adapted here; the rest come straight from LovyanGFX.

#include <cstdlib>
#include <cstring>
#include "lgfx_ps.hpp"

class TFT_eSPI : public LGFX_PS
{
public:
    // ---------------------------------------------------------------- DMA
    void dmaWait() { waitDMA(); }

    // LVGL is built with LV_COLOR_16_SWAP=1, so the buffer is already in panel
    // byte order: hand it to LovyanGFX as swap565_t so it goes out untouched.
    // As in TFT_eSPI, a non-null `buffer` is a DMA-capable staging copy -
    // my_disp_flush() passes internal RAM because the LVGL buffers are PSRAM.
    void pushImageDMA(int32_t x, int32_t y, int32_t w, int32_t h,
                      uint16_t *image, uint16_t *buffer = nullptr)
    {
        const uint16_t *src = image;
        if (buffer && buffer != image)
        {
            memcpy(buffer, image, (size_t)w * h * 2);
            src = buffer;
        }
        LovyanGFX::pushImageDMA(x, y, w, h, reinterpret_cast<const lgfx::swap565_t *>(src));
    }

    void pushColors(uint16_t *data, uint32_t len, bool swap = true) { pushPixels(data, len, swap); }

    // -------------------------------------------------------------- touch
    // Byte-for-byte TFT_eSPI XPT2046 reads (Processors/TFT_eSPI_ESP32*.c),
    // done through LovyanGFX's low-level SPI on the same bus. LovyanGFX's own
    // XPT2046 driver uses a different pressure formula and median filtering,
    // which made presses register inconsistently against fastTouch()'s
    // thresholds and the stored calibration - both tuned to these exact reads.
    uint16_t getTouchRawZ()
    {
        uint8_t d[5] = {0xB0, 0x00, 0xC0, 0x00, 0x00}; // Z1, then Z2
        touchXfer(d, sizeof d);
        int16_t tz = 0xFFF;
        tz += ((d[1] << 8) | d[2]) >> 3;
        tz -= ((d[3] << 8) | d[4]) >> 3;
        if (tz == 4095)
            tz = 0;
        return (uint16_t)tz;
    }

    void getTouchRaw(uint16_t *x, uint16_t *y)
    {
        // X: 0xD0 four times, keep the last; Y: 0x90 four times, keep the last
        uint8_t d[17] = {0xD0, 0, 0xD0, 0, 0xD0, 0, 0xD0, 0, 0x90,
                         0, 0x90, 0, 0x90, 0, 0x90, 0, 0};
        touchXfer(d, sizeof d);
        *x = (uint16_t)(d[7] << 5) | (0x1F & (d[8] >> 3));
        *y = (uint16_t)(d[15] << 5) | (0x1F & (d[16] >> 3));
    }

    bool getTouch(uint16_t *x, uint16_t *y, uint16_t threshold = 600)
    {
        uint16_t xr, yr;
        if (!validTouch(&xr, &yr, threshold))
            return false;
        convertRawXY(&xr, &yr);
        if (xr >= width() || yr >= height())
            return false;
        *x = xr;
        *y = yr;
        return true;
    }

    // ---- Verbatim ports of TFT_eSPI's Extensions/Touch.cpp, so calibration
    // ---- data and behaviour are identical between the two builds.
    void convertRawXY(uint16_t *x, uint16_t *y)
    {
        const int32_t _width = width(), _height = height();
        uint16_t x_tmp = *x, y_tmp = *y, xx, yy;
        if (!_tcRot)
        {
            xx = (x_tmp - _tcx0) * _width / _tcx1;
            yy = (y_tmp - _tcy0) * _height / _tcy1;
        }
        else
        {
            xx = (y_tmp - _tcx0) * _width / _tcx1;
            yy = (x_tmp - _tcy0) * _height / _tcy1;
        }
        if (_tcInvX)
            xx = _width - xx;
        if (_tcInvY)
            yy = _height - yy;
        *x = xx;
        *y = yy;
    }

    void setTouch(uint16_t *p)
    {
        _tcx0 = p[0] ? p[0] : 1;
        _tcx1 = p[1] ? p[1] : 1;
        _tcy0 = p[2] ? p[2] : 1;
        _tcy1 = p[3] ? p[3] : 1;
        _tcRot = p[4] & 0x01;
        _tcInvX = p[4] & 0x02;
        _tcInvY = p[4] & 0x04;
    }

    void calibrateTouch(uint16_t *parameters, uint32_t color_fg, uint32_t color_bg, uint8_t size)
    {
        const int32_t _width = width(), _height = height();
        int16_t values[] = {0, 0, 0, 0, 0, 0, 0, 0};
        for (uint8_t i = 0; i < 4; i++)
        {
            fillRect(0, 0, size + 1, size + 1, color_bg);
            fillRect(0, _height - size - 1, size + 1, size + 1, color_bg);
            fillRect(_width - size - 1, 0, size + 1, size + 1, color_bg);
            fillRect(_width - size - 1, _height - size - 1, size + 1, size + 1, color_bg);
            switch (i)
            {
            case 0: // up left
                drawLine(0, 0, 0, size, color_fg);
                drawLine(0, 0, size, 0, color_fg);
                drawLine(0, 0, size, size, color_fg);
                break;
            case 1: // bot left
                drawLine(0, _height - size - 1, 0, _height - 1, color_fg);
                drawLine(0, _height - 1, size, _height - 1, color_fg);
                drawLine(size, _height - size - 1, 0, _height - 1, color_fg);
                break;
            case 2: // up right
                drawLine(_width - size - 1, 0, _width - 1, 0, color_fg);
                drawLine(_width - size - 1, size, _width - 1, 0, color_fg);
                drawLine(_width - 1, size, _width - 1, 0, color_fg);
                break;
            case 3: // bot right
                drawLine(_width - size - 1, _height - size - 1, _width - 1, _height - 1, color_fg);
                drawLine(_width - 1, _height - 1 - size, _width - 1, _height - 1, color_fg);
                drawLine(_width - 1 - size, _height - 1, _width - 1, _height - 1, color_fg);
                break;
            }
            if (i > 0)
                delay(1000); // let the user release
            for (uint8_t j = 0; j < 8; j++)
            {
                uint16_t x_tmp, y_tmp;
                while (!validTouch(&x_tmp, &y_tmp, 350 / 2)) // Z_THRESHOLD/2, as upstream
                    ;
                values[i * 2] += x_tmp;
                values[i * 2 + 1] += y_tmp;
            }
            values[i * 2] /= 8;
            values[i * 2 + 1] /= 8;
        }
        _tcRot = false;
        if (abs(values[0] - values[2]) > abs(values[1] - values[3]))
        {
            _tcRot = true;
            _tcx0 = (values[1] + values[3]) / 2;
            _tcx1 = (values[5] + values[7]) / 2;
            _tcy0 = (values[0] + values[4]) / 2;
            _tcy1 = (values[2] + values[6]) / 2;
        }
        else
        {
            _tcx0 = (values[0] + values[2]) / 2;
            _tcx1 = (values[4] + values[6]) / 2;
            _tcy0 = (values[1] + values[5]) / 2;
            _tcy1 = (values[3] + values[7]) / 2;
        }
        _tcInvX = false;
        if (_tcx0 > _tcx1)
        {
            uint16_t t = _tcx0; _tcx0 = _tcx1; _tcx1 = t;
            _tcInvX = true;
        }
        _tcInvY = false;
        if (_tcy0 > _tcy1)
        {
            uint16_t t = _tcy0; _tcy0 = _tcy1; _tcy1 = t;
            _tcInvY = true;
        }
        _tcx1 -= _tcx0;
        _tcy1 -= _tcy0;
        if (!_tcx0) _tcx0 = 1;
        if (!_tcx1) _tcx1 = 1;
        if (!_tcy0) _tcy0 = 1;
        if (!_tcy1) _tcy1 = 1;
        if (parameters)
        {
            parameters[0] = _tcx0;
            parameters[1] = _tcx1;
            parameters[2] = _tcy0;
            parameters[3] = _tcy1;
            parameters[4] = _tcRot | (_tcInvX << 1) | (_tcInvY << 2);
        }
    }

private:
    uint16_t _tcx0 = 1, _tcx1 = 1, _tcy0 = 1, _tcy1 = 1;
    bool _tcRot = false, _tcInvX = false, _tcInvY = false;

    // Full-duplex transfer to the XPT2046 at the touch clock, exactly like
    // LovyanGFX's own touch driver drives the shared bus. If a display write
    // is open, close it around the read (as Panel_Device::getTouchRaw does).
    void touchXfer(uint8_t *d, size_t n)
    {
        const bool open = getStartCount() > 0;
        if (open)
            endTransaction();
        lgfx::spi::beginTransaction(SPI2_HOST, SPI_TOUCH_FREQUENCY, 0);
        lgfx::gpio_lo(TOUCH_CS);
        lgfx::spi::readBytes(SPI2_HOST, d, n);
        lgfx::gpio_hi(TOUCH_CS);
        lgfx::spi::endTransaction(SPI2_HOST);
        if (open)
            beginTransaction();
    }

    // Port of TFT_eSPI's validTouch(): debounce on rising pressure, then two
    // position samples that must agree within _RAWERR (20).
    bool validTouch(uint16_t *x, uint16_t *y, uint16_t threshold)
    {
        uint16_t x_tmp, y_tmp, x_tmp2, y_tmp2;
        uint16_t z1 = 1, z2 = 0;
        while (z1 > z2)
        {
            z2 = z1;
            z1 = getTouchRawZ();
            delay(1);
            if (z1 <= threshold)
                return false;
        }
        getTouchRaw(&x_tmp, &y_tmp);
        delay(1);
        if (getTouchRawZ() <= threshold)
            return false;
        delay(2);
        getTouchRaw(&x_tmp2, &y_tmp2);
        if (abs((int)x_tmp - (int)x_tmp2) > 20 || abs((int)y_tmp - (int)y_tmp2) > 20)
            return false;
        *x = x_tmp;
        *y = y_tmp;
        return true;
    }
};

class TFT_eSprite : public LGFX_Sprite
{
public:
    TFT_eSprite() : LGFX_Sprite() { _psram = true; }
    TFT_eSprite(LovyanGFX *parent) : LGFX_Sprite(parent) { _psram = true; }
};
