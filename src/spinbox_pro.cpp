#include "spinbox_pro.h"
#include <cmath>

// If you use tones; otherwise, you can comment these out:
#include "buzzer.h" // provides myTone + NOTE_A4 in your project

// If you have a custom font (graph_R_16) defined elsewhere, keep this extern.
// Otherwise, feel free to switch to lv_font_montserrat_16 below.
extern const lv_font_t graph_R_16;

// ---------------- Styles (must outlive users) ----------------
static bool s_styles_inited = false;
static lv_style_t s_style_red_border;
static lv_style_t s_style_spinbox_lbl;
static lv_style_t s_style_spinbox;

// Legacy global selected pointer (your other files read it)
lv_obj_t *obj_selected_spinbox = nullptr;

static void ensure_styles_once()
{
    if (s_styles_inited)
        return;
    s_styles_inited = true;

    lv_style_init(&s_style_red_border);
    lv_style_set_border_width(&s_style_red_border, 2);
    lv_style_set_border_color(&s_style_red_border, lv_color_hex(0xFF0000));
    lv_style_set_border_opa(&s_style_red_border, LV_OPA_COVER);
    lv_style_set_radius(&s_style_red_border, 3);

    lv_style_init(&s_style_spinbox);
    // Use your custom font if available; else montserrat_16.
    lv_style_set_text_font(&s_style_spinbox, &graph_R_16);
    lv_style_set_pad_ver(&s_style_spinbox, 2);
    lv_style_set_pad_left(&s_style_spinbox, 2);
    lv_style_set_pad_right(&s_style_spinbox, 0);
    lv_style_set_radius(&s_style_spinbox, 3);
    lv_style_set_border_width(&s_style_spinbox, 2);
    lv_style_set_border_color(&s_style_spinbox, lv_color_hex(0x8f8f8f));
    lv_style_set_text_color(&s_style_spinbox, lv_color_hex(0xFFFF00));

    lv_style_init(&s_style_spinbox_lbl);
    lv_style_set_text_font(&s_style_spinbox_lbl, &lv_font_montserrat_12);
    lv_style_set_bg_color(&s_style_spinbox_lbl, lv_color_hex(0xFF0000));
}

void spinbox_pro_init_styles(void) { ensure_styles_once(); }

// ---------------- Small helpers used across files ----------------
static void set_red_border(lv_obj_t *sb)
{
    lv_obj_add_style(sb, &s_style_red_border, LV_PART_MAIN);
}

void remove_red_border(lv_obj_t *sb)
{
    if (!sb)
        return;
    lv_obj_remove_style(sb, &s_style_red_border, LV_PART_MAIN);
}

// Highlight/select the spinbox on press
static void select_highlight(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);
    ensure_styles_once();
    if (obj_selected_spinbox && obj_selected_spinbox != obj)
        remove_red_border(obj_selected_spinbox);
    set_red_border(obj);
    obj_selected_spinbox = obj;
}

// Free user data when the spinbox is deleted
static void spinbox_del_event_cb(lv_event_t *e)
{
    lv_obj_t *sb = lv_event_get_target(e);
    if (obj_selected_spinbox == sb)
        obj_selected_spinbox = nullptr;

    SpinboxData *data = static_cast<SpinboxData *>(lv_obj_get_user_data(sb));
    if (data)
    {
        lv_mem_free(data);
        lv_obj_set_user_data(sb, nullptr);
    }
}

// Optional click beep
static void pressed_beep_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_CLICKED)
    {
        myTone(NOTE_A4, 5);
    }
}

// ---------------- Public API (1:1 with your old code) ----------------
lv_obj_t *spinbox_pro(lv_obj_t *parent, const char *labelText,
                      int32_t range_min, int32_t range_max,
                      uint8_t digit_count, uint8_t separator_position,
                      lv_align_t align, lv_coord_t x_ofs, lv_coord_t y_ofs,
                      lv_coord_t width, uint8_t id)
{
    ensure_styles_once();

    // Create spinbox
    lv_obj_t *sb = lv_spinbox_create(parent);
    lv_obj_set_width(sb, width);
    lv_obj_align_to(sb, parent, align, x_ofs, y_ofs);
    lv_spinbox_set_range(sb, range_min, range_max);
    lv_spinbox_set_digit_format(sb, digit_count, separator_position);
    lv_spinbox_set_cursor_pos(sb, 0);
    lv_spinbox_step_prev(sb);

    // Minor flags like your original
    lv_obj_add_flag(sb, LV_OBJ_FLAG_OVERFLOW_VISIBLE);
    lv_obj_clear_flag(sb, LV_OBJ_FLAG_CLICK_FOCUSABLE);
    lv_obj_clear_flag(sb, LV_OBJ_FLAG_SCROLL_ELASTIC);
    lv_obj_clear_flag(sb, LV_OBJ_FLAG_SCROLL_MOMENTUM);
    lv_obj_clear_flag(sb, LV_OBJ_FLAG_SCROLLABLE);

    // Styles
    lv_obj_add_style(sb, &s_style_spinbox, LV_STATE_DEFAULT);

    // Create and style the label that sits “on” the spinbox
    lv_obj_t *lbl = lv_label_create(sb);
    lv_label_set_recolor(lbl, true);
    lv_label_set_text(lbl, labelText ? labelText : "");
    lv_obj_align(lbl, LV_ALIGN_OUT_LEFT_TOP, -2, -20);
    lv_obj_add_style(lbl, &s_style_spinbox_lbl, LV_STATE_DEFAULT);

    // Allocate user data
    SpinboxData *data = (SpinboxData *)lv_mem_alloc(sizeof(SpinboxData));
    if (data)
    {
        data->cursor_pos = 0;
        data->digit_count = (digit_count > 0) ? (digit_count - 1) : 0;
        data->id = id;
        lv_obj_set_user_data(sb, data);
    }

    // Events
    lv_obj_add_event_cb(sb, pressed_beep_cb, LV_EVENT_SHORT_CLICKED, NULL);
    lv_obj_add_event_cb(sb, select_highlight, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(sb, spinbox_del_event_cb, LV_EVENT_DELETE, NULL);

    // Hide scrollbar
    lv_obj_set_scrollbar_mode(sb, LV_SCROLLBAR_MODE_OFF);

    return sb;
}

lv_obj_t *find_spinbox_with_id(lv_obj_t *parent, uint32_t id)
{
    uint32_t child_cnt = lv_obj_get_child_cnt(parent);
    for (uint32_t i = 0; i < child_cnt; ++i)
    {
        lv_obj_t *child = lv_obj_get_child(parent, i);
        SpinboxData *data = static_cast<SpinboxData *>(lv_obj_get_user_data(child));
        if (data && data->id == id)
            return child;
    }
    return NULL;
}

int32_t get_spinbox_data_by_id(lv_obj_t *parent, uint32_t id)
{
    lv_obj_t *obj = find_spinbox_with_id(parent, id);
    return obj ? lv_spinbox_get_value(obj) : 0;
}

void set_spinbox_data_by_id(lv_obj_t *parent, uint32_t id, int32_t value)
{
    lv_obj_t *obj = find_spinbox_with_id(parent, id);
    if (obj)
        lv_spinbox_set_value(obj, value);
}

void move_left(lv_obj_t *sb) { 
    lv_spinbox_step_next(sb);
 } // select higher digit

void move_right(lv_obj_t *sb)
{
    lv_spinbox_step_prev(sb);
} // select lower digit