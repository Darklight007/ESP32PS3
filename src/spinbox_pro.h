#pragma once
#include <lvgl.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// --- public data used by existing code ---
extern lv_obj_t *obj_selected_spinbox;  // legacy global your code checks

// Small struct you were storing in user_data
typedef struct {
    uint8_t cursor_pos;
    uint8_t digit_count;
    uint8_t id;
} SpinboxData;

// --- 1:1 APIs your existing code expects ---
lv_obj_t *spinbox_pro(lv_obj_t *parent, const char *labelText,
                      int32_t range_min, int32_t range_max,
                      uint8_t digit_count, uint8_t separator_position,
                      lv_align_t align, lv_coord_t x_ofs, lv_coord_t y_ofs,
                      lv_coord_t width, uint8_t id);

lv_obj_t *find_spinbox_with_id(lv_obj_t *parent, uint32_t id);
int32_t   get_spinbox_data_by_id(lv_obj_t *parent, uint32_t id);
void      set_spinbox_data_by_id(lv_obj_t *parent, uint32_t id, int32_t value);


void move_left(lv_obj_t *sb);
void move_right(lv_obj_t *sb);   // select lower digit

void remove_red_border(lv_obj_t *spinbox);   // your code calls this
void spinbox_pro_init_styles(void);          // safe to call multiple times


#ifdef __cplusplus
}
#endif


