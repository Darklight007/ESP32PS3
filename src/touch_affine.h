#pragma once
#include <Arduino.h>

// Touch mapping raw -> screen by a 6-coefficient affine fit (9-point
// calibration): x = a*rx + b*ry + c, y = d*rx + e*ry + f. Corrects scale,
// offset, rotation and skew; TFT_eSPI's 4-corner calibration only scales and
// offsets each axis, so a slightly rotated panel read worse on one side.
// Falls back to TFT_eSPI's mapping when no fit is stored.

void touchAffineLoad();                                         // at boot, after initializeTouch()
bool touchAffineActive();
bool touchAffineMap(uint16_t rx, uint16_t ry, uint16_t *x, uint16_t *y); // false = not active / off screen
// Full calibration screen flow (raw TFT, blocking). Returns true if saved.
bool touchCalibrateScreen();
// Raw TFT touch read with the active mapping (for screens drawn without LVGL)
bool touchReadScreen(uint16_t *x, uint16_t *y);
