#pragma once

#include <iostream>
#include <chrono>
#include <thread>
// #include "config.hpp"
#include "input_device.h"
#include "setting_menu.h"
#include <string>
#include <math.h>

static lv_obj_t *spinbox;

// Define styles
static lv_style_t style_no_border;
static lv_style_t style_red_border;

// Structure to hold cursor position and digit count
typedef struct
{
    uint8_t cursor_pos;
    uint8_t digit_count;
} SpinboxData;

// Initialize styles
void init_styles()
{
    static bool hasRun = false;
    if (hasRun)
        return;
    hasRun = true;

    // No border style
    lv_style_init(&style_no_border);

    // Red border style
    lv_style_init(&style_red_border);
    lv_style_set_border_color(&style_red_border, lv_color_hex(0xFF0000));
}

// Function to set red border
void set_red_border(lv_obj_t *spinbox)
{
    lv_obj_add_style(spinbox, &style_red_border, LV_PART_MAIN);
}

// Function to remove red border and revert to no border
void remove_red_border(lv_obj_t *spinbox)
{
    lv_obj_remove_style(spinbox, &style_red_border, LV_PART_MAIN);
    // lv_obj_add_style(spinbox, &style_no_border, LV_PART_MAIN);
}

lv_obj_t *obj_old = nullptr;

static void select_highlight(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);

    init_styles();

    if (obj_old)
        remove_red_border(obj_old);
    set_red_border(obj);
    obj_old = obj;
}

/********************
 * Spinbox
 ********************/
static lv_style_t style_spinboxLbl0;
// lv_style_init(&style_spinboxLbl0);

static lv_style_t style_spinbox0;
// lv_style_init(&style_spinbox0);

// Initialize styles
void init_spinbox_pro_styles()
{
    static bool hasRun = false;
    if (hasRun)
        return;
    hasRun = true;

    lv_style_init(&style_spinboxLbl0);
    lv_style_init(&style_spinbox0);

    lv_style_set_text_font(&style_spinbox0, &graph_R_16); // Undertale_16b1
    lv_style_set_pad_ver(&style_spinbox0, 2);
    // lv_style_set_pad_hor(&style_spinbox, 3);
    lv_style_set_pad_left(&style_spinbox0, 2);
    lv_style_set_pad_right(&style_spinbox0, 0);
    lv_style_set_radius(&style_spinbox0, 3);
    lv_style_set_border_width(&style_spinbox0, 2);
    lv_style_set_border_color(&style_spinbox0, lv_color_hex(0x8f8f8f));
    lv_style_set_text_color(&style_spinbox0, lv_color_hex(0xFFFF00));

    lv_style_set_text_font(&style_spinboxLbl0, &lv_font_montserrat_12); //    &Undertale_16b1); //
    lv_style_set_bg_color(&style_spinboxLbl0, lv_color_hex(0xFF0000));
}

void set_spinbox_pro_styles(lv_obj_t *spinbox)
{
}

// Function to move cursor left
void move_spinbox_cursor_left(lv_obj_t *spinbox)
{
    SpinboxData *data = static_cast<SpinboxData *>(lv_obj_get_user_data(spinbox));
    if (data->cursor_pos > 0)
    {
        data->cursor_pos--;

        lv_spinbox_set_cursor_pos(spinbox, 0);
        lv_spinbox_set_cursor_pos(spinbox, data->cursor_pos);
        // lv_obj_invalidate(spinbox);
    }
}

// Function to move cursor right
void move_spinbox_cursor_right(lv_obj_t *spinbox)
{
    // SpinboxData *data = (SpinboxData *)lv_obj_get_user_data(spinbox);
    SpinboxData *data = static_cast<SpinboxData *>(lv_obj_get_user_data(spinbox));
    if (data->cursor_pos < data->digit_count)
    {
        data->cursor_pos++;

        lv_spinbox_set_cursor_pos(spinbox, 0);
        lv_spinbox_set_cursor_pos(spinbox, data->cursor_pos);
        // lv_obj_invalidate(spinbox);
    }
}

// Cleanup function to free user data
void spinbox_del_event_cb(lv_event_t *e)
{
    lv_obj_t *spinbox = lv_event_get_target(e);
    SpinboxData *data = static_cast<SpinboxData *>(lv_obj_get_user_data(spinbox));
    if (data != nullptr)
    {
        lv_mem_free(data);
    }
}

 lv_obj_t * spinbox_pro(lv_obj_t *parent, const char *labelText, int32_t range_min, int32_t range_max,
                 uint8_t digit_count, uint8_t separator_position, lv_align_t align,
                 lv_coord_t x_ofs, lv_coord_t y_ofs, lv_coord_t width)
{
    init_spinbox_pro_styles();

    lv_obj_t *spinbox = lv_spinbox_create(parent);
    lv_obj_t *_label = lv_label_create(spinbox);

    // Spinbox Label set ******************************************************
    lv_label_set_recolor(_label, true); /*Enable re-coloring by commands in the text*/
    lv_label_set_text(_label, labelText);
    lv_obj_align(_label, LV_ALIGN_OUT_LEFT_TOP, -2, -20);
    // lv_obj_set_style_text_align(_label, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_remove_style(_label, &style_spinboxLbl0, LV_STATE_DEFAULT);
    // Spinbox set  ******************************************************
    lv_obj_set_width(spinbox, width);
    lv_spinbox_set_cursor_pos(spinbox, 0);

    // spinbox->user_data = (void *)(int(0));
    lv_obj_set_user_data(spinbox, (void *)(intptr_t)0);

    lv_obj_align_to(spinbox, parent, align, x_ofs, y_ofs);
    lv_spinbox_set_range(spinbox, range_min, range_max);
    lv_spinbox_set_digit_format(spinbox, digit_count, separator_position);
    lv_spinbox_step_prev(spinbox);
    lv_obj_add_flag(spinbox, LV_OBJ_FLAG_OVERFLOW_VISIBLE);
    //*************************************************************************
    lv_obj_add_style(spinbox, &style_spinbox0, LV_STATE_DEFAULT);
    lv_obj_add_style(_label, &style_spinboxLbl0, LV_STATE_DEFAULT);
    //*************************************************************************
    lv_obj_add_event_cb(spinbox, PRESSED_event_cb, LV_EVENT_SHORT_CLICKED, NULL);
    lv_obj_add_event_cb(spinbox, select_highlight, LV_EVENT_PRESSED, NULL);
    lv_obj_set_scrollbar_mode(spinbox, LV_SCROLLBAR_MODE_OFF);

    // Allocate and set user data with explicit cast
    SpinboxData *data = (SpinboxData *)lv_mem_alloc(sizeof(SpinboxData));
    if (data != NULL)
    {
        data->cursor_pos = 0;
        data->digit_count = digit_count - 1;
        lv_obj_set_user_data(spinbox, data);
    }

    // lv_obj_add_event_cb(spinbox, trackChild, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_set_scrollbar_mode(spinbox, LV_SCROLLBAR_MODE_OFF);

    // Add delete event callback to free user data
    lv_obj_add_event_cb(spinbox, spinbox_del_event_cb, LV_EVENT_DELETE, NULL);

return spinbox;
}