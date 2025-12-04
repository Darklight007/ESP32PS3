#pragma once

#include <lvgl.h>
#include "device.hpp"
#include "globals.h"
#include "functions.h"

// Forward declarations
void SetupOVP();
void textarea(lv_obj_t *parent);
void Utility_tabview(lv_obj_t *parent);
void StatusBar();
void StatsPositions(lv_obj_t *parent, DispObjects &dObj, lv_style_t *style_, lv_coord_t x, lv_coord_t y);
void updateObjectPos_cb(lv_event_t *e);

// Event handler declarations
void btn_event_cb(lv_event_t *e);
void slider_x_event_cb(lv_event_t *e);
static void slider_x_event_enc_cb(lv_event_t *e);
void slider_y_event_cb(lv_event_t *e);
static void drag_event_handler(lv_event_t *e);
void draw_event_stat_chart_cb(lv_event_t *e);
static void draw_event_stat_chart_cb_old(lv_event_t *e);
void draw_event_cb2(lv_event_t *e);
static void draw_event_cb2_old(lv_event_t *e);
static void sw_event_cb(lv_event_t *e);
static void textarea_event_handler(lv_event_t *e);
static void btnm_event_handler(lv_event_t *e);
static void scroll_begin_event(lv_event_t *e);
static void mem_btn_event_cb(lv_event_t *e);
static void table_get_event_cb(lv_event_t *e);
static void table_touch_event_cb(lv_event_t *e);
static void table_draw_cell_event_cb(lv_event_t *e);
void select_next_row(lv_obj_t *table, lv_coord_t row_height);
void select_previous_row(lv_obj_t *table, lv_coord_t row_height);
void btn_function_gen_event_cb(lv_event_t *e);
void switch_fun_only_event_cb(lv_event_t *e);
static void spinbox_change_event_cb(lv_event_t *e);
static void draw_event_util_Arbit_chart_cb(lv_event_t *e);
static void dropdownEventCb(lv_event_t *e);

// External declarations for global variables
extern lv_obj_t *slider_x;
extern lv_obj_t *ta;
extern lv_obj_t *unit_label;
extern lv_obj_t *btnm;
extern lv_obj_t *tabview_utility;
extern lv_obj_t *util_Arbit_chart;
extern lv_chart_series_t *util_Arbit_chart_series;
extern lv_obj_t *btn_function_gen;


