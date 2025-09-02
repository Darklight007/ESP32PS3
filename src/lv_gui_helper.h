#pragma once
#include <lvgl.h>
#include <initializer_list>
#include <Arduino.h>

class LVButton
{
public:
    LVButton(lv_obj_t *parent,
             const char *label,
             lv_coord_t x, lv_coord_t y,
             lv_coord_t w, lv_coord_t h,
             lv_style_t *style,
             lv_event_cb_t callback);

    lv_obj_t *get_lv_obj() { return btn_; }
    lv_coord_t pos_x;
    lv_coord_t pos_y;
    lv_coord_t width;
    lv_coord_t height;

    void center()
    {
        lv_obj_center(btn_);
    }

    void centerHorizontally()
    {
        lv_obj_t *parent = lv_obj_get_parent(btn_);
        lv_coord_t y = lv_obj_get_y(btn_);
        lv_obj_align_to(btn_, parent, LV_ALIGN_TOP_MID, 0, y);
    }

    void centerVertically()
    {
        lv_obj_t *parent = lv_obj_get_parent(btn_);
        lv_coord_t x = lv_obj_get_x(btn_);
        lv_obj_align_to(btn_, parent, LV_ALIGN_LEFT_MID, x, 0);
    }

    void set_align(lv_align_t align)
    {
        lv_obj_set_style_align(btn_, align, 0);
    }
    // lv_obj_t *get_lv_obj()
    // {
    //     return btn_;
    // }

private:
    lv_obj_t *btn_{nullptr};
};

class LVLabel
{
public:
    // Creates a label aligned relative to `base` with optional style.
    static lv_obj_t *create(lv_obj_t *parent, const char *labelText, const lv_obj_t *base,
                            lv_coord_t x_ofs, lv_coord_t y_ofs, lv_style_t *style);

    

        // helper (works only if you pass lv_obj_t directly)
    static void set_text(lv_obj_t *lbl, const char *text);
};


class LVLabel_class {
public:
    // Constructor creates the label
    LVLabel_class(lv_obj_t *parent,
            const char *labelText,
            const lv_obj_t *base = nullptr,
            lv_coord_t x_ofs = 0,
            lv_coord_t y_ofs = 0,
            lv_style_t *style = nullptr);

    // API
    void set_text(const char *text);
    void set_color(lv_color_t color);
    void align(lv_align_t align, lv_coord_t x_ofs, lv_coord_t y_ofs); 
    void align_to(lv_align_t align, LVLabel_class base , lv_coord_t x_ofs, lv_coord_t y_ofs) ;

    // Access to raw pointer if needed
    lv_obj_t *get_lv_obj() const { return label; }

    // Define template fully in header!
    template <typename... Args>
    void set_text_fmt(const char *fmt, Args... args) {
        if (label)
            lv_label_set_text_fmt(label, fmt, args...);
    }


private:
    lv_obj_t *label{nullptr};
};



class LVLine2
{
public:
    // Uses caller-provided points[2]. Caller must keep points alive as long as the line exists.
    static lv_obj_t *create(lv_obj_t *parent,
                            const lv_point_t points[2],
                            lv_coord_t width,
                            lv_style_t *style,
                            uint32_t color,
                            lv_coord_t dash_width,
                            lv_coord_t dash_gap,
                            lv_coord_t x_ofs, lv_coord_t y_ofs);
};

class LVLine
{
public:
    // Safer convenience overload: takes exactly 2 points and internally keeps a copy alive
    // until the object is deleted.
    static lv_obj_t *create(lv_obj_t *parent,
                            const std::initializer_list<lv_point_t> &points,
                            lv_coord_t width,
                            lv_style_t *style,
                            uint32_t color,
                            lv_coord_t dash_width,
                            lv_coord_t dash_gap,
                            lv_coord_t x_ofs,
                            lv_coord_t y_ofs);
};

// #ifndef LV_GUI_HELPER_H
// #define LV_GUI_HELPER_H

// #include <lvgl.h>

// class LVButton
// {
// public:
//     LVButton(lv_obj_t *parent,
//              const char *label,
//              lv_coord_t x, lv_coord_t y,
//              lv_coord_t w, lv_coord_t h,
//              lv_style_t *style,
//              lv_event_cb_t callback)
//     {
//         btn = lv_btn_create(parent);
//         lv_obj_t *lbl = lv_label_create(btn);
//         lv_label_set_text(lbl, label);
//         lv_obj_set_pos(btn, x, y);
//         lv_obj_set_size(btn, w, h);
//         lv_obj_center(lbl);

//         if (style)
//         {
//             lv_obj_remove_style(btn, style, LV_STATE_DEFAULT);
//             lv_obj_add_style(btn, style, LV_STATE_DEFAULT);
//         }

//         if (callback)
//             lv_obj_add_event_cb(btn, callback, LV_EVENT_RELEASED, nullptr);
//     }

//     lv_obj_t *get() { return btn; }

// private:
//     lv_obj_t *btn;
// };

// class LVLabel {
// public:
//     static lv_obj_t* create(lv_obj_t* parent, const char* labelText, const lv_obj_t* base,
//                             lv_coord_t x_ofs, lv_coord_t y_ofs, lv_style_t* style) {
//         lv_obj_t* label = lv_label_create(parent);
//         lv_obj_remove_style_all(label);
//         lv_label_set_recolor(label, true);
//         lv_label_set_text(label, labelText);
//         lv_obj_align_to(label, base, LV_ALIGN_OUT_BOTTOM_LEFT, x_ofs, y_ofs);
//         lv_obj_add_style(label, style, LV_STATE_DEFAULT);
//         return label;
//     }
// };
// class LVLine2 {
//     public:
//         static lv_obj_t* create(lv_obj_t* parent,
//                                 const lv_point_t points[2],
//                                 lv_coord_t width,
//                                 lv_style_t* style,
//                                 uint32_t color,
//                                 lv_coord_t dash_width,
//                                 lv_coord_t dash_gap,
//                                 lv_coord_t x_ofs, lv_coord_t y_ofs)
//         {
//             if (!parent || !style) return nullptr;

//             lv_style_set_line_width(style, width);
//             lv_style_set_line_color(style, lv_color_hex(color));
//             lv_style_set_line_rounded(style, true);
//             lv_style_set_line_dash_width(style, dash_width);
//             lv_style_set_line_dash_gap(style, dash_gap);

//             lv_obj_t* line = lv_line_create(parent);
//             lv_line_set_points(line, points, 2);
//             lv_obj_remove_style_all(line);
//             lv_obj_add_style(line, style, 0);
//             lv_obj_align_to(line, parent, LV_ALIGN_TOP_LEFT, x_ofs, y_ofs);

//             return line;
//         }
//     };

// class LVLine {
// public:
//     static lv_obj_t* create(lv_obj_t* parent,
//                             const std::initializer_list<lv_point_t>& points,
//                             lv_coord_t width,
//                             lv_style_t* style,
//                             uint32_t color,
//                             lv_coord_t dash_width,
//                             lv_coord_t dash_gap,
//                             lv_coord_t x_ofs,
//                             lv_coord_t y_ofs)
//     {
//         if (!parent || !style || points.size() != 2) return nullptr; // Safety check: need 2 points

//         lv_style_set_line_width(style, width);
//         lv_style_set_line_color(style, lv_color_hex(color));
//         lv_style_set_line_rounded(style, true);
//         lv_style_set_line_dash_width(style, dash_width);
//         lv_style_set_line_dash_gap(style, dash_gap);

//         lv_obj_t* line = lv_line_create(parent);
//         lv_line_set_points(line, points.begin(), points.size()); // Use initializer_list

//         lv_obj_remove_style_all(line);
//         lv_obj_add_style(line, style, 0);
//         lv_obj_align_to(line, parent, LV_ALIGN_TOP_LEFT, x_ofs, y_ofs);

//         return line;
//     }
// };

// #endif // LV_GUI_HELPER_H