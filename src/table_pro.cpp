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


lv_coord_t table_get_row_h(lv_obj_t *table)
{
    const lv_font_t *font = lv_obj_get_style_text_font(table, LV_PART_ITEMS);
    lv_coord_t h = lv_font_get_line_height(font) + lv_obj_get_style_pad_top(table, LV_PART_ITEMS) + lv_obj_get_style_pad_bottom(table, LV_PART_ITEMS);
    //  + lv_obj_get_style_pad_row(table, LV_PART_ITEMS);

    return h;
}


void select_row(lv_obj_t *table, uint16_t cur_row_number, lv_coord_t row_height)
{
    uint16_t row_cnt = lv_table_get_row_cnt(table);

    lv_coord_t scroll_y = lv_obj_get_scroll_y(table);
    lv_coord_t visible_h = lv_obj_get_height(table);
    lv_coord_t y_pos = cur_row_number * row_height;

    if (y_pos < scroll_y)
        lv_obj_scroll_to_y(table, y_pos, LV_ANIM_OFF);
    else if (y_pos + row_height > scroll_y + visible_h)
        lv_obj_scroll_to_y(table, y_pos + row_height - visible_h, LV_ANIM_OFF);

    // lv_obj_invalidate(table);
}



void table_set_selected_row(lv_obj_t *table, uint16_t row)
{
    uint16_t row_cnt = lv_table_get_row_cnt(table);
    if (row < 0 || row > (row_cnt - 1))
        return;
    lv_obj_set_user_data(table, (void *)(uintptr_t)row);
    // select_row(table, row, 2 * 7 + 2 * 5);

    // Serial.println(table_get_row_h(table));
    select_row(table, row, table_get_row_h(table));

    lv_obj_invalidate(table); // redraw -> will highlight via draw cb

    // select_next_row(table, 2 * 7 + 2 * 5);
}



void table_draw_cell_event_cb2(lv_event_t *e)
{
    lv_obj_draw_part_dsc_t *d = (lv_obj_draw_part_dsc_t *)lv_event_get_param(e);
    if (d->part != LV_PART_ITEMS)
        return;

    lv_obj_t *table = lv_event_get_target(e);
    uint16_t col_cnt = lv_table_get_col_cnt(table);

    uint32_t id = d->id; // linear cell id
    uint16_t row = id / col_cnt;

    uint16_t selected_row = (uint16_t)(uintptr_t)lv_obj_get_user_data(table);
    if (row == selected_row)
    {
        // background
        d->rect_dsc->bg_color = lv_color_hex(0x2D6CDF); // blue
        d->rect_dsc->bg_opa = LV_OPA_COVER;

        // optional: rounded and no border
        d->rect_dsc->radius = 4;
        d->rect_dsc->border_opa = LV_OPA_TRANSP;

        // text contrast (if label_dsc exists)
        if (d->label_dsc)
        {
            d->label_dsc->color = lv_color_hex(0xFFFFFF);
        }
    }
}
 

void table_autofit_columns(lv_obj_t *table)
{
    lv_obj_update_layout(table); // ensure styles computed

    const uint16_t rows = lv_table_get_row_cnt(table);
    const uint16_t cols = lv_table_get_col_cnt(table);

    const lv_font_t *font = lv_obj_get_style_text_font(table, LV_PART_ITEMS);
    lv_coord_t lsp = lv_obj_get_style_text_letter_space(table, LV_PART_ITEMS);
    lv_coord_t lsp_line = lv_obj_get_style_text_line_space(table, LV_PART_ITEMS);
    lv_coord_t pad_l = lv_obj_get_style_pad_left(table, LV_PART_ITEMS);
    lv_coord_t pad_r = lv_obj_get_style_pad_right(table, LV_PART_ITEMS);

    lv_coord_t total_w = 0;

    for (uint16_t c = 0; c < cols; ++c)
    {
        lv_coord_t w_max = 0;

        for (uint16_t r = 0; r < rows; ++r)
        {
            const char *txt = lv_table_get_cell_value(table, r, c);
            if (!txt)
                txt = "";

            lv_point_t sz;
            // single-line measurement; no wrapping
            lv_txt_get_size(&sz, txt, font, lsp, lsp_line, LV_COORD_MAX, LV_TEXT_FLAG_NONE);

            lv_coord_t w = sz.x + pad_l + pad_r;
            if (w > w_max)
                w_max = w;
        }

        // (optional) add a tiny margin so text isn't tight
        w_max += 4;

        lv_table_set_col_width(table, c, w_max);
        total_w += w_max;
    }

    // Make the table as wide as needed (enable horizontal scroll if it overflows parent)
    lv_obj_set_width(table, total_w);
    lv_obj_set_scroll_dir(table, LV_DIR_VER | LV_DIR_HOR); // allow horizontal if needed
    lv_obj_invalidate(table);
}
