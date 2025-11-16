#pragma once

#include <iostream>
#include <chrono>
#include <thread>
// #include "config.hpp"
#include "input_device.h"
#include "setting_menu.h"
// #include "setting_menu2.h"
#include <string>
#include <math.h>
#include "spinbox_pro.h"
#include "SpinboxPro.hpp"
#include "table_pro.h"

#include "setting_menu.h"
#include "globals.h"
#include "functions.h"
#include "ui_helpers.h"
#include "tasks.h"
#include "input_handler.h"
#include "intervals.h"

// Forward declarations for functions defined later in this file
void getSettingEncoder(lv_indev_drv_t *drv, lv_indev_data_t *data);
bool functionGenerator();

// struct FunGen
// {
//     double frequency;
//     double amplitude;
//     double offset;
//     double dutyCycle;

// };
// FunGen funGen;

// bool chartPause = false;

#define LV_TICK_CUSTOM 1

/**
 * @brief Construct a new ESP32Encoder object
 * @param ...
}

/********************************************************/
void btn_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);
    // Serial.printf("Code: %i",code );
    if (code == LV_EVENT_VALUE_CHANGED)
    {
        if (lv_obj_get_state(btn) & LV_STATE_CHECKED)
        {
            myTone(NOTE_A5, 200);
            PowerSupply.setStatus(DEVICE::ON);
        }
        else
        {
            myTone(NOTE_A3, 200);
            PowerSupply.setStatus(DEVICE::OFF);
        }
    }
}

/* Source: https://github.com/ankur219/ECG-Arrhythmia-classification/blob/642230149583adfae1e4bd26c6f0e1fd8af2be0e/sample.csv*/
// static lv_coord_t ecg_sample[800] = {0};

void slider_x_event_cb(lv_event_t *e)
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
    int value = (uint16_t)lv_slider_get_value(obj);
    lv_chart_set_zoom_x(PowerSupply.graph.chart, value);

    globalSliderXValue = value;
    // encoder2_value = value;

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

static void slider_x_event_enc_cb(lv_event_t *e)
{
    if (!e)
        return; // Safety check
    lv_obj_t *obj = lv_event_get_target(e);
    if (!obj)
        return; // Safety check
    int value = (uint16_t)lv_slider_get_value(obj);
    // lv_chart_set_zoom_x(PowerSupply.graph.chart, value);

    encoder2_value = value;
}
void slider_y_event_cb(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);
    int32_t v = lv_slider_get_value(obj);
    lv_coord_t v0 = lv_chart_get_zoom_y(PowerSupply.graph.chart);
    lv_chart_set_zoom_y(PowerSupply.graph.chart, (uint16_t)v);
    lv_coord_t y = lv_obj_get_scroll_y(PowerSupply.graph.chart);
    // lv_obj_scroll_to_y(PowerSupply.graph.chart, y * v / v0 + 1, LV_ANIM_OFF);
    lv_obj_scroll_to_y(PowerSupply.graph.chart, 32000, LV_ANIM_OFF);
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

lv_obj_t *slider_x;

void draw_event_stat_chart_cb(lv_event_t *e)
{
    lv_obj_draw_part_dsc_t *dsc = lv_event_get_draw_part_dsc(e);

    // Check if the part being drawn is a tick label on the primary X-axis
    if (dsc->id == LV_CHART_AXIS_PRIMARY_Y &&
        dsc->text && dsc->label_dsc && dsc->part == LV_PART_TICKS)

    {
        dsc->label_dsc->font = &lv_font_montserrat_10;
        dsc->label_dsc->color = lv_color_hex(0xFFFFFF);
    }

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

// tickLabels_x moved to globals.h/cpp

void draw_event_cb2(lv_event_t *e)
{
    lv_obj_draw_part_dsc_t *dsc = lv_event_get_draw_part_dsc(e);

    if (!dsc)
        return;

    /* Customize division lines */
    if (dsc->part == LV_PART_MAIN)
    {
        if (!dsc->line_dsc || !dsc->p1 || !dsc->p2)
            return;

        dsc->line_dsc->color = lv_palette_main(LV_PALETTE_GREY);

        /* Vertical lines */
        if (dsc->p1->x == dsc->p2->x)
        {
            if (dsc->id == 0)
            {
                dsc->line_dsc->width = 1;
                dsc->line_dsc->dash_gap = 0;
                dsc->line_dsc->dash_width = 0;
            }
            else
            {
                dsc->line_dsc->width = 1;
                dsc->line_dsc->dash_gap = 5;
                dsc->line_dsc->dash_width = 5;
            }
            return;
        }
        /* Horizontal lines */
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

    /* Customize tick labels */
    if (dsc->part == LV_PART_TICKS && dsc->text)
    {
        /* Handle LV_CHART_AXIS_PRIMARY_X */
        if (dsc->id == LV_CHART_AXIS_PRIMARY_X)
        {
            for (int i = 0; i < NUM_LABELS; i++)
            {
                if (i == NUM_LABELS - 1)
                {
                    strcpy(tickLabels_x[i], "0 pts");
                }
                else
                {
                    int value = CHART_SIZE * (NUM_LABELS - 1 - i) / (NUM_LABELS - 1);
                    sprintf(tickLabels_x[i], "%d", value);
                }
            }

            static int index_x = 0;
            // static char *tickLabels_x[] = {"300", "250", "200", "150", "100", "50", "0 pts"};
            // Initialize tick labels based on CHART_SIZE and fractions

            if (index_x == 7)
                index_x = 0;

            lv_snprintf(dsc->text, dsc->text_length, "%s", tickLabels_x[index_x++]);

            if (dsc->label_dsc)
                dsc->label_dsc->font = &lv_font_montserrat_10;
        }
        /* Handle LV_CHART_AXIS_PRIMARY_Y */
        else if (dsc->id == LV_CHART_AXIS_PRIMARY_Y)
        {
            static int index_y = 0;
            static char *tickLabels_y[] = {"32.0V", "28.0", "24.0", "20.0", "16.0", "12.0", "8.0", "4.0", "0.0"};

            if (strcmp(dsc->text, "32000") == 0)
                index_y = 0;

            lv_snprintf(dsc->text, dsc->text_length, "%s", tickLabels_y[index_y++]);

            if (dsc->label_dsc)
                dsc->label_dsc->font = &lv_font_montserrat_10;
        }
        /* Handle LV_CHART_AXIS_SECONDARY_Y */
        else if (dsc->id == LV_CHART_AXIS_SECONDARY_Y)
        {
            static int index_sy = 0;
            static char *tickLabels_sy[] = {"8.0A", "7.0", "6.0", "5.0", "4.0", "3.0", "2.0", "1.0", "0.0"};

            if (strcmp(dsc->text, "8000") == 0)
                index_sy = 0;

            lv_snprintf(dsc->text, dsc->text_length, "%s", tickLabels_sy[index_sy++]);

            if (dsc->label_dsc)
                dsc->label_dsc->font = &lv_font_montserrat_10;
        }
    }
}

static void draw_event_cb2_old(lv_event_t *e)
{
    // lv_obj_t *obj = lv_event_get_target(e);
    /*Hook the division lines too*/
    lv_obj_draw_part_dsc_t *dsc = lv_event_get_draw_part_dsc(e);
    // if (!lv_obj_draw_part_check_type(dsc, &lv_chart_class, LV_CHART_DRAW_PART_TICK_LABEL))
    //     return;

    // int VI_hidden = 0;

    // if (PowerSupply.graph.serV->hidden == PowerSupply.graph.serI->hidden &&
    //     PowerSupply.graph.serV->hidden == false)
    //     VI_hidden = 3;
    // else if (PowerSupply.graph.serV->hidden == false)
    //     VI_hidden = 1;
    // else if (PowerSupply.graph.serI->hidden == false)
    //     VI_hidden = 2;

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

            if (dsc->label_dsc)
                dsc->label_dsc->font = &lv_font_montserrat_10; // Replace with your desired font
        }
        else if (dsc->id == LV_CHART_AXIS_SECONDARY_Y)
        {
            static int i = 0;

            static char *tickLabel_sy[] = {"8.0A", "7.0", "6.0", "5.0", "4.0", "3.0", "2.0", "1.0", "0.0"};

            if (strcmp(dsc->text, "8000") == 0)
                i = 0;
            lv_snprintf(dsc->text, dsc->text_length, "%s", tickLabel_sy[i++]);

            if (dsc->label_dsc)
                dsc->label_dsc->font = &lv_font_montserrat_10; // Replace with your desired font
        }

        else if (dsc->id == LV_CHART_AXIS_PRIMARY_Y)
        {
            static int i = 0;
            static char *tickLabel_y[] = {"32.0\nVolt", "28.0", "24.0", "20.0", "16.0", "12.0", "8.0", "4.0", "0.0"};

            if (strcmp(dsc->text, "32000") == 0)
                i = 0;
            lv_snprintf(dsc->text, dsc->text_length, "%s", tickLabel_y[i++]);

            if (dsc->label_dsc)
                dsc->label_dsc->font = &lv_font_montserrat_10; // Replace with your desired font
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
// touch_calibrate() and TouchAttr moved to input_handler.cpp

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 180
#define BUCKET_COUNT 100 // Reduce X buckets from 320 to 100
#define DOT_SIZE 3
#define CHART_Y_MAX 100 // Adjust this to match your lv_chart_set_range()
#define CHART_POINTS 20 // Adjust this to match your lv_chart_set_range()

// dataBuckets, dropdown_active, util_Arbit_chart moved to globals.h/cpp

// Update a single X with a new Y
// void plotToBucket2(uint16_t x, uint16_t y, TFT_eSPI &tft)
// {
//     if (x >= SCREEN_WIDTH)
//         return; // Out of bounds check

//     uint16_t bucketX = x * BUCKET_COUNT / SCREEN_WIDTH; // Map x to 100 buckets

//     if (Tabs::getCurrentPage() == 3 && lv_tabview_get_tab_act(tabview_utility) == 2)
//     {
//         lv_obj_clear_flag(lv_obj_get_parent(PowerSupply.page[3]), LV_OBJ_FLAG_SCROLLABLE);

//         if (y > 44 && y < 170 && x > 5)
//         {
//             // Erase old point by drawing background color
//             tft.fillRect(bucketX * (SCREEN_WIDTH / BUCKET_COUNT), dataBuckets[bucketX], DOT_SIZE, DOT_SIZE, TFT_BLACK);

//             // Store new value
//             dataBuckets[bucketX] = y;

//             // Draw new 3x3 dot
//             tft.fillRect(bucketX * (SCREEN_WIDTH / BUCKET_COUNT), y, DOT_SIZE, DOT_SIZE, TFT_CYAN);
//         }
//     }
// }

// void plotToBucket3(uint16_t x, uint16_t y, lv_obj_t *chart, lv_chart_series_t *series)
// {
//     if (!chart || !series) return;

//     // Swap X and Y values
//     uint16_t temp = x;
//     x = y;
//     y = CHART_Y_MAX - temp;  // Invert Y so 0 is at the bottom

//     // Prevent scrolling during update
//     // lv_obj_clear_flag(chart, LV_OBJ_FLAG_SCROLLABLE);
//     lv_obj_add_flag(lv_obj_get_parent(PowerSupply.page[3]), LV_OBJ_FLAG_SCROLLABLE);

//     // Set value
//     lv_chart_set_value_by_id(chart, series, x/(CHART_POINTS*3.0), y/1.4+10);

//     // Refresh the chart
//     lv_chart_refresh(chart);

//     // Re-enable scrolling after update
//     // lv_obj_add_flag(chart, LV_OBJ_FLAG_SCROLLABLE);
// }
//         lv_obj_add_flag(lv_obj_get_parent(PowerSupply.page[3]), LV_OBJ_FLAG_SCROLLABLE);
//     }
// }

// my_touchpad_read() and init_touch() moved to input_handler.cpp

// Task_BarGraph and Task_ADC moved to tasks.cpp


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
            lv_obj_add_flag(PowerSupply.gui.textarea_set_value, LV_OBJ_FLAG_HIDDEN);
            ismyTextHiddenChange = true;
        }
        lv_textarea_del_char(ta);
    }
    else if (strcmp(txt, LV_SYMBOL_NEW_LINE) == 0)
        lv_event_send(ta, LV_EVENT_READY, NULL);
    else if (strcmp(txt, "E") == 0)
    {
        lv_obj_add_flag(PowerSupply.gui.textarea_set_value, LV_OBJ_FLAG_HIDDEN);
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
        lv_obj_add_flag(PowerSupply.gui.textarea_set_value, LV_OBJ_FLAG_HIDDEN);
        myTone(NOTE_A5, 100);
        const char *txt = lv_textarea_get_text(ta);
        // PowerSupply.Voltage.SetUpdate(round(strtod(txt, NULL) * 2000.0)  - 0 * PowerSupply.Voltage.adjOffset);
        PowerSupply.Voltage.SetUpdate((strtod(txt, NULL) * PowerSupply.Voltage.adjFactor) + PowerSupply.Voltage.adjOffset);
        lv_textarea_set_text(ta, "");
        ismyTextHiddenChange = true;
    }

    else if (strcmp(txt, "mV") == 0)
    {
        lv_obj_add_flag(PowerSupply.gui.textarea_set_value, LV_OBJ_FLAG_HIDDEN);
        myTone(NOTE_A5, 100);
        const char *txt = lv_textarea_get_text(ta);
        PowerSupply.Voltage.SetUpdate((strtod(txt, NULL) * 2.0) + PowerSupply.Voltage.adjOffset);
        lv_textarea_set_text(ta, "");
        ismyTextHiddenChange = true;
    }

    else if (strcmp(txt, "A") == 0)
    {
        lv_obj_add_flag(PowerSupply.gui.textarea_set_value, LV_OBJ_FLAG_HIDDEN);
        myTone(NOTE_A5, 100);
        const char *txt = lv_textarea_get_text(ta);
        PowerSupply.Current.SetUpdate(strtod(txt, NULL) * PowerSupply.Current.adjFactor + PowerSupply.Current.adjOffset);
        lv_textarea_set_text(ta, "");
        ismyTextHiddenChange = true;
    }

    else if (strcmp(txt, "mA") == 0)
    {
        lv_obj_add_flag(PowerSupply.gui.textarea_set_value, LV_OBJ_FLAG_HIDDEN);
        myTone(NOTE_A5, 100);
        const char *txt = lv_textarea_get_text(ta);
        PowerSupply.Current.SetUpdate(strtod(txt, NULL) * 10.0 + PowerSupply.Current.adjOffset);
        lv_textarea_set_text(ta, "");
        ismyTextHiddenChange = true;
    }
    else
        lv_textarea_add_text(ta, txt);
}
static void key_event_handler(uint16_t btn_id)
{
    lv_obj_clear_flag(PowerSupply.gui.textarea_set_value, LV_OBJ_FLAG_HIDDEN);
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
        lv_textarea_set_text(ta, std::to_string((dp.adjValue - PowerSupply.Voltage.adjOffset) / PowerSupply.Voltage.adjFactor).c_str());
        lv_label_set_text(unit_label, "V");
    }
    else
    {
        lv_textarea_set_text(ta, std::to_string((dp.adjValue - PowerSupply.Current.adjOffset) / PowerSupply.Current.adjFactor).c_str());
        lv_label_set_text(unit_label, "A");
    }

    lv_textarea_del_char(ta);
    lv_textarea_del_char(ta);
}

static void key_event_handler_readBack_k(DispObjects dp)
{
    if (strcmp(lv_label_get_text(dp.label_unit), "V") == 0)
    {
        lv_textarea_set_text(ta, std::to_string((dp.adjValue - PowerSupply.Voltage.adjOffset) / 2.0).c_str());
        lv_label_set_text(unit_label, "mV");
    }
    else
    {
        lv_textarea_set_text(ta, std::to_string((dp.adjValue - PowerSupply.Current.adjOffset) / 10.0).c_str());
        lv_label_set_text(unit_label, "mA");
    }

    lv_textarea_del_char(ta);
    lv_textarea_del_char(ta);
    lv_textarea_del_char(ta);
    lv_textarea_del_char(ta);
    lv_textarea_del_char(ta);
}

static lv_style_t style_font_unit;
static lv_style_t style_font;
static lv_style_t style_font_btnm;
static lv_style_t style_backgound;

void textarea(lv_obj_t *parent)
{

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
    PowerSupply.gui.textarea_set_value = lv_obj_create(lv_scr_act());
    lv_obj_set_size(PowerSupply.gui.textarea_set_value, 320, 240);
    lv_style_init(&style_backgound);

    lv_style_set_bg_opa(&style_backgound, LV_OPA_80);
    lv_obj_remove_style(PowerSupply.gui.textarea_set_value, &style_backgound, LV_STATE_DEFAULT);
    lv_obj_add_style(PowerSupply.gui.textarea_set_value, &style_backgound, LV_STATE_DEFAULT);

    ta = lv_textarea_create(PowerSupply.gui.textarea_set_value);
    lv_textarea_set_one_line(ta, true);
    lv_obj_align(ta, LV_ALIGN_TOP_MID, 0, 50);
    lv_obj_add_event_cb(ta, textarea_event_handler, LV_EVENT_READY, ta);

    lv_obj_remove_style(ta, &style_font, LV_STATE_DEFAULT);
    lv_obj_add_style(ta, &style_font, LV_STATE_DEFAULT);
    lv_obj_add_state(ta, LV_STATE_FOCUSED); /*To be sure the cursor is visible*/

    lv_obj_t *unit_box = lv_obj_create(PowerSupply.gui.textarea_set_value);
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
                                     "0", ".", "E", LV_SYMBOL_NEW_LINE, "+", NULL};

    lv_style_init(&style_font_btnm);
    // lv_style_set_bg_opa(&style_font, LV_OPA_50);
    // lv_style_set_bg_color(&style_font_btnm, lv_palette_main(LV_PALETTE_LIGHT_GREEN));

    lv_style_set_text_font(&style_font_btnm, &lv_font_montserrat_20); // graph_R_16

    btnm = lv_btnmatrix_create(PowerSupply.gui.textarea_set_value);
    lv_obj_remove_style_all(btnm);
    // return;
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
double scaleVoltage(uint16_t voltage)
{
    return (voltage - PowerSupply.Voltage.adjOffset) / PowerSupply.Voltage.adjFactor;
}

double scaleCurrent(uint16_t current)
{
    return (current - PowerSupply.Current.adjOffset) / PowerSupply.Current.adjFactor;
}

void loadMemory(lv_obj_t *btn)
{
    myTone(NOTE_A4, 50, true);
    int memAddress = (int)btn->user_data;
    Serial.printf("\n ****** Load from : %i ", memAddress);
    MemArray mem = PowerSupply.LoadMemory("myDataKey");

    PowerSupply.Voltage.SetUpdate(mem.memory[memAddress].voltage);
    PowerSupply.Current.SetUpdate(mem.memory[memAddress].current);
    Tabs::setCurrentPage(2);
}

void saveMemory(lv_obj_t *btn)
{
    myTone(NOTE_A4, 150);
    int memAddress = (int)btn->user_data;
    Serial.printf("\n ****** Save to : %i ", memAddress);
    MemArray mem = PowerSupply.LoadMemory("myDataKey");
    mem.memory[memAddress].voltage = PowerSupply.Voltage.adjValue;

    mem.memory[memAddress].current = PowerSupply.Current.adjValue;
    PowerSupply.SaveMemory("myDataKey", mem);

    lv_obj_t *labelV = lv_obj_get_child(btn, 0);
    lv_obj_t *labelI = lv_obj_get_child(labelV, 0);
    lv_label_set_text_fmt(labelV, "%+08.4fV", scaleVoltage(mem.memory[memAddress].voltage));
    lv_label_set_text_fmt(labelI, "%+08.4fA", scaleCurrent(mem.memory[memAddress].current));

    Serial.printf("\n ****** Saved : %+08.4fV", scaleVoltage(mem.memory[memAddress].voltage));
}

static void mem_btn_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);
    // Serial.printf("Code: %i",code );
    if (code == LV_EVENT_SHORT_CLICKED)
        loadMemory(btn);

    if (code == LV_EVENT_LONG_PRESSED)
        saveMemory(btn);
}
//**********************************************************************************/
// Define the style for the selected row
// Event callback to handle row selection
static void table_get_event_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED)
    {
        lv_obj_t *table = lv_event_get_target(e);
        uint16_t row, col;
        lv_table_get_selected_cell(table, &row, &col);
        // table->user_data = (void *)row;
        // lv_obj_invalidate(table); // Redraw the table to apply changes
        const char *cell_str = lv_table_get_cell_value(table, row, 1);
        Utility_objs.table_current_value = atof(cell_str);
        lv_spinbox_set_value(Utility_objs.table_spinbox_value, Utility_objs.table_current_value);
    }
}

// Event callback to handle row selection
static void table_touch_event_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED)
    {
        lv_obj_t *table = lv_event_get_target(e);
        uint16_t row, col;
        lv_table_get_selected_cell(table, &row, &col);
        table->user_data = (void *)row;
        lv_obj_invalidate(table); // Redraw the table to apply changes
        lv_table_get_cell_value(table, row, col);

        const char *cell_str = lv_table_get_cell_value(table, row, 1);
        Utility_objs.table_current_value = atof(cell_str);
        lv_spinbox_set_value(Utility_objs.table_spinbox_value, Utility_objs.table_current_value * 10000.0);
        lv_obj_invalidate(Utility_objs.table_spinbox_value);
        remove_selected_spinbox();
    }
}

// Custom draw callback for the table
static void table_draw_cell_event_cb(lv_event_t *e)
{
    lv_obj_draw_part_dsc_t *draw_part_dsc = (lv_obj_draw_part_dsc_t *)lv_event_get_param(e);

    if (draw_part_dsc->part == LV_PART_ITEMS)
    {
        lv_obj_t *table = lv_event_get_target(e);
        uint32_t id = draw_part_dsc->id;
        uint16_t col_cnt = lv_table_get_col_cnt(table);

        // Calculate row and column from cell id
        uint16_t row = id / col_cnt;
        uint16_t col = id % col_cnt;
        uint16_t selected_row = (int)table->user_data;

        if (row == selected_row)
        {
            draw_part_dsc->rect_dsc->bg_color = lv_color_hex(0x0000FF); // Highlight color
            draw_part_dsc->rect_dsc->bg_opa = LV_OPA_COVER;
        }
    }
}

// Function to select the next row
void select_next_row(lv_obj_t *table, lv_coord_t row_height)
{
    uint16_t row_cnt = lv_table_get_row_cnt(table);
    // if (selected_row < row_cnt - 1)
    // {
    //     selected_row++;
    // }
    uint16_t cur_row_number = (int)table->user_data;

    if (cur_row_number < row_cnt - 1)
    {
        cur_row_number++;
        table->user_data = (void *)cur_row_number;
    }

    lv_coord_t scroll_y = lv_obj_get_scroll_y(table);
    lv_coord_t visible_h = lv_obj_get_height(table);
    lv_coord_t y_pos = cur_row_number * row_height;

    if (y_pos < scroll_y)
        lv_obj_scroll_to_y(table, y_pos, LV_ANIM_OFF);
    else if (y_pos + row_height > scroll_y + visible_h)
        lv_obj_scroll_to_y(table, y_pos + row_height - visible_h, LV_ANIM_OFF);

    lv_obj_invalidate(table);

    const char *cell_str = lv_table_get_cell_value(table, cur_row_number, 1);
    Utility_objs.table_current_value = atof(cell_str);
    lv_spinbox_set_value(Utility_objs.table_spinbox_value, Utility_objs.table_current_value * 10000.0);
    lv_obj_invalidate(Utility_objs.table_spinbox_value);
}

// Function to select the previous row
void select_previous_row(lv_obj_t *table, lv_coord_t row_height)
{
    // uint16_t row_cnt = lv_table_get_row_cnt(table);
    // if (selected_row > 0)
    // {
    //     selected_row--;
    // }

    int cur_row_number = (int)table->user_data;

    if (cur_row_number > 0)
    {
        cur_row_number--;
        table->user_data = (void *)cur_row_number;
    }

    lv_coord_t scroll_y = lv_obj_get_scroll_y(table);
    lv_coord_t visible_h = lv_obj_get_height(table);
    lv_coord_t y_pos = cur_row_number * row_height;

    if (y_pos < scroll_y)
        lv_obj_scroll_to_y(table, y_pos, LV_ANIM_OFF);
    else if (y_pos + row_height > scroll_y + visible_h)
        lv_obj_scroll_to_y(table, y_pos + row_height - visible_h, LV_ANIM_OFF);

    lv_obj_invalidate(table);

    const char *cell_str = lv_table_get_cell_value(table, cur_row_number, 1);
    Utility_objs.table_current_value = atof(cell_str);
    lv_spinbox_set_value(Utility_objs.table_spinbox_value, Utility_objs.table_current_value * 10000.0);
    lv_obj_invalidate(Utility_objs.table_spinbox_value);
}

void btn_function_gen_event_cb(lv_event_t *e)
{

    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);
    lv_obj_t *label = lv_obj_get_child(btn, 0);

    if (lv_obj_has_state(btn_function_gen, LV_STATE_CHECKED))
    {
        lv_label_set_text(label, "ON");
        if (PowerSupply.getStatus() != DEVICE::FUN)
            PowerSupply.setStatus(DEVICE::FUN);
    }
    else
    {
        lv_label_set_text(label, "OFF");
        PowerSupply.setStatus(DEVICE::ON);
    }

    // if (get_selected_spinbox())
    // {
    //     remove_red_border(get_selected_spinbox());
    //     get_selected_spinbox() = nullptr;
    // }
    remove_selected_spinbox();
    // Serial.printf("\nDebug check points!");
    // trackLoopExecution(__func__);
}

static void spinbox_change_event_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED)
    {
        lv_obj_t *spinbox = lv_event_get_target(e);
        double value = lv_spinbox_get_value(spinbox) / 10000.0;

        int row = (int)Utility_objs.table_point_list->user_data;
        lv_table_set_cell_value_fmt(Utility_objs.table_point_list, row, 1, "%06.4f", value);
        // Utility_objs.table_points[row] = value;
        PowerSupply.funGenMem.table_points[row][0] = value;
    }
}

#define NUM_LABELS 7

char tickLabels_x_II[NUM_LABELS][10]; // 7 labels, max 9 characters each

static void draw_event_util_Arbit_chart_cb(lv_event_t *e)
{
    lv_obj_draw_part_dsc_t *dsc = lv_event_get_draw_part_dsc(e);

    if (!dsc)
        return;

    /* Customize division lines */
    if (dsc->part == LV_PART_MAIN)
    {
        if (!dsc->line_dsc || !dsc->p1 || !dsc->p2)
            return;

        dsc->line_dsc->color = lv_palette_main(LV_PALETTE_GREY);
    }

    /* Customize tick labels */
    if (dsc->part == LV_PART_TICKS && dsc->text)
    {
        /* Handle LV_CHART_AXIS_PRIMARY_X */
        if (dsc->id == LV_CHART_AXIS_PRIMARY_X)
        {
            // for (int i = 0; i < NUM_LABELS; i++)
            // {
            //     if (i == NUM_LABELS - 1)
            //     {
            //         strcpy(tickLabels_x_II[i], "0 pts");
            //     }
            //     else
            //     {
            //         int value = CHART_SIZE * (NUM_LABELS - 1 - i) / (NUM_LABELS - 1);
            //         sprintf(tickLabels_x_II[i], "%d", value);
            //     }
            // }

            static int index_x = 0;
            static char *tickLabels_x_II[] = {"40", "30", "20", "10", "0 pts"};
            // Initialize tick labels based on CHART_SIZE and fractions

            if (index_x == 5)
                index_x = 0;

            lv_snprintf(dsc->text, dsc->text_length, "%s", tickLabels_x_II[index_x++]);

            if (dsc->label_dsc)
                dsc->label_dsc->font = &lv_font_montserrat_10;
        }
        /* Handle LV_CHART_AXIS_PRIMARY_Y */
        else if (dsc->id == LV_CHART_AXIS_PRIMARY_Y)
        {
            static int index_y = 0;
            static char *tickLabels_y[] = {"1.0", "0.5", "0.0"};

            // if (strcmp(dsc->text, "32000") == 0)
            //     index_y = 0;
            if (index_y == 3)
                index_y = 0;

            lv_snprintf(dsc->text, dsc->text_length, "%s", tickLabels_y[index_y++]);

            if (dsc->label_dsc)
                dsc->label_dsc->font = &lv_font_montserrat_8;
        }
        /* Handle LV_CHART_AXIS_SECONDARY_Y */
    }
}
void saveToBank(int bank)
{
    LV_LOG_USER("Saved settings to Bank %d", bank);

    // Construct memory bank name dynamically
    char bankName[32];
    snprintf(bankName, sizeof(bankName), "Arbitrary_Bank_%d", bank);

    // Get the y-array from the chart
    lv_coord_t *y_array = lv_chart_get_y_array(util_Arbit_chart, util_Arbit_chart_series);
    if (!y_array)
    {
        LV_LOG_USER("Error: y_array is NULL.");
        return;
    }

    // Save data points from chart to memory
    for (int i = 0; i < CHART_POINTS; i++)
    {
        PowerSupply.funGenArbitraryMem.arbitrary_points[i][bank] = y_array[i]; // Store y-values in PowerSupply.funGenMem
    }

    // Save function generator memory
    PowerSupply.SaveMemoryFgen("Arbit", PowerSupply.funGenArbitraryMem);
}

void loadFromBank(int bank)
{
    LV_LOG_USER("Loaded settings from Bank %d", bank);
    // Implement loading logic here
    PowerSupply.funGenArbitraryMem = PowerSupply.LoadMemoryFgen("Arbit");
    // Fill with zero data initially
    for (int i = 0; i < CHART_POINTS; i++)
    {
        lv_chart_set_next_value(util_Arbit_chart, util_Arbit_chart_series, PowerSupply.funGenArbitraryMem.arbitrary_points[i][bank]);

        // lv_chart_set_next_value(util_Arbit_chart, util_Arbit_chart_series, 0);
    }
};

static void dropdownEventCb(lv_event_t *e)
{

    // Lambda functions for handling save/load operations

    lv_obj_t *dropdown = lv_event_get_target(e);
    char buf[64];
    lv_dropdown_get_selected_str(dropdown, buf, sizeof(buf));
    LV_LOG_USER("'%s' is selected", buf);

    dropdown_active = false;

    std::string selection(buf);
    if (selection == "Save to Bank 0")
    {
        // LV_LOG_USER("Saving to Bank 0");
        saveToBank(0);
    }
    else if (selection == "Save to Bank 1")
    {
        // LV_LOG_USER("Saving to Bank 1");
        saveToBank(1);
    }
    else if (selection == "Load to Bank 0")
    {
        // LV_LOG_USER("Loading from Bank 0");
        loadFromBank(0);
    }
    else if (selection == "Load to Bank 1")
    {
        // LV_LOG_USER("Loading from Bank 1");
        loadFromBank(1);
    }
    else
    {
        LV_LOG_USER("Unknown selection: %s", buf);
        // int selected_row = (int)Utility_objs.table_fun_gen_list->user_data+1;
        int selected_row = lv_dropdown_get_selected(dropdown) - 1; // Get selected index
        if (selected_row == -1)
        {
            // Clear the chart (optional)
            lv_chart_set_all_value(util_Arbit_chart, util_Arbit_chart_series, 0);
            lv_chart_refresh(util_Arbit_chart); // Refresh chart
            return;
        }
        Waveform currentWaveform = waveforms[selected_row];

        for (int i = 0; i < CHART_POINTS; i++)
        {
            double value = currentWaveform.function((double)i / CHART_POINTS); // Map i to function
            double outputValue = value * 1.0;
            // PowerSupply.funGenArbitraryMem.arbitrary_points[i][0] = outputValue; // Store in memory
            lv_chart_set_value_by_id(util_Arbit_chart, util_Arbit_chart_series, i, outputValue * 140);
        }

        lv_chart_refresh(util_Arbit_chart); // Refresh chart
    }
}

void Utility_tabview(lv_obj_t *parent)
{
    lv_obj_set_size(parent, 320, 216);

    static lv_style_t style_utility;
    static lv_style_t style_btn;
    lv_style_init(&style_utility);
    lv_style_init(&style_btn);

    lv_style_set_pad_all(&style_utility, 0);
    lv_obj_add_style(parent, &style_utility, LV_STATE_DEFAULT);
    lv_style_set_text_letter_space(&style_utility, 0);
    lv_style_set_text_color(&style_utility, lv_palette_main(LV_PALETTE_GREY));
    lv_style_set_text_font(&style_utility, &monofont_R_16);

    lv_style_set_pad_all(&style_btn, 0);
    lv_style_set_bg_color(&style_btn, lv_palette_darken(LV_PALETTE_INDIGO, 2));
    lv_style_set_text_color(&style_btn, lv_palette_main(LV_PALETTE_AMBER));

    // lv_style_set_pad_top(&style_btn,0);

    // /*Create a Tab view object*/
    // lv_obj_t *tabview_utility;

    tabview_utility = lv_tabview_create(parent, LV_DIR_TOP, 25);
    lv_obj_add_event_cb(lv_tabview_get_content(tabview_utility), scroll_begin_event, LV_EVENT_SCROLL_BEGIN, NULL);
    // lv_obj_set_style_bg_color(tabview_utility, lv_palette_darken(LV_PALETTE_DEEP_PURPLE, 3), 0);
    lv_obj_set_style_pad_all(tabview_utility, 0, LV_PART_MAIN);

    lv_obj_t *tab_btns = lv_tabview_get_tab_btns(tabview_utility);
    // lv_obj_set_style_bg_color(tab_btns, lv_palette_darken(LV_PALETTE_GREY, 3), 0);
    // lv_obj_set_style_text_color(tab_btns, lv_palette_lighten(LV_PALETTE_GREY, 5), 0);
    // lv_obj_set_style_border_side(tab_btns, LV_BORDER_SIDE_RIGHT, LV_PART_ITEMS | LV_STATE_CHECKED);

    /*Add 4 tabs (the tabs are page (lv_page) and can be scrolled*/
    lv_obj_t *tab1 = lv_tabview_add_tab(tabview_utility, "Memory");
    lv_obj_t *tab2 = lv_tabview_add_tab(tabview_utility, "F. Gen.");
    lv_obj_t *tab3 = lv_tabview_add_tab(tabview_utility, "Arbit");
    lv_obj_t *tab4 = lv_tabview_add_tab(tabview_utility, "Table");

    // lv_obj_set_style_bg_color(tab1, lv_palette_darken(LV_PALETTE_BLUE, 3), 0);
    // lv_obj_set_style_bg_opa(tab1, LV_OPA_COVER, 10);

    // lv_obj_set_style_bg_color(tab2, lv_palette_lighten(LV_PALETTE_AMBER, 3), 0);
    // lv_obj_set_style_bg_opa(tab2, LV_OPA_COVER, 0);

    // lv_obj_set_style_bg_color(tab3, lv_palette_lighten(LV_PALETTE_GREEN, 3), 0);
    // lv_obj_set_style_bg_opa(tab2, LV_OPA_COVER, 0);

    // lv_label_set_text(label, "0: 0.000V\t\t0.000A\n1: 0.100V\t\t0.010A\n2: 2.048V\t\t0.050A\n3: 3.000V 0.050A\n4: 4.096V 0.050A\n5: 5.000V 0.050A\n6: 0.000V 0.000A\n7: 0.100V 0.010A\n8: 20.480V 0.050A\n9: 32.000V 0.050A");

    // Utility page Tab 1 ****************************************************************************************************************************
    /*Add content to the tabs*/
    lv_obj_t *btn;
    lv_obj_t *label;
    lv_obj_t *labelV;
    lv_obj_t *labelI;
    int8_t yOff = 32, verOff = 125, yStart = -2, xStart = 38;

    MemArray mem = PowerSupply.LoadMemory("myDataKey");
    lv_obj_set_flex_flow(tab1, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_style_pad_column(tab1, 13, LV_PART_MAIN); // 10px horizontal gap
    lv_obj_set_style_pad_row(tab1, 9, LV_PART_MAIN);     // 10px vertical gap

    // lv_obj_set_flex_align(tab1, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_flex_align(tab1, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    int order[10] = {7, 8, 9, 4, 5, 6, 1, 2, 3, 0};

    for (int i = 0; i < 10; i++)
    {

        btn = lv_btn_create(tab1);
        lv_obj_set_style_pad_all(btn, 0, LV_PART_MAIN);
        lv_obj_set_size(btn, 89, 34);

        lv_obj_add_event_cb(btn, mem_btn_event_cb, LV_EVENT_SHORT_CLICKED, NULL);
        lv_obj_add_event_cb(btn, mem_btn_event_cb, LV_EVENT_LONG_PRESSED, NULL);

        lv_obj_add_flag(btn, LV_OBJ_FLAG_OVERFLOW_VISIBLE);
        lv_obj_add_style(btn, &style_btn, LV_STATE_DEFAULT);
        btn->user_data = (void *)(order[i]);

        labelV = lv_label_create(btn);
        lv_label_set_text_fmt(labelV, "%+08.4fV", scaleVoltage(mem.memory[(int)btn->user_data].voltage));
        lv_obj_align(labelV, LV_ALIGN_CENTER, 0, -6);
        lv_obj_add_flag(labelV, LV_OBJ_FLAG_OVERFLOW_VISIBLE);

        labelI = lv_label_create(labelV);
        lv_label_set_text_fmt(labelI, "%+08.4fA", scaleCurrent(mem.memory[(int)btn->user_data].current));
        lv_obj_align(labelI, LV_ALIGN_BOTTOM_MID, 0, 15);

        // lv_obj_set_parent(labelV, btn);

        label = lv_label_create(btn);
        lv_label_set_text_fmt(label, "%i", (int)btn->user_data);
        lv_obj_remove_style_all(label);
        lv_obj_add_style(label, &style_utility, LV_STATE_DEFAULT);
        lv_obj_set_style_text_font(label, &graph_R_8, 0);
        // lv_obj_set_pos(label, -8 + xStart + verOff * (i >= 5), i * yOff * (i < 5) + (i - 5) * yOff * (i >= 5) + yStart);
        // lv_obj_add_flag(label, LV_OBJ_FLAG_OVERFLOW_VISIBLE);
        lv_obj_align(label, LV_ALIGN_OUT_TOP_LEFT, -8, 2);
    }

    lv_obj_t *label_Vset;
    label_Vset = lv_label_create(tab1);
    lv_obj_set_style_text_font(label_Vset, &graph_R_16, 0);
    lv_label_set_text_fmt(label_Vset, "V-Set:%+08.4f", 32.7675);
    lv_obj_set_pos(label_Vset, 150, 160);
    lv_obj_add_flag(label_Vset, LV_OBJ_FLAG_OVERFLOW_VISIBLE);
    label_Vset->user_data = (void *)(13);

    lv_obj_t *label_Iset = lv_label_create(label_Vset);
    lv_obj_set_style_text_font(label_Iset, &graph_R_16, 0);
    lv_label_set_text_fmt(label_Iset, "I-Set:%+08.4f", +6.5535);
    lv_obj_align(label_Iset, LV_ALIGN_BOTTOM_LEFT, 0, 18);

    // Utility page Tab 2 ****************************************************************************************************************************

    lv_obj_set_style_pad_ver(tabview_utility, 0, LV_PART_ITEMS);
    lv_obj_set_style_pad_all(tab2, 0, LV_PART_MAIN);
    lv_obj_clear_flag(tab2, LV_OBJ_FLAG_SCROLLABLE);

    static lv_style_t style_func;
    lv_style_init(&style_func);

    Utility_objs.table_fun_gen_list = table_pro(tab2, &style_func, &lv_font_montserrat_12, LV_ALIGN_DEFAULT, 3, 3, 160, 130, 5, 5);

    for (int i = 0; i < numWaveforms; i++)
    {
        lv_table_set_cell_value_fmt(Utility_objs.table_fun_gen_list, i, 0, "%i", i + 1);
        lv_table_set_cell_value_fmt(Utility_objs.table_fun_gen_list, i, 1, "%s", waveforms[i].name);
    }
    // Set column widths
    int table_signals_width = 186;
    lv_table_set_col_width(Utility_objs.table_fun_gen_list, 0, 30);
    lv_table_set_col_width(Utility_objs.table_fun_gen_list, 1, table_signals_width - 50);
    // Add the event callbacks
    lv_obj_add_event_cb(Utility_objs.table_fun_gen_list, table_touch_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(Utility_objs.table_fun_gen_list, table_draw_cell_event_cb, LV_EVENT_DRAW_PART_BEGIN, NULL);

    /********************
     * Spinbox
     ********************/

    int verPad = 46;
    int verOffset = -58;
    int XOffset = -5;

    // createSpinbox(tab3, "#FFFFF7 Amplitude:#", -10000, 400000, 6, 2, LV_ALIGN_RIGHT_MID, XOffset, verOffset + verPad * 0);
    // createSpinbox(tab3, "#FFFFFF Frequency:#", 0000, 100000, 7, 2, LV_ALIGN_RIGHT_MID, XOffset, verOffset + verPad * 1);
    // createSpinbox(tab3, "#FFFFF7 Offset:#", -10000, 400000, 6, 2, LV_ALIGN_RIGHT_MID, XOffset, verOffset + verPad * 2);
    // createSpinbox(tab3, "#FFFFF7 Code2:#", -10000, 8388608, 7, 0, LV_ALIGN_RIGHT_MID, XOffset , verOffset + verPad * 3);

    Utility_objs.fun.Amplitude = spinbox_pro(tab2, "#FFFFF7 Amplitude:#", 0, 32750, 5, 2, LV_ALIGN_RIGHT_MID, XOffset, verOffset + verPad * 0, 98, 0, &graph_R_16);
    Utility_objs.fun.Frequency = spinbox_pro(tab2, "#FFFFF7 Frequency [Hz]:#", 0, 10000, 5, 2, LV_ALIGN_RIGHT_MID, XOffset, verOffset + verPad * 1, 98, 1, &graph_R_16);
    Utility_objs.fun.Offset = spinbox_pro(tab2, "#FFFFF7 Offset [v]:#", -32000, 32750, 5, 2, LV_ALIGN_RIGHT_MID, XOffset, verOffset + verPad * 2, 98, 2, &graph_R_16);
    Utility_objs.fun.Duty = spinbox_pro(tab2, "#FFFFF7 Duty [%]:#", 0, 10000, 5, 3, LV_ALIGN_RIGHT_MID, XOffset, verOffset + verPad * 3, 98, 3, &graph_R_16);

    PowerSupply.funGenMem = PowerSupply.LoadMemoryFgen("FunGen");

    set_spinbox_data_by_id(tab2, 0, PowerSupply.funGenMem.amplitude * 1000);
    set_spinbox_data_by_id(tab2, 1, PowerSupply.funGenMem.frequency * 1000);
    set_spinbox_data_by_id(tab2, 2, PowerSupply.funGenMem.offset * 1000);
    set_spinbox_data_by_id(tab2, 3, PowerSupply.funGenMem.dutyCycle * 10000);

    // PowerSupply.setupSwitch(tab2, 0, 0, 20, nullptr);

    btn_function_gen = lv_btn_create(tab2);

    lv_obj_align(btn_function_gen, LV_ALIGN_DEFAULT, 10, 140);
    lv_obj_add_flag(btn_function_gen, LV_OBJ_FLAG_CHECKABLE);

    lv_obj_set_style_radius(btn_function_gen, 2, LV_PART_MAIN);
    lv_obj_set_size(btn_function_gen, 70, 35);

    label = lv_label_create(btn_function_gen);
    lv_label_set_text(label, "OFF");
    lv_obj_center(label);

    lv_obj_add_event_cb(btn_function_gen, btn_function_gen_event_cb, LV_EVENT_SHORT_CLICKED, NULL);

    static lv_style_t style_btn_red, style_btn_blue, style_btn_gray;
    lv_style_init(&style_btn_red);
    lv_style_init(&style_btn_blue);
    lv_style_init(&style_btn_gray);

    lv_style_set_bg_color(&style_btn_gray, lv_palette_darken(LV_PALETTE_GREY, 0));
    lv_obj_add_style(btn_function_gen, &style_btn_gray, LV_PART_MAIN | LV_STATE_DISABLED);

    Utility_objs.switch_keys_scan = lv_switch_create(tab2);
    lv_obj_add_state(Utility_objs.switch_keys_scan, LV_STATE_CHECKED);
    lv_obj_align(Utility_objs.switch_keys_scan, LV_ALIGN_DEFAULT, 100, 140);
    lv_obj_set_size(Utility_objs.switch_keys_scan, 48, 22);

    Utility_objs.switch_keys_label = lv_label_create(tab2);
    lv_label_set_text(Utility_objs.switch_keys_label, "Keys scan");
    lv_obj_align_to(Utility_objs.switch_keys_label, Utility_objs.switch_keys_scan, LV_ALIGN_BOTTOM_MID, 0, 14);

    // Utility page Tab 3 ****************************************************************************************************************************

    lv_obj_clear_flag(lv_obj_get_parent(PowerSupply.page[3]), LV_OBJ_FLAG_SCROLLABLE);
    // Create the chart
    util_Arbit_chart = lv_chart_create(tab3);
    lv_obj_set_size(util_Arbit_chart, 280, 130);
    lv_obj_align(util_Arbit_chart, LV_ALIGN_DEFAULT, 10, -10);
    lv_obj_set_style_bg_color(util_Arbit_chart, lv_color_hex(0x000000), LV_PART_MAIN);

    // Set chart range
    lv_chart_set_range(util_Arbit_chart, LV_CHART_AXIS_PRIMARY_X, 0, CHART_POINTS);
    lv_chart_set_range(util_Arbit_chart, LV_CHART_AXIS_PRIMARY_Y, 0, 140);

    // Enable line chart mode
    lv_chart_set_type(util_Arbit_chart, LV_CHART_TYPE_LINE);
    lv_chart_set_point_count(util_Arbit_chart, CHART_POINTS); // 100 x-buckets

    // Create series
    util_Arbit_chart_series = lv_chart_add_series(util_Arbit_chart, lv_color_hex(0x00FFFF), LV_CHART_AXIS_PRIMARY_Y);

    // Fill with zero data initially
    for (int i = 0; i < CHART_POINTS; i++)
    {
        lv_chart_set_next_value(util_Arbit_chart, util_Arbit_chart_series, PowerSupply.funGenMem.arbitrary_points[i][0]);
        // lv_chart_get_y_array
        // lv_chart_set_next_value(util_Arbit_chart, util_Arbit_chart_series, 0);
    }

    // lv_chart_set_range(util_Arbit_chart, LV_CHART_AXIS_PRIMARY_Y, 0, 140);
    // Refresh to apply
    lv_obj_add_event_cb(util_Arbit_chart, draw_event_util_Arbit_chart_cb, LV_EVENT_DRAW_PART_BEGIN, NULL);

    /*Add ticks and label to every axis*/
    lv_chart_set_axis_tick(util_Arbit_chart, LV_CHART_AXIS_PRIMARY_X, 2, 1, 5, 1, true, 50);
    lv_chart_set_axis_tick(util_Arbit_chart, LV_CHART_AXIS_PRIMARY_Y, 0, 1, 3, 1, true, 50);

    lv_chart_refresh(util_Arbit_chart);

    // lv_obj_clear_flag(util_Arbit_chart, LV_OBJ_FLAG_SCROLLABLE);
    // lv_obj_clear_flag(util_Arbit_chart, LV_OBJ_FLAG_FLOATING);
    // lv_obj_clear_flag(lv_obj_get_parent(PowerSupply.page[3]), LV_OBJ_FLAG_SCROLLABLE);
    // lv_obj_clear_flag(lv_obj_get_parent(util_Arbit_chart), LV_OBJ_FLAG_SCROLLABLE);
    // lv_obj_add_flag(lv_obj_get_parent(util_Arbit_chart), LV_OBJ_FLAG_FLOATING);
    // lv_obj_clear_flag(lv_obj_get_parent(lv_obj_get_parent(util_Arbit_chart)), LV_OBJ_FLAG_SCROLLABLE);
    // lv_obj_clear_flag(lv_obj_get_parent(lv_obj_get_parent(lv_obj_get_parent(util_Arbit_chart))), LV_OBJ_FLAG_SCROLLABLE);
    // static const char *opts = "Linear\n"
    //                           "Smooth\n"
    //                           "Stair";

    // lv_obj_t *dd;
    // dd = lv_dropdown_create(tab3);
    // lv_dropdown_set_options_static(dd, opts);
    // lv_dropdown_set_symbol(dd, NULL);
    // lv_obj_align(dd, LV_ALIGN_BOTTOM_LEFT, -10, 0);
    // lv_obj_set_style_text_font(dd, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT); // Set smaller font
    // lv_obj_set_size(dd, 72, 25);                                                             // Reduce width
    // lv_obj_set_style_pad_all(dd, 4, LV_PART_MAIN);

    // Lambda function for dropdown event callback
    // auto dropdownEventCb = [](lv_event_t *e)
    // {
    //     lv_obj_t *dropdown = lv_event_get_target(e);
    //     char buf[64];
    //     lv_dropdown_get_selected_str(dropdown, buf, sizeof(buf));
    //     LV_LOG_USER("'%s' is selected", buf);

    //     dropdown_active = false;  // Reset flag when selection is made

    // };

    auto dropdownOpenCb = [](lv_event_t *e)
    {
        dropdown_active = true; // Set flag when dropdown is opened
    };

    // Lambda function to create and configure a dropdown
    auto createDropdown = [&](lv_obj_t *parent, lv_coord_t x, lv_coord_t y,
                              const char *options, const char *name)
    {
        lv_obj_t *dropdown = lv_dropdown_create(parent);
        lv_dropdown_set_options(dropdown, options);
        lv_dropdown_set_text(dropdown, name);
        lv_obj_align(dropdown, LV_ALIGN_BOTTOM_LEFT, x, y);

        // Remove symbol and apply transformation
        lv_dropdown_set_symbol(dropdown, NULL);
        lv_obj_set_style_transform_angle(dropdown, 1800, LV_PART_INDICATOR | LV_STATE_CHECKED);

        // Disable highlight for selected item
        lv_dropdown_set_selected_highlight(dropdown, false);

        // Attach event callback
        lv_obj_add_event_cb(dropdown, dropdownEventCb, LV_EVENT_VALUE_CHANGED, NULL);
        lv_obj_add_event_cb(dropdown, dropdownOpenCb, LV_EVENT_PRESSED, NULL);

        // Apply styles
        lv_obj_set_style_text_font(dropdown, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_all(dropdown, 4, LV_PART_MAIN);
        lv_obj_set_size(dropdown, 72, 25); // Reduce width
    };

    // Dropdown options
    const char *saveOptions =
        "Save to Bank 0\n"
        "Save to Bank 1\n"
        "Exit";

    const char *loadOptions =
        "Load to Bank 0\n"
        "Load to Bank 1\n"
        "Exit";

    // Create save and load dropdowns
    createDropdown(tab3, -10, 0, saveOptions, "Save");
    createDropdown(tab3, 65, 0, loadOptions, "Load");

    // Buffer to hold waveform dropdown options
    constexpr size_t numWaveforms = 17; // Ensure it matches actual count
    static char waveformOptions[512];   // Ensure enough space

    // Lambda function to generate waveform options
    auto generateWaveformOptions = []()
    {
        waveformOptions[0] = '\0'; // Clear buffer

        strcat(waveformOptions, "Clear\n"); // Default option
        for (size_t i = 0; i < numWaveforms; i++)
        {
            strcat(waveformOptions, waveforms[i].name);
            strcat(waveformOptions, "\n"); // Separate entries
        }
        strcat(waveformOptions, "Exit"); // Final option
    };

    // Generate waveform options and create the dropdown
    generateWaveformOptions();
    createDropdown(tab3, 140, 0, waveformOptions, "Fun");

    // Saving  ************************************************************
    // lv_obj_t *saveButton = lv_btn_create(tab4);
    // label = lv_label_create(saveButton);
    // lv_label_set_text(label, "Save");
    // lv_obj_align(saveButton, LV_ALIGN_CENTER, 0, 0);
    // lv_obj_align_to(saveButton, Utility_objs.table_spinbox_value, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

    // lv_obj_add_event_cb(saveButton, save_table_data_cb, LV_EVENT_CLICKED, NULL);

    // Utility page Tab 4 ****************************************************************************************************************************
    lv_obj_clear_flag(lv_tabview_get_content(tabview_utility), LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(tab4, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_ver(tab4, 0, LV_PART_ITEMS);
    lv_obj_set_style_pad_all(tab4, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(tab4, 0, LV_PART_ITEMS);

    static lv_style_t style_stats;
    lv_style_init(&style_stats);

    Utility_objs.table_point_list = table_pro(tab4, &style_stats, &graph_R_16, LV_ALIGN_DEFAULT, 3, 3, 160, 180, 0, 5);

    for (int i = 0; i < 100; i++)
    {
        // Utility_objs.table_points[i] = PowerSupply.funGenMem.table_points[i];
        lv_table_set_cell_value_fmt(Utility_objs.table_point_list, i, 0, "%0i", i);
        lv_table_set_cell_value_fmt(Utility_objs.table_point_list, i, 1, "%1.4f", PowerSupply.funGenMem.table_points[i][0]);
    }

    lv_table_set_col_width(Utility_objs.table_point_list, 0, 40);
    lv_table_set_col_width(Utility_objs.table_point_list, 1, 100);

    lv_obj_add_event_cb(Utility_objs.table_point_list, table_touch_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(Utility_objs.table_point_list, table_draw_cell_event_cb, LV_EVENT_DRAW_PART_BEGIN, NULL);

    // lv_obj_add_event_cb(Utility_objs.table_point_list, table_get_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    Utility_objs.table_spinbox_value = spinbox_pro(tab4, "#FFFFF7 Value:#", 0, 10000, 5, 1, LV_ALIGN_RIGHT_MID, -35, -50, 98, 4, &graph_R_16);
    lv_obj_add_event_cb(Utility_objs.table_spinbox_value, spinbox_change_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    // // setup (same as before)
    // SpinboxPro::Config cfg;
    // cfg.spinboxFont = &graph_R_16;
    // cfg.labelFont   = &lv_font_montserrat_12;
    // cfg.clickBeep   = true;

    // SpinboxPro::BeepFn clickBeep = [](){ myTone(NOTE_A4, 5); };
    // SpinboxPro spx(cfg, clickBeep);
    // // one‑liner builder
    // auto spinOskol = SpinboxPro::Builder(spx)
    //     .parent(tab4)
    //     .label("#FFFFF7 Zero Voltage:#")
    //     .range(0, 65535)
    //     .digits(5, 0)
    //     .align(LV_ALIGN_DEFAULT, 165, 160 )
    //     .width(86)
    //     .id(15)
    //     .create();

    // Saving  ************************************************************
    lv_obj_t *saveButton = lv_btn_create(tab4);
    label = lv_label_create(saveButton);
    lv_label_set_text(label, "Save");
    lv_obj_align(saveButton, LV_ALIGN_CENTER, 0, 0);
    lv_obj_align_to(saveButton, Utility_objs.table_spinbox_value, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

    auto save_table_data_cb = [](lv_event_t *e)
    {
        lv_event_code_t code = lv_event_get_code(e);
        if (code == LV_EVENT_CLICKED)
            PowerSupply.SaveMemoryFgen("FunGen", PowerSupply.funGenMem);
    };
    lv_obj_add_event_cb(saveButton, save_table_data_cb, LV_EVENT_CLICKED, NULL);

    // Loading  ************************************************************
    lv_obj_t *loadButton = lv_btn_create(tab4);
    label = lv_label_create(loadButton);
    lv_label_set_text(label, "Load");
    lv_obj_align(loadButton, LV_ALIGN_CENTER, 0, 0);
    lv_obj_align_to(loadButton, saveButton, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

    auto load_table_data_cb = [](lv_event_t *e)
    {
        lv_event_code_t code = lv_event_get_code(e);
        if (code == LV_EVENT_CLICKED)
        {
            PowerSupply.funGenMem = PowerSupply.LoadMemoryFgen("FunGen");
            for (int i = 0; i < 100; i++)
            {
                lv_table_set_cell_value_fmt(Utility_objs.table_point_list, i, 1, "%06.4f", PowerSupply.funGenMem.table_points[i][0]);
            }
            lv_obj_invalidate(Utility_objs.table_point_list);
        }
    };

    lv_obj_add_event_cb(loadButton, load_table_data_cb, LV_EVENT_CLICKED, NULL);
}

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

lv_obj_t *find_btn_by_tag(lv_obj_t *parent, uint32_t tag)
{
    uint32_t child_cnt = lv_obj_get_child_cnt(parent);
    for (uint32_t i = 0; i < child_cnt; i++)
    {
        lv_obj_t *child = lv_obj_get_child(parent, i);
        if (child && (int)child->user_data == tag)
        {
            return child; // Found the child with the matchin  g tag
        }
    }
    return NULL; // No child with the specified tag found
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
    //   Serial.printf("\nkeyCheckLoop() run on core: #%i \n\n", xPortGetCoreID());
    keyMenus('O', " RELEASED.", []
             { 
                blockAll = true;
                //  vTaskDelay(10);
                 PowerSupply.toggle();
                 //  lv_event_send(PowerSupply.powerSwitch.btn,LV_EVENT_LONG_PRESSED, NULL);
                  blockAll =false; });

    keyMenus('O', " HOLD.", [] // Output button
             {
                 //  myTone(NOTE_A5, 200, true);
                 //  myTone(NOTE_A3, 200, true);
                 //  ESP.restart();
             });

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
                 if (!lv_obj_has_flag(PowerSupply.gui.textarea_set_value, LV_OBJ_FLAG_HIDDEN))
                     return;
                 Tabs::previousPage();
                 //  UpdateTabs();
                 //  updateObjectParrents();
             });

    // next page
    keyMenus('k', " RELEASED.",
             [&]
             {
                 if (!lv_obj_has_flag(PowerSupply.gui.textarea_set_value, LV_OBJ_FLAG_HIDDEN))
                     return;
                 Tabs::nextPage();
                 //  UpdateTabs();
                 //  updateObjectParrents();
             });

    keyMenus('k', " RELEASED.", []
             {
                 //  if (lv_obj_is_visible(voltageCurrentCalibration))
                 //  {
                 //      lv_obj_add_flag(voltageCurrentCalibration, LV_OBJ_FLAG_HIDDEN);
                 //  }
             }

    );

    keyMenus('H', " RELEASED.", []
             {
                 Tabs::goToHomeTab();
                //  lv_obj_add_flag(voltageCurrentCalibration,LV_OBJ_FLAG_HIDDEN);
                 hide( PowerSupply.gui.calibration.win_ADC_voltage_calibration);
                 hide( PowerSupply.gui.calibration.win_ADC_current_calibration);
                 hide( PowerSupply.gui.calibration.win_int_current_calibration);
                 hide( PowerSupply.gui.calibration.win_ADC_INL_Voltage_calibration);
                 hide( PowerSupply.gui.calibration.win_DAC_calibration); }

    );

    keyMenus('H', " HOLD.", [] // Home button
             {
                 myTone(NOTE_A3, 200, true);
                 myTone(NOTE_A5, 200, true);
                 ESP.restart(); });

    keyMenus('M', " RELEASED.", []
             { Tabs::setCurrentPage(4); });

    keyMenus('m', " RELEASED.", []
             {
                 if (!lv_obj_has_flag(PowerSupply.gui.textarea_set_value, LV_OBJ_FLAG_HIDDEN))
                     return;
                 Tabs::setCurrentPage(3); }

    );

    keyMenusPage('T', " RELEASED.", 2, [&]
                 {
                     myTone(NOTE_A4, 10);
                     PowerSupply.toggle_measure_unit();

                     blockAll = true;
                     btn_calibration_ADC_current_event_cb(nullptr); // Refresh the calibration window if open
                     lv_obj_add_flag(PowerSupply.gui.calibration.win_ADC_current_calibration, LV_OBJ_FLAG_HIDDEN);
                     PowerSupply.calibrationUpdate();
                     PowerSupply.Current.displayUpdate(true);
                     blockAll = false; });

    keyMenusPage('T', " RELEASED.", 4, [&]
                 {
                     myTone(NOTE_A4, 10, false);
                     PowerSupply.toggle_measure_unit();
                     
                     blockAll = true;
                     btn_calibration_ADC_current_event_cb(nullptr); // Refresh the calibration window if open
                     lv_obj_invalidate(PowerSupply.gui.calibration.win_ADC_current_calibration);
                     blockAll = false; });

    keyMenusPage('-', " RELEASED.", 0, [&]
                 {
                     int32_t newEncoder1Value = encoder1_value +1;
                     int32_t newEncoder2Value = encoder2_value;
                     handleHistogramPage(newEncoder1Value, newEncoder2Value);

                     PowerSupply.Current.hist.Reset();
                     PowerSupply.Voltage.hist.Reset(); });

    keyMenusPage('+', " RELEASED.", 0, [&]
                 {
                     int32_t newEncoder1Value = encoder1_value - 1;
                     int32_t newEncoder2Value = encoder2_value;
                     handleHistogramPage(newEncoder1Value, newEncoder2Value);

                     PowerSupply.Current.hist.Reset();
                     PowerSupply.Voltage.hist.Reset(); });

    keyMenusPage('+', " RELEASED.", 1, [&]

                 {

                 });

    keyMenusPage('V', " RELEASED.", 0, []
                 {
                     static bool hide = false;
                     hide = !hide;
                     lv_chart_hide_series(PowerSupply.stats.chart, PowerSupply.stats.serV, hide); 
                     lv_chart_hide_series(PowerSupply.graph.chart, PowerSupply.graph.serV, hide);
                     
                     if (!hide)
                         lv_obj_clear_flag(label_legend1, LV_OBJ_FLAG_HIDDEN);

                     else
                         lv_obj_add_flag(label_legend1, LV_OBJ_FLAG_HIDDEN); });

    keyMenusPage('A', " RELEASED.", 0, []
                 {
                     static bool hide = false;
                     hide = !hide;
                     lv_chart_hide_series( PowerSupply.stats.chart,  PowerSupply.stats.serI,hide); 
                     lv_chart_hide_series(PowerSupply.graph.chart, PowerSupply.graph.serI, hide);
                     
                     if (!hide)
                         lv_obj_clear_flag(label_legend2, LV_OBJ_FLAG_HIDDEN);

                     else
                         lv_obj_add_flag(label_legend2, LV_OBJ_FLAG_HIDDEN); });

    keyMenusPage('V', " RELEASED.", 1, []
                 {
                     static bool hide = false;
                     hide = !hide;
                     lv_chart_hide_series(PowerSupply.stats.chart, PowerSupply.stats.serV, hide); 
                     lv_chart_hide_series(PowerSupply.graph.chart, PowerSupply.graph.serV, hide);

                     if (!hide)
                         lv_obj_clear_flag(label_legend1, LV_OBJ_FLAG_HIDDEN);

                     else
                         lv_obj_add_flag(label_legend1, LV_OBJ_FLAG_HIDDEN); });

    keyMenusPage('A', " RELEASED.", 1, []
                 {
                     static bool hide = false;
                     hide = !hide;
                     lv_chart_hide_series(PowerSupply.graph.chart, PowerSupply.graph.serI, hide);
                     lv_chart_hide_series( PowerSupply.stats.chart,  PowerSupply.stats.serI,hide); 
                     if (!hide)
                         lv_obj_clear_flag(label_legend2, LV_OBJ_FLAG_HIDDEN);

                     else
                         lv_obj_add_flag(label_legend2, LV_OBJ_FLAG_HIDDEN); });

    // Statistics Reset and auto ajdust histogram window
    keyMenusPage('j', " RELEASED.", 0, []
                 {
                     if (PowerSupply.stats.serV->hidden == 0)
                     {
                         PowerSupply.Voltage.hist.histWinMin = PowerSupply.Voltage.measured.Mean() -
                                                               PowerSupply.Voltage.Statistics.StandardDeviation() * 6;

                         PowerSupply.Voltage.hist.histWinMax = PowerSupply.Voltage.measured.Mean() +
                                                               PowerSupply.Voltage.Statistics.StandardDeviation() * 6;
                     }
                     if (PowerSupply.stats.serI->hidden == 0)
                     {
                         PowerSupply.Current.hist.histWinMin = PowerSupply.Current.measured.Mean() -
                                                               PowerSupply.Current.Statistics.StandardDeviation() * 6;

                         PowerSupply.Current.hist.histWinMax = PowerSupply.Current.measured.Mean() +
                                                               PowerSupply.Current.Statistics.StandardDeviation() * 6;
                     }
                     //  PowerSupply.stats.serI->y_points = PowerSupply.Current.hist.data;
                     //  PowerSupply.stats.serV->y_points = PowerSupply.Voltage.hist.data;

                     PowerSupply.ResetStats(); });

    keyMenusPage('j', " RELEASED.", 1, []
                 {
                     //  Serial.printf("\nget_scroll_top:%i ", lv_obj_get_scroll_top(PowerSupply.graph.chart));
                     // Reset statistics
                     //  PowerSupply.ResetStats();

                     // Set the desired zoom levels
                     //  uint16_t desired_zoom_x = 512;  // Adjust as needed (default is 256)
                     //  uint16_t desired_zoom_y = 5120; // Adjust as needed

                     // Apply zoom to the chart
                     //  lv_chart_set_zoom_x(PowerSupply.graph.chart, desired_zoom_x);
                     //  lv_chart_set_zoom_y(PowerSupply.graph.chart, desired_zoom_y);

                     // Refresh the chart to apply zoom changes
                     //  lv_chart_refresh(PowerSupply.graph.chart);
                     //  Serial.printf("\nget_scroll_top:%i ", lv_obj_get_scroll_top(PowerSupply.graph.chart));

                     if (!lvglChartIsBusy)
                     {
                         lvglChartIsBusy = true;

                         autoScrollY();

                         //  lv_chart_refresh(PowerSupply.graph.chart);
                         lvglChartIsBusy = false;
                     }

                     //   Serial.printf("\nget_scroll_top:%i " ,  lv_obj_get_scroll_top(PowerSupply.graph.chart));
                 });

    keyMenusPage('j', " RELEASED.", 2, []
                 { PowerSupply.ResetStats(); });

    keyMenusPage('j', " RELEASED.", 4, []
                 { PowerSupply.ResetStats(); });

    keyMenus('Z', " RELEASED.", []
             {
                uint16_t w;
                w = PowerSupply.Voltage.measured.NofAvgs;
                w = (w == MAX_NO_OF_AVG) ? 1 : w * 2;
                // PowerSupply.Voltage.effectiveResolution(64);
                PowerSupply.ResetStats();

                lv_slider_set_value(lv_obj_get_child(PowerSupply.gui.slider_Avgs, -1), log2(w), LV_ANIM_OFF);
                lv_event_send(lv_obj_get_child(PowerSupply.gui.slider_Avgs, -1), LV_EVENT_VALUE_CHANGED, NULL); });

    keyMenusPage('+', " RELEASED.", 4, []
                 {
                     //  if (lv_obj_is_visible(voltageCurrentCalibration)) {
                     //  lv_event_send(spinboxes.btn_plus[spinboxes.id_index], LV_EVENT_SHORT_CLICKED, NULL);}
                 });

    keyMenusPage('-', " RELEASED.", 4, []
                 {
                     //  if (lv_obj_is_visible(voltageCurrentCalibration))
                     //  {
                     //      lv_event_send(spinboxes.btn_minus[spinboxes.id_index], LV_EVENT_SHORT_CLICKED, NULL);
                     //  }
                 });
    if ((Tabs::getCurrentPage() == 3))
    {
        int a = -1;

        keyMenus('9', " RELEASED.", [&]
                 { a = 9; });
        keyMenus('8', " RELEASED.", [&]
                 { a = 8; });
        keyMenus('7', " RELEASED.", [&]
                 { a = 7; });
        keyMenus('6', " RELEASED.", [&]
                 { a = 6; });
        keyMenus('5', " RELEASED.", [&]
                 { a = 5; });
        keyMenus('4', " RELEASED.", [&]
                 { a = 4; });
        keyMenus('3', " RELEASED.", [&]
                 { a = 3; });
        keyMenus('2', " RELEASED.", [&]
                 { a = 2; });
        keyMenus('1', " RELEASED.", [&]
                 { a = 1; });
        keyMenus('0', " RELEASED.", [&]
                 { a = 0; });

        if (a >= 0) //-> released
        {
            lv_obj_t *tab = lv_obj_get_child(lv_obj_get_child(lv_obj_get_child(PowerSupply.page[3], 0), 1), 0);
            lv_obj_t *btn = find_btn_by_tag(tab, a);
            loadMemory(btn);
            return;
        }

        keyMenus('0', " HOLD.", [&]
                 { a = 0; });
        keyMenus('1', " HOLD.", [&]
                 { a = 1; });
        keyMenus('2', " HOLD.", [&]
                 { a = 2; });
        keyMenus('3', " HOLD.", [&]
                 { a = 3; });
        keyMenus('4', " HOLD.", [&]
                 { a = 4; });
        keyMenus('5', " HOLD.", [&]
                 { a = 5; });
        keyMenus('6', " HOLD.", [&]
                 { a = 6; });
        keyMenus('7', " HOLD.", [&]
                 { a = 7; });
        keyMenus('8', " HOLD.", [&]
                 { a = 8; });
        keyMenus('9', " HOLD.", [&]
                 { a = 9; });

        if (a >= 0)
        {
            myTone(NOTE_A4, 150);

            lv_obj_t *tab = lv_obj_get_child(lv_obj_get_child(lv_obj_get_child(PowerSupply.page[3], 0), 1), 0);
            lv_obj_t *btn = find_btn_by_tag(tab, a);
            saveMemory(btn);
        }
    }

    if ((Tabs::getCurrentPage() == 2) || (Tabs::getCurrentPage() == 4 /*&& lv_obj_is_visible(voltageCurrentCalibration)*/))
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
                     {  if (!lv_obj_is_visible(PowerSupply.gui.textarea_set_value))
                        lv_obj_clear_flag(PowerSupply.gui.textarea_set_value, LV_OBJ_FLAG_HIDDEN);
                    else
                        lv_obj_add_flag(PowerSupply.gui.textarea_set_value, LV_OBJ_FLAG_HIDDEN);
                        ismyTextHiddenChange = true; });

        keyMenusPage('E', " RELEASED.", 4, []
                     {
                         // if (!lv_obj_is_visible(PowerSupply.gui.textarea_set_value))
                         // {
                         //     // lv_obj_clear_flag(PowerSupply.gui.textarea_set_value, LV_OBJ_FLAG_HIDDEN);

                         //     // lv_obj_t *spinBox = lv_obj_get_child(voltageCurrentCalibration, spinboxes.current_index);
                         //     // int32_t v = lv_spinbox_get_value(spinBox);

                         //     // char str[12];

                         //     // if (spinboxes.current_index % 2 != 0)
                         //     // {
                         //     //     sprintf(str, "%+08.0f",double( v ));
                         //     //     key_event_handler_readBack_clb(str);
                         //     // }

                         //     // else
                         //     // {
                         //     //     sprintf(str, "%+08.4f", double (v/10000.0));
                         //     //     key_event_handler_readBack_clb(str);
                         //     //     // key_event_handler_readBack_clb(std::to_string(v).c_str());
                         //     // }
                         //     // ismyTextHiddenChange = true;
                         // }
                         // else
                         // {
                         //     // lv_obj_add_flag(PowerSupply.gui.textarea_set_value, LV_OBJ_FLAG_HIDDEN);
                         //     // myTone(NOTE_A5, 100);
                         //     // const char *txt = lv_textarea_get_text(ta);
                         //     // lv_obj_t *spinBox = lv_obj_get_child(voltageCurrentCalibration, spinboxes.current_index);
                         //     // if (spinboxes.current_index % 2 != 0)
                         //     //     lv_spinbox_set_value(spinBox, strtod(txt, NULL));
                         //     // else
                         //     //     lv_spinbox_set_value(spinBox, 10000.0 * strtod(txt, NULL));
                         //     // lv_textarea_set_text(ta,"");
                         //     // ismyTextHiddenChange = true;

                         // }
                     });
    }

    keyMenusPage('^', " RELEASED.", 2, []
                 { PowerSupply.Voltage.setLock(!PowerSupply.Voltage.getLock()); });
    keyMenusPage('>', " RELEASED.", 2, []
                 { PowerSupply.Current.setLock(!PowerSupply.Current.getLock()); });
    keyMenusPage('M', " RELEASED.", 4, []
                 {
                     // lv_obj_add_flag(voltageCurrentCalibration, LV_OBJ_FLAG_HIDDEN);
                 });

    keyMenusPage('V', " RELEASED.", 2, []
                 {
                     if (!lv_obj_is_visible(PowerSupply.gui.textarea_set_value))
                     {
                         lv_obj_clear_flag(PowerSupply.gui.textarea_set_value, LV_OBJ_FLAG_HIDDEN);
                         key_event_handler_readBack(PowerSupply.Voltage);
                         ismyTextHiddenChange = true;
                         delay(100);
                     }
                     else if (strcmp(lv_label_get_text(unit_label), "V") == 0 || strcmp(lv_label_get_text(unit_label), "mV/V/mA/A") == 0)

                         key_event_handler(8);
                        lv_obj_invalidate(lv_scr_act()); });

    keyMenusPage('v', " RELEASED.", 2, []
                 {
                     if (!lv_obj_is_visible(PowerSupply.gui.textarea_set_value))
                     {
                         lv_obj_clear_flag(PowerSupply.gui.textarea_set_value, LV_OBJ_FLAG_HIDDEN);
                         key_event_handler_readBack_k(PowerSupply.Voltage);
                         ismyTextHiddenChange = true;
                         delay(100);
                     }
                     else if (strcmp(lv_label_get_text(unit_label), "mV") == 0 || strcmp(lv_label_get_text(unit_label), "mV/V/mA/A") == 0)
                         key_event_handler(9);lv_obj_invalidate(lv_scr_act());  });

    keyMenusPage('A', " RELEASED.", 2, []
                 {
                     if (!lv_obj_is_visible(PowerSupply.gui.textarea_set_value))
                     {
                         lv_obj_clear_flag(PowerSupply.gui.textarea_set_value, LV_OBJ_FLAG_HIDDEN);
                         key_event_handler_readBack(PowerSupply.Current);
                         ismyTextHiddenChange = true;
                           delay(100);
                     }
                     else if (strcmp(lv_label_get_text(unit_label), "A") == 0 || strcmp(lv_label_get_text(unit_label), "mV/V/mA/A") == 0)
                         key_event_handler(13); lv_obj_invalidate(lv_scr_act()); });

    keyMenusPage('a', " RELEASED.", 2, []
                 {
            if (!lv_obj_is_visible(PowerSupply.gui.textarea_set_value))
            {
                lv_obj_clear_flag(PowerSupply.gui.textarea_set_value, LV_OBJ_FLAG_HIDDEN);
                key_event_handler_readBack_k(PowerSupply.Current);
                ismyTextHiddenChange = true;
                  delay(100);
            }
            else if (strcmp(lv_label_get_text(unit_label), "mA") == 0 || strcmp(lv_label_get_text(unit_label), "mV/V/mA/A") == 0)
                key_event_handler(14);lv_obj_invalidate(lv_scr_act());  });

    keyMenusPage('W', " RELEASED.", 2, []
                 {
                     PowerSupply.Voltage.SetRotaryStep(1); //0.0005
                     PowerSupply.Current.SetRotaryStep(1);
                     lv_obj_align(PowerSupply.Voltage.highlight_adjValue, 0, 10 * 12, -1000);
                     lv_obj_align(PowerSupply.Current.highlight_adjValue, 0, 10 * 12, -1000); });

    keyMenusPage('W', " RELEASED.", 1, []
                 {
                     static bool chart_mode = false;
                     if (chart_mode)
                         lv_chart_set_update_mode(PowerSupply.graph.chart, LV_CHART_UPDATE_MODE_SHIFT);
                     else
                         lv_chart_set_update_mode(PowerSupply.graph.chart, LV_CHART_UPDATE_MODE_CIRCULAR);

                     chart_mode = !chart_mode; });

    keyMenusPage('X', " RELEASED.", 2, []
                 {
                     PowerSupply.Voltage.SetRotaryStep(1); //0.0005
                     PowerSupply.Current.SetRotaryStep(1);
                     lv_obj_align(PowerSupply.Voltage.highlight_adjValue, 0, 10 * 12, -1000);
                     lv_obj_align(PowerSupply.Current.highlight_adjValue, 0, 10 * 12, -1000); });

    keyMenusPage('Y', " RELEASED.", 2, []
                 {
                     PowerSupply.Voltage.SetRotaryStep(1); //0.0005
                     PowerSupply.Current.SetRotaryStep(1);
                     lv_obj_align(PowerSupply.Voltage.highlight_adjValue, 0, 10 * 12, -1000);
                     lv_obj_align(PowerSupply.Current.highlight_adjValue, 0, 10 * 12, -1000); });

    keyMenusPage('W', " HOLD.", 2, []
                 {
            PowerSupply.Voltage.SetRotaryStep(2000.0000);
            PowerSupply.Current.SetRotaryStep(10000.000);
            lv_obj_align(PowerSupply.Voltage.highlight_adjValue, 0, 7 * 12, -10);
            lv_obj_align(PowerSupply.Current.highlight_adjValue, 0, 7 * 12, 72);
            lv_obj_clear_flag(PowerSupply.Voltage.highlight_adjValue, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(PowerSupply.Current.highlight_adjValue, LV_OBJ_FLAG_HIDDEN); });

    keyMenusPage('X', " HOLD.", 2, []
                 {
            PowerSupply.Voltage.SetRotaryStep(200);
            PowerSupply.Current.SetRotaryStep(1000.000);
            lv_obj_align(PowerSupply.Voltage.highlight_adjValue, 0, 9 * 12, -10);
            lv_obj_align(PowerSupply.Current.highlight_adjValue, 0, 9 * 12, 72);
            lv_obj_clear_flag(PowerSupply.Voltage.highlight_adjValue, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(PowerSupply.Current.highlight_adjValue, LV_OBJ_FLAG_HIDDEN); });

    keyMenusPage('Y', " HOLD.", 2, []
                 {
            PowerSupply.Voltage.SetRotaryStep(20);
            PowerSupply.Current.SetRotaryStep(100.000);
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
    if (Tabs::getCurrentPage() < 2)
    {
        StatsPositions(PowerSupply.page[Tabs::getCurrentPage()], PowerSupply.Voltage, &PowerSupply.stats.style_statsVolt, 0, 177);
        StatsPositions(PowerSupply.page[Tabs::getCurrentPage()], PowerSupply.Current, &PowerSupply.stats.style_statsCurrent, 0, 187);
        lv_obj_set_parent(label_legend1, PowerSupply.page[Tabs::getCurrentPage()]);
        lv_obj_set_parent(label_legend2, PowerSupply.page[Tabs::getCurrentPage()]);
    }
    else if (Tabs::getCurrentPage() == 3)
    {
        lv_obj_t *tab = lv_obj_get_child(lv_obj_get_child(lv_obj_get_child(PowerSupply.page[3], 0), 1), 0);

        lv_obj_t *labelVset = find_btn_by_tag(tab, 13);
        lv_obj_t *labelIset = lv_obj_get_child(labelVset, 0);

        lv_label_set_text_fmt(labelVset, "V-Set%+08.4fV", scaleVoltage(PowerSupply.Voltage.adjValue));
        lv_label_set_text_fmt(labelIset, "I-Set%+08.4fA", scaleCurrent(PowerSupply.Current.adjValue));
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

    // if (win_ADC_voltage_calibration != nullptr)
    //     Serial.printf("\nwin_ADC_voltage_calibration:%i is vis:%i", win_ADC_voltage_calibration, lv_obj_is_visible(win_ADC_voltage_calibration));
    // if (win_ADC_current_calibration != nullptr)
    //     Serial.printf("\nwin_ADC_current_calibration:%i is vis:%i", win_ADC_current_calibration, lv_obj_is_visible(win_ADC_current_calibration));

    if (PowerSupply.gui.calibration.win_ADC_voltage_calibration != nullptr && lv_obj_is_visible(PowerSupply.gui.calibration.win_ADC_voltage_calibration))
    {

        int code1 = lv_spinbox_get_value(Calib_GUI.Voltage.code_1);
        int code2 = lv_spinbox_get_value(Calib_GUI.Voltage.code_2);
        double vin1 = double(lv_spinbox_get_value(Calib_GUI.Voltage.vin_1)) / 10000.0;
        double vin2 = double(lv_spinbox_get_value(Calib_GUI.Voltage.vin_2)) / 10000.0;

        double m = get_m(code1, code2, vin1, vin2);

        lv_label_set_text_fmt(Calib_GUI.Voltage.lbl_calib_m, "%f", m);
        lv_label_set_text_fmt(Calib_GUI.Voltage.lbl_calib_b, "%f", get_b(code1, m, vin1));

        lv_label_set_text_fmt(Calib_GUI.Voltage.lbl_rawCode, "%+08i", PowerSupply.Voltage.rawValue);
        lv_label_set_text_fmt(Calib_GUI.Voltage.lbl_rawAVG_, "%+08.0f", PowerSupply.Voltage.measured.Mean() * m + get_b(code1, m, vin1));
        lv_label_set_text_fmt(Calib_GUI.Voltage.lbl_calibratedValue, "%+09.4f", PowerSupply.Voltage.measured.value);
        lv_label_set_text_fmt(Calib_GUI.Voltage.lbl_calibValueAVG_, "%+09.4f", PowerSupply.Voltage.measured.Mean());
        lv_label_set_text_fmt(Calib_GUI.Voltage.lbl_ER, "%+02.2f", PowerSupply.Voltage.effectiveResolution.Mean());

        PowerSupply.CalBank[PowerSupply.bankCalibId].vCal.code_1 = code1;
        PowerSupply.CalBank[PowerSupply.bankCalibId].vCal.code_2 = code2;
        PowerSupply.CalBank[PowerSupply.bankCalibId].vCal.value_1 = vin1;
        PowerSupply.CalBank[PowerSupply.bankCalibId].vCal.value_2 = vin2;

        PowerSupply.calibrationUpdate();

        PowerSupply.Power.measured.ResetStats();
        PowerSupply.Voltage.measured.ResetStats();

        PowerSupply.Voltage.hist.Reset();
        // SaveCalibrationData();
        // for (int i = 0; i < 35; i++)
        // Serial.printf("\nMeasuere:%+09.5f Ideal:%+07.3f", CalBank[bankCalibId].adc_inl_measure[i], CalBank[bankCalibId].adc_inl_ideal[i]); // g_voltINL.printKnotTable();

        // std::vector<double> X(PowerSupply.CalBank[PowerSupply.bankCalibId].adc_inl_measure, PowerSupply.CalBank[PowerSupply.bankCalibId].adc_inl_measure + 35); // ideal volts (Mean)
        // std::vector<double> Y(PowerSupply.CalBank[PowerSupply.bankCalibId].adc_inl_ideal, PowerSupply.CalBank[PowerSupply.bankCalibId].adc_inl_ideal + 35);     // true volts (set)

        // g_voltINL.setPoints(X, Y);

        // g_voltINL.build();
        // g_voltINL_ready = true;
    }
    if (PowerSupply.gui.calibration.win_ADC_current_calibration != nullptr && lv_obj_is_visible(PowerSupply.gui.calibration.win_ADC_current_calibration))
    {

        int code1 = lv_spinbox_get_value(Calib_GUI.Current.code_1);
        int code2 = lv_spinbox_get_value(Calib_GUI.Current.code_2);
        double vin1 = double(lv_spinbox_get_value(Calib_GUI.Current.vin_1)) / 10000.0;
        double vin2 = double(lv_spinbox_get_value(Calib_GUI.Current.vin_2)) / 10000.0;

        double m = get_m(code1, code2, vin1, vin2);

        lv_label_set_text_fmt(Calib_GUI.Current.lbl_calib_m, "%f", m);
        lv_label_set_text_fmt(Calib_GUI.Current.lbl_calib_b, "%f", get_b(code1, m, vin1));

        lv_label_set_text_fmt(Calib_GUI.Current.lbl_rawCode, "%+08i", PowerSupply.Current.rawValue);
        lv_label_set_text_fmt(Calib_GUI.Current.lbl_rawAVG_, "%+08.0f", PowerSupply.Current.measured.Mean() * m + get_b(code1, m, vin1));
        lv_label_set_text_fmt(Calib_GUI.Current.lbl_calibratedValue, "%+09.4f", PowerSupply.Current.measured.value);
        lv_label_set_text_fmt(Calib_GUI.Current.lbl_calibValueAVG_, "%+09.4f", PowerSupply.Current.measured.Mean());
        lv_label_set_text_fmt(Calib_GUI.Current.lbl_ER, "%+02.2f", PowerSupply.Current.effectiveResolution.Mean());

        PowerSupply.CalBank[PowerSupply.bankCalibId].iCal[PowerSupply.mA_Active].code_1 = code1;
        PowerSupply.CalBank[PowerSupply.bankCalibId].iCal[PowerSupply.mA_Active].code_2 = code2;
        PowerSupply.CalBank[PowerSupply.bankCalibId].iCal[PowerSupply.mA_Active].value_1 = vin1;
        PowerSupply.CalBank[PowerSupply.bankCalibId].iCal[PowerSupply.mA_Active].value_2 = vin2;

        PowerSupply.calibrationUpdate();

        // PowerSupply.Current.calib_m = (PowerSupply.CalBank[PowerSupply.bankCalibId].iCal[PowerSupply.mA_Active].code_2 - PowerSupply.CalBank[PowerSupply.bankCalibId].iCal[PowerSupply.mA_Active].code_1) /
        //                               (PowerSupply.CalBank[PowerSupply.bankCalibId].iCal[PowerSupply.mA_Active].value_2 - PowerSupply.CalBank[PowerSupply.bankCalibId].iCal[PowerSupply.mA_Active].value_1);
        // PowerSupply.Current.calib_b = PowerSupply.CalBank[PowerSupply.bankCalibId].iCal[PowerSupply.mA_Active].code_1 -
        //                               PowerSupply.Current.calib_m * PowerSupply.CalBank[PowerSupply.bankCalibId].iCal[PowerSupply.mA_Active].value_1;

        // PowerSupply.Current.calib_1m = 1.0 / PowerSupply.Current.calib_m;

        PowerSupply.Power.measured.ResetStats();
        PowerSupply.Voltage.measured.ResetStats();
        PowerSupply.Voltage.hist.Reset();

        PowerSupply.Current.measured.ResetStats();
        PowerSupply.Current.hist.Reset();
    }

    // if (PowerSupply.eepromWriteFlag)
    // {
    //     EEPROM.commit();
    //     PowerSupply.eepromWriteFlag = false;
    // }
}

// schedule() functions moved to intervals.cpp
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

    lv_chart_set_next_value(PowerSupply.graph.chart, PowerSupply.graph.serV, PowerSupply.Voltage.measured.value * 1000.0);
    lv_chart_set_next_value(PowerSupply.graph.chart, PowerSupply.graph.serI, PowerSupply.Current.measured.value * 1000.0);

    // Histogram
    // PowerSupply.Voltage.hist[PowerSupply.Voltage.measured.value];
    // PowerSupply.Current.hist[PowerSupply.Current.measured.value];
    // lv_chart_refresh(PowerSupply.stats.chart);
}

// GraphPush() and HistPush() moved to ui_helpers.cpp
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
            encoder2_value++;
        // PowerSupply.Current.SetUpdate(PowerSupply.Current.adjValue + PowerSupply.Current.rotaryEncoderStep);

        else if (count < rotaryOldValue2)
            encoder2_value--;
        // PowerSupply.Current.SetUpdate(PowerSupply.Current.adjValue - PowerSupply.Current.rotaryEncoderStep);

        rotaryOldValue2 = count;
        // encoder2_value = std::clamp(encoder2_value, -500, 500);
        // encoder2Flag = (encoder2Flag > 300) ? 300 : encoder2Flag - 1;
        encoder2Flag = 0;
        // if (encoder2Flag == 0)
        //   PowerSupply.Current.encoder.setCount(0);
        switch (Tabs::getCurrentPage())
        {
        // case 4:
        //     break;
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
        // case 4:
        //     break;
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
        Serial.printf("\n%s: Loop Count: %5.0f @ %07.2f seconds.%c", callerName, loopCount * 1000.0 / loopInterval, currentTime / 1000.0, keyChar);
        lastLoopTime = currentTime;
        loopCount = 0;
        // Serial.printf("\n%i", digitalRead(PowerSupply.CCCVPin));
    }
}

// All interval functions (LvglUpdatesInterval, FFTUpdateInterval, LvglFullUpdates, StatusBarUpdateInterval,
// FlushMeasuresInterval, statisticUpdateInterval, EncoderRestartInterval, KeyCheckInterval, and VCCCInterval)
// have been moved to intervals.cpp

// void DACInterval(unsigned long interval)
// {
//     static unsigned long timer_ = {0};
//     schedule([]
//              {
//                  // if (!lvglChartIsBusy)

//                  PowerSupply.DACUpdate(); },
//              interval, timer_);
// }

// **Helper Functions**

// Put these small helpers above handleCalibrationPage()
// enum class UIMode { Menu, DAC, ADC };

// // Decide which UI we are in (reads your existing windows).
// static inline UIMode detect_mode() {
//     if (lv_obj_is_visible(PowerSupply.gui.calibration.win_DAC_calibration))           return UIMode::DAC;
//     if (lv_obj_is_visible(PowerSupply.gui.calibration.win_ADC_voltage_calibration) ||
//         lv_obj_is_visible(PowerSupply.gui.calibration.win_ADC_current_calibration))   return UIMode::ADC;
//     return UIMode::Menu;
// }

// ===================== calibration_input.cpp =====================
// Private helpers (no globals exported)

// Encoder baselines and private menu index
static int32_t g_last_enc1_dac = 0, g_last_enc2_dac = 0;
static int32_t g_last_enc1_adc = 0, g_last_enc2_adc = 0;
static int32_t g_last_enc2_menu = 0;
static int g_menu_index = 0;             // replaces old global lastButton
static lv_obj_t *g_last_focus = nullptr; // to avoid “jump back” when focus changes

// Your app state (use whatever you already have)
// extern lv_obj_t *get_selected_spinbox();

// ----- small helpers -----
enum class UIMode
{
    Menu,
    DAC,
    ADC
};

static inline lv_obj_t *current_sidebar_list()
{
    lv_obj_t *page = lv_menu_get_cur_sidebar_page(PowerSupply.gui.setting_menu);
    return page ? lv_obj_get_child(page, 0) : nullptr; // first child is the list container in LVGL menu
}

static inline UIMode detect_mode()
{
    const bool dac =
        PowerSupply.gui.calibration.win_DAC_calibration &&
        lv_obj_is_visible(PowerSupply.gui.calibration.win_DAC_calibration);

    const bool adc =
        (PowerSupply.gui.calibration.win_ADC_voltage_calibration && lv_obj_is_visible(PowerSupply.gui.calibration.win_ADC_voltage_calibration)) ||
        (PowerSupply.gui.calibration.win_ADC_current_calibration && lv_obj_is_visible(PowerSupply.gui.calibration.win_ADC_current_calibration)) ||
        (PowerSupply.gui.calibration.win_ADC_INL_Voltage_calibration && lv_obj_is_visible(PowerSupply.gui.calibration.win_ADC_INL_Voltage_calibration)) ||
        (PowerSupply.gui.calibration.win_int_current_calibration && lv_obj_is_visible(PowerSupply.gui.calibration.win_int_current_calibration)); // NEW

    return (!dac && !adc) ? UIMode::Menu : (dac ? UIMode::DAC : UIMode::ADC);
}

// static inline UIMode detect_mode()
// {
//     const bool dac =
//         PowerSupply.gui.calibration.win_DAC_calibration &&
//         lv_obj_is_visible(PowerSupply.gui.calibration.win_DAC_calibration);

//     const bool adc =
//         (PowerSupply.gui.calibration.win_ADC_voltage_calibration && lv_obj_is_visible(PowerSupply.gui.calibration.win_ADC_voltage_calibration)) ||
//         (PowerSupply.gui.calibration.win_ADC_current_calibration && lv_obj_is_visible(PowerSupply.gui.calibration.win_ADC_current_calibration));

//     return (!dac && !adc) ? UIMode::Menu : (dac ? UIMode::DAC : UIMode::ADC);
// }

// ----- handlers (no binding, pure polling) -----
static void handle_menu_mode()
{
    lv_obj_t *list = current_sidebar_list();
    if (!list)
        return;

    if (encoder2_value != g_last_enc2_menu)
    {
        // Map wheel direction to list movement (flip if your hardware feels backwards)
        const int dir = (encoder2_value > g_last_enc2_menu) ? +1 : -1;
        const int count = lv_obj_get_child_cnt(list);
        g_menu_index = std::clamp(g_menu_index + dir, 0, std::max(0, count - 1));

        // "Activate" the target row
        lv_event_send(lv_obj_get_child(list, g_menu_index), LV_EVENT_CLICKED, nullptr);

        g_last_enc2_menu = encoder2_value;
    }
}

static void handle_dac_mode()
{
    if (!get_selected_spinbox())
        return;

    // Resync baseline when focus changes (e.g., touch switched spinbox)
    if (g_last_focus != get_selected_spinbox())
    {
        g_last_focus = get_selected_spinbox();
        g_last_enc2_dac = encoder2_value;
    }

    // Encoder 2 selects digit
    if (encoder2_value != g_last_enc2_dac)
    {
        (encoder2_value > g_last_enc2_dac) ? move_left(get_selected_spinbox())
                                           : move_right(get_selected_spinbox());
        g_last_enc2_dac = encoder2_value;
    }

    // Encoder 1 changes value
    if (encoder1_value != g_last_enc1_dac)
    {
        (encoder1_value > g_last_enc1_dac) ? increment(get_selected_spinbox())
                                           : decrement(get_selected_spinbox());
        g_last_enc1_dac = encoder1_value;
    }
}

static void handle_adc_mode()
{
    if (!get_selected_spinbox())
        return;

    if (g_last_focus != get_selected_spinbox())
    {
        g_last_focus = get_selected_spinbox();
        g_last_enc2_adc = encoder2_value;
    }

    // Encoder 2 moves cursor/digit
    if (encoder2_value != g_last_enc2_adc)
    {
        (encoder2_value > g_last_enc2_adc) ? move_left(get_selected_spinbox())
                                           : move_right(get_selected_spinbox());
        g_last_enc2_adc = encoder2_value;
        return; // only moved cursor this tick
    }

    // Encoder 1 changes value
    if (encoder1_value != g_last_enc1_adc)
    {
        (encoder1_value > g_last_enc1_adc) ? increment(get_selected_spinbox())
                                           : decrement(get_selected_spinbox());
        g_last_enc1_adc = encoder1_value;
    }
}

// ----- single public entry you call repeatedly -----
void handleCalibrationPage(int32_t /*unused1*/, int32_t /*unused2*/)
{
    switch (detect_mode())
    {
    case UIMode::Menu:
        handle_menu_mode();
        break;
    case UIMode::DAC:
        handle_dac_mode();
        break;
    case UIMode::ADC:
        handle_adc_mode();
        break;
    }
}

void handleGraphPage(int32_t &encoder1_last_value, int32_t &encoder2_last_value)
{

    // Handle horizontal scrolling and zooming with encoder 2
    if (encoder2_last_value != encoder2_value)
    {
        // Serial.printf("\nencoder2_value %i", encoder2_value);
        // Serial.printf("\nencoder2_last_value %i", encoder2_last_value);
        // int32_t _posX = lv_slider_get_value(slider_x);
        // static int32_t _posX = lv_slider_get_value(lv_obj_get_child(PowerSupply.page[1], 3));
        int32_t _posX = lv_slider_get_value(slider_x);

        if (encoder2_last_value < encoder2_value)
            _posX += 160;
        else if (encoder2_last_value > encoder2_value)
            _posX -= 160;
        // encoder2_last_value = encoder2_value;

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
        int32_t _posY = lv_slider_get_value(lv_obj_get_child(PowerSupply.page[1], 2));

        if (encoder1_last_value < encoder1_value)
            _posY += 64;
        else if (encoder1_last_value > encoder1_value)
            _posY -= 64;
        // encoder1_last_value = encoder1_value;

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

                autoScrollY();

                lvglChartIsBusy = false;
            }
        }
    }
}
void handleHistogramPage(int32_t &encoder1_last_value, int32_t &encoder2_last_value)
{

    // static int32_t encoder1_last_value = 0;
    // static int32_t encoder2_last_value = 0;
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
void handleUtilityPage(int32_t encoder1_last_value, int32_t encoder2_last_value)
{
    // static int32_t encoder1_last_value = 0;
    // static int32_t encoder2_last_value = 0;
    // int32_t  encoder2_last_value;

    // **Handle Vertical Shift/Zoom with Encoder 1**
    if (encoder1_last_value != encoder1_value)
    {

        static u8_t _posY = 0;

        // // Determine the direction of encoder 1 rotation
        if (encoder1_last_value < encoder1_value)

            _posY++; // Rotated clockwise

        else if (encoder1_last_value > encoder1_value)

            _posY--; // Rotated counter-clockwise

        static lv_obj_t *table = lv_obj_get_child(PowerSupply.page[3], 0);
        lv_tabview_set_act(table, _posY % 4, LV_ANIM_ON);
    }

    if (encoder2_last_value != encoder2_value)
    {

        static u8_t _posX = 0;
        // // Determine the direction of encoder 1 rotation
        if (encoder2_last_value < encoder2_value)
        {

            if (lv_tabview_get_tab_act(lv_obj_get_child(PowerSupply.page[3], 0)) == 1)
                select_previous_row(Utility_objs.table_fun_gen_list, 25);
            else
                select_previous_row(Utility_objs.table_point_list, 2 * 7 + 2 * 5);
        }
        else if (encoder2_last_value > encoder2_value)
        {

            if (lv_tabview_get_tab_act(lv_obj_get_child(PowerSupply.page[3], 0)) == 1)
                select_next_row(Utility_objs.table_fun_gen_list, 25);
            else
                select_next_row(Utility_objs.table_point_list, 2 * 7 + 2 * 5);
        }

        // static lv_obj_t *table = lv_obj_get_child(PowerSupply.page[3], 0);

        // lv_tabview_set_act(table, _posY % 4, LV_ANIM_ON);
    }
}

// Declare external class pointers for the object types you are interested in
// extern const lv_obj_class_t lv_btn_class;
// extern const lv_obj_class_t lv_label_class;
// extern const lv_obj_class_t lv_tabview_class;
// Add other classes as needed

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
    // { &lv_tabview_tab_class, "lv_tabview_tab" },
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
    // Add any other mappings as needed
    {NULL, NULL} // Sentinel to mark the end of the array
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

// void print_obj_type(lv_obj_t * obj) {
//     const lv_obj_class_t * obj_class = lv_obj_get_class(obj);

//     if(obj_class == &lv_btn_class) {
//         printf("Object is a button\n");
//     } else if(obj_class == &lv_label_class) {
//         printf("Object is a label\n");
//     } else if(obj_class == &lv_tabview_class) {
//         printf("Object is a tabview\n");
//     } else {
//         printf("Unknown object class\n");
//     }
// }

void handleUtility_function_Page(int32_t encoder1_last_value, int32_t encoder2_last_value)
{
    if (!get_selected_spinbox())
    {

        handleUtilityPage(encoder1_last_value, encoder2_last_value);
    }
    else
    {
        static int32_t cursor_pos = 0;

        // Check if encoder values have changed
        if (encoder2_last_value == encoder2_value && encoder1_last_value == encoder1_value)
            return;

        // Update cursor position based on encoder 2
        if (encoder2_last_value < encoder2_value)
        {
            move_left(get_selected_spinbox());
            encoder2_last_value = encoder2_value;
            return;
        }

        else if (encoder2_last_value > encoder2_value)
        {
            move_right(get_selected_spinbox());
            encoder2_last_value = encoder2_value;
            return;
        }

        if (encoder1_last_value < encoder1_value)
            increment(get_selected_spinbox());

        else if (encoder1_last_value > encoder1_value)
            decrement(get_selected_spinbox());

        encoder1_last_value = encoder1_value;

        // Serial.printf("\n***********************************");
        // print_obj_type(fgen_tabview); // Output: Object is************ a button
        // Serial.printf("\nSpinbox_pro%i", get_spinbox_data_by_id(fgen_tabview, 2));

        PowerSupply.funGenMem.amplitude = lv_spinbox_get_value(Utility_objs.fun.Amplitude) / 1000.0;
        PowerSupply.funGenMem.frequency = lv_spinbox_get_value(Utility_objs.fun.Frequency) / 1000.0;
        PowerSupply.funGenMem.offset = lv_spinbox_get_value(Utility_objs.fun.Offset) / 1000.0;
        PowerSupply.funGenMem.dutyCycle = lv_spinbox_get_value(Utility_objs.fun.Duty) / 10000.0;

        // PowerSupply.funGenMem.amplitude = double(get_spinbox_data_by_id(fgen_tabview, 0) / 1000.0);
        // PowerSupply.funGenMem.frequency = double(get_spinbox_data_by_id(fgen_tabview, 1) / 1000.0);
        // PowerSupply.funGenMem.offset = double(get_spinbox_data_by_id(fgen_tabview, 2) / 1000.0);
        // PowerSupply.funGenMem.dutyCycle = double(get_spinbox_data_by_id(fgen_tabview, 3) / 10000.0);

        if (lv_tabview_get_tab_act(lv_obj_get_child(PowerSupply.page[3], 0)) == 1)
            PowerSupply.SaveMemoryFgen("FunGen", PowerSupply.funGenMem);

        // Serial.printf("\n********************");
        // Serial.printf("\namplitude %f", PowerSupply.funGenMem.amplitude);
        // Serial.printf("\nfrequency %f", PowerSupply.funGenMem.frequency);
        // Serial.printf("\noffset %f", PowerSupply.funGenMem.offset);
        // Serial.printf("\ndutyCycle %f", PowerSupply.funGenMem.dutyCycle);
        // Serial.printf("\n********************");
        // print_obj_type(parent); // Output: Object is a button
        // Serial.printf("\nSpinbox_pro%i", get_spinbox_data_by_id(parent, 2));
    }
}

void managePageEncoderInteraction()
{
    // // Variables to store the last encoder values
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
        // lv_slider_set_value(slider_x, encoder2_last_value,LV_ANIM_OFF);
        break;

    case 2: // main Page
        break;
    case 3:
        handleUtility_function_Page(encoder1_last_value, encoder2_last_value);
        break;

    case 4: // Calibration Page

        handleCalibrationPage(encoder1_last_value, encoder2_last_value);
        break;

    default:

        break;
    }

    if (encoder1_last_value != encoder1_value)
        encoder1_last_value = encoder1_value;
    if (encoder2_last_value != encoder2_value)
        encoder2_last_value = encoder2_value;
}

// Might be in the llop
void MiscPriority()
{
    if (ismyTextHiddenChange && false)
    {

        if (priorityFlag != 1 && lv_obj_has_flag(PowerSupply.gui.textarea_set_value, LV_OBJ_FLAG_HIDDEN)) // lv_obj_has_flag(PowerSupply.gui.textarea_set_value, LV_OBJ_FLAG_HIDDEN) || !
        {
            priorityFlag = 1;
            vTaskPrioritySet(Task_adc, priorityFlag);
        }

        else if (priorityFlag != 0 && !lv_obj_has_flag(PowerSupply.gui.textarea_set_value, LV_OBJ_FLAG_HIDDEN))
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
        // PowerSupply.turn(SWITCH::OFF);
        PowerSupply.powerSwitch.turn(SWITCH::OFF);
        PowerSupply.Current.SetUpdate(-0.001 - 0 * PowerSupply.Current.adjOffset);
    }
    // LV_LOG_USER("%i",digitalRead(PowerSupply.CCCVPin));
    // if (g_wifiConnection)
    // ArduinoOTA.handle();
    // Serial.printf("3 \n");
}

#define CHANGE_THRESHOLD 0.09 / 32 / .5 // Adjust this for desired sensitivity

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
            // Serial.printf("Stable period ended at %1.3f  micros diff:%f\n", endStableTime, diff);

            // Print the duration
            double duration = endStableTime - startStableTime;
            // Serial.printf("Stable period duration: %1.3f  micros\n", duration);
            // triggerTime = startStableTime;
            isStable = false;
        }
    }

    lastValue = currentValue;
    return fabs(startStableTime - currentTimeMicros);
}

// Function to generate waveform based on parameters
bool functionGenerator()
{

    static unsigned long startTime = micros();
    unsigned long currentTime = micros();
    double elapsedTime = (currentTime - startTime) / 1'000'000.0;
    double t = fmod(elapsedTime * PowerSupply.funGenMem.frequency, 1.0);
    int selected_row = (int)Utility_objs.table_fun_gen_list->user_data;
    Waveform currentWaveform = waveforms[selected_row];
    double value = currentWaveform.function(t);
    // Serial.println(t);
    double outputValue = value * PowerSupply.funGenMem.amplitude + PowerSupply.funGenMem.offset;

    static double lastOutputValue = 0.0;
    if (outputValue != lastOutputValue)
    {
        PowerSupply.Voltage.SetUpdate(outputValue * PowerSupply.Voltage.adjFactor);
        // PowerSupply.Voltage.adjValue = outputValue;
        lastOutputValue = outputValue;
        // Serial.printf("\nSet output: %8.4f ", outputValue*2000.0);
    }

    // Track minimal change intervals
    //  Serial.printf("\nmonitor output: %1.3f ", monitorMinChanges(value, t) );
    // return (monitorMinChanges(value, t) < (.008));
    return true;
    Serial.print("Waveform: ");
    Serial.print(currentWaveform.name);
    Serial.print(" - Value: ");
    Serial.println(outputValue);
}

void functionGenerator_demo()
{
    static const unsigned long periodTotal = 100000000UL;    // 10 seconds in microseconds
    static const unsigned long periodWave = periodTotal / 3; // ~3.333 seconds per period
    static unsigned long startTime = micros();

    unsigned long currentTime = micros();
    unsigned long elapsedTime = currentTime - startTime;

    unsigned long totalDuration = numWaveforms * periodTotal;
    unsigned long timeInTotal = elapsedTime % totalDuration;

    int currentWaveformIndex = timeInTotal / periodTotal;
    unsigned long timeInWave = timeInTotal % periodTotal;
    unsigned long timeInPeriod = timeInWave % periodWave;
    double t = (double)timeInPeriod / (double)periodWave;

    double amplitude = 16.0;
    double offset = 16.0;

    // Get the current waveform struct
    Waveform currentWaveform = waveforms[currentWaveformIndex];
    double value = currentWaveform.function(t);
    double outputValue = value * amplitude + offset;

    // Get the current waveform name
    const char *currentWaveformName = currentWaveform.name;

    // Print the output value and waveform name
    if (!lv_obj_has_state(btn_function_gen, LV_STATE_CHECKED))
        return;

    Serial.print("Waveform: ");
    Serial.print(currentWaveformName);
    Serial.print(" - Value: ");
    Serial.println(outputValue);

    // Reduce delay to improve smoothness
    delay(1);
}