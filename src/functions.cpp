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

// External references
extern char keyChar;

// Threshold for detecting stable values
#ifndef CHANGE_THRESHOLD
#define CHANGE_THRESHOLD 0.09 / 32 / .5 // Adjust this for desired sensitivity
#endif

void trackLoopExecution(const char *callerName)
{
    static constexpr int loopInterval = 1000; // Time interval in milliseconds
    static unsigned long lastLoopTime = 0;
    static unsigned long loopCount = 0;

    loopCount++;
    unsigned long currentTime = millis();
    if ((currentTime - lastLoopTime) >= loopInterval)
    {
        // Print the caller's name along with the loop count and time
        Serial.printf("\n%s: Loop Count: %5.0f @ %07.2f seconds.%c", callerName, loopCount * 1000.0 / loopInterval, currentTime / 1000.0, keyChar);
        lastLoopTime = currentTime;
        loopCount = 0;
    }
}

double monitorMinChanges(double currentValue, double currentTimeMicros)
{
    static double lastValue = 0.0;
    static bool isFirstRun = true;
    static bool isStable = false;
    static double startStableTime = 0;
    static double endStableTime = 0;
    static double diff_min = 0;

    // For the first run, just initialize lastValue and return
    if (isFirstRun)
    {
        lastValue = currentValue;
        isFirstRun = false;
        return 10.0;
    }

    double diff = fabs(currentValue - lastValue);

    if (diff < CHANGE_THRESHOLD)
    {
        // Value is nearly unchanged
        if (!isStable)
        {
            // Just entered a stable period
            startStableTime = currentTimeMicros;
            Serial.printf("Stable period started at %1.3f\n", startStableTime);
            isStable = true;
        }
        // If already stable, continue without printing
    }

    else
    {
        // Value changed significantly
        if (isStable)
        {
            // We are leaving a stable period
            endStableTime = currentTimeMicros;

            // Print the duration
            double duration = endStableTime - startStableTime;
            isStable = false;
        }
    }

    lastValue = currentValue;
    return fabs(startStableTime - currentTimeMicros);
}
