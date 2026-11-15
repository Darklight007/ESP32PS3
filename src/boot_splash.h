#pragma once
// Boot splash on the raw display before LVGL takes over (boot_splash.cpp)
void splashBegin();
void splashProgress(int pct, const char *what);
