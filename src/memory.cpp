#include "memory.h"
#include "device.hpp"
#include "globals.h"
#include "buzzer.h"
#include "tabs.h"
#include "functions.h"
#include "ui_helpers.h"

// External references
extern Device PowerSupply;
extern lv_obj_t *util_Arbit_chart;
extern lv_chart_series_t *util_Arbit_chart_series;

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

void saveToBank(int bank)
{
    LV_LOG_USER("Saving to Bank %d", bank);

    // IMPORTANT: Load existing data first to preserve other banks!
    PowerSupply.funGenMem = PowerSupply.LoadMemoryFgen("FunGen");

    // Get the y-array from the chart
    lv_coord_t *y_array = lv_chart_get_y_array(util_Arbit_chart, util_Arbit_chart_series);
    if (!y_array)
    {
        LV_LOG_USER("Error: y_array is NULL.");
        return;
    }

    // Update only the specified bank (preserves other banks)
    for (int i = 0; i < CHART_POINTS; i++)
    {
        PowerSupply.funGenMem.arbitrary_points[i][bank] = y_array[i];
    }

    // Save the entire FunGen memory structure back to flash
    PowerSupply.SaveMemoryFgen("FunGen", PowerSupply.funGenMem);

    Serial.printf("Bank %d saved successfully!\n", bank);
}

void loadFromBank(int bank)
{
    LV_LOG_USER("Loading from Bank %d", bank);

    // Load the entire FunGen memory structure
    PowerSupply.funGenMem = PowerSupply.LoadMemoryFgen("FunGen");

    // Update the chart with data from the specified bank
    for (int i = 0; i < CHART_POINTS; i++)
    {
        lv_chart_set_value_by_id(util_Arbit_chart, util_Arbit_chart_series, i, PowerSupply.funGenMem.arbitrary_points[i][bank]);
    }
    lv_chart_refresh(util_Arbit_chart);
}
