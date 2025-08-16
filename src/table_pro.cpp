#include "table_pro.h"

lv_obj_t* table_pro(lv_obj_t* parent,
                    lv_style_t* style,
                    const lv_font_t* font,
                    lv_align_t align,
                    lv_coord_t pos_x, lv_coord_t pos_y,
                    lv_coord_t width, lv_coord_t height,
                    lv_coord_t pad_left, lv_coord_t pad_ver)
{
    // Create
    lv_obj_t* table = lv_table_create(parent);

    // Size + placement
    lv_obj_set_size(table, width, height);
    lv_obj_align_to(table, parent, align, pos_x, pos_y);

    // Object-level padding (safe even without a style)
    lv_obj_set_style_pad_all(table, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_ver(table, pad_ver, LV_PART_ITEMS);
    lv_obj_set_style_pad_left(table, pad_left, LV_PART_ITEMS);

    // Optional style: caller owns lifetime and must have called lv_style_init(style)
    if (style) {
        if (font) {
            lv_style_set_text_font(style, font);
        }
        // A couple of common text tweaks (adjust/remove to taste)
        lv_style_set_text_letter_space(style, -1);
        lv_style_set_text_color(style, lv_palette_main(LV_PALETTE_GREY));

        // Apply style to the table cells (items). You can also add to MAIN if needed.
        lv_obj_add_style(table, style, LV_PART_ITEMS);
    }

    // Typical for tables; drop if you want scrollbars
    lv_obj_set_scrollbar_mode(table, LV_SCROLLBAR_MODE_OFF);

    return table;
}
