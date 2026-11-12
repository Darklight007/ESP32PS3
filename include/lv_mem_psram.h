#pragma once
// LVGL heap in PSRAM (lv_conf.h: LV_MEM_CUSTOM_ALLOC / _REALLOC).
#include <stdlib.h>
#ifdef __cplusplus
extern "C" {
#endif
void *lvAlloc(size_t n);
void *lvRealloc(void *p, size_t n);
#ifdef __cplusplus
}
#endif
