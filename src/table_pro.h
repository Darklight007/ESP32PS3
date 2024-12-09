#pragma once

#include <iostream>
#include <chrono>
#include <thread>
// #include "config.hpp"
// #include "input_device.h"
// #include "setting_menu.h"
#include <string>
#include <math.h>

/**
 * Create a new Table object with specified styling and dimensions, and optionally align and position it.
 * @param parent    pointer to an Parent object on which the table will be created
 * @param style_    pointer to an LVGL style to apply to the table
 * @param font_     pointer to an LVGL font to be used for the table's text
 * @param align     alignment option for positioning the table relative to its parent
 * @param pos_x     X coordinate offset from the alignment point
 * @param pos_y     Y coordinate offset from the alignment point
 * @param width     width of the table
 * @param height    height of the table
 * @param pad_left  left padding inside the table
 * @param pad_ver   vertical padding inside the table (applies both top and bottom)
 * @return          pointer to the newly created Table object (lv_obj_t*)
 * @note            Ensure that the provided parent object and style/font pointers are valid.
 */
lv_obj_t *table_pro(lv_obj_t *parent, lv_style_t *style_,
                    const lv_font_t *font_, lv_align_t align,
                    lv_coord_t pos_x, lv_coord_t pos_y,
                    lv_coord_t width, lv_coord_t height,
                    lv_coord_t pad_left, lv_coord_t pad_ver)
{

    // lv_obj_set_style_pad_ver(tabview, 0, LV_PART_ITEMS);

    lv_obj_t *table = lv_table_create(parent);
    lv_obj_set_pos(table, pos_x, pos_y);
    lv_obj_set_height(table, height);

    lv_obj_set_style_pad_all(table, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_ver(table, pad_ver, LV_PART_ITEMS);
    lv_obj_set_style_pad_left(table, pad_left, LV_PART_ITEMS);

    lv_obj_remove_style(table, style_, LV_STATE_DEFAULT);

    lv_style_set_text_font(style_, font_);
    lv_style_set_text_letter_space(style_, 1);
    lv_style_set_text_color(style_, lv_palette_main(LV_PALETTE_GREY));
    lv_style_set_text_letter_space(style_, -1);

    lv_obj_add_style(table, style_, LV_STATE_DEFAULT);

    table->user_data = (void *)(uint16_t)0;
    return table;
}