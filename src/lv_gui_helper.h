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


class LVLabelHelper {
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


#endif // LV_GUI_HELPER_H