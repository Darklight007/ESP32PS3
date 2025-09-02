#include "lv_gui_helper.h"

LVButton::LVButton(lv_obj_t *parent,
                   const char *label,
                   lv_coord_t x, lv_coord_t y,
                   lv_coord_t w, lv_coord_t h,
                   lv_style_t *style,
                   lv_event_cb_t callback)
{

    btn_ = lv_btn_create(parent);
    lv_obj_t *lbl = lv_label_create(btn_);
    lv_label_set_text(lbl, label ? label : "");
    lv_obj_center(lbl);

    lv_obj_set_pos(btn_, x, y);
    lv_obj_set_size(btn_, w, h);
    lv_obj_align_to(btn_, parent, LV_ALIGN_DEFAULT, x, y);

    pos_x = x;
    pos_y = y;
    width = w;
    height = h;

    if (style)
        lv_obj_add_style(btn_, style, LV_PART_MAIN);
    if (callback)
        lv_obj_add_event_cb(btn_, callback, LV_EVENT_RELEASED, nullptr);
}

LVLabel_class::LVLabel_class(lv_obj_t *parent,
                             const char *labelText,
                             const lv_obj_t *base,
                             lv_coord_t x_ofs,
                             lv_coord_t y_ofs,
                             lv_style_t *style)
{
    label = lv_label_create(parent);
    lv_obj_remove_style_all(label);
    lv_label_set_recolor(label, true);
    lv_label_set_text(label, labelText ? labelText : "");

    if (base)
        lv_obj_align_to(label, base, LV_ALIGN_OUT_BOTTOM_LEFT, x_ofs, y_ofs);

    if (style)
        lv_obj_add_style(label, style, LV_PART_MAIN);
}

void LVLabel_class::set_text(const char *text)
{
    if (label)
        lv_label_set_text(label, text);
}





void LVLabel_class::set_color(lv_color_t color)
{
    if (label)
        lv_obj_set_style_text_color(label, color, LV_PART_MAIN);
}

void LVLabel_class::align(lv_align_t align, lv_coord_t x_ofs, lv_coord_t y_ofs)
{
    if (label)
        lv_obj_align(label, align, x_ofs, y_ofs);
}

void LVLabel_class::align_to(lv_align_t align, LVLabel_class base, lv_coord_t x_ofs, lv_coord_t y_ofs)
{
    if (label)
        lv_obj_align(label, align, x_ofs, y_ofs);
    lv_obj_align_to(label, base.get_lv_obj(), align, x_ofs, y_ofs);
}

lv_obj_t *LVLabel::create(lv_obj_t *parent, const char *labelText, const lv_obj_t *base,
                          lv_coord_t x_ofs, lv_coord_t y_ofs, lv_style_t *style)
{
    lv_obj_t *label = lv_label_create(parent);
    lv_obj_remove_style_all(label);
    lv_label_set_recolor(label, true);
    lv_label_set_text(label, labelText ? labelText : "");

    if (base)
        lv_obj_align_to(label, base, LV_ALIGN_OUT_BOTTOM_LEFT, x_ofs, y_ofs);
    if (style)
        lv_obj_add_style(label, style, LV_PART_MAIN);
    return label;
}

void LVLabel::set_text(lv_obj_t *lbl, const char *text)
{
    if (lbl)
        lv_label_set_text(lbl, text);
}

lv_obj_t *LVLine2::create(lv_obj_t *parent,
                          const lv_point_t points[2],
                          lv_coord_t width,
                          lv_style_t *style,
                          uint32_t color,
                          lv_coord_t dash_width,
                          lv_coord_t dash_gap,
                          lv_coord_t x_ofs, lv_coord_t y_ofs)
{
    if (!parent || !style || !points)
        return nullptr;

    lv_style_set_line_width(style, width);
    lv_style_set_line_color(style, lv_color_hex(color));
    lv_style_set_line_rounded(style, true);
    lv_style_set_line_dash_width(style, dash_width);
    lv_style_set_line_dash_gap(style, dash_gap);

    lv_obj_t *line = lv_line_create(parent);
    lv_line_set_points(line, points, 2); // points must outlive line (caller owns)
    lv_obj_remove_style_all(line);
    lv_obj_add_style(line, style, LV_PART_MAIN);
    lv_obj_align_to(line, parent, LV_ALIGN_TOP_LEFT, x_ofs, y_ofs);
    return line;
}

static void line_points_free_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_DELETE)
        return;
    void *pts = lv_event_get_user_data(e);
    if (pts)
        lv_mem_free(pts);
}

lv_obj_t *LVLine::create(lv_obj_t *parent,
                         const std::initializer_list<lv_point_t> &points,
                         lv_coord_t width,
                         lv_style_t *style,
                         uint32_t color,
                         lv_coord_t dash_width,
                         lv_coord_t dash_gap,
                         lv_coord_t x_ofs,
                         lv_coord_t y_ofs)
{
    if (!parent || !style || points.size() != 2)
        return nullptr;

    // Copy the 2 points so they persist for the widget's lifetime.
    auto *pts = static_cast<lv_point_t *>(lv_mem_alloc(sizeof(lv_point_t) * 2));
    if (!pts)
        return nullptr;
    auto it = points.begin();
    pts[0] = *it++;
    pts[1] = *it;

    lv_style_set_line_width(style, width);
    lv_style_set_line_color(style, lv_color_hex(color));
    lv_style_set_line_rounded(style, true);
    lv_style_set_line_dash_width(style, dash_width);
    lv_style_set_line_dash_gap(style, dash_gap);

    lv_obj_t *line = lv_line_create(parent);
    lv_line_set_points(line, pts, 2); // we own memory now
    lv_obj_remove_style_all(line);
    lv_obj_add_style(line, style, LV_PART_MAIN);
    lv_obj_align_to(line, parent, LV_ALIGN_TOP_LEFT, x_ofs, y_ofs);

    // Free the copied points when the object is deleted.
    lv_obj_add_event_cb(line, line_points_free_cb, LV_EVENT_DELETE, pts);
    return line;
}
