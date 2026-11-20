#pragma once
// Boot splash on the raw display before LVGL takes over (boot_splash.cpp)
void splashBegin();
void splashProgress(int pct, const char *what);
void splashEnd(); // stop the twinkling before LVGL draws
void splashShow(); // About page: the splash until a tap or key (raw display)
#include <stdint.h>
void splashShot(uint32_t waitMs, bool show = true, bool hiFirst = false); // hiFirst: DISP:SHOT? byte order // show=false: read the LCD as is (DISP:LCD?) // DISP:SPLASH:SHOT? - LCD read back over Serial
#include <Arduino.h>
String splashLife(int kind); // DISP:SPLASH:LIFE n: creature n next; frame avg,max ms,count
void splashHold(bool hold); // pause splash drawing (SPI reconfig, e.g. initDMA)
