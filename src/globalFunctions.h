#pragma once

#include <iostream>
#include <chrono>
#include <thread>
#include "input_device.h"
#include "setting_menu.h"
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
#include "memory.h"
#include "ui_creation.h"

// Forward declarations for functions defined later in this file
void getSettingEncoder(lv_indev_drv_t *drv, lv_indev_data_t *data);
bool functionGenerator();
lv_obj_t *find_btn_by_tag(lv_obj_t *parent, uint32_t tag);

#define LV_TICK_CUSTOM 1

// draw_event_stat_chart_cb() moved to ui_helpers.cpp

static void draw_event_stat_chart_cb_old(lv_event_t *e)
{
    /*Hook the division lines too*/
    lv_obj_draw_part_dsc_t *dsc = lv_event_get_draw_part_dsc(e);

    if (dsc->text)
    {
        if (dsc->id == LV_CHART_AXIS_PRIMARY_X)
        {
            static const char *tickLabel[] = {"0.0\n0.0", "5.0\n1.0", "10\n2.0", "15\n3.0", "20\n4.0", "25\n5.0", "30[V]\n6.0[A]"};

            static char str[7][18];

            lv_snprintf(dsc->text, dsc->text_length, "%s", tickLabel[dsc->value]);
        }
    }
};

// tickLabels_x moved to globals.h/cpp

// draw_event_cb2() moved to ui_helpers.cpp

static void draw_event_cb2_old(lv_event_t *e)
{
    lv_obj_draw_part_dsc_t *dsc = lv_event_get_draw_part_dsc(e);

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
            }
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

            std::sprintf(c[5], "%d", pointCount);
            std::sprintf(c[4], "%d", pointCount * 5 / 6);
            std::sprintf(c[3], "%d", pointCount * 4 / 6);
            std::sprintf(c[2], "%d", pointCount * 3 / 6);
            std::sprintf(c[1], "%d", pointCount * 2 / 6);
            std::sprintf(c[0], "%d", pointCount * 1 / 6);

            static char *tickLabel[] = {c[5], c[4], c[3], c[2], c[1], c[0], "pts"};

            lv_snprintf(dsc->text, dsc->text_length, "%s", tickLabel[dsc->value]);

            if (dsc->label_dsc)
                dsc->label_dsc->font = &lv_font_montserrat_10;
        }
        else if (dsc->id == LV_CHART_AXIS_SECONDARY_Y)
        {
            static int i = 0;

            static char *tickLabel_sy[] = {"8.0A", "7.0", "6.0", "5.0", "4.0", "3.0", "2.0", "1.0", "0.0"};

            if (strcmp(dsc->text, "8000") == 0)
                i = 0;
            lv_snprintf(dsc->text, dsc->text_length, "%s", tickLabel_sy[i++]);

            if (dsc->label_dsc)
                dsc->label_dsc->font = &lv_font_montserrat_10;
        }

        else if (dsc->id == LV_CHART_AXIS_PRIMARY_Y)
        {
            static int i = 0;
            static char *tickLabel_y[] = {"32.0\nVolt", "28.0", "24.0", "20.0", "16.0", "12.0", "8.0", "4.0", "0.0"};

            if (strcmp(dsc->text, "32000") == 0)
                i = 0;
            lv_snprintf(dsc->text, dsc->text_length, "%s", tickLabel_y[i++]);

            if (dsc->label_dsc)
                dsc->label_dsc->font = &lv_font_montserrat_10;
        }
    }
}

// touch_calibrate() and TouchAttr moved to input_handler.cpp

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 180
#define BUCKET_COUNT 100
#define DOT_SIZE 3
#define CHART_Y_MAX 100
#define CHART_POINTS 20

// dataBuckets, dropdown_active, util_Arbit_chart moved to globals.h/cpp

// my_touchpad_read() and init_touch() moved to input_handler.cpp

// Task_BarGraph and Task_ADC moved to tasks.cpp

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

static void key_event_handler(uint16_t btn_id)
{
    lv_obj_clear_flag(PowerSupply.gui.textarea_set_value, LV_OBJ_FLAG_HIDDEN);
    lv_btnmatrix_set_selected_btn(btnm, btn_id);
    lv_event_send(btnm, LV_EVENT_VALUE_CHANGED, NULL);
    lv_label_set_text(unit_label, "mV/V/mA/A");
    ismyTextHiddenChange = true;
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

// scaleVoltage and scaleCurrent moved to functions.cpp
// loadMemory and saveMemory moved to memory.cpp

static void table_get_event_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED)
    {
        lv_obj_t *table = lv_event_get_target(e);
        uint16_t row, col;
        lv_table_get_selected_cell(table, &row, &col);
        const char *cell_str = lv_table_get_cell_value(table, row, 1);
        Utility_objs.table_current_value = atof(cell_str);
        lv_spinbox_set_value(Utility_objs.table_spinbox_value, Utility_objs.table_current_value);
    }
}

#define NUM_LABELS 7

// saveToBank and loadFromBank moved to memory.cpp

static void dropdownEventCb(lv_event_t *e)
{
    lv_obj_t *dropdown = lv_event_get_target(e);
    char buf[64];
    lv_dropdown_get_selected_str(dropdown, buf, sizeof(buf));
    LV_LOG_USER("'%s' is selected", buf);

    dropdown_active = false;

    std::string selection(buf);
    if (selection == "Save to Bank 0")
    {
        saveToBank(0);
    }
    else if (selection == "Save to Bank 1")
    {
        saveToBank(1);
    }
    else if (selection == "Load to Bank 0")
    {
        loadFromBank(0);
    }
    else if (selection == "Load to Bank 1")
    {
        loadFromBank(1);
    }
    else
    {
        LV_LOG_USER("Unknown selection: %s", buf);
        int selected_row = lv_dropdown_get_selected(dropdown) - 1;
        if (selected_row == -1)
        {
            lv_chart_set_all_value(util_Arbit_chart, util_Arbit_chart_series, 0);
            lv_chart_refresh(util_Arbit_chart);
            return;
        }
        Waveform currentWaveform = waveforms[selected_row];

        for (int i = 0; i < CHART_POINTS; i++)
        {
            double value = currentWaveform.function((double)i / CHART_POINTS);
            double outputValue = value * 1.0;
            lv_chart_set_value_by_id(util_Arbit_chart, util_Arbit_chart_series, i, outputValue * 140);
        }

        lv_chart_refresh(util_Arbit_chart);
    }
}

// select_next_row and select_previous_row moved to table_pro.cpp

// btn_function_gen_event_cb() moved to ui_helpers.cpp

// schedule() functions moved to intervals.cpp

// GraphPush() and HistPush() moved to ui_helpers.cpp
// getSettingEncoder() moved to input_handler.cpp

// trackLoopExecution() moved to functions.cpp

// All interval functions (LvglUpdatesInterval, FFTUpdateInterval, LvglFullUpdates, StatusBarUpdateInterval,
// FlushMeasuresInterval, statisticUpdateInterval, EncoderRestartInterval, KeyCheckInterval, and VCCCInterval)
// have been moved to intervals.cpp

// ===================== Page handler functions moved to input_handler.cpp =====================
// The following functions have been moved to input_handler.cpp:
// - handleCalibrationPage()
// - handleGraphPage()
// - handleHistogramPage()
// - handleUtilityPage()
// - handleUtility_function_Page()
// Along with their helper functions and static variables.

// managePageEncoderInteraction() moved to input_handler.cpp

// CHANGE_THRESHOLD and monitorMinChanges() moved to functions.cpp

// functionGenerator() and functionGenerator_demo() moved to waveform_generator.cpp
