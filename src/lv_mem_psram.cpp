#include "lv_mem_psram.h"
#include "esp_heap_caps.h"

// LVGL's heap lives in PSRAM (8 MB), not internal RAM.
//
// The UI's ~10,000 small blocks (objects, styles) used ~200 KB of internal
// RAM and left ~12 KB: Task_ADC's stack barely fit, and there was no room to
// keep the hot drawing code in RAM (ld_iram_hot.py). Measured: redraw speed is
// the same with the objects in PSRAM - a redraw touches few of them and they
// stay in the data cache. Falls back to the normal heap if PSRAM is full.
extern "C" void *lvAlloc(size_t n)
{
    if (void *p = heap_caps_malloc(n, MALLOC_CAP_SPIRAM))
        return p;
    return malloc(n);
}

extern "C" void *lvRealloc(void *p, size_t n)
{
    if (void *q = heap_caps_realloc(p, n, MALLOC_CAP_SPIRAM))
        return q;
    return realloc(p, n);
}
