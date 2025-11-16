#pragma once

#include <lvgl.h>
#include <TFT_eSPI.h>
#include "config.hpp"
#include "device.hpp"
#include "globals.h"

// Constants
#ifndef SCREEN_WIDTH
#define SCREEN_WIDTH 320
#endif

#ifndef SCREEN_HEIGHT
#define SCREEN_HEIGHT 240
#endif

#ifndef DOT_SIZE
#define DOT_SIZE 2
#endif

#ifndef CHART_Y_MAX
#define CHART_Y_MAX 200
#endif

#ifndef CHART_POINTS
#define CHART_POINTS 20
#endif

#ifndef NUM_LABELS
#define NUM_LABELS 7
#endif

// UI Helper Functions

// Simple hide function for LVGL objects
void hide(lv_obj_t *obj);

// Chart creation functions
void GraphChart(lv_obj_t *parent, lv_coord_t x, lv_coord_t y);
void StatsChart(lv_obj_t *parent, lv_coord_t x, lv_coord_t y);
void stat_measure(lv_obj_t *parent, lv_coord_t x, lv_coord_t y);

// Bucket/plotting functions for histogram
void initBuckets(TFT_eSPI &tft);
void plotToBucket(uint16_t x, uint16_t y, lv_obj_t *chart, lv_chart_series_t *series);

// Chart data update functions
void GraphPush();
void HistPush();
