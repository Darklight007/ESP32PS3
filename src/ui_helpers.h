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
void graphReset();
extern bool g_graphPaused;
extern int g_graphPushCount;

// UI styling helper functions
void legend(lv_obj_t *parent, lv_color16_t c1, const char *ser1, lv_color16_t c2, const char *ser2, int x, int y);
void overlay(lv_obj_t *label, const char *text, lv_style_t *style, lv_color16_t c1, int x, int y);

// UI event callbacks
void draw_event_stat_chart_cb(lv_event_t *e);
void draw_event_cb2(lv_event_t *e);
void updateStatChartSize();
void btn_function_gen_event_cb(lv_event_t *e);
void switch_fun_only_event_cb(lv_event_t *e);

// Three-state X-key view mode for chart pages.
// Each page (page 0 histogram, page 1 graph) keeps its own state and persists across nav.
enum class ChartViewMode : uint8_t {
    NORMAL     = 0,  // chart at normal height, stats labels visible
    EXPANDED   = 1,  // chart taller, stats hidden
    FULLSCREEN = 2,  // chart fills page, all chrome hidden, tick labels drawn inside
};

// Graph page (page 1) chart heights
constexpr int GRAPH_CHART_H_NORMAL     = 154;
constexpr int GRAPH_CHART_H_EXPANDED   = 185;
constexpr int GRAPH_CHART_H_FULLSCREEN = 206;

// Histogram page (page 0) chart heights
constexpr int HIST_CHART_H_NORMAL      = 140;
constexpr int HIST_CHART_H_EXPANDED    = 170;
constexpr int HIST_CHART_H_FULLSCREEN  = 206;

extern ChartViewMode g_graphViewMode;
extern ChartViewMode g_histViewMode;

void applyGraphStatsVisibility(bool visible);
void applyGraphViewMode(ChartViewMode mode);
void applyHistViewMode(ChartViewMode mode);

// =====================================================================
// REVERT SWITCH for X-key view-mode change deferral:
//   1 → X-key handlers (Core 0) stash the target mode in volatile globals;
//        Core 1 drainPendingViewModeChange() runs apply*ViewMode() safely.
//   0 → X handlers call apply*ViewMode() directly from Core 0 (original,
//        unsafe — keeps the freeze; flip to compare/revert quickly).
// =====================================================================
#ifndef DEFER_VIEW_MODE_TO_CORE1
#define DEFER_VIEW_MODE_TO_CORE1 1
#endif

extern volatile bool          g_pendingViewModeChange;
extern volatile ChartViewMode g_pendingViewMode;
extern volatile int           g_pendingViewModePage;  // 0 = histogram, 1 = graph

void drainPendingViewModeChange();

// =====================================================================
// SAFETY SWITCH for updateStatChartSize():
//   0 → fixed version runs: only calls lv_obj_set_size when VI series-state
//        actually transitions (tracked by static cache, no LVGL readback).
//   1 → function returns immediately (no-op). Use only if a regression
//        re-introduces the SIZE_CHANGED recursion freeze.
// =====================================================================
#ifndef DISABLE_STAT_CHART_SIZE_UPDATE
#define DISABLE_STAT_CHART_SIZE_UPDATE 0
#endif

// External UI objects
extern lv_obj_t *label_legend1;
extern lv_obj_t *label_legend2;
