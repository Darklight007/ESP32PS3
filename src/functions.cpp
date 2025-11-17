#include "functions.h"
#include "device.hpp"

// External reference
extern Device PowerSupply;

double scaleVoltage(uint16_t voltage)
{
    return (voltage - PowerSupply.Voltage.adjOffset) / PowerSupply.Voltage.adjFactor;
}

double scaleCurrent(uint16_t current)
{
    return (current - PowerSupply.Current.adjOffset) / PowerSupply.Current.adjFactor;
}

// Debug utility for printing LVGL object type names
typedef struct
{
    const lv_obj_class_t *class_p;
    const char *class_name;
} class_map_t;

static const class_map_t class_map[] = {
    {&lv_obj_class, "lv_obj"},
    {&lv_btn_class, "lv_btn"},
    {&lv_label_class, "lv_label"},
    {&lv_img_class, "lv_img"},
    {&lv_line_class, "lv_line"},
    {&lv_arc_class, "lv_arc"},
    {&lv_bar_class, "lv_bar"},
    {&lv_slider_class, "lv_slider"},
    {&lv_switch_class, "lv_switch"},
    {&lv_checkbox_class, "lv_checkbox"},
    {&lv_list_class, "lv_list"},
    {&lv_dropdown_class, "lv_dropdown"},
    {&lv_roller_class, "lv_roller"},
    {&lv_textarea_class, "lv_textarea"},
    {&lv_keyboard_class, "lv_keyboard"},
    {&lv_table_class, "lv_table"},
    {&lv_tabview_class, "lv_tabview"},
    {&lv_tileview_class, "lv_tileview"},
    {&lv_tileview_tile_class, "lv_tileview_tile"},
    {&lv_led_class, "lv_led"},
    {&lv_imgbtn_class, "lv_imgbtn"},
    {&lv_spinner_class, "lv_spinner"},
    {&lv_msgbox_class, "lv_msgbox"},
    {&lv_colorwheel_class, "lv_colorwheel"},
    {&lv_calendar_class, "lv_calendar"},
    {&lv_meter_class, "lv_meter"},
    {&lv_chart_class, "lv_chart"},
    {&lv_canvas_class, "lv_canvas"},
    {&lv_win_class, "lv_win"},
    {&lv_spangroup_class, "lv_spangroup"},
    {&lv_menu_class, "lv_menu"},
    {&lv_spinbox_class, "lv_spinbox"},
    {&lv_btnmatrix_class, "lv_btnmatrix"},
    {NULL, NULL} // Sentinel
};

void print_obj_type(lv_obj_t *obj)
{
    const lv_obj_class_t *obj_class = lv_obj_get_class(obj);
    const class_map_t *map = class_map;

    while (map->class_p != NULL)
    {
        if (obj_class == map->class_p)
        {
            printf("Object class name: %s\n", map->class_name);
            return;
        }
        map++;
    }

    printf("Unknown object class\n");
}
