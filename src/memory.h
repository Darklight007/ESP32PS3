#pragma once

#include <lvgl.h>

// Memory management functions
void loadMemory(lv_obj_t *btn);
void saveMemory(lv_obj_t *btn);
void saveToBank(int bank);
void loadFromBank(int bank);
