#pragma once

#include <lvgl.h>
#include <stdint.h>

// Utility functions for scaling ADC values
double scaleVoltage(uint16_t voltage);
double scaleCurrent(uint16_t current);

// Forward declarations for functions defined in globalFunctions.h
// These are needed by ui_helpers.cpp to avoid circular includes

void slider_x_event_cb(lv_event_t *e);
void slider_y_event_cb(lv_event_t *e);
void legend(lv_obj_t *parent, lv_color16_t c1, const char *ser1, lv_color16_t c2, const char *ser2, int x, int y);
void overlay(lv_obj_t *label, const char *text, lv_style_t *style, lv_color16_t c1, int x, int y);
void draw_event_cb2(lv_event_t *e);
void draw_event_stat_chart_cb(lv_event_t *e);

// External variables from globalFunctions.h
extern lv_obj_t *label_legend1;
extern lv_obj_t *label_legend2;
extern lv_obj_t *slider_x;
