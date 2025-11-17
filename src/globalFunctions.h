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
#include "globals.h"
#include "functions.h"
#include "ui_helpers.h"
#include "tasks.h"
#include "input_handler.h"
#include "intervals.h"
#include "memory.h"
#include "ui_creation.h"

// ============================================================================
// Forward Declarations
// ============================================================================

void getSettingEncoder(lv_indev_drv_t *drv, lv_indev_data_t *data);
bool functionGenerator();
lv_obj_t *find_btn_by_tag(lv_obj_t *parent, uint32_t tag);

// ============================================================================
// Configuration Defines
// ============================================================================

#define LV_TICK_CUSTOM 1
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 180
#define BUCKET_COUNT 100
#define DOT_SIZE 3
#define CHART_Y_MAX 100
#define CHART_POINTS 20
#define NUM_LABELS 7

// ============================================================================
// Active Event Callbacks (still in use)
// ============================================================================

// Switch event callback for list scrolling mode
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

// Key event handler - shows textarea for value entry
static void key_event_handler(uint16_t btn_id)
{
    lv_obj_clear_flag(PowerSupply.gui.textarea_set_value, LV_OBJ_FLAG_HIDDEN);
    lv_btnmatrix_set_selected_btn(btnm, btn_id);
    lv_event_send(btnm, LV_EVENT_VALUE_CHANGED, NULL);
    lv_label_set_text(unit_label, "mV/V/mA/A");
    ismyTextHiddenChange = true;
}

// Callback version of readback handler
static void key_event_handler_readBack_clb(const char *txt)
{
    lv_textarea_set_text(ta, txt);
    lv_label_set_text(unit_label, "");
}

// Read back current value from display object in base units (V or A)
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

// Read back current value in milli units (mV or mA)
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

// Table selection event - updates spinbox with selected value
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

// Dropdown event callback - handles bank save/load and waveform selection
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
