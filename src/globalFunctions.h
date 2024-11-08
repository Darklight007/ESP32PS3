#pragma once

#include <iostream>
#include <chrono>
#include <thread>
// #include "config.hpp"
#include "input_device.h"
#include "setting_menu.h"
#include <string>

/**********************
 *   PROTOTYPES
 **********************/
void init_display(void);
void init_touch(void);
void ChartUpdate();
void HistPush();
void StatusBar();
void schedule(std::function<void(void)> func, unsigned long &&interval, unsigned long &startTime);
void keyCheckLoop();
void KeyCheckInterval(unsigned long interval);
void DACInterval(unsigned long interval);
void trackLoopExecution(const char *);
void handleHistogramPage(int32_t &encoder1_last_value, int32_t &encoder2_last_value);
void updateObjectParrents();
/**********************
 *   GLOBAL VARIABLES
 **********************/

// volatile long chartInterruptCounter;

int32_t encoder1_value = 0, encoder2_value = 0;

// extern std::map<DEVICE, deviceColors> stateColor;

void getSettingEncoder(lv_indev_drv_t *drv, lv_indev_data_t *data);

static void draw_event_cb(lv_event_t *e);
static void draw_event_cb2(lv_event_t *e);

bool chartPause = false;

#define LV_TICK_CUSTOM 1

/**
 * @brief Construct a new ESP32Encoder object
 * @param ...
 */

/********************************************************/
void btn_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);
    if (code == LV_EVENT_VALUE_CHANGED)
    {
        lv_obj_t *label = lv_obj_get_child(btn, 0);

        if (lv_obj_get_state(btn) & LV_STATE_CHECKED)
        {
            myTone(NOTE_A5, 200);
            lv_label_set_text(label, "ON");
            PowerSupply.setStatus(DEVICE::ON);
            PowerSupply.settingParameters.isPowerSupplyOn = true;
        }
        else
        {
            myTone(NOTE_A3, 200);
            lv_label_set_text(label, "OFF");
            PowerSupply.setStatus(DEVICE::OFF);
            PowerSupply.settingParameters.isPowerSupplyOn = false;
        }
    }
}

/* Source: https://github.com/ankur219/ECG-Arrhythmia-classification/blob/642230149583adfae1e4bd26c6f0e1fd8af2be0e/sample.csv*/
// static lv_coord_t ecg_sample[800] = {0};

static void slider_x_event_cb(lv_event_t *e)
{
    if (!e)
        return; // Safety check
    lv_obj_t *obj = lv_event_get_target(e);
    if (!obj)
        return; // Safety check
    //  int32_t v = lv_slider_get_value(obj);

    // lv_coord_t l = lv_obj_get_scroll_left(chart); unused variable
    // lv_coord_t r = lv_obj_get_scroll_right(chart);unused variable
    // lv_coord_t x = lv_obj_get_scroll_x(PowerSupply.graph.chart);
    // lv_coord_t v0 = lv_chart_get_zoom_x(PowerSupply.graph.chart);

    lv_chart_set_zoom_x(PowerSupply.graph.chart, (uint16_t)lv_slider_get_value(obj));

    // Serial.print("v0:");
    // Serial.print(int(v0));
    // Serial.print("v:");
    // Serial.print(int(v));
    // Serial.print("\tx:");
    // Serial.print(int(x));
    // Serial.print("\tl:");
    // Serial.print(int(l));
    // Serial.print("\tr:");
    // Serial.print(int(r));
    // Serial.print("\tmean");
    // Serial.println(int((l+r)/2));

    // lv_obj_scroll_to_x(PowerSupply.graph.chart, 1 + x * v / v0, LV_ANIM_OFF);
    lv_obj_scroll_to_x(PowerSupply.graph.chart, 32000, LV_ANIM_OFF);
}

static void slider_y_event_cb(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);
    int32_t v = lv_slider_get_value(obj);
    lv_coord_t v0 = lv_chart_get_zoom_y(PowerSupply.graph.chart);
    lv_chart_set_zoom_y(PowerSupply.graph.chart, (uint16_t)v);
    lv_coord_t y = lv_obj_get_scroll_y(PowerSupply.graph.chart);
    // lv_obj_scroll_to_y(PowerSupply.graph.chart, y * v / v0 + 1, LV_ANIM_OFF);
    lv_obj_scroll_to_y(PowerSupply.graph.chart, 32000, LV_ANIM_OFF);
}

static void legend(lv_obj_t *parent, lv_color16_t c1, const char *ser1, lv_color16_t c2, const char *ser2, int x, int y)
{

    int height = 0;
    // int x = 30;

    lv_style_init(&PowerSupply.graph.style_legend1);
    lv_style_set_text_letter_space(&PowerSupply.graph.style_legend1, -2);
    lv_style_set_text_color(&PowerSupply.graph.style_legend1, c1);
    lv_style_set_text_font(&PowerSupply.graph.style_legend1, &Undertale_16b);
    lv_style_set_bg_color(&PowerSupply.graph.style_legend1, lv_palette_darken(LV_PALETTE_GREY, 4));
    lv_style_set_bg_opa(&PowerSupply.graph.style_legend1, LV_OPA_50);
    lv_style_set_border_opa(&PowerSupply.graph.style_legend1, LV_OPA_50);
    lv_style_set_border_width(&PowerSupply.graph.style_legend1, 2);

    static lv_obj_t *label_legend1;
    label_legend1 = lv_label_create(parent);
    lv_label_set_text(label_legend1, "---  V");
    lv_obj_align(label_legend1, LV_ALIGN_DEFAULT, x, y);
    lv_obj_remove_style(label_legend1, &PowerSupply.graph.style_legend1, LV_STATE_DEFAULT);
    lv_obj_add_style(label_legend1, &PowerSupply.graph.style_legend1, LV_STATE_DEFAULT);

    lv_style_init(&PowerSupply.graph.style_legend2);
    lv_style_set_text_letter_space(&PowerSupply.graph.style_legend2, -2);
    lv_style_set_text_color(&PowerSupply.graph.style_legend2, c2);
    lv_style_set_text_font(&PowerSupply.graph.style_legend2, &Undertale_16b);
    // lv_style_set_bg_color(&style_legend2, lv_palette_darken(LV_PALETTE_GREY, 255));
    // lv_style_set_bg_opa(&style_legend2, LV_OPA_50);
    // lv_style_set_border_opa(&style_legend2, LV_OPA_50);
    // lv_style_set_border_width(&style_legend2, 4);

    static lv_obj_t *label_legend2;
    label_legend2 = lv_label_create(label_legend1);
    lv_label_set_text(label_legend2, "---  A");
    lv_obj_align(label_legend2, LV_ALIGN_DEFAULT, 0, height + 2 + 8);
    lv_obj_remove_style(label_legend2, &PowerSupply.graph.style_legend2, LV_STATE_DEFAULT);
    lv_obj_add_style(label_legend2, &PowerSupply.graph.style_legend2, LV_STATE_DEFAULT);
}

static void drag_event_handler(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);

    lv_indev_t *indev = lv_indev_get_act();
    if (indev == NULL)
        return;

    lv_point_t vect;
    lv_indev_get_vect(indev, &vect);

    lv_coord_t x = lv_obj_get_x(obj) + vect.x;
    lv_coord_t y = lv_obj_get_y(obj) + vect.y;
    lv_obj_set_pos(obj, x, y);
}

static void overlay(lv_obj_t *label, const char *text, lv_style_t *style, lv_color16_t c1, int x, int y)
{
    lv_style_init(style);
    lv_style_set_text_letter_space(style, -2);
    lv_style_set_text_color(style, c1);
    lv_style_set_text_font(style, &lv_font_unscii_8);
    lv_style_set_bg_color(style, lv_palette_darken(LV_PALETTE_GREY, 4));
    lv_style_set_bg_opa(style, LV_OPA_50);
    lv_style_set_border_opa(style, LV_OPA_50);
    lv_style_set_border_width(style, 2);

    lv_label_set_text(label, text);
    lv_obj_align(label, LV_ALIGN_DEFAULT, x, y);
    lv_obj_remove_style(label, style, LV_STATE_DEFAULT);
    lv_obj_add_style(label, style, LV_STATE_DEFAULT);
    //   lv_obj_set_size(label, 150, 50);
    lv_obj_add_event_cb(label, drag_event_handler, LV_EVENT_PRESSING, NULL);
}

lv_obj_t *slider_x;
void GraphChart(lv_obj_t *parent, lv_coord_t x, lv_coord_t y)
{ /*Create a chart*/

    PowerSupply.graph.chart = lv_chart_create(parent);
    lv_obj_set_size(PowerSupply.graph.chart, 242, 154);
    lv_obj_align(PowerSupply.graph.chart, LV_ALIGN_DEFAULT, x, y);

    lv_chart_set_range(PowerSupply.graph.chart, LV_CHART_AXIS_PRIMARY_Y, -.3 * 1000, 32000);  // 40000mv
    lv_chart_set_range(PowerSupply.graph.chart, LV_CHART_AXIS_SECONDARY_Y, -.3 * 1000, 8000); // 8000ma
    lv_chart_set_range(PowerSupply.graph.chart, LV_CHART_AXIS_PRIMARY_X, 0, 200);

    lv_chart_set_div_line_count(PowerSupply.graph.chart, 9, 13);
    lv_obj_set_style_text_color(PowerSupply.graph.chart, lv_palette_main(LV_PALETTE_GREY), LV_PART_TICKS);
    lv_chart_set_axis_tick(PowerSupply.graph.chart, LV_CHART_AXIS_PRIMARY_Y, 5, 3, 9, 4, true, 40);
    lv_chart_set_axis_tick(PowerSupply.graph.chart, LV_CHART_AXIS_PRIMARY_X, 5, 3, 7, 10, true, 50);
    lv_chart_set_axis_tick(PowerSupply.graph.chart, LV_CHART_AXIS_SECONDARY_Y, 5, 3, 9, 10, true, 50);

    // /*Do not display points on the data*/
    // lv_obj_set_style_size(PowerSupply.graph.chart, 0, LV_PART_INDICATOR);
    lv_chart_set_type(PowerSupply.graph.chart, LV_CHART_TYPE_LINE);

    lv_chart_set_update_mode(PowerSupply.graph.chart, LV_CHART_UPDATE_MODE_SHIFT);
    lv_obj_add_event_cb(PowerSupply.graph.chart, draw_event_cb2, LV_EVENT_DRAW_PART_BEGIN, NULL);

    PowerSupply.graph.serI = lv_chart_add_series(PowerSupply.graph.chart, lv_palette_main(LV_PALETTE_AMBER), LV_CHART_AXIS_SECONDARY_Y);
    PowerSupply.graph.serV = lv_chart_add_series(PowerSupply.graph.chart, lv_palette_main(LV_PALETTE_BLUE), LV_CHART_AXIS_PRIMARY_Y);

    // uint32_t pcnt = sizeof(ecg_sample) / sizeof(ecg_sample[0]);
    lv_chart_set_point_count(PowerSupply.graph.chart, 600 * 2);
    // lv_chart_set_ext_y_array(PowerSupply.graph.chart, ser, (lv_coord_t *)ecg_sample);

    static lv_style_t style_slider;
    lv_style_init(&style_slider);
    lv_style_set_bg_opa(&style_slider, LV_OPA_50);
    lv_style_set_bg_color(&style_slider, lv_palette_main(LV_PALETTE_LIGHT_GREEN));

    // lv_obj_t *slider_x;
    slider_x = lv_slider_create(parent);

    lv_obj_remove_style(slider_x, &style_slider, LV_STATE_DEFAULT);
    // lv_obj_remove_style_all(slider);
    lv_slider_set_range(slider_x, LV_IMG_ZOOM_NONE, LV_IMG_ZOOM_NONE * 120);
    lv_obj_add_event_cb(slider_x, slider_x_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    lv_obj_add_event_cb(slider_x, slider_x_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_set_size(slider_x, 228, 6);
    lv_obj_align_to(slider_x, PowerSupply.graph.chart, LV_ALIGN_OUT_BOTTOM_MID, 0, -5);
    // return;

    //  lv_obj_add_style(slider, &style_slider, LV_STATE_DEFAULT);
    lv_obj_add_style(slider_x, &style_slider, LV_PART_INDICATOR);
    lv_obj_add_style(slider_x, &style_slider, LV_PART_MAIN);
    lv_obj_add_style(slider_x, &style_slider, LV_PART_KNOB);
    lv_obj_t *slider;
    slider = lv_slider_create(parent);
    lv_slider_set_range(slider, LV_IMG_ZOOM_NONE, LV_IMG_ZOOM_NONE * 20);
    lv_obj_add_event_cb(slider, slider_y_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_set_size(slider, 6, 140);
    lv_obj_align_to(slider, PowerSupply.graph.chart, LV_ALIGN_OUT_RIGHT_MID, -4, +2);

    //  lv_obj_add_style(slider, &style_slider, LV_STATE_DEFAULT);
    lv_obj_add_style(slider, &style_slider, LV_PART_INDICATOR);
    lv_obj_add_style(slider, &style_slider, LV_PART_MAIN);
    lv_obj_add_style(slider, &style_slider, LV_PART_KNOB);

    // Chart Scrollbar
    // static lv_style_t style;
    // lv_style_init(&style);
    // lv_style_set_width(&style, 4);       /*Width of the scrollbar*/
    // lv_style_set_pad_right(&style, 10);  /*Space from the parallel side*/
    // lv_style_set_pad_top(&style, 5);     /*Space from the perpendicular side*/
    // lv_style_set_pad_bottom(&style, 10); /*Space from the perpendicular side*/

    // lv_style_set_radius(&style, 2);
    // lv_style_set_bg_opa(&style, LV_OPA_70);
    // lv_style_set_bg_color(&style, lv_palette_lighten(LV_PALETTE_GREY, 1));
    // lv_style_set_border_color(&style, lv_palette_lighten(LV_PALETTE_GREY, 2));
    // lv_style_set_border_width(&style, 2);
    // lv_style_set_shadow_width(&style, 8);
    // lv_style_set_shadow_spread(&style, 2);
    // lv_style_set_shadow_color(&style, lv_palette_darken(LV_PALETTE_GREY, 1));

    // lv_style_set_transition(&style, &trans);

    /*Make the scrollbars wider and use 100% opacity when scrolled*/
    // static lv_style_t style_scrolled;
    // lv_style_init(&style_scrolled);
    // lv_style_set_width(&style_scrolled, 8);
    // lv_style_set_bg_opa(&style_scrolled, LV_OPA_COVER);

    // lv_obj_remove_style(PowerSupply.graph.chart, &style, LV_STATE_DEFAULT);
    // lv_obj_add_style(PowerSupply.graph.chart, &style, LV_PART_SCROLLBAR);
    // lv_obj_add_style(PowerSupply.graph.chart, &style_scrolled, LV_PART_SCROLLBAR | LV_STATE_SCROLLED);

    // legend(parent, lv_palette_main(LV_PALETTE_BLUE), "V", lv_palette_main(LV_PALETTE_AMBER), "A", 30, 0);

    // lv_obj_t * bed;
    // bed = lv_obj_create(parent);
    // lv_obj_set_size(bed, 100, 30);
    // lv_obj_add_event_cb(bed, drag_event_handler, LV_EVENT_PRESSING, NULL);

    // PowerSupply.Voltage.statLabels.label_fft = lv_label_create(parent);
    // label_graphMenu_IFFT = lv_label_create(parent);

    legend(parent, lv_palette_main(LV_PALETTE_BLUE), "V-Set", lv_palette_main(LV_PALETTE_AMBER), "I-set", 25, 0);

    static lv_style_t style_FFT;
    overlay(PowerSupply.Voltage.statLabels.label_fft, "", &style_FFT, lv_palette_lighten(LV_PALETTE_AMBER, 4), 160, 0);
    overlay(PowerSupply.Current.statLabels.label_fft, "", &style_FFT, lv_palette_lighten(LV_PALETTE_AMBER, 4), 160, 14);

    lv_obj_set_parent(PowerSupply.Voltage.statLabels.label_fft, parent);
    lv_obj_set_parent(PowerSupply.Current.statLabels.label_fft, parent);
    // lv_obj_add_event_cb(PowerSupply.Voltage.statLabels.label_fft, drag_event_handler, LV_EVENT_PRESSING, NULL);

    // /******************************
    //  **   Stats
    //  ******************************/
    // auto stat_ = [](lv_obj_t *parent, lv_coord_t x, lv_coord_t y)
    // {
    //     static lv_style_t style_stats;
    //     lv_style_init(&style_stats);
    //     lv_style_set_text_font(&style_stats, &lv_font_unscii_8);
    //     lv_style_set_text_letter_space(&style_stats, 1);
    //     lv_style_set_text_color(&style_stats, lv_palette_main(LV_PALETTE_GREY));
    //     lv_style_set_text_letter_space(&style_stats, -1);

    //     lv_obj_set_parent(PowerSupply.Voltage.statLabels.label_legend, parent);
    //     lv_label_set_text(PowerSupply.Voltage.statLabels.label_legend, " Set      Mean     STD      Max      Min");
    //     //                                                            "+00.0000 +00.0000 +00.0000 +00.0000 +00.0000
    //     lv_obj_align(PowerSupply.Voltage.statLabels.label_legend, LV_ALIGN_DEFAULT, x, y);
    //     lv_obj_remove_style(PowerSupply.Voltage.statLabels.label_legend, &style_stats, LV_STATE_DEFAULT);
    //     lv_obj_add_style(PowerSupply.Voltage.statLabels.label_legend, &style_stats, LV_STATE_DEFAULT);

    //     // int offset = x;  unused variable
    //     int offsetY = 10 + y;

    //     lv_style_init(&PowerSupply.graph.style_statsVolt);
    //     lv_style_set_text_color(&PowerSupply.graph.style_statsVolt, lv_palette_main(LV_PALETTE_BLUE));

    //     lv_style_init(&PowerSupply.graph.style_statsCurrent);
    //     lv_style_set_text_color(&PowerSupply.graph.style_statsCurrent, lv_palette_main(LV_PALETTE_AMBER));
    //     // Display mean max min ...
    //     auto stat_obj = [](lv_obj_t *parent, DispObjects &dObj, lv_style_t *style_, lv_coord_t x, lv_coord_t y)
    //     {
    //         lv_style_set_text_font(style_, &lv_font_unscii_8);
    //         lv_style_set_text_letter_space(style_, 1);
    //         lv_style_set_text_letter_space(style_, -1);

    //         int space = 61;

    //         lv_obj_remove_style_all(dObj.statLabels.label_unit);
    //         lv_obj_set_parent(dObj.statLabels.label_unit, parent);
    //         lv_label_set_text(dObj.statLabels.label_unit, lv_label_get_text(dObj.label_unit));
    //         lv_obj_align(dObj.statLabels.label_unit, LV_ALIGN_DEFAULT, x + -10, y);
    //         lv_obj_add_style(dObj.statLabels.label_unit, style_, LV_STATE_DEFAULT);

    //         lv_obj_remove_style_all(dObj.statLabels.label_value);
    //         lv_obj_set_parent(dObj.statLabels.label_value, parent);
    //         lv_obj_align(dObj.statLabels.label_value, LV_ALIGN_DEFAULT, 0, -500);
    //         lv_obj_add_style(dObj.statLabels.label_value, style_, LV_STATE_DEFAULT);

    //         lv_obj_remove_style_all(dObj.statLabels.label_setSmallFont);
    //         lv_obj_set_parent(dObj.statLabels.label_setSmallFont, parent);
    //         lv_obj_align(dObj.statLabels.label_setSmallFont, LV_ALIGN_DEFAULT, x + space * 0, y);
    //         lv_obj_add_style(dObj.statLabels.label_setSmallFont, style_, LV_STATE_DEFAULT);

    //         lv_obj_remove_style_all(dObj.statLabels.label_mean);
    //         lv_obj_set_parent(dObj.statLabels.label_mean, parent);
    //         lv_obj_align(dObj.statLabels.label_mean, LV_ALIGN_DEFAULT, x + space * 1, y);
    //         lv_obj_add_style(dObj.statLabels.label_mean, style_, LV_STATE_DEFAULT);

    //         lv_obj_remove_style_all(dObj.statLabels.label_std);
    //         lv_obj_set_parent(dObj.statLabels.label_std, parent);
    //         lv_obj_align(dObj.statLabels.label_std, LV_ALIGN_DEFAULT, x + space * 2 + 4, y);
    //         lv_obj_add_style(dObj.statLabels.label_std, style_, LV_STATE_DEFAULT);

    //         lv_obj_remove_style_all(dObj.statLabels.label_max);
    //         lv_obj_set_parent(dObj.statLabels.label_max, parent);
    //         lv_obj_align(dObj.statLabels.label_max, LV_ALIGN_DEFAULT, x + space * 3, y);
    //         lv_obj_add_style(dObj.statLabels.label_max, style_, LV_STATE_DEFAULT);

    //         lv_obj_remove_style_all(dObj.statLabels.label_min);
    //         lv_obj_set_parent(dObj.statLabels.label_min, parent);
    //         lv_obj_align(dObj.statLabels.label_min, LV_ALIGN_DEFAULT, x + space * 4, y);
    //         lv_obj_add_style(dObj.statLabels.label_min, style_, LV_STATE_DEFAULT);
    //     };

    //     stat_obj(parent, PowerSupply.Voltage, &PowerSupply.graph.style_statsVolt, x - 8, offsetY);
    //     stat_obj(parent, PowerSupply.Current, &PowerSupply.graph.style_statsCurrent, x - 8, offsetY + 10);
    // };
    // stat_(parent, 10, 167);
}

static void draw_event_stat_chart_cb(lv_event_t *e)
{
    lv_obj_draw_part_dsc_t *dsc = lv_event_get_draw_part_dsc(e);

    // Check if the part being drawn is a tick label on the primary X-axis
    if (dsc->part == LV_PART_TICKS && dsc->id == LV_CHART_AXIS_PRIMARY_X && dsc->text)
    {

        int VI_hidden = 0;

        if (PowerSupply.stats.serV->hidden == PowerSupply.stats.serI->hidden &&
            PowerSupply.stats.serV->hidden == false)
            VI_hidden = 3;
        else if (PowerSupply.stats.serV->hidden == false)
            VI_hidden = 1;
        else if (PowerSupply.stats.serI->hidden == false)
            VI_hidden = 2;

        // Number of ticks is 7
        const int numTicks = 3;

        // Arrays to hold tick labels for 7 ticks
        char tickLabels[3][80]; // Adjust size as needed

        // Calculate the step sizes for voltage and current
        double voltageStep = (PowerSupply.Voltage.hist.histWinMax - PowerSupply.Voltage.hist.histWinMin) / (numTicks - 1);
        double currentStep = (PowerSupply.Current.hist.histWinMax - PowerSupply.Current.hist.histWinMin) / (numTicks - 1);

        // Generate tick labels
        for (int i = 0; i < numTicks; i++)
        {
            // **Set labels at positions 1, 2, 4, and 5 to empty strings**
            // if (i == 1 || i == 2 || i == 4 || i == 5)
            // {
            //     strcpy(tickLabels[i], "");
            //     continue; // Skip to the next iteration
            // }

            double voltageValue = PowerSupply.Voltage.hist.histWinMin + i * voltageStep;
            double currentValue = PowerSupply.Current.hist.histWinMin + i * currentStep;

            // Initialize units as empty strings
            char v_unit[3] = "V";
            char c_unit[3] = "A";

            // Format the voltage value with appropriate precision
            char voltageStr[20];
            if (fabs(voltageStep) >= 1.0)
                snprintf(voltageStr, sizeof(voltageStr), "%3.4f", voltageValue);
            else if (fabs(voltageStep) >= 0.1)
                snprintf(voltageStr, sizeof(voltageStr), "%3.1f", voltageValue);
            else if (fabs(voltageStep) >= 0.01)
                snprintf(voltageStr, sizeof(voltageStr), "%3.2f", voltageValue);
            else
            {
                // Convert to millivolts if the step is very small
                voltageValue *= 1000;
                snprintf(voltageStr, sizeof(voltageStr), "%3.2f", voltageValue);
                strcpy(v_unit, "mV");
            }

            // Format the current value with appropriate precision
            char currentStr[20];
            if (fabs(currentStep) >= 1.0)
                snprintf(currentStr, sizeof(currentStr), "%3.0f", currentValue);
            else if (fabs(currentStep) >= 0.1)
                snprintf(currentStr, sizeof(currentStr), "%3.1f", currentValue);
            else if (fabs(currentStep) >= 0.01)
                snprintf(currentStr, sizeof(currentStr), "%3.2f", currentValue);
            else
            {
                // Convert to milliamps if the step is very small
                currentValue *= 1000;
                snprintf(currentStr, sizeof(currentStr), "%3.2f", currentValue);
                strcpy(c_unit, "mA");
            }

            // **Add units only to the middle tick label (position 3)**
            if (i == 1)
            {
                strcat(voltageStr, v_unit);
                strcat(currentStr, c_unit);
            }

            // Combine voltage and current strings into one label

            switch (VI_hidden)
            {
            case 1:
                lv_obj_set_size(PowerSupply.stats.chart, 260, 150);
                snprintf(tickLabels[i], sizeof(tickLabels[i]), "%s", voltageStr);
                break;

            case 2:
                lv_obj_set_size(PowerSupply.stats.chart, 260, 150);
                snprintf(tickLabels[i], sizeof(tickLabels[i]), "%s", currentStr);
                break;

            case 3:
                lv_obj_set_size(PowerSupply.stats.chart, 260, 140);
                snprintf(tickLabels[i], sizeof(tickLabels[i]), "%s\n%s", voltageStr, currentStr);
                break;

            default:
                snprintf(tickLabels[i], sizeof(tickLabels[i]), "%s", "");
                break;
            }
        }

        // **Boundary Check to Prevent Out-of-Bounds Access**
        if (dsc->value >= 0 && dsc->value < numTicks)
        {
            // Set the tick label for the current value
            lv_snprintf(dsc->text, dsc->text_length, "%s", tickLabels[dsc->value]);
        }
        else
        {
            // If out of bounds, set an empty label
            lv_snprintf(dsc->text, dsc->text_length, "");
        }

        // **Change the font size of the tick labels**
        // Ensure that the label_dsc is valid before modifying it
        if (dsc->label_dsc)
        {
            // Set the desired font
            // You can use one of the built-in fonts or a custom font
            // Example using a built-in font: &lv_font_montserrat_14
            dsc->label_dsc->font = &lv_font_montserrat_10; // Replace with your desired font
                                                           // Optionally, adjust other text properties

            switch (VI_hidden)
            {
            case 1:
                dsc->label_dsc->color = lv_palette_main(LV_PALETTE_BLUE);
                break;

            case 2:
                dsc->label_dsc->color = lv_palette_main(LV_PALETTE_AMBER);
                break;

            case 3:

                dsc->label_dsc->color = lv_color_hex(0xFFFFFF);
                break;

            default:
                break;
            }
        }
    }
}

static void draw_event_stat_chart_cb_old(lv_event_t *e)
{
    /*Hook the division lines too*/
    lv_obj_draw_part_dsc_t *dsc = lv_event_get_draw_part_dsc(e);

    // if (!lv_obj_draw_part_check_type(dsc, &lv_chart_class, LV_CHART_DRAW_PART_TICK_LABEL))
    //     return;
    // Serial.println(dsc->id);
    if (dsc->text)
    //   if (dsc->part == LV_PART_MAIN)
    {
        if (dsc->id == LV_CHART_AXIS_PRIMARY_X)
        {
            // Serial.println(dsc->id);
            // Serial.println(PowerSupply.Current.hist.histWinMax);
            static const char *tickLabel[] = {"0.0\n0.0", "5.0\n1.0", "10\n2.0", "15\n3.0", "20\n4.0", "25\n5.0", "30[V]\n6.0[A]"};
            // const char *tickLabel[8][7] = {""};

            // for (int i = 0; i < 8; i++)
            // {
            //     // (char *)malloc(8);
            //     tickLabel[i][0] = "%+03.1f";
            //     // strcpy(tickLabel[i][0], "%+03.1f");
            //     strcat(tickLabel[i][0], PowerSupply.Current.hist.histWinMax);
            // }

            static char str[7][18];

            // static const char *restrict {"%+4.1f"};
            // for (int i = 0; i < 7; i++)
            // {
            //     if (i != 6)

            //     {
            //         if (PowerSupply.Voltage.hist.histWinMax - PowerSupply.Voltage.hist.histWinMin > .6)
            //             std::sprintf(str[i], "%3.1f\n%4.1f",
            //                          i / 6.0 * (PowerSupply.Voltage.hist.histWinMax - PowerSupply.Voltage.hist.histWinMin) + PowerSupply.Voltage.hist.histWinMin,
            //                          i / 6.0 * (PowerSupply.Current.hist.histWinMax - PowerSupply.Current.hist.histWinMin) + PowerSupply.Current.hist.histWinMin);
            //         else
            //             std::sprintf(str[i], "%3.2f\n%4.1f",
            //                          i / 6.0 * (PowerSupply.Voltage.hist.histWinMax - PowerSupply.Voltage.hist.histWinMin) + PowerSupply.Voltage.hist.histWinMin,
            //                          i / 6.0 * (PowerSupply.Current.hist.histWinMax - PowerSupply.Current.hist.histWinMin) + PowerSupply.Current.hist.histWinMin);
            //     }

            //     else
            //     {
            //         if (PowerSupply.Voltage.hist.histWinMax - PowerSupply.Voltage.hist.histWinMin > .6)
            //             std::sprintf(str[i], "%3.1f[V]\n%4.1f[A]",
            //                          i / 6.0 * PowerSupply.Voltage.hist.histWinMax,
            //                          i / 6.0 * PowerSupply.Current.hist.histWinMax);
            //         else
            //             std::sprintf(str[i], "%3.2f[V]\n%4.1f[A]",
            //                          i / 6.0 * PowerSupply.Voltage.hist.histWinMax,
            //                          i / 6.0 * PowerSupply.Current.hist.histWinMax);
            //     }
            //     tickLabel[i] = str[i];
            // }

            // char * a="%+08.4f";
            // strcpy(a, "%+08.4f");
            // strcat(a, lv_label_get_text(label_unit));

            lv_snprintf(dsc->text, dsc->text_length, "%s", tickLabel[dsc->value]);
            // Serial.println(dsc->text);
        }
        // if (dsc->id == LV_CHART_AXIS_SECONDARY_X)
        // {
        //     const char *tickLabel[] = {"0.0", "1.0", "2.0", "3.0", "4.0", "5.0", "6.0[A]"};
        //     lv_snprintf(dsc->text, dsc->text_length, "%s", tickLabel[dsc->value]);
        // }
    }
};

void stat_measure(lv_obj_t *parent, lv_coord_t x, lv_coord_t y)
{

    /******************************}
     **   Stats
     ******************************/
    // auto stat_ = [](lv_obj_t *parent, lv_coord_t x, lv_coord_t y)
    // {
    static lv_style_t style_stats;
    lv_style_init(&style_stats);
    lv_style_set_text_font(&style_stats, &lv_font_unscii_8);
    lv_style_set_text_letter_space(&style_stats, 1);
    lv_style_set_text_color(&style_stats, lv_palette_main(LV_PALETTE_GREY));
    lv_style_set_text_letter_space(&style_stats, -1);

    lv_obj_set_parent(PowerSupply.Voltage.statLabels.label_legend, parent);
    lv_label_set_text(PowerSupply.Voltage.statLabels.label_legend, " Set      Mean     STD      Max      Min");
    //                                                            "+00.0000 +00.0000 +00.0000 +00.0000 +00.0000
    lv_obj_align(PowerSupply.Voltage.statLabels.label_legend, LV_ALIGN_DEFAULT, x, y);
    lv_obj_remove_style(PowerSupply.Voltage.statLabels.label_legend, &style_stats, LV_STATE_DEFAULT);
    lv_obj_add_style(PowerSupply.Voltage.statLabels.label_legend, &style_stats, LV_STATE_DEFAULT);

    // int offset = x;  unused variable
    int offsetY = 10 + y;

    lv_style_init(&PowerSupply.graph.style_statsVolt);
    lv_style_set_text_color(&PowerSupply.graph.style_statsVolt, lv_palette_main(LV_PALETTE_BLUE));

    lv_style_init(&PowerSupply.graph.style_statsCurrent);
    lv_style_set_text_color(&PowerSupply.graph.style_statsCurrent, lv_palette_main(LV_PALETTE_AMBER));
    // Display mean max min ...
    auto stat_obj = [](lv_obj_t *parent, DispObjects &dObj, lv_style_t *style_, lv_coord_t x, lv_coord_t y)
    {
        lv_style_set_text_font(style_, &lv_font_unscii_8);
        lv_style_set_text_letter_space(style_, 1);
        lv_style_set_text_letter_space(style_, -1);

        int space = 61;

        lv_obj_remove_style_all(dObj.statLabels.label_unit);
        lv_obj_set_parent(dObj.statLabels.label_unit, parent);
        lv_label_set_text(dObj.statLabels.label_unit, lv_label_get_text(dObj.label_unit));
        lv_obj_align(dObj.statLabels.label_unit, LV_ALIGN_DEFAULT, x + -10, y);
        lv_obj_add_style(dObj.statLabels.label_unit, style_, LV_STATE_DEFAULT);

        lv_obj_remove_style_all(dObj.statLabels.label_value);
        lv_obj_set_parent(dObj.statLabels.label_value, parent);
        lv_obj_align(dObj.statLabels.label_value, LV_ALIGN_DEFAULT, 0, -500);
        lv_obj_add_style(dObj.statLabels.label_value, style_, LV_STATE_DEFAULT);

        lv_obj_remove_style_all(dObj.statLabels.label_setSmallFont);
        lv_obj_set_parent(dObj.statLabels.label_setSmallFont, parent);
        lv_obj_align(dObj.statLabels.label_setSmallFont, LV_ALIGN_DEFAULT, x + space * 0, y);
        lv_obj_add_style(dObj.statLabels.label_setSmallFont, style_, LV_STATE_DEFAULT);

        lv_obj_remove_style_all(dObj.statLabels.label_mean);
        lv_obj_set_parent(dObj.statLabels.label_mean, parent);
        lv_obj_align(dObj.statLabels.label_mean, LV_ALIGN_DEFAULT, x + space * 1, y);
        lv_obj_add_style(dObj.statLabels.label_mean, style_, LV_STATE_DEFAULT);

        lv_obj_remove_style_all(dObj.statLabels.label_std);
        lv_obj_set_parent(dObj.statLabels.label_std, parent);
        lv_obj_align(dObj.statLabels.label_std, LV_ALIGN_DEFAULT, x + space * 2 + 4, y);
        lv_obj_add_style(dObj.statLabels.label_std, style_, LV_STATE_DEFAULT);

        lv_obj_remove_style_all(dObj.statLabels.label_max);
        lv_obj_set_parent(dObj.statLabels.label_max, parent);
        lv_obj_align(dObj.statLabels.label_max, LV_ALIGN_DEFAULT, x + space * 3, y);
        lv_obj_add_style(dObj.statLabels.label_max, style_, LV_STATE_DEFAULT);

        lv_obj_remove_style_all(dObj.statLabels.label_min);
        lv_obj_set_parent(dObj.statLabels.label_min, parent);
        lv_obj_align(dObj.statLabels.label_min, LV_ALIGN_DEFAULT, x + space * 4, y);
        lv_obj_add_style(dObj.statLabels.label_min, style_, LV_STATE_DEFAULT);
    };

    stat_obj(parent, PowerSupply.Voltage, &PowerSupply.graph.style_statsVolt, x - 8, offsetY);
    stat_obj(parent, PowerSupply.Current, &PowerSupply.graph.style_statsCurrent, x - 8, offsetY + 10);
    // };
    // stat_(parent, 10, 167);
}

void StatsChart(lv_obj_t *parent, lv_coord_t x, lv_coord_t y)
{ /*Create a chart*/

    PowerSupply.stats.chart = lv_chart_create(parent);
    lv_obj_set_size(PowerSupply.stats.chart, 260, 140);
    lv_obj_center(PowerSupply.stats.chart);
    lv_obj_align(PowerSupply.stats.chart, LV_ALIGN_DEFAULT, x, y + 0);

    lv_chart_set_range(PowerSupply.stats.chart, LV_CHART_AXIS_PRIMARY_Y, 0, 160);   // 40000mv
    lv_chart_set_range(PowerSupply.stats.chart, LV_CHART_AXIS_SECONDARY_Y, 0, 160); // 8000ma
    lv_chart_set_range(PowerSupply.stats.chart, LV_CHART_AXIS_PRIMARY_X, 0, 350);
    // lv_chart_set_range(PowerSupply.stats.chart, LV_CHART_AXIS_SECONDARY_X, -10, 50);

    lv_chart_set_div_line_count(PowerSupply.stats.chart, 5, 13);
    lv_obj_set_style_text_color(PowerSupply.stats.chart, lv_palette_main(LV_PALETTE_GREY), LV_PART_TICKS);

    lv_chart_set_axis_tick(PowerSupply.stats.chart, LV_CHART_AXIS_PRIMARY_Y, 5, 3, 5, 4, true, 40);
    lv_chart_set_axis_tick(PowerSupply.stats.chart, LV_CHART_AXIS_PRIMARY_X, 5, 3, 3, 10, true, 50);
    // lv_chart_set_axis_tick(PowerSupply.stats.chart, LV_CHART_AXIS_SECONDARY_X, 5, 3, 7, 10, true, 50);

    lv_chart_set_type(PowerSupply.stats.chart, LV_CHART_TYPE_LINE);
    lv_chart_set_update_mode(PowerSupply.stats.chart, LV_CHART_UPDATE_MODE_CIRCULAR);
    lv_obj_add_event_cb(PowerSupply.stats.chart, draw_event_stat_chart_cb, LV_EVENT_DRAW_PART_BEGIN, NULL);
    lv_obj_set_style_line_width(PowerSupply.stats.chart, 2, LV_PART_ITEMS);

    // /*Do not display points on the data*/
    lv_obj_set_style_size(PowerSupply.stats.chart, 0, LV_PART_INDICATOR);

    PowerSupply.stats.serI = lv_chart_add_series(PowerSupply.stats.chart, lv_palette_main(LV_PALETTE_AMBER), LV_CHART_AXIS_SECONDARY_Y);
    PowerSupply.stats.serV = lv_chart_add_series(PowerSupply.stats.chart, lv_palette_main(LV_PALETTE_BLUE), LV_CHART_AXIS_PRIMARY_Y);

    lv_chart_set_point_count(PowerSupply.stats.chart, 350);
    PowerSupply.stats.serI->y_points = PowerSupply.Current.hist.data;
    PowerSupply.stats.serV->y_points = PowerSupply.Voltage.hist.data;

    legend(parent, lv_palette_main(LV_PALETTE_BLUE), "V", lv_palette_main(LV_PALETTE_AMBER), "A", 25, 0);
}

static void draw_event_cb2(lv_event_t *e)
{
    // lv_obj_t *obj = lv_event_get_target(e);
    /*Hook the division lines too*/
    lv_obj_draw_part_dsc_t *dsc = lv_event_get_draw_part_dsc(e);
    // if (!lv_obj_draw_part_check_type(dsc, &lv_chart_class, LV_CHART_DRAW_PART_TICK_LABEL))
    //     return;

    /*Hook the division lines too*/
    if (dsc->part == LV_PART_MAIN)
    {
        if (dsc->line_dsc == NULL || dsc->p1 == NULL || dsc->p2 == NULL)
            return;

        dsc->line_dsc->color = lv_palette_main(LV_PALETTE_GREY);
        /*Vertical line*/
        if (dsc->p1->x == dsc->p2->x)
        {

            if (dsc->id == 0)
            {
                dsc->line_dsc->width = 1;
                dsc->line_dsc->dash_gap = 0;
                dsc->line_dsc->dash_width = 0;
                // dsc->line_dsc->color = lv_palette_main(LV_PALETTE_BLUE);
            }
            // else if (dsc->id == 20)
            // {
            //     dsc->line_dsc->width = 7;
            //     dsc->line_dsc->dash_gap = 0;
            //     dsc->line_dsc->dash_width = 0;
            //     dsc->line_dsc->color = lv_palette_main(LV_PALETTE_AMBER);
            // }
            else
            {
                dsc->line_dsc->width = 1;
                dsc->line_dsc->dash_gap = 5;
                dsc->line_dsc->dash_width = 5;
            }

            return;
        }
        /*Horizontal line*/
        else
        {
            if (dsc->id == 8)
            {
                dsc->line_dsc->width = 1;
                dsc->line_dsc->dash_gap = 0;
            }
            else
            {
                dsc->line_dsc->width = 1;
                dsc->line_dsc->dash_gap = 5;
                dsc->line_dsc->dash_width = 5;
            }
            return;
        }
    }
    if (dsc->text)
    {

        if (dsc->id == LV_CHART_AXIS_PRIMARY_X)
        {
            static char c[6][6];
            static uint16_t pointCount = lv_chart_get_point_count(PowerSupply.graph.chart);

            // sprintf(str, "-%i -%i -%i -%i -%i -%i -%i",double( v ));
            // string t = to_string(N);
            // char const *n_char = t.c_str();

            std::sprintf(c[5], "%d", pointCount);
            std::sprintf(c[4], "%d", pointCount * 5 / 6);
            std::sprintf(c[3], "%d", pointCount * 4 / 6);
            std::sprintf(c[2], "%d", pointCount * 3 / 6);
            std::sprintf(c[1], "%d", pointCount * 2 / 6);
            std::sprintf(c[0], "%d", pointCount * 1 / 6);

            static char *tickLabel[] = {c[5], c[4], c[3], c[2], c[1], c[0], "pts"};

            // const char *tickLabel[] = {"-30.0", "-25.0", "-20.0", "-15.0", "-10.0", "-5.0", "0.0[s]"};

            lv_snprintf(dsc->text, dsc->text_length, "%s", tickLabel[dsc->value]);
        }
        else if (dsc->id == LV_CHART_AXIS_SECONDARY_Y)
        {
            static int i = 0;

            static char *tickLabel_sy[] = {"8.0A", "7.0", "6.0", "5.0", "4.0", "3.0", "2.0", "1.0", "0.0"};

            if (strcmp(dsc->text, "8000") == 0)
                i = 0;
            lv_snprintf(dsc->text, dsc->text_length, "%s", tickLabel_sy[i++]);
        }

        else if (dsc->id == LV_CHART_AXIS_PRIMARY_Y)
        {
            static int i = 0;
            static char *tickLabel_y[] = {"32.0\nVolt", "28.0", "24.0", "20.0", "16.0", "12.0", "8.0", "4.0", "0.0"};

            if (strcmp(dsc->text, "32000") == 0)
                i = 0;
            lv_snprintf(dsc->text, dsc->text_length, "%s", tickLabel_y[i++]);
        }
    }
}

void SetupOVP()
{
    static lv_style_t style_shadow;
    lv_style_init(&style_shadow);

    lv_style_set_shadow_width(&style_shadow, 4);
    lv_style_set_shadow_spread(&style_shadow, 2);
    lv_style_set_shadow_color(&style_shadow, lv_palette_main(LV_PALETTE_AMBER));

    lv_style_set_width(&style_shadow, 38);
    lv_style_set_height(&style_shadow, 120);
    lv_style_set_pad_all(&style_shadow, 0);

    lv_style_set_bg_color(&style_shadow, PowerSupply.stateColor[DEVICE::ON].pageColor);
    lv_style_set_x(&style_shadow, 256);
    lv_style_set_y(&style_shadow, 20);

    lv_style_set_border_width(&style_shadow, 2);
    lv_style_set_border_opa(&style_shadow, 100);
    lv_style_set_border_color(&style_shadow, lv_palette_main(LV_PALETTE_BLUE));

    lv_obj_t *obj = lv_obj_create(PowerSupply.page[2]);
    // lv_obj_set_parent(obj, parent);
    lv_obj_add_style(obj, &style_shadow, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);

    static lv_style_t style_ovctp;
    lv_style_init(&style_ovctp);
    lv_obj_t *ovpLabel = lv_label_create(obj);
    // lv_obj_center(ovpLabel);
    lv_obj_align(ovpLabel, LV_ALIGN_CENTER, 0, -54);

    lv_label_set_text(ovpLabel, "OVP");
    lv_style_set_text_font(&style_ovctp, &ATARISTOCRAT_16bt);
    lv_obj_add_style(ovpLabel, &style_ovctp, LV_STATE_DEFAULT);
};
// Code to run a screen calibration, not needed when calibration values set in setup()
void touch_calibrate()
{
    uint16_t calData[5];
    uint8_t calDataOK = 0;

    // Calibrate
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(20, 0);
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    tft.println("Touch corners as indicated");

    tft.setTextFont(1);
    tft.println();

    tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);

    Serial.println();
    Serial.println();
    Serial.println("// Use this calibration code in setup():");
    Serial.print("  uint16_t calData[5] = ");
    Serial.print("{ ");

    for (uint8_t i = 0; i < 5; i++)
    {
        Serial.print(calData[i]);
        if (i < 4)
            Serial.print(", ");
    }

    Serial.println(" };");
    Serial.print("  tft.setTouch(calData);");
    Serial.println();
    Serial.println();

    tft.fillScreen(TFT_BLACK);

    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.println("Calibration complete!");
    tft.println("Calibration code sent to Serial port.");

    delay(4000);
}
// Touch VAriables
struct TouchAttr_
{
    uint16_t x = 0, y = 0; // To store the touch coordinates
    bool pressed;
    // uint32_t startTime;
    // uint32_t stopTime;
    // uint16_t x0 = 0, y0 = 0; // To store the first touch coordinates
    // uint16_t x1 = 0, y1 = 0; // To store the last touch coordinates
    // int xDir;
    // int yDir;

    bool getTouched(TFT_eSPI t)
    {
        return (t.getTouch(&x, &y));
    }

} TouchAttr;

/*Read the touchpad*/
void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
    //   uint16_t x = 0, y = 0; // To store the touch coordinates

    // Pressed will be set true is there is a valid touch on the screen
    //   bool pressed = tft.getTouch(&x, &y);
    //     Serial.println(pressed);
    //   // Draw a white spot at the detected coordinates
    //   if (pressed) {
    //     tft.fillCircle(x, y, 2, TFT_WHITE);
    //     Serial.print("x,y = ");
    //     Serial.print(x);
    //     Serial.print(",");
    //     Serial.println(y);
    //   }

    // uint16_t touchX, touchY; unused variable
    // bool touched = tft.getTouch(&touchX, &touchY, 600);
    // Serial.print("\nTouches!!!! ");
    if (TouchAttr.getTouched(tft))
    {
        data->state = LV_INDEV_STATE_PR;

        /*Set the coordinates*/
        data->point.x = TouchAttr.x;
        data->point.y = TouchAttr.y;

        // Serial.print("Data x ");
        // Serial.println(TouchAttr.x);

        // Serial.print("Data y ");
        // Serial.println(TouchAttr.y);
        // myTone(NOTE_A4, 5);
        // lv_tick_inc(5);
    }
    else
    {
        data->state = LV_INDEV_STATE_REL;
    }
}

/// @brief
void init_touch()
{
    /*Initialize the (dummy) input device driver*/
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touchpad_read;
    lv_indev_drv_register(&indev_drv);
}

void Task_BarGraph(void *pvParameters)
{
    for (;;)
    {

        if (Tabs::getCurrentPage() != 2)
        // trackLoopExecution(__func__);
        {
            vTaskDelay(1);
            continue;
        }

        // if (!lvglIsBusy)
        {
            PowerSupply.Voltage.barUpdate();
            PowerSupply.Current.barUpdate();
            //   getSettingEncoder(NULL, NULL);
        }
        // else
        // {
        //     toneOff();
        //     vTaskDelay(3);
        // }
        // trackLoopExecution(__func__);
    }
}

void Task_ADC(void *pvParameters)
{

    // Need to refresh for the first time when boot
    encoder1Flag = 1;
    encoder2Flag = 1;
    PowerSupply.Voltage.SetEncoderUpdate();
    PowerSupply.Current.SetEncoderUpdate();
    // Serial.print("\nTask1 running on core ");
    // Serial.println(xPortGetCoreID());
    //   const size_t stackSize = (size_t)pvParameters;
    // const size_t stackUsage = stackSize - uxTaskGetStackHighWaterMark(NULL);
    // printf("Stack usage: %d bytes\n", stackUsage);

    // ************************ Temperature DS18B20 Sensor ********************************************************
    for (;;)
    {
        // if (lvglIsBusy)
        // {

        //  toneOff();
        // vTaskDelay(1);

        // }
        // Serial.printf("\nMeasured adcDataReady :%i",adcDataReady);
        if (!adcDataReady)
        {
            toneOff();

            // if (!lvglIsBusy)

            if (wireConnected)
            {
                isMyTextBoxHidden = lv_obj_has_flag(myTextBox, LV_OBJ_FLAG_HIDDEN);
                if (!isMyTextBoxHidden)

                    KeyCheckInterval(10);

                else
                    KeyCheckInterval(105);
            }
            getSettingEncoder(NULL, NULL);

            // DACInterval(49);

            static unsigned long timer_ = {0};
            schedule([]
                     { PowerSupply.DACUpdate(); },
                     100, timer_);

            // trackLoopExecution(__func__);

            // if (Tabs::getCurrentPage() == 2)
            //     vTaskDelay(1);
            // else if (Tabs::getCurrentPage() == 1)
            //     vTaskDelay(5);
            //  vTaskDelay(1);
            continue;
        }
        // if (xSemaphoreTake(timerSemaphore, portMAX_DELAY) != pdTRUE)
        // {
        //     vTaskDelay(1);
        //     continue;
        // }

        // New ADC sample is ready
        if (Tabs::getCurrentPage() == 1)
        {
            if (adcDataReady && PowerSupply.adc.busyChannel == VOLTAGE)
                V.shift(); // Shift for new sample

            if (adcDataReady && PowerSupply.adc.busyChannel == CURRENT)
                I.shift(); // Shift for new sample
        }

        PowerSupply.readVoltage();
        PowerSupply.readCurrent();
        PowerSupply.Power.measureUpdate(PowerSupply.Current.measured.Mean() * PowerSupply.Voltage.measured.Mean());
        if (Tabs::getCurrentPage() != 2)
        {
            PowerSupply.Voltage.StatisticsUpdate(PowerSupply.Voltage.measured.value);
            PowerSupply.Current.StatisticsUpdate(PowerSupply.Current.measured.value);
        }
        static bool lastCCCVStatus = false;

        if (lastCCCVStatus != digitalRead(PowerSupply.CCCVPin))
        {
            lastCCCVStatus = digitalRead(PowerSupply.CCCVPin);
            // myTone(NOTE_A4, 50);
        }

        HistPush();

        if (!lvglIsBusy && Tabs::getCurrentPage() == 0)

            if (!lvglChartIsBusy)
            {
                lvglChartIsBusy = true;
                lv_chart_refresh(PowerSupply.stats.chart);
                lvglChartIsBusy = false;
            }

        static unsigned long NoAvgTime;

        if (!lvglIsBusy && !chartPause)
            schedule([]
                     {
                            if (!lvglChartIsBusy)
         {
                          lvglChartIsBusy = true;
                         lv_chart_set_next_value(PowerSupply.graph.chart, PowerSupply.graph.serV, PowerSupply.Voltage.measured.value * 1000.0);
                         lv_chart_set_next_value(PowerSupply.graph.chart, PowerSupply.graph.serI, PowerSupply.Current.measured.value * 1000.0);
                         lvglChartIsBusy = false; 
        } },
                     PowerSupply.Voltage.measured.NofAvgs * 1000 / /* pixels per point*/ ((double)PowerSupply.adc.realADCSpeed), NoAvgTime);

        if (Tabs::getCurrentPage() == 1)
        {
            if (V.sampleReady)
            {
                V.push(PowerSupply.Voltage.measured.Mean());
                V.sampleReady = false;
            }

            if (I.sampleReady)
            {
                I.push(PowerSupply.Current.measured.Mean());
                I.sampleReady = false;
            }
        }
    }
}

//*******************
// static lv_obj_t *list1;
// // lv_obj_t *btn;
// static void event_handler(lv_event_t *e)
// {
//     lv_event_code_t code = lv_event_get_code(e);
//     lv_obj_t *obj = lv_event_get_target(e);
//     if (code == LV_EVENT_CLICKED)
//     {
//         LV_LOG_USER("Clicked: %s", lv_list_get_btn_text(list1, obj));
//     }
// }

static void sw_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *sw = lv_event_get_target(e);

    if (code == LV_EVENT_VALUE_CHANGED)
    {
        lv_obj_t *list = (lv_obj_t *)lv_event_get_user_data(e);

        if (lv_obj_has_state(sw, LV_STATE_CHECKED))
            lv_obj_add_flag(list, LV_OBJ_FLAG_SCROLL_ONE);
        else
            lv_obj_clear_flag(list, LV_OBJ_FLAG_SCROLL_ONE);
    }
}

lv_obj_t *ta;
lv_obj_t *unit_label;
lv_obj_t *btnm;

static void textarea_event_handler(lv_event_t *e)
{
    ta = lv_event_get_target(e);
    // LV_LOG_USER("Enter was pressed. \nThe current text is: \"%s\"", lv_textarea_get_text(ta));
}

static void btnm_event_handler(lv_event_t *e)
{
    // myTone(NOTE_A4, 10);
    static bool flag_dec = 0;
    lv_obj_t *obj = lv_event_get_target(e);
    // ta = (lv_obj_t *)lv_event_get_user_data(e);
    const char *txt = lv_btnmatrix_get_btn_text(obj, lv_btnmatrix_get_selected_btn(obj));

    // LV_LOG_USER("Pressed: \"%s\"", txt);

    if (strcmp(txt, LV_SYMBOL_BACKSPACE) == 0)
    {
        if (strlen(lv_textarea_get_text(ta)) == 0)
        {
            lv_obj_add_flag(myTextBox, LV_OBJ_FLAG_HIDDEN);
            ismyTextHiddenChange = true;
        }
        lv_textarea_del_char(ta);
    }
    else if (strcmp(txt, LV_SYMBOL_NEW_LINE) == 0)
        lv_event_send(ta, LV_EVENT_READY, NULL);
    else if (strcmp(txt, "E") == 0)
    {
        lv_obj_add_flag(myTextBox, LV_OBJ_FLAG_HIDDEN);
        ismyTextHiddenChange = true;
    }
    else if (strcmp(txt, "Clr") == 0)
    {
        for (int i = 0; i < 100; i++)
            lv_textarea_del_char(ta);
    }

    else if (strcmp(txt, ".") == 0)
    {
        const char *ta_txt = lv_textarea_get_text(ta);
        for (int i = 0; i < strlen(lv_textarea_get_text(ta)); i++)
        {
            if (ta_txt[i] == '.')
                return;
        }
        lv_textarea_add_text(ta, txt);
    }

    else if (strcmp(txt, "V") == 0)
    {
        lv_obj_add_flag(myTextBox, LV_OBJ_FLAG_HIDDEN);
        myTone(NOTE_A5, 100);
        const char *txt = lv_textarea_get_text(ta);
        PowerSupply.Voltage.SetUpdate(round(strtod(txt, NULL) * 2000.0) / 2000.0 - PowerSupply.Voltage.adjOffset);
        lv_textarea_set_text(ta, "");
        ismyTextHiddenChange = true;
    }

    else if (strcmp(txt, "mV") == 0)
    {
        lv_obj_add_flag(myTextBox, LV_OBJ_FLAG_HIDDEN);
        myTone(NOTE_A5, 100);
        const char *txt = lv_textarea_get_text(ta);
        PowerSupply.Voltage.SetUpdate(round(strtod(txt, NULL) * 2.0) / 2000.0 - PowerSupply.Voltage.adjOffset);
        lv_textarea_set_text(ta, "");
        ismyTextHiddenChange = true;
    }

    else if (strcmp(txt, "A") == 0)
    {
        lv_obj_add_flag(myTextBox, LV_OBJ_FLAG_HIDDEN);
        myTone(NOTE_A5, 100);
        const char *txt = lv_textarea_get_text(ta);
        PowerSupply.Current.SetUpdate(strtod(txt, NULL) - PowerSupply.Current.adjOffset);
        lv_textarea_set_text(ta, "");
        ismyTextHiddenChange = true;
    }

    else if (strcmp(txt, "mA") == 0)
    {
        lv_obj_add_flag(myTextBox, LV_OBJ_FLAG_HIDDEN);
        myTone(NOTE_A5, 100);
        const char *txt = lv_textarea_get_text(ta);
        PowerSupply.Current.SetUpdate(strtod(txt, NULL) / 1000.0 - PowerSupply.Current.adjOffset);
        lv_textarea_set_text(ta, "");
        ismyTextHiddenChange = true;
    }
    else
        lv_textarea_add_text(ta, txt);
}
static void key_event_handler(uint16_t btn_id)
{
    lv_obj_clear_flag(myTextBox, LV_OBJ_FLAG_HIDDEN);
    lv_btnmatrix_set_selected_btn(btnm, btn_id);
    lv_event_send(btnm, LV_EVENT_VALUE_CHANGED, NULL);
    lv_label_set_text(unit_label, "mV/V/mA/A");
    ismyTextHiddenChange = true;
    //  LV_LOG_USER("Enter was pressed. \nThe current text is: \"%s\"", lv_textarea_get_text(ta));
}

static void key_event_handler_readBack_clb(const char *txt)
{
    lv_textarea_set_text(ta, txt);
    lv_label_set_text(unit_label, "");
}
static void key_event_handler_readBack(DispObjects dp)
{
    if (strcmp(lv_label_get_text(dp.label_unit), "V") == 0)
    {
        lv_textarea_set_text(ta, std::to_string(dp.adjValue + PowerSupply.Voltage.adjOffset).c_str());
        lv_label_set_text(unit_label, "V");
    }
    else
    {
        lv_textarea_set_text(ta, std::to_string(dp.adjValue + PowerSupply.Current.adjOffset).c_str());
        lv_label_set_text(unit_label, "A");
    }

    lv_textarea_del_char(ta);
    lv_textarea_del_char(ta);
}

static void key_event_handler_readBack_k(DispObjects dp)
{
    if (strcmp(lv_label_get_text(dp.label_unit), "V") == 0)
    {
        lv_textarea_set_text(ta, std::to_string((dp.adjValue + PowerSupply.Voltage.adjOffset) * 1000.0).c_str());
        lv_label_set_text(unit_label, "mV");
    }
    else
    {
        lv_textarea_set_text(ta, std::to_string((dp.adjValue + PowerSupply.Current.adjOffset) * 1000.0).c_str());
        lv_label_set_text(unit_label, "mA");
    }

    lv_textarea_del_char(ta);
    lv_textarea_del_char(ta);
    lv_textarea_del_char(ta);
    lv_textarea_del_char(ta);
    lv_textarea_del_char(ta);
}

static lv_style_t style_font_unit;

void textarea(lv_obj_t *parent)
{
    static lv_style_t style_font;

    static lv_style_t style_font_btnm;
    static lv_style_t style_backgound;

    lv_style_init(&style_font);
    // lv_style_set_bg_opa(&style_font, LV_OPA_50);
    lv_style_set_bg_color(&style_font, lv_palette_lighten(LV_PALETTE_GREEN, 1));

    lv_style_set_text_font(&style_font, &graph_R_35);
    lv_style_set_text_color(&style_font, lv_color_hex(0x010141)); // lv_palette_darken(LV_PALETTE_RED,4));

    //             style.init();
    // style.set_border_color(lv.palette_main(lv.PALETTE.BLUE));
    // scr = lv.obj();
    // ta = lv.textarea(scr);
    // ta.add_style(style, lv.PART.CURSOR | lv.STATE.FOCUSED);
    // lv.scr_load(scr);
    myTextBox = lv_obj_create(lv_scr_act());
    lv_obj_set_size(myTextBox, 320, 240);
    lv_style_init(&style_backgound);

    lv_style_set_bg_opa(&style_backgound, LV_OPA_80);
    lv_obj_remove_style(myTextBox, &style_backgound, LV_STATE_DEFAULT);
    lv_obj_add_style(myTextBox, &style_backgound, LV_STATE_DEFAULT);

    ta = lv_textarea_create(myTextBox);
    lv_textarea_set_one_line(ta, true);
    lv_obj_align(ta, LV_ALIGN_TOP_MID, 0, 50);
    lv_obj_add_event_cb(ta, textarea_event_handler, LV_EVENT_READY, ta);

    lv_obj_remove_style(ta, &style_font, LV_STATE_DEFAULT);
    lv_obj_add_style(ta, &style_font, LV_STATE_DEFAULT);
    lv_obj_add_state(ta, LV_STATE_FOCUSED); /*To be sure the cursor is visible*/

    lv_obj_t *unit_box = lv_obj_create(myTextBox);
    lv_obj_align(unit_box, LV_ALIGN_TOP_RIGHT, -15, 24);
    // lv_obj_remove_style_all(unit_box);

    // lv_obj_remove_style(unit_box, &style_font_unit, LV_STATE_DEFAULT);
    lv_obj_set_size(unit_box, 128, 26);

    lv_style_init(&style_font_unit);
    // lv_style_set_bg_color(&style_font_unit, lv_palette_lighten(LV_PALETTE_GREEN, 1));
    lv_style_set_text_font(&style_font_unit, &graph_R_16);
    lv_style_set_text_color(&style_font_unit, lv_color_hex(0x010101)); // lv_color_hex(0x010101)); // lv_palette_darken(LV_PALETTE_RED,4));
    lv_style_set_bg_color(&style_font_unit, lv_palette_lighten(LV_PALETTE_GREEN, 1));

    lv_obj_remove_style(unit_box, &style_font_unit, LV_STATE_DEFAULT);
    lv_obj_add_style(unit_box, &style_font_unit, LV_STATE_DEFAULT);
    lv_obj_set_scrollbar_mode(unit_box, LV_SCROLLBAR_MODE_OFF);

    unit_label = lv_label_create(unit_box);
    lv_label_set_text(unit_label, "mV/V/mA/A");

    lv_obj_add_style(unit_label, &style_font_unit, LV_STATE_DEFAULT);

    lv_obj_center(unit_label);

    static const char *btnm_map[] = {"7", "8", "9", LV_SYMBOL_BACKSPACE, "-", "\n",
                                     "4", "5", "6", "V", "mV", "\n",
                                     "1", "2", "3", "A", "mA", "\n",
                                     "0", ".", "E", LV_SYMBOL_NEW_LINE, "+"};

    lv_style_init(&style_font_btnm);
    // lv_style_set_bg_opa(&style_font, LV_OPA_50);
    // lv_style_set_bg_color(&style_font_btnm, lv_palette_main(LV_PALETTE_LIGHT_GREEN));

    lv_style_set_text_font(&style_font_btnm, &lv_font_montserrat_20); // graph_R_16

    btnm = lv_btnmatrix_create(myTextBox);
    lv_obj_remove_style_all(btnm);

    lv_obj_set_size(btnm, 290, 100);
    lv_obj_add_flag(btnm, LV_OBJ_FLAG_HIDDEN);
    lv_style_set_bg_color(&style_font_btnm, lv_palette_lighten(LV_PALETTE_GREY, 1));
    lv_obj_add_style(btnm, &style_font_btnm, LV_STATE_DEFAULT);
    lv_obj_align(btnm, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_add_event_cb(btnm, btnm_event_handler, LV_EVENT_VALUE_CHANGED, ta);
    lv_obj_clear_flag(btnm, LV_OBJ_FLAG_CLICK_FOCUSABLE); /*To keep the text area focused on button clicks*/
    lv_btnmatrix_set_map(btnm, btnm_map);
}
static void scroll_begin_event(lv_event_t *e)
{
    /*Disable the scroll animations. Triggered when a tab button is clicked */
    if (lv_event_get_code(e) == LV_EVENT_SCROLL_BEGIN)
    {
        lv_anim_t *a = (lv_anim_t *)lv_event_get_param(e);
        if (a)
            a->time = 0;
    }
}

void Utility_tabview(lv_obj_t *parent)
{
    lv_obj_set_size(parent, 320, 194);

    static lv_style_t style_utility;
    lv_style_init(&style_utility);

    lv_style_set_pad_all(&style_utility, 0);

    lv_obj_add_style(parent, &style_utility, LV_STATE_DEFAULT);
    lv_style_set_text_letter_space(&style_utility, 0);
    lv_style_set_text_color(&style_utility, lv_palette_main(LV_PALETTE_GREY));

    /*Create a Tab view object*/
    lv_obj_t *tabview;
    tabview = lv_tabview_create(parent, LV_DIR_TOP, 25);
    lv_obj_add_event_cb(lv_tabview_get_content(tabview), scroll_begin_event, LV_EVENT_SCROLL_BEGIN, NULL);

    // lv_obj_set_style_bg_color(tabview, lv_palette_darken(LV_PALETTE_DEEP_PURPLE, 3), 0);

    lv_obj_t *tab_btns = lv_tabview_get_tab_btns(tabview);
    // lv_obj_set_style_bg_color(tab_btns, lv_palette_darken(LV_PALETTE_GREY, 3), 0);
    // lv_obj_set_style_text_color(tab_btns, lv_palette_lighten(LV_PALETTE_GREY, 5), 0);
    // lv_obj_set_style_border_side(tab_btns, LV_BORDER_SIDE_RIGHT, LV_PART_ITEMS | LV_STATE_CHECKED);

    /*Add 3 tabs (the tabs are page (lv_page) and can be scrolled*/
    lv_obj_t *tab1 = lv_tabview_add_tab(tabview, "Memory");
    lv_obj_t *tab2 = lv_tabview_add_tab(tabview, "F. Gen.");
    lv_obj_t *tab3 = lv_tabview_add_tab(tabview, "Arbit");
    lv_obj_t *tab4 = lv_tabview_add_tab(tabview, "Table");

    // lv_obj_set_style_bg_color(tab1, lv_palette_darken(LV_PALETTE_BLUE, 3), 0);
    // lv_obj_set_style_bg_opa(tab1, LV_OPA_COVER, 10);

    // lv_obj_set_style_bg_color(tab2, lv_palette_lighten(LV_PALETTE_AMBER, 3), 0);
    // lv_obj_set_style_bg_opa(tab2, LV_OPA_COVER, 0);

    // lv_obj_set_style_bg_color(tab3, lv_palette_lighten(LV_PALETTE_GREEN, 3), 0);
    // lv_obj_set_style_bg_opa(tab2, LV_OPA_COVER, 0);

    // lv_label_set_text(label, "0: 0.000V\t\t0.000A\n1: 0.100V\t\t0.010A\n2: 2.048V\t\t0.050A\n3: 3.000V 0.050A\n4: 4.096V 0.050A\n5: 5.000V 0.050A\n6: 0.000V 0.000A\n7: 0.100V 0.010A\n8: 20.480V 0.050A\n9: 32.000V 0.050A");
    /*Add content to the tabs*/
    lv_obj_t *label;
    int8_t yOff = 34, verOff = 120, yStart = -10, xS = 10;
    for (int i = 0; i < 10; i++)
    {
        label = lv_label_create(tab1);
        lv_label_set_text_fmt(label, "%i:", i);
        lv_obj_remove_style_all(label);
        lv_obj_add_style(label, &style_utility, LV_STATE_DEFAULT);
        lv_obj_set_style_text_font(label, &monofont_R_20, 0);
        lv_obj_set_pos(label, xS + -4 + verOff * (i >= 5), i * yOff * (i < 5) + (i - 5) * yOff * (i >= 5) + yStart);

        label = lv_label_create(tab1);
        uint16_t adjValue;
        // PowerSupply.EEPROMwrite(90 + i * 4, i);
        PowerSupply.EEPROMread(90 + i * 4, adjValue);
        lv_obj_remove_style_all(label);
        lv_obj_add_style(label, &style_utility, LV_STATE_DEFAULT);
        lv_label_set_text_fmt(label, "%+07.3fV", (i * 2000.0 * 3.5555) / 2000.0);
        lv_obj_set_style_text_font(label, &monofont_R_16, 0); // graph_R_16 monofont_R_16
        lv_obj_set_pos(label, xS + 18 + verOff * (i >= 5), i * yOff * (i < 5) + (i - 5) * yOff * (i >= 5) + yStart);
        label = lv_label_create(tab1);

        // PowerSupply.EEPROMwrite(90 + i * 4 + 2, i);
        PowerSupply.EEPROMread(90 + i * 4 + 2, adjValue);
        lv_obj_remove_style_all(label);
        lv_obj_add_style(label, &style_utility, LV_STATE_DEFAULT);
        lv_label_set_text_fmt(label, "%+07.3fA", adjValue / 10000.0);
        lv_obj_set_style_text_font(label, &monofont_R_16, 0);
        lv_obj_set_pos(label, xS + 18 + verOff * (i >= 5), i * yOff * (i < 5) + 15 + (i - 5) * yOff * (i >= 5) + yStart);
    }

    lv_obj_set_style_pad_ver(tabview, 0, LV_PART_ITEMS);
    lv_obj_t *table = lv_table_create(tab4);
    lv_obj_set_pos(table, -4, -4);
    // lv_table_set_cell_value(table, 0, 0, "#");
    // lv_table_set_cell_value(table, 0, 1, "V");
    // lv_table_set_cell_value(table, 0, 2, "C");

    // lv_table_set_row_cnt(table, 2);
    lv_obj_clear_flag(tab4, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_ver(tab4, 0, LV_PART_ITEMS);
    lv_obj_set_style_pad_ver(table, 3, LV_PART_ITEMS);

    for (int i = 0; i < 100; i++)
    {
        lv_table_set_cell_value_fmt(table, i, 0, "%0i", i);
        lv_table_set_cell_value_fmt(table, i, 1, "%06.3fV", i * 2000.0 * 3.5555 / 2000.0);
        lv_table_set_cell_value_fmt(table, i, 2, "%06.3fA", i * 2000.0 * 3.5555 / 10000.0);
    }

    static lv_style_t style_stats;
    lv_style_init(&style_stats);
    lv_style_set_text_font(&style_stats, &graph_R_16);
    lv_style_set_text_letter_space(&style_stats, 1);
    lv_style_set_text_color(&style_stats, lv_palette_main(LV_PALETTE_GREY));
    lv_style_set_text_letter_space(&style_stats, -1);

    lv_obj_remove_style(table, &style_stats, LV_STATE_DEFAULT);
    lv_obj_add_style(table, &style_stats, LV_STATE_DEFAULT);

    // lv_obj_add_style(table, LV_TABLE_DRAW_PART_CELL, &style_cellx);
    /*Set a smaller height to the table. It'll make it scrollable*/
    lv_obj_set_height(table, 172);
    //  lv_obj_clear_flag(lv_tabview_get_content(table), LV_OBJ_FLAG_SCROLLABLE);
    // lv_obj_set_width(tab1, 25);
    // lv_obj_set_width(table, 294);
    // lv_obj_center(table);

    lv_table_set_col_width(table, 0, 27 * 2);
    lv_table_set_col_width(table, 1, 140);
    lv_table_set_col_width(table, 2, 140);

    lv_obj_clear_flag(lv_tabview_get_content(tabview), LV_OBJ_FLAG_SCROLLABLE);
    return;
    const lv_font_t *font_set{&lv_font_montserrat_16}; // graph_R_16 dseg_b_20

    static lv_style_t style_set;
    lv_style_init(&style_set);
    lv_style_set_text_color(&style_set, lv_palette_main(LV_PALETTE_YELLOW));
    lv_style_set_text_font(&style_set, font_set);
    lv_obj_add_style(label, &style_set, LV_STATE_DEFAULT);

    label = lv_label_create(tab2);
    lv_label_set_text(label, "Second tab");

    label = lv_label_create(tab3);
    lv_label_set_text(label, "Third tab");

    label = lv_label_create(tab3);
    lv_label_set_text(label, "Third tab");

    // label = lv_label_create(tab4);
    // lv_label_set_text(label, "Fourth tab");
}

void keyCheckLoop()
{
    // unsigned long mi = micros();
    getKeys();
    if (msg == " IDLE.")
    {
        // vTaskDelay(1);
        return;
    }
    // Serial.printf(" time %i \n", micros() - mi);
    // return;
    keyMenus('O', " RELEASED.", []
             {
                 PowerSupply.toggle();
                 //  lv_event_send(PowerSupply.powerSwitch.btn,LV_EVENT_LONG_PRESSED, NULL);
             });

    keyMenus('O', " HOLD.", [] // Output button
             {myTone(NOTE_A5, 200, true);
            myTone(NOTE_A3, 200, true);

            ESP.restart(); });

    keyMenus('m', " HOLD.", [] // Output button
             {
                myTone(NOTE_A5, 200, true);
                myTone(NOTE_A3, 200, true);
                PowerSupply.SaveSetting(); 
                return; });

    // Previous Page
    keyMenus('l', " RELEASED.",
             [&]
             {
                 if (!lv_obj_has_flag(myTextBox, LV_OBJ_FLAG_HIDDEN))
                     return;
                 Tabs::previousPage();
                 //  UpdateTabs();
                 //  updateObjectParrents();
             });

    // next page
    keyMenus('k', " RELEASED.",
             [&]
             {
                 if (!lv_obj_has_flag(myTextBox, LV_OBJ_FLAG_HIDDEN))
                     return;
                 Tabs::nextPage();
                 //  UpdateTabs();
                 //  updateObjectParrents();
             });

    keyMenus('k', " RELEASED.", []
             {  if (lv_obj_is_visible(voltageCurrentCalibration)) {
                lv_obj_add_flag(voltageCurrentCalibration, LV_OBJ_FLAG_HIDDEN); 
                 } });

    keyMenus('H', " RELEASED.", []
             {
                 Tabs::goToHomeTab();
                 lv_obj_add_flag(voltageCurrentCalibration, LV_OBJ_FLAG_HIDDEN);
             });
    keyMenus('M', " RELEASED.", []
             {
                 Tabs::setCurrentPage(4);

             });

    keyMenus('m', " RELEASED.", []
             {
                 if (!lv_obj_has_flag(myTextBox, LV_OBJ_FLAG_HIDDEN))
                     return;
                 Tabs::setCurrentPage(3);
             }

    );

    keyMenus('T', " RELEASED.", [&]
             {
                 //  lv_obj_del_async(Utility);
                 //  lv_obj_del(Utility);
                 //   hackerUtilityObj = NULL;
             });
    keyMenusPage('-', " RELEASED.", 0, [encoder1_value, encoder2_value]
                 {
                     int32_t newEncoder1Value = encoder1_value +1;
                     int32_t newEncoder2Value = encoder2_value;
                     handleHistogramPage(newEncoder1Value, newEncoder2Value);

                     PowerSupply.Current.hist.Reset();
                     PowerSupply.Voltage.hist.Reset(); });

    keyMenusPage('+', " RELEASED.", 0, [encoder1_value, encoder2_value]
                 {
                     int32_t newEncoder1Value = encoder1_value - 1;
                     int32_t newEncoder2Value = encoder2_value;
                     handleHistogramPage(newEncoder1Value, newEncoder2Value);

                     PowerSupply.Current.hist.Reset();
                     PowerSupply.Voltage.hist.Reset(); });

    keyMenusPage('V', " RELEASED.", 0, []
                 {
                     static bool show = false;
                     show = !show;
                     lv_chart_hide_series( PowerSupply.stats.chart,  PowerSupply.stats.serV,show); });

    keyMenusPage('A', " RELEASED.", 0, []
                 {
                     static bool show = false;
                     show = !show;
                     lv_chart_hide_series( PowerSupply.stats.chart,  PowerSupply.stats.serI,show); });

    // Statistics Reset and auto ajdust histogram window
    keyMenus('j', " RELEASED.", []
             {

                if ( PowerSupply.stats.serV->hidden==0 ){
                    PowerSupply.Voltage.hist.histWinMin = PowerSupply.Voltage.measured.Mean() -
                                                        PowerSupply.Voltage.Statistics.StandardDeviation() * 6;

                    PowerSupply.Voltage.hist.histWinMax = PowerSupply.Voltage.measured.Mean() +
                                                        PowerSupply.Voltage.Statistics.StandardDeviation() * 6;
                }
                if ( PowerSupply.stats.serI->hidden==0 ){
                    PowerSupply.Current.hist.histWinMin = PowerSupply.Current.measured.Mean() -
                                                       PowerSupply.Current.Statistics.StandardDeviation() * 6;

                    PowerSupply.Current.hist.histWinMax = PowerSupply.Current.measured.Mean() +
                                                       PowerSupply.Current.Statistics.StandardDeviation() * 6;
                }
                //  PowerSupply.stats.serI->y_points = PowerSupply.Current.hist.data;
                 //  PowerSupply.stats.serV->y_points = PowerSupply.Voltage.hist.data;



                 PowerSupply.ResetStats(); });

    keyMenus('Z', " RELEASED.", []
             {
                uint16_t w;
                w = PowerSupply.Voltage.measured.NofAvgs;
                w = (w == MAX_NO_OF_AVG) ? 1 : w * 2;
                // PowerSupply.Voltage.effectiveResolution(64);
                PowerSupply.ResetStats();

                lv_slider_set_value(lv_obj_get_child(slider_Avgs, -1), log2(w), LV_ANIM_OFF);
                lv_event_send(lv_obj_get_child(slider_Avgs, -1), LV_EVENT_VALUE_CHANGED, NULL); });

    keyMenusPage('+', " RELEASED.", 4, []
                 {  if (lv_obj_is_visible(voltageCurrentCalibration)) {
                     lv_event_send(spinboxes.btn_plus[spinboxes.id_index], LV_EVENT_SHORT_CLICKED, NULL);} });

    keyMenusPage('-', " RELEASED.", 4, []
                 {  if (lv_obj_is_visible(voltageCurrentCalibration)) {
                    lv_event_send(spinboxes.btn_minus[spinboxes.id_index], LV_EVENT_SHORT_CLICKED, NULL);} });

    if ((Tabs::getCurrentPage() == 2) || (Tabs::getCurrentPage() == 3) || (Tabs::getCurrentPage() == 4 && lv_obj_is_visible(voltageCurrentCalibration)))
    {
        keyMenus('7', " RELEASED.", []
                 { key_event_handler(0); });
        keyMenus('8', " RELEASED.", []
                 { key_event_handler(1); });
        keyMenus('9', " RELEASED.", []
                 { key_event_handler(2); });
        keyMenus('<', " RELEASED.", []
                 { key_event_handler(3); });

        keyMenus('4', " RELEASED.", []
                 { key_event_handler(5); });
        keyMenus('5', " RELEASED.", []
                 { key_event_handler(6); });
        keyMenus('6', " RELEASED.", []
                 { key_event_handler(7); });

        keyMenus('1', " RELEASED.", []
                 { key_event_handler(10); });
        keyMenus('2', " RELEASED.", []
                 { key_event_handler(11); });
        keyMenus('3', " RELEASED.", []
                 { key_event_handler(12); });

        keyMenus('0', " RELEASED.", []
                 { key_event_handler(15); });
        keyMenus('.', " RELEASED.", []
                 { key_event_handler(16); });

        keyMenusPage('E', " RELEASED.", 2, []
                     {  if (!lv_obj_is_visible(myTextBox))
                        lv_obj_clear_flag(myTextBox, LV_OBJ_FLAG_HIDDEN);
                    else
                        lv_obj_add_flag(myTextBox, LV_OBJ_FLAG_HIDDEN);
                        ismyTextHiddenChange = true; });

        keyMenusPage('E', " RELEASED.", 4, []
                     {if (!lv_obj_is_visible(myTextBox))
                        {
                            lv_obj_clear_flag(myTextBox, LV_OBJ_FLAG_HIDDEN);

                            lv_obj_t *spinBox = lv_obj_get_child(voltageCurrentCalibration, spinboxes.current_index);
                            int32_t v = lv_spinbox_get_value(spinBox);

// Serial.printf("\nspinboxes.current_index:%i",spinboxes.current_index);

                            char str[12];

                            if (spinboxes.current_index % 2 != 0)
                            {
                                sprintf(str, "%+08.0f",double( v ));
                                key_event_handler_readBack_clb(str);
                            }

                            else
                            {
                                sprintf(str, "%+08.4f", double (v/10000.0));
                                key_event_handler_readBack_clb(str);
                                // key_event_handler_readBack_clb(std::to_string(v).c_str());
                            }
                            ismyTextHiddenChange = true;
                        }
                        else
                        {
                            lv_obj_add_flag(myTextBox, LV_OBJ_FLAG_HIDDEN);
                            myTone(NOTE_A5, 100);
                            const char *txt = lv_textarea_get_text(ta);
                            lv_obj_t *spinBox = lv_obj_get_child(voltageCurrentCalibration, spinboxes.current_index);
                            if (spinboxes.current_index % 2 != 0)
                                lv_spinbox_set_value(spinBox, strtod(txt, NULL));
                            else
                                lv_spinbox_set_value(spinBox, 10000.0 * strtod(txt, NULL));
                            lv_textarea_set_text(ta,"");
                            ismyTextHiddenChange = true;

                        } });
    }

    keyMenusPage('^', " RELEASED.", 2, []
                 { PowerSupply.Voltage.setLock(!PowerSupply.Voltage.getLock()); });
    keyMenusPage('>', " RELEASED.", 2, []
                 { PowerSupply.Current.setLock(!PowerSupply.Current.getLock()); });
    keyMenusPage('M', " RELEASED.", 4, []
                 { lv_obj_add_flag(voltageCurrentCalibration, LV_OBJ_FLAG_HIDDEN); });

    keyMenusPage('V', " RELEASED.", 2, []
                 {
                     if (!lv_obj_is_visible(myTextBox))
                     {
                         lv_obj_clear_flag(myTextBox, LV_OBJ_FLAG_HIDDEN);
                         key_event_handler_readBack(PowerSupply.Voltage);
                         ismyTextHiddenChange = true;
                     }
                     else if (strcmp(lv_label_get_text(unit_label), "V") == 0 || strcmp(lv_label_get_text(unit_label), "mV/V/mA/A") == 0)
                         key_event_handler(8);

                     //  lv_obj_move_foreground(PowerSupply.stats.chart);
                     static bool show = false;
                     show = !show;
                     lv_chart_hide_series(PowerSupply.stats.chart, PowerSupply.stats.serV, show); });

    keyMenusPage('v', " RELEASED.", 2, []
                 {
            if (!lv_obj_is_visible(myTextBox))
            {
                lv_obj_clear_flag(myTextBox, LV_OBJ_FLAG_HIDDEN);
                key_event_handler_readBack_k(PowerSupply.Voltage);
                ismyTextHiddenChange = true;
            }
            else if (strcmp(lv_label_get_text(unit_label), "mV") == 0 || strcmp(lv_label_get_text(unit_label), "mV/V/mA/A") == 0)
                key_event_handler(9); });

    keyMenusPage('A', " RELEASED.", 2, []
                 {
            if (!lv_obj_is_visible(myTextBox))
            {
                lv_obj_clear_flag(myTextBox, LV_OBJ_FLAG_HIDDEN);
                key_event_handler_readBack(PowerSupply.Current);
                ismyTextHiddenChange = true;
            }
            else if (strcmp(lv_label_get_text(unit_label), "A") == 0 || strcmp(lv_label_get_text(unit_label), "mV/V/mA/A") == 0)
                key_event_handler(13); });

    keyMenusPage('a', " RELEASED.", 2, []
                 {
            if (!lv_obj_is_visible(myTextBox))
            {
                lv_obj_clear_flag(myTextBox, LV_OBJ_FLAG_HIDDEN);
                key_event_handler_readBack_k(PowerSupply.Current);
                ismyTextHiddenChange = true;
            }
            else if (strcmp(lv_label_get_text(unit_label), "mA") == 0 || strcmp(lv_label_get_text(unit_label), "mV/V/mA/A") == 0)
                key_event_handler(14); });

    keyMenusPage('W', " RELEASED.", 2, []
                 {
                     PowerSupply.Voltage.SetRotaryStep(0.0005);
                     PowerSupply.Current.SetRotaryStep(0.0001);
                     lv_obj_align(PowerSupply.Voltage.highlight_adjValue, 0, 10 * 12, -1000);
                     lv_obj_align(PowerSupply.Current.highlight_adjValue, 0, 10 * 12, -1000); });

    keyMenusPage('W', " RELEASED.", 1, []
                 { chartPause = !chartPause; });

    keyMenusPage('X', " RELEASED.", 2, []
                 {
                     PowerSupply.Voltage.SetRotaryStep(0.0005);
                     PowerSupply.Current.SetRotaryStep(0.0001);
                     lv_obj_align(PowerSupply.Voltage.highlight_adjValue, 0, 10 * 12, -1000);
                     lv_obj_align(PowerSupply.Current.highlight_adjValue, 0, 10 * 12, -1000); });

    keyMenusPage('Y', " RELEASED.", 2, []
                 {
                     PowerSupply.Voltage.SetRotaryStep(0.0005);
                     PowerSupply.Current.SetRotaryStep(0.0001);
                     lv_obj_align(PowerSupply.Voltage.highlight_adjValue, 0, 10 * 12, -1000);
                     lv_obj_align(PowerSupply.Current.highlight_adjValue, 0, 10 * 12, -1000); });

    keyMenusPage('W', " HOLD.", 2, []
                 {
            PowerSupply.Voltage.SetRotaryStep(1.0000);
            PowerSupply.Current.SetRotaryStep(1.0000);
            lv_obj_align(PowerSupply.Voltage.highlight_adjValue, 0, 7 * 12, -10);
            lv_obj_align(PowerSupply.Current.highlight_adjValue, 0, 7 * 12, 72);
            lv_obj_clear_flag(PowerSupply.Voltage.highlight_adjValue, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(PowerSupply.Current.highlight_adjValue, LV_OBJ_FLAG_HIDDEN); });

    keyMenusPage('X', " HOLD.", 2, []
                 {
            PowerSupply.Voltage.SetRotaryStep(0.1000);
            PowerSupply.Current.SetRotaryStep(0.1000);
            lv_obj_align(PowerSupply.Voltage.highlight_adjValue, 0, 9 * 12, -10);
            lv_obj_align(PowerSupply.Current.highlight_adjValue, 0, 9 * 12, 72);
            lv_obj_clear_flag(PowerSupply.Voltage.highlight_adjValue, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(PowerSupply.Current.highlight_adjValue, LV_OBJ_FLAG_HIDDEN); });

    keyMenusPage('Y', " HOLD.", 2, []
                 {
            PowerSupply.Voltage.SetRotaryStep(0.0100);
            PowerSupply.Current.SetRotaryStep(0.0100);
            lv_obj_align(PowerSupply.Voltage.highlight_adjValue, 0, 10 * 12, -10);
            lv_obj_align(PowerSupply.Current.highlight_adjValue, 0, 10 * 12, 72);
            lv_obj_clear_flag(PowerSupply.Voltage.highlight_adjValue, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(PowerSupply.Current.highlight_adjValue, LV_OBJ_FLAG_HIDDEN); });

    if ((msg == " IDLE.") && (Tabs::getCurrentPage() == 2))
    {
        // switch (keyChar)
        // {

        // case 'Z':
        //     uint8_t w;
        //     w = PowerSupply.Voltage.measured.NofAvgs;
        //     w = (w == 128) ? 1 : w * 2;

        //     lv_slider_set_value(lv_obj_get_child(slider_Avgs, -1), log2(w), LV_ANIM_OFF);
        //     lv_event_send(lv_obj_get_child(slider_Avgs, -1), LV_EVENT_VALUE_CHANGED, NULL);

        //     break;
        // }

        // msg = "";
        keyChar = ' ';
        //   lv_obj_clear_flag(PowerSupply.Voltage.highlight_adjValue, LV_OBJ_FLAG_HIDDEN);
        // lv_obj_clear_flag(PowerSupply.Current.highlight_adjValue, LV_OBJ_FLAG_HIDDEN);
    }
}

void StatsPositions(lv_obj_t *parent, DispObjects &dObj, lv_style_t *style_, lv_coord_t x, lv_coord_t y)
{
    lv_obj_set_parent(dObj.statLabels.label_unit, parent);
    lv_obj_set_parent(dObj.statLabels.label_setSmallFont, parent);
    lv_obj_set_parent(dObj.statLabels.label_value, parent);
    lv_obj_set_parent(dObj.statLabels.label_mean, parent);
    lv_obj_set_parent(dObj.statLabels.label_std, parent);
    lv_obj_set_parent(dObj.statLabels.label_max, parent);
    lv_obj_set_parent(dObj.statLabels.label_min, parent);
    lv_obj_set_parent(dObj.statLabels.label_fft, parent);
    lv_obj_set_parent(dObj.statLabels.label_legend, parent);
};

void updateObjectPos_cb(lv_event_t *e)
{
    // Serial.printf("Page Changed to %i!\n", Tabs::getCurrentPage());
    // updateObjectParrents();
    if (Tabs::getCurrentPage() > 1)
        return;

    StatsPositions(PowerSupply.page[Tabs::getCurrentPage()], PowerSupply.Voltage, &PowerSupply.stats.style_statsVolt, 0, 177);
    StatsPositions(PowerSupply.page[Tabs::getCurrentPage()], PowerSupply.Current, &PowerSupply.stats.style_statsCurrent, 0, 187);

    // switch (Tabs::getCurrentPage())
    // {

    // case 0:
    //     StatsPositions(PowerSupply.page[0], PowerSupply.Voltage, &PowerSupply.stats.style_statsVolt, 0, 177);
    //     StatsPositions(PowerSupply.page[0], PowerSupply.Current, &PowerSupply.stats.style_statsCurrent, 0, 187);
    //     PowerSupply.SaveSetting();
    //     break;
    // case 1:
    //     StatsPositions(PowerSupply.page[1], PowerSupply.Voltage, &PowerSupply.graph.style_statsVolt, 0, 177);
    //     StatsPositions(PowerSupply.page[1], PowerSupply.Current, &PowerSupply.graph.style_statsCurrent, 0, 187);
    //     PowerSupply.SaveSetting();
    //     break;
    // case 4:
    //     PowerSupply.SaveSetting();
    //     break;
    // }
}

void updateObjectParrents()
{
    // Serial.printf("Page Changed to %i!\n", Tabs::getCurrentPage());

    auto StatsPositions = [&](lv_obj_t *parent, DispObjects &dObj, lv_style_t *style_, lv_coord_t x, lv_coord_t y)
    {
        int space = 61;

        // lv_obj_set_parent(dObj.statLabels.label_mean, parent);
        // lv_obj_align(dObj.statLabels.label_mean, LV_ALIGN_DEFAULT, x, y);

        lv_obj_set_parent(dObj.statLabels.label_value, parent);
        lv_obj_align(dObj.statLabels.label_value, LV_ALIGN_DEFAULT, x, y);

        lv_obj_set_parent(dObj.statLabels.label_mean, parent);
        lv_obj_align(dObj.statLabels.label_mean, LV_ALIGN_DEFAULT, x + space * 1, y);

        lv_obj_set_parent(dObj.statLabels.label_std, parent);
        lv_obj_align(dObj.statLabels.label_std, LV_ALIGN_DEFAULT, x + space * 2 + 4, y);

        lv_obj_set_parent(dObj.statLabels.label_max, parent);
        lv_obj_align(dObj.statLabels.label_max, LV_ALIGN_DEFAULT, x + space * 3, y);

        lv_obj_set_parent(dObj.statLabels.label_min, parent);
        lv_obj_align(dObj.statLabels.label_min, LV_ALIGN_DEFAULT, x + space * 4, y);
    };

    switch (Tabs::getCurrentPage())
    {

    case 0:
        StatsPositions(PowerSupply.page[0], PowerSupply.Voltage, &PowerSupply.stats.style_statsVolt, 0, 177);
        StatsPositions(PowerSupply.page[0], PowerSupply.Current, &PowerSupply.stats.style_statsCurrent, 0, 187);
        // PowerSupply.SaveSetting();
        break;
    case 1:
        StatsPositions(PowerSupply.page[1], PowerSupply.Voltage, &PowerSupply.graph.style_statsVolt, 0, 177);
        StatsPositions(PowerSupply.page[1], PowerSupply.Current, &PowerSupply.graph.style_statsCurrent, 0, 187);
        // PowerSupply.SaveSetting();
        break;
    case 4:
        PowerSupply.SaveSetting();
        break;
    }
}

void StatusBar()
{
    static lv_obj_t *statusLabel_time;
    static lv_obj_t *statusLabel_avg;
    static bool statusCreationFlag = false;

    // lv_obj_remove_style(unit_box, &style_font_unit, LV_STATE_DEFAULT);

    if (!statusCreationFlag)
    {
        static lv_style_t style;
        static lv_style_t style_font;
        lv_style_init(&style);

        /*Set a background color and a radius*/
        lv_style_set_radius(&style, 2);
        lv_style_set_bg_opa(&style, LV_OPA_COVER);
        lv_style_init(&style_font);
        // lv_style_set_bg_opa(&style_font, LV_OPA_50);
        // lv_style_set_bg_color(&style_font, lv_palette_lighten(LV_PALETTE_GREEN, 1));
        lv_style_set_text_font(&style_font, &graph_R_8);                        // graph_R_8 lv_font_montserrat_12
        lv_style_set_text_color(&style_font, lv_palette_main(LV_PALETTE_GREY)); //) lv_color_hex(0xFFFFFF)  lv_palette_darken(LV_PALETTE_RED,4));
        lv_obj_t *status = lv_obj_create(lv_scr_act());
        lv_obj_set_size(status, 314, 14);

        lv_obj_align(status, LV_ALIGN_TOP_MID, 0, 226);
        lv_obj_set_scrollbar_mode(status, LV_SCROLLBAR_MODE_OFF);
        lv_obj_add_style(status, &style, 0);

        lv_style_set_text_letter_space(&style_font, -1);
        statusLabel_time = lv_label_create(status);
        statusLabel_avg = lv_label_create(status);

        lv_obj_remove_style_all(statusLabel_time);
        lv_obj_remove_style_all(statusLabel_avg);

        lv_obj_add_style(statusLabel_time, &style_font, LV_STATE_DEFAULT);
        lv_obj_add_style(statusLabel_avg, &style_font, LV_STATE_DEFAULT);

        lv_obj_align(statusLabel_time, LV_ALIGN_LEFT_MID, -12, 0);
        lv_obj_align(statusLabel_avg, LV_ALIGN_RIGHT_MID, 0, 0);
        statusCreationFlag = true;
    }
    static time_t now;
    char strftime_buf[16];
    struct tm timeinfo;

    time(&now);
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%H:%M:%S", &timeinfo); // W:week. j: days of year
    // tft.drawString(strftime_buf, 249, 232);
    // Serial.printf("time:%s \n", strftime_buf);
    // lv_label_set_text(label, strftime_buf);
    lv_label_set_text_fmt(statusLabel_time, "%s", strftime_buf);

    static char str[35];

    sprintf(str, "SPS:%3i;Used memory:%04d;#Avgs ", PowerSupply.adc.realADCSpeed, heepMemory - ESP.getFreeHeap()); // memory - getFreeHeap getFreeSketchSpace getFreePsram

    // const char *str2 = "%i";
    strcat(str, "%i");

    lv_label_set_text_fmt(statusLabel_avg, str, PowerSupply.Voltage.measured.NofAvgs);

    // auto StatsPosion = [&](lv_obj_t *parent, DispObjects &dObj, lv_style_t *style_, lv_coord_t x, lv_coord_t y)
    // {
    //     int space = 70;

    //     lv_obj_set_parent(dObj.statLabels.label_value, parent);
    //     lv_obj_align(dObj.statLabels.label_value, LV_ALIGN_DEFAULT, x, y);

    //     lv_obj_set_parent(dObj.statLabels.label_mean, parent);
    //     lv_obj_align(dObj.statLabels.label_mean, LV_ALIGN_DEFAULT, x + space * 1, y);

    //     lv_obj_set_parent(dObj.statLabels.label_max, parent);
    //     lv_obj_align(dObj.statLabels.label_max, LV_ALIGN_DEFAULT, x + space * 2, y);

    //     lv_obj_set_parent(dObj.statLabels.label_min, parent);
    //     lv_obj_align(dObj.statLabels.label_min, LV_ALIGN_DEFAULT, x + space * 3, y);
    // };

    auto StatsPositions = [&](lv_obj_t *parent, DispObjects &dObj, lv_style_t *style_, lv_coord_t x, lv_coord_t y)
    {
        int space = 61;

        // lv_obj_set_parent(dObj.statLabels.label_mean, parent);
        // lv_obj_align(dObj.statLabels.label_mean, LV_ALIGN_DEFAULT, x, y);

        lv_obj_set_parent(dObj.statLabels.label_value, parent);
        lv_obj_align(dObj.statLabels.label_value, LV_ALIGN_DEFAULT, x, y);

        lv_obj_set_parent(dObj.statLabels.label_mean, parent);
        lv_obj_align(dObj.statLabels.label_mean, LV_ALIGN_DEFAULT, x + space * 1, y);

        lv_obj_set_parent(dObj.statLabels.label_std, parent);
        lv_obj_align(dObj.statLabels.label_std, LV_ALIGN_DEFAULT, x + space * 2 + 4, y);

        lv_obj_set_parent(dObj.statLabels.label_max, parent);
        lv_obj_align(dObj.statLabels.label_max, LV_ALIGN_DEFAULT, x + space * 3, y);

        lv_obj_set_parent(dObj.statLabels.label_min, parent);
        lv_obj_align(dObj.statLabels.label_min, LV_ALIGN_DEFAULT, x + space * 4, y);
    };

    lv_obj_t *vin1_ = lv_obj_get_child(voltageCurrentCalibration, spinboxes.ids[0]);
    lv_obj_t *code1_ = lv_obj_get_child(voltageCurrentCalibration, spinboxes.ids[1]);
    lv_obj_t *vin2_ = lv_obj_get_child(voltageCurrentCalibration, spinboxes.ids[2]);
    lv_obj_t *code2_ = lv_obj_get_child(voltageCurrentCalibration, spinboxes.ids[3]);

    int code1 = (lv_spinbox_get_value(code1_));
    int code2 = (lv_spinbox_get_value(code2_));
    double vin1 = double(lv_spinbox_get_value(vin1_)) / 10000.0;
    double vin2 = double(lv_spinbox_get_value(vin2_)) / 10000.0;

    double m = get_m(code1, code2, vin1, vin2);

    lv_label_set_text(lbl_voltageCalib_m, std::to_string(m).c_str());
    lv_label_set_text(lbl_voltageCalib_b, std::to_string(get_b(code1, m, vin1)).c_str());
    if (lv_obj_is_visible(voltageCurrentCalibration))
    {
        if (lv_dropdown_get_selected(dd_calibration) == 0)
        {
            lv_label_set_text_fmt(lbl_rawCode, "%+08i", PowerSupply.Voltage.rawValue);
            lv_label_set_text_fmt(lbl_rawAVG_, "%+08.0f", PowerSupply.Voltage.measured.Mean() * m + get_b(code1, m, vin1));

            lv_label_set_text_fmt(lbl_calibratedValue, "%+09.4f", PowerSupply.Voltage.measured.value);
            lv_label_set_text_fmt(lbl_calibValueAVG_, "%+09.4f", PowerSupply.Voltage.measured.Mean());
            lv_label_set_text_fmt(lbl_ER_, "%+02.2f", PowerSupply.Voltage.effectiveResolution.Mean());

            PowerSupply.CalBank[PowerSupply.bankCalibId].vCal.code_1 = code1;
            PowerSupply.CalBank[PowerSupply.bankCalibId].vCal.code_2 = code2;
            PowerSupply.CalBank[PowerSupply.bankCalibId].vCal.value_1 = vin1;
            PowerSupply.CalBank[PowerSupply.bankCalibId].vCal.value_2 = vin2;
        }

        else
        {
            lv_label_set_text_fmt(lbl_rawCode, "%+08i", PowerSupply.Current.rawValue);
            lv_label_set_text_fmt(lbl_rawAVG_, "%+08.0f", PowerSupply.Current.measured.Mean() * m + get_b(code1, m, vin1));

            lv_label_set_text_fmt(lbl_calibratedValue, "%+09.4f", PowerSupply.Current.measured.value);
            lv_label_set_text_fmt(lbl_calibValueAVG_, "%+09.4f", PowerSupply.Current.measured.Mean());
            lv_label_set_text_fmt(lbl_ER_, "%+02.2f", PowerSupply.Current.effectiveResolution.Mean());

            PowerSupply.CalBank[PowerSupply.bankCalibId].iCal.code_1 = code1;
            PowerSupply.CalBank[PowerSupply.bankCalibId].iCal.code_2 = code2;
            PowerSupply.CalBank[PowerSupply.bankCalibId].iCal.value_1 = vin1;
            PowerSupply.CalBank[PowerSupply.bankCalibId].iCal.value_2 = vin2;
        }

        // lv_event_send(btn_load, LV_EVENT_CLICKED, NULL);
        PowerSupply.calibrationUpdate();
    }

    if (PowerSupply.eepromWriteFlag)
    {
        // EEPROMwrite(PowerSupply.Voltage.adjEEPROMAddress + 8 * 0, Voltage.adjValue); // PowerSupply.Voltage.adjValue
        // EEPROMwrite(PowerSupply.Current.adjEEPROMAddress + 8 * 1, Current.adjValue); // PowerSupply.Voltage.adjValue

        // PowerSupply.SaveSetting();
        EEPROM.commit();

        PowerSupply.eepromWriteFlag = false;
        // Serial.printf("%\nSaved!");
    }
}

void schedule(std::function<void(void)> func, unsigned long &&interval, unsigned long &startTime)
{
    if ((millis() - startTime) < interval)
        return;

    startTime = millis();
    func();
}
void schedule(std::function<void(void)> func, unsigned long &interval, unsigned long &startTime)
{
    if ((millis() - startTime) < interval)
        return;
    startTime = millis();
    func();
    // getSettingEncoder(NULL, NULL);
}
// class RepeatedScheduler
// {
// public:
//     bool flag = false;

//     // RepeatedScheduler(int &&interval_ms)
//     // {
//     //     schedule(interval_ms);
//     // }
//     // Schedules a task to be run repeatedly with a specified interval
//     void schedule(unsigned long interval_ms)
//     {
//         task_ = [&]
//         { flag = true; };
//         interval_ms_ = interval_ms;
//         xTaskCreate(runTask, "RepeatedTask", 680, this, 2, &handle_);
//     }

//     // Stops the scheduled task
//     // void stop()
//     // {
//     //     vTaskDelete(handle_);
//     // }
//     bool   itsTheTime()
//     {
//         if (!flag) return false;
//         else {
//             flag= false;
//             return true;
//         }

//     }

// private:
//     static void runTask(void *pvParameter)
//     {
//         auto scheduler = static_cast<RepeatedScheduler *>(pvParameter);
//         while (1)
//         {
//             scheduler->task_();
//             vTaskDelay(scheduler->interval_ms_ / portTICK_PERIOD_MS);
//         }
//     }

//     std::function<void(void)> task_;
//     int interval_ms_;
//     TaskHandle_t handle_;
// };
// RepeatedScheduler scheduler[10];

void ChartUpdate()

{

    if (!chartPause)
    {
        lv_chart_set_next_value(PowerSupply.graph.chart, PowerSupply.graph.serV, PowerSupply.Voltage.measured.value * 1000.0);
        lv_chart_set_next_value(PowerSupply.graph.chart, PowerSupply.graph.serI, PowerSupply.Current.measured.value * 1000.0);
    }

    // Histogram
    // PowerSupply.Voltage.hist[PowerSupply.Voltage.measured.value];
    // PowerSupply.Current.hist[PowerSupply.Current.measured.value];
    // lv_chart_refresh(PowerSupply.stats.chart);
}
void HistPush()

{
    // Histogram
    PowerSupply.Current.hist[PowerSupply.Current.measured.Mean()];
    PowerSupply.Voltage.hist[PowerSupply.Voltage.measured.Mean()];
    // lv_chart_refresh(PowerSupply.stats.chart);
}
void getSettingEncoder(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
    if (encoder2Flag /*&& (Tabs::getCurrentPage() == 4)*/)
    {
        // encoder2Flag = 0;

        int64_t count = PowerSupply.Current.encoder.getCount() >> 1; // / 2;
        // PowerSupply.Current.encoder.pauseCount();

        static int64_t rotaryOldValue2 = 0;

        // encoder2_value = count;
        if (count > rotaryOldValue2)
        {
            encoder2_value++;
            // PowerSupply.Current.SetUpdate(PowerSupply.Current.adjValue + PowerSupply.Current.rotaryEncoderStep);
        }
        else if (count < rotaryOldValue2)
        {
            encoder2_value--;
            // PowerSupply.Current.SetUpdate(PowerSupply.Current.adjValue - PowerSupply.Current.rotaryEncoderStep);
        }

        rotaryOldValue2 = count;
        // encoder2_value = std::clamp(encoder2_value, -500, 500);
        // encoder2Flag = (encoder2Flag > 300) ? 300 : encoder2Flag - 1;
        encoder2Flag = 0;
        // if (encoder2Flag == 0)
        //   PowerSupply.Current.encoder.setCount(0);
        switch (Tabs::getCurrentPage())
        {
        case 4:
            break;
        case 2:
            PowerSupply.Current.SetEncoderUpdate();
            // PowerSupply.FlushSettings();
            // PowerSupply.Voltage.SetEncoderUpdate();
            break;
        }
        // LV_LOG_USER("encoder2_value:%i encoder2Flag:%i count:%i", encoder2_value, encoder2Flag, count);
        encoderTimeStamp = millis();
    }

    if (encoder1Flag /*&& (Tabs::getCurrentPage() == 4)*/)
    {

        int64_t count = PowerSupply.Voltage.encoder.getCount() >> 1; // / 2;
        // // PowerSupply.Current.encoder.pauseCount();

        static int64_t rotaryOldValue = 0;

        // encoder2_value = count;
        if (count > rotaryOldValue)
            encoder1_value++;

        else if (count < rotaryOldValue)
            encoder1_value--;

        rotaryOldValue = count;
        // encoder1Flag = (encoder1Flag > 300) ? 300 : encoder1Flag - 1;
        // encoder1_value = PowerSupply.Voltage.encoder.getCount() / 2;
        encoder1Flag = 0;
        switch (Tabs::getCurrentPage())
        {
        case 4:
            break;
        case 2:
            PowerSupply.Voltage.SetEncoderUpdate();
            // PowerSupply.FlushSettings();

            // PowerSupply.Voltage.Flush();
            break;
        }
        // PowerSupply.Current.encoder.resumeCount();

        // LV_LOG_USER("encoder2_value:%i encoder1Flag:%i count:%i", 0, encoder1Flag,
        encoderTimeStamp = millis();
    }
}

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
        Serial.printf("\n%s: Loop Count: %5.0f @ %07.2f seconds", callerName, loopCount * 1000.0 / loopInterval, currentTime / 1000.0);
        lastLoopTime = currentTime;
        loopCount = 0;
        // Serial.printf("\n%i", digitalRead(PowerSupply.CCCVPin));
    }
}

void LvglUpdatesInterval(unsigned long interval)
{
    static unsigned long timer_ = {0}; // Interval in milliseconds
    if (lvglChartIsBusy)
    {
        vTaskDelay(1);
        // return;
    }
    schedule([]
             {
                 //  if (!lvglChartIsBusy)
                 {
                     lvglIsBusy = 1;
                     lv_timer_handler();
                     lvglIsBusy = 0;
                 }
                 //  else
                 //  delay(10);
             },

             interval, timer_);
}

void StatusBarUpdateInterval(unsigned long interval)
{
    static unsigned long timer_ = {0}; // Interval in milliseconds
    schedule(&StatusBar, interval, timer_);
}

void FlushMeasuresInterval(unsigned long interval)
{
    static unsigned long timer_ = {0}; // Interval in milliseconds
    schedule([]
             { PowerSupply.FlushMeasures(); }, interval, timer_);
}
void statisticUpdateInterval(unsigned long interval)
{
    static unsigned long timer_ = {0};
    schedule([]
             {
            // if (!lvglIsBusy)
            //     lv_chart_refresh(PowerSupply.stats.chart);
        PowerSupply.settingParameters.SetVoltage = PowerSupply.Voltage.adjValue;
        PowerSupply.settingParameters.SetCurrent = PowerSupply.Current.adjValue;
        PowerSupply.SaveSetting();

             PowerSupply.Voltage.statUpdate();
             PowerSupply.Current.statUpdate(); },
             interval, timer_);
}

void FFTUpdateInterval(unsigned long interval)
{
    static unsigned long timer_ = {0};
    schedule([]
             {
                 /*FFT_v*/

                 V.computeFFT(PowerSupply.adc.realADCSpeed * 0.495049505 /*2.02*/);
                 I.computeFFT(PowerSupply.adc.realADCSpeed * 0.495049505 /**/);

                 lv_label_set_text_fmt(PowerSupply.Voltage.statLabels.label_fft, "V-FFT:%5.1f Hz", V.peak);
                 lv_label_set_text_fmt(PowerSupply.Current.statLabels.label_fft, "I-FFT:%5.1f Hz", I.peak);

                 //   lv_label_set_text_fmt(label_statMenu_VFFT, "V-FFT_v:%5.1f Hz", V.peak);
                 //   lv_label_set_text_fmt(label_statMenu_IFFT, "I-FFT_i:%5.1f Hz", I.peak);
             },
             interval, timer_);
}

void EncoderRestartInterval(unsigned long interval)
{
    static unsigned long timer_ = {0};
    schedule([]
             {
                 /*FFT_v*/
                 int64_t temp = PowerSupply.Voltage.encoder.getCount();
                 PowerSupply.Voltage.encoder.clearCount();
                 PowerSupply.Voltage.encoder.setCount(temp);
                 // toneOff();
                 temp = PowerSupply.Current.encoder.getCount();
                 PowerSupply.Current.encoder.clearCount();
                 PowerSupply.Current.encoder.setCount(temp);
                 // toneOff();
             },
             interval, timer_);
}

void KeyCheckInterval(unsigned long interval)
{
    static unsigned long timer_ = {0}; // Interval in milliseconds
    // static unsigned long timer_;
    schedule([]
             { keyCheckLoop(); },
             interval, timer_);
}

void VCCCInterval(unsigned long interval)
{
    static unsigned long timer_ = {0};
    schedule([]
             {
                if (!lvglChartIsBusy)
                     PowerSupply.VCCCStatusUpdate(); },
             interval, timer_);
}

void DACInterval(unsigned long interval)
{
    static unsigned long timer_ = {0};
    schedule([]
             {
                // if (!lvglChartIsBusy)
             PowerSupply.DACUpdate(); },
             interval, timer_);
}

// **Helper Functions**

void handleCalibrationPage(int32_t &encoder1_last_value, int32_t &encoder2_last_value)
{
    if (lv_obj_is_visible(voltageCurrentCalibration))
    {
        static int32_t cursor_pos = 0;

        // Check if encoder values have changed
        if (encoder2_last_value == encoder2_value && encoder1_last_value == encoder1_value)
            return;

        // Update cursor position based on encoder 2
        if (encoder2_last_value < encoder2_value)
            cursor_pos++;
        else if (encoder2_last_value > encoder2_value)
            cursor_pos--;

        int width = spinboxes.digit_count[spinboxes.id_index] - 1;
        cursor_pos = std::clamp(cursor_pos, 0, width);

        lv_obj_t *spinBox = lv_obj_get_child(voltageCurrentCalibration, spinboxes.current_index);

        // Set cursor position in the spinbox
        lv_spinbox_set_cursor_pos(spinBox, 0);
        lv_spinbox_set_cursor_pos(spinBox, width - cursor_pos);

        encoder2_last_value = encoder2_value;

        // Adjust spinbox value based on encoder 1
        if (encoder1_last_value < encoder1_value)
            lv_event_send(spinboxes.btn_plus[spinboxes.id_index], LV_EVENT_SHORT_CLICKED, NULL);
        else if (encoder1_last_value > encoder1_value)
            lv_event_send(spinboxes.btn_minus[spinboxes.id_index], LV_EVENT_SHORT_CLICKED, NULL);

        // Perform calibration
        PowerSupply.calibrate();
        encoder1_last_value = encoder1_value;
        // SaveCalibrationData(); // Uncomment if needed

        // Update calibration label (if needed)
        // lv_label_set_text(lbl_voltageCalib_m, std::to_string(get_voltageCalib_m()).c_str());
    }
    else
    {
        // Handle menu navigation when calibration page is not visible
        static int32_t lastValue = 0;

        if (lastValue == encoder2_value)
            return;

        if (lastValue < encoder2_value)
            lastButton++;
        else if (lastValue > encoder2_value)
            lastButton--;

        lastValue = encoder2_value;

        int8_t temp = lastButton;
        lastButton = std::clamp(int(lastButton), 0, 6);

        lv_obj_t *theMenu = lv_obj_get_child(lv_menu_get_cur_sidebar_page(menu), 0);

        // Navigate the menu
        lv_event_send(lv_obj_get_child(theMenu, lastButton), LV_EVENT_CLICKED, NULL);

        if (temp == lastButton)
            myTone(NOTE_A4, 3); // Play a tone if selection didn't change
    }
}

void handleGraphPage(int32_t &encoder1_last_value, int32_t &encoder2_last_value)
{
    // Handle horizontal scrolling and zooming with encoder 2
    if (encoder2_last_value != encoder2_value)
    {
        // int32_t _posX = lv_slider_get_value(slider_x);
        static int32_t _posX = lv_slider_get_value(lv_obj_get_child(PowerSupply.page[1], 3));

        if (encoder2_last_value < encoder2_value)
            _posX += 160;
        else if (encoder2_last_value > encoder2_value)
            _posX -= 160;
        encoder2_last_value = encoder2_value;

        if (keyChar == 'W' && msg == " HOLD.")
        {
            // Scroll the graph horizontally
            _posX = std::clamp(_posX, 0, 57 * (lv_chart_get_zoom_x(PowerSupply.graph.chart) - 256) / 16);
            // _posY = std::clamp(_posY, 0, int(33.5 * (lv_chart_get_zoom_y(PowerSupply.graph.chart) - 256) / 64));
            if (!lvglChartIsBusy)
            {
                lvglChartIsBusy = true;
                lv_obj_scroll_to_x(PowerSupply.graph.chart, _posX, LV_ANIM_OFF);
                lvglChartIsBusy = false;
            }
        }
        else
        {
            // Zoom the graph horizontally
            _posX = std::clamp(_posX, 256, LV_IMG_ZOOM_NONE * 120);
            lv_slider_set_value(slider_x, _posX, LV_ANIM_OFF);

            if (!lvglChartIsBusy)
            {
                lvglChartIsBusy = true;
                lv_event_send(lv_obj_get_child(PowerSupply.page[1], 1), LV_EVENT_VALUE_CHANGED, NULL);
                lvglChartIsBusy = false;
            }
        }
    }

    // Handle vertical scrolling and zooming with encoder 1
    if (encoder1_last_value != encoder1_value)
    {
        static int32_t _posY = lv_slider_get_value(lv_obj_get_child(PowerSupply.page[1], 2));

        if (encoder1_last_value < encoder1_value)
            _posY += 64;
        else if (encoder1_last_value > encoder1_value)
            _posY -= 64;
        encoder1_last_value = encoder1_value;
        if (keyChar == 'W' && msg == " HOLD.")
        {
            // Scroll the graph vertically
            _posY = std::clamp(_posY, 0, int(33.5 * (lv_chart_get_zoom_y(PowerSupply.graph.chart) - 256) / 64));
            if (!lvglChartIsBusy)
            {
                lvglChartIsBusy = true;
                lv_obj_scroll_to_y(PowerSupply.graph.chart, _posY, LV_ANIM_OFF);
                lvglChartIsBusy = false;
            }
        }
        else
        {
            // Zoom the graph vertically
            _posY = std::clamp(_posY, 256, 5120);
            lv_slider_set_value(lv_obj_get_child(PowerSupply.page[1], 2), _posY, LV_ANIM_OFF);

            if (!lvglChartIsBusy)
            {
                lvglChartIsBusy = true;
                lv_event_send(lv_obj_get_child(PowerSupply.page[1], 2), LV_EVENT_VALUE_CHANGED, NULL);
                lvglChartIsBusy = false;
            }
        }
    }
}
void handleHistogramPage(int32_t &encoder1_last_value, int32_t &encoder2_last_value)
{
    bool histogramUpdated = false; // Flag to indicate if histogram needs to be reset

    // **Handle Vertical Shift/Zoom with Encoder 1**
    if (encoder1_last_value != encoder1_value)
    {
        int32_t _posY = 0;

        // Determine the direction of encoder 1 rotation
        if (encoder1_last_value < encoder1_value)
            _posY = 1; // Rotated clockwise
        else if (encoder1_last_value > encoder1_value)
            _posY = -1; // Rotated counter-clockwise

        encoder1_last_value = encoder1_value;
        if (keyChar == 'W' && msg == " HOLD.")
        {
            // **Shift the histogram window up or down**
            double shiftAmountVoltage = 0.025; // Adjust as needed
            double shiftAmountCurrent = 0.025; // Adjust as needed

            if (_posY > 0)
            {
                // Shift up
                PowerSupply.Voltage.hist.histWinMin += shiftAmountVoltage;
                PowerSupply.Voltage.hist.histWinMax += shiftAmountVoltage;

                PowerSupply.Current.hist.histWinMin += shiftAmountCurrent;
                PowerSupply.Current.hist.histWinMax += shiftAmountCurrent;
            }
            else if (_posY < 0)
            {
                // Shift down
                PowerSupply.Voltage.hist.histWinMin -= shiftAmountVoltage;
                PowerSupply.Voltage.hist.histWinMax -= shiftAmountVoltage;

                PowerSupply.Current.hist.histWinMin -= shiftAmountCurrent;
                PowerSupply.Current.hist.histWinMax -= shiftAmountCurrent;
            }
        }
        else
        {
            // **Zoom the histogram window vertically centered around the middle value**
            if (_posY > 0)
            {
                // Zoom in (reduce range)
                if (PowerSupply.stats.serV->hidden == 0)
                {
                    double midValueVoltage = (PowerSupply.Voltage.hist.histWinMin + PowerSupply.Voltage.hist.histWinMax) / 2.0;
                    double rangeVoltage = (PowerSupply.Voltage.hist.histWinMax - PowerSupply.Voltage.hist.histWinMin) / 2.0;
                    double newRangeVoltage = rangeVoltage * 0.5; // Zoom in by reducing the range by half
                    if (newRangeVoltage < .00004)
                        return;
                    PowerSupply.Voltage.hist.histWinMin = midValueVoltage - newRangeVoltage;
                    PowerSupply.Voltage.hist.histWinMax = midValueVoltage + newRangeVoltage;
                }
                if (PowerSupply.stats.serI->hidden == 0)
                {
                    double midValueCurrent = (PowerSupply.Current.hist.histWinMin + PowerSupply.Current.hist.histWinMax) / 2.0;
                    double rangeCurrent = (PowerSupply.Current.hist.histWinMax - PowerSupply.Current.hist.histWinMin) / 2.0;
                    double newRangeCurrent = rangeCurrent * 0.5;
                    if (newRangeCurrent < .00001)
                        return;
                    PowerSupply.Current.hist.histWinMin = midValueCurrent - newRangeCurrent;
                    PowerSupply.Current.hist.histWinMax = midValueCurrent + newRangeCurrent;
                }
            }
            else if (_posY < 0)
            {
                // Zoom out (increase range)
                if (PowerSupply.stats.serV->hidden == 0)
                {
                    double midValueVoltage = (PowerSupply.Voltage.hist.histWinMin + PowerSupply.Voltage.hist.histWinMax) / 2.0;
                    double rangeVoltage = (PowerSupply.Voltage.hist.histWinMax - PowerSupply.Voltage.hist.histWinMin) / 2.0;
                    double newRangeVoltage = rangeVoltage * 2.0; // Zoom out by doubling the range
                    if (newRangeVoltage > 50)
                        return;
                    PowerSupply.Voltage.hist.histWinMin = midValueVoltage - newRangeVoltage;
                    PowerSupply.Voltage.hist.histWinMax = midValueVoltage + newRangeVoltage;
                }
                if (PowerSupply.stats.serI->hidden == 0)
                {
                    double midValueCurrent = (PowerSupply.Current.hist.histWinMin + PowerSupply.Current.hist.histWinMax) / 2.0;
                    double rangeCurrent = (PowerSupply.Current.hist.histWinMax - PowerSupply.Current.hist.histWinMin) / 2.0;
                    double newRangeCurrent = rangeCurrent * 2.0;
                    if (newRangeCurrent > 25)
                        return;
                    PowerSupply.Current.hist.histWinMin = midValueCurrent - newRangeCurrent;
                    PowerSupply.Current.hist.histWinMax = midValueCurrent + newRangeCurrent;
                }
            }
        }

        encoder1_last_value = encoder1_value;
        histogramUpdated = true; // Mark histogram as updated
    }

    // **Handle Horizontal Shift with Encoder 2**
    if (encoder2_last_value != encoder2_value)
    {
        int32_t _posX = 0;

        // Determine the direction of encoder 2 rotation
        if (encoder2_last_value < encoder2_value)
            _posX = 1; // Rotated clockwise
        else if (encoder2_last_value > encoder2_value)
            _posX = -1; // Rotated counter-clockwise
        encoder2_last_value = encoder2_value;
        // Shift the histogram window left or right by 10% of the window size
        double windowSizeVoltage = PowerSupply.Voltage.hist.histWinMax - PowerSupply.Voltage.hist.histWinMin;
        double shiftAmountVoltage = 0.10 * windowSizeVoltage; // 10% of voltage window size

        double windowSizeCurrent = PowerSupply.Current.hist.histWinMax - PowerSupply.Current.hist.histWinMin;
        double shiftAmountCurrent = 0.10 * windowSizeCurrent; // 10% of current window size

        if (_posX > 0)
        {
            // Shift right
            if (PowerSupply.stats.serV->hidden == 0)
            {
                if (PowerSupply.Voltage.hist.histWinMin > 50)
                    return;
                PowerSupply.Voltage.hist.histWinMin += shiftAmountVoltage;
                PowerSupply.Voltage.hist.histWinMax += shiftAmountVoltage;
            }
            if (PowerSupply.stats.serI->hidden == 0)
            {
                if (PowerSupply.Current.hist.histWinMin > 20)
                    return;

                PowerSupply.Current.hist.histWinMin += shiftAmountCurrent;
                PowerSupply.Current.hist.histWinMax += shiftAmountCurrent;
            }
        }
        else if (_posX < 0)
        {
            // Shift left
            if (PowerSupply.stats.serV->hidden == 0)
            {
                if (PowerSupply.Voltage.hist.histWinMin < -50)
                    return;
                PowerSupply.Voltage.hist.histWinMin -= shiftAmountVoltage;
                PowerSupply.Voltage.hist.histWinMax -= shiftAmountVoltage;
            }
            if (PowerSupply.stats.serI->hidden == 0)
            {
                if (PowerSupply.Current.hist.histWinMin < -20)
                    return;
                PowerSupply.Current.hist.histWinMin -= shiftAmountCurrent;
                PowerSupply.Current.hist.histWinMax -= shiftAmountCurrent;
            }
        }

        histogramUpdated = true; // Mark histogram as updated
    }

    // **Reset Histograms if Updated**
    if (histogramUpdated)
    {
        // Reset histograms to apply new window settings
        if (PowerSupply.stats.serV->hidden == 0)
            PowerSupply.Voltage.hist.Reset();
        if (PowerSupply.stats.serI->hidden == 0)
            PowerSupply.Current.hist.Reset();
    }
}

void managePageInteraction()
{
    // Variables to store the last encoder values
    static int32_t encoder1_last_value = 0;
    static int32_t encoder2_last_value = 0;

    // Get the current page from the Tabs
    switch (Tabs::getCurrentPage())
    {
    case 0: // Histogram Page
        handleHistogramPage(encoder1_last_value, encoder2_last_value);
        break;

    case 1: // Graph Page
        handleGraphPage(encoder1_last_value, encoder2_last_value);
        break;

    case 4: // Calibration Page
        handleCalibrationPage(encoder1_last_value, encoder2_last_value);
        break;

    default:
        break;
    }
}

// Might be in the llop
void MiscPriority()
{
    if (ismyTextHiddenChange && false)
    {

        if (priorityFlag != 1 && lv_obj_has_flag(myTextBox, LV_OBJ_FLAG_HIDDEN)) // lv_obj_has_flag(myTextBox, LV_OBJ_FLAG_HIDDEN) || !
        {
            priorityFlag = 1;
            vTaskPrioritySet(Task_adc, priorityFlag);
        }

        else if (priorityFlag != 0 && !lv_obj_has_flag(myTextBox, LV_OBJ_FLAG_HIDDEN))
        {
            priorityFlag = 0;
            vTaskPrioritySet(Task_adc, priorityFlag);
        }

        // Serial.printf("\nPriority changes! %i", priorityFlag);
        ismyTextHiddenChange = false;
    }
    if (Serial.available() > 0)
    {
        String received_command = Serial.readStringUntil('\n');
        Serial.print("\nUploading .......");

        pinMode(PowerSupply.CCCVPin, INPUT_PULLDOWN);
        digitalWrite(PowerSupply.CCCVPin, INPUT_PULLDOWN);
        PowerSupply.turn(SWITCH::OFF);
        PowerSupply.Current.SetUpdate(-0.001 - PowerSupply.Current.adjOffset);
    }
    // LV_LOG_USER("%i",digitalRead(PowerSupply.CCCVPin));
    // if (g_wifiConnection)
    // ArduinoOTA.handle();
    // Serial.printf("3 \n");
}