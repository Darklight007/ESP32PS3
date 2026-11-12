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

void autoScrollY()
{

    double ratio;

    if (PowerSupply.graph.serV->hidden == PowerSupply.graph.serI->hidden &&
        PowerSupply.graph.serV->hidden == false)

        ratio = .5 * ((PowerSupply.Current.measured.Mean() / (PowerSupply.Current.maxValue / PowerSupply.Current.adjFactor) +
                       PowerSupply.Voltage.measured.Mean() / (PowerSupply.Voltage.maxValue / PowerSupply.Voltage.adjFactor)));

    else if (!PowerSupply.graph.serV->hidden)
        ratio = (PowerSupply.Voltage.measured.Mean() / (PowerSupply.Voltage.maxValue / PowerSupply.Voltage.adjFactor));

    else if (!PowerSupply.graph.serI->hidden)
        ratio = (PowerSupply.Current.measured.Mean() / (PowerSupply.Current.maxValue / PowerSupply.Current.adjFactor));

    double calc = (lv_chart_get_zoom_y(PowerSupply.graph.chart) -
                   (ratio) * (lv_chart_get_zoom_y(PowerSupply.graph.chart) + 256)) /
                  2;

    Serial.printf("\nhidden%i ratio:%5.3f  Calc Y:%5.3f", PowerSupply.graph.serV->hidden,
                  ratio, lv_coord_t(calc));

    lv_obj_scroll_to_y(PowerSupply.graph.chart, lv_coord_t(calc), LV_ANIM_OFF);
}

// Graph page, Y key: set the vertical zoom and scroll so the visible part of
// the trace fills the view - V alone, I alone, or both (zoomed out as far as
// needed to show both). V and I have their own axes (-4..32 V, -1..8 A), so
// each point is first mapped to its 0..1 height on the chart.
// Core 1 only (LVGL).
extern lv_obj_t *slider_y;
void fitGraphY()
{
    lv_obj_t *chart = PowerSupply.graph.chart;
    if (!chart)
        return;
    lv_obj_update_layout(chart);

    const uint16_t n = lv_chart_get_point_count(chart);
    const lv_coord_t cw = lv_obj_get_content_width(chart);
    const lv_coord_t ch = lv_obj_get_content_height(chart);
    if (n < 2 || cw <= 0 || ch <= 0)
        return;

    // Points currently on screen horizontally (the X zoom/scroll window)
    const int32_t fullW = (int32_t)cw * lv_chart_get_zoom_x(chart) / 256;
    const int32_t sx = lv_obj_get_scroll_x(chart);
    int32_t i0 = (int64_t)sx * (n - 1) / fullW;
    int32_t i1 = (int64_t)(sx + cw) * (n - 1) / fullW + 1;
    i0 = LV_CLAMP(0, i0, n - 1);
    i1 = LV_CLAMP(0, i1, n - 1);

    float lo = 2.0f, hi = -1.0f;
    auto scan = [&](lv_chart_series_t *ser, float min, float max)
    {
        if (!ser || ser->hidden)
            return;
        const lv_coord_t *y = lv_chart_get_y_array(chart, ser);
        const uint16_t start = lv_chart_get_x_start_point(chart, ser);
        for (int32_t p = i0; p <= i1; p++)
        {
            const lv_coord_t v = y[(start + p) % n];
            if (v == LV_CHART_POINT_NONE)
                continue;
            const float f = (v - min) / (max - min);
            lo = f < lo ? f : lo;
            hi = f > hi ? f : hi;
        }
    };
    scan(PowerSupply.graph.serV, -4000.0f, 32000.0f); // ranges as set in ui_helpers.cpp
    scan(PowerSupply.graph.serI, -1000.0f, 8000.0f);
    if (hi < lo)
        return; // nothing visible

    // Zoom: the trace's height plus 10% margin each side fills the view.
    // Slider range 256 (1x) .. 5120 (20x).
    const float span = (hi - lo) * 1.2f;
    int32_t zoom = span > 0.0f ? (int32_t)(256.0f / span) : 5120;
    zoom = LV_CLAMP(256, zoom, 5120);

    if (slider_y)
        lv_slider_set_value(slider_y, zoom, LV_ANIM_OFF);
    lv_chart_set_zoom_y(chart, (uint16_t)zoom);
    lv_obj_update_layout(chart);

    // Scroll: put the middle of the trace in the middle of the view
    const int32_t fullH = (int32_t)ch * zoom / 256;
    int32_t top = (int32_t)(fullH * (1.0f - (lo + hi) / 2.0f)) - ch / 2;
    top = LV_CLAMP(0, top, fullH - ch);
    lv_obj_scroll_to_y(chart, top, LV_ANIM_OFF);
}

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
        Serial.printf("\n%s: Loop Count: %5.0f @ %07.2f seconds. Last key:  %c",
                      callerName, loopCount * 1000.0 / loopInterval,
                      currentTime / 1000.0,
                    //   0*PowerSupply.Current.adjFactor,
                    //   0* PowerSupply.Current.adjOffset,
                      keyChar
                    );
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
