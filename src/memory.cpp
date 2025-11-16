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
}
