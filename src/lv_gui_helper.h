#ifndef LV_GUI_HELPER_H
#define LV_GUI_HELPER_H

#include <lvgl.h>

class LVButton
{
public:
    LVButton(lv_obj_t *parent,
             const char *label,
             lv_coord_t x, lv_coord_t y,
             lv_coord_t w, lv_coord_t h,
             lv_style_t *style,
             lv_event_cb_t callback)
    {
        btn = lv_btn_create(parent);
        lv_obj_t *lbl = lv_label_create(btn);
        lv_label_set_text(lbl, label);
        lv_obj_set_pos(btn, x, y);
        lv_obj_set_size(btn, w, h);
        lv_obj_center(lbl);

        if (style)
        {
            lv_obj_remove_style(btn, style, LV_STATE_DEFAULT);
            lv_obj_add_style(btn, style, LV_STATE_DEFAULT);
        }

        if (callback)
            lv_obj_add_event_cb(btn, callback, LV_EVENT_RELEASED, nullptr);
    }

    lv_obj_t *get() { return btn; }

private:
    lv_obj_t *btn;
};


class LVLabel {
public:
    static lv_obj_t* create(lv_obj_t* parent, const char* labelText, const lv_obj_t* base,
                            lv_coord_t x_ofs, lv_coord_t y_ofs, lv_style_t* style) {
        lv_obj_t* label = lv_label_create(parent);
        lv_obj_remove_style_all(label);
        lv_label_set_recolor(label, true);
        lv_label_set_text(label, labelText);
        lv_obj_align_to(label, base, LV_ALIGN_OUT_BOTTOM_LEFT, x_ofs, y_ofs);
        lv_obj_add_style(label, style, LV_STATE_DEFAULT);
        return label;
    }
};
class LVLine2 {
    public:
        static lv_obj_t* create(lv_obj_t* parent,
                                const lv_point_t points[2],
                                lv_coord_t width,
                                lv_style_t* style,
                                uint32_t color,
                                lv_coord_t dash_width,
                                lv_coord_t dash_gap,
                                lv_coord_t x_ofs, lv_coord_t y_ofs)
        {
            if (!parent || !style) return nullptr;
    
            lv_style_set_line_width(style, width);
            lv_style_set_line_color(style, lv_color_hex(color));
            lv_style_set_line_rounded(style, true);
            lv_style_set_line_dash_width(style, dash_width);
            lv_style_set_line_dash_gap(style, dash_gap);
    
            lv_obj_t* line = lv_line_create(parent);
            lv_line_set_points(line, points, 2);
            lv_obj_remove_style_all(line);
            lv_obj_add_style(line, style, 0);
            lv_obj_align_to(line, parent, LV_ALIGN_TOP_LEFT, x_ofs, y_ofs);
    
            return line;
        }
    };
    
class LVLine {
public:
    static lv_obj_t* create(lv_obj_t* parent,
                            const std::initializer_list<lv_point_t>& points,
                            lv_coord_t width,
                            lv_style_t* style,
                            uint32_t color,
                            lv_coord_t dash_width,
                            lv_coord_t dash_gap,
                            lv_coord_t x_ofs,
                            lv_coord_t y_ofs)
    {
        if (!parent || !style || points.size() != 2) return nullptr; // Safety check: need 2 points

        lv_style_set_line_width(style, width);
        lv_style_set_line_color(style, lv_color_hex(color));
        lv_style_set_line_rounded(style, true);
        lv_style_set_line_dash_width(style, dash_width);
        lv_style_set_line_dash_gap(style, dash_gap);

        lv_obj_t* line = lv_line_create(parent);
        lv_line_set_points(line, points.begin(), points.size()); // Use initializer_list

        lv_obj_remove_style_all(line);
        lv_obj_add_style(line, style, 0);
        lv_obj_align_to(line, parent, LV_ALIGN_TOP_LEFT, x_ofs, y_ofs);

        return line;
    }
};

#endif // LV_GUI_HELPER_H