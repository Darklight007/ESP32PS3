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
    Serial.printf("\n========== SAVE TO BANK %d ==========\n", bank);

    // IMPORTANT: Load existing data first to preserve other banks!
    PowerSupply.funGenMem = PowerSupply.LoadMemoryFgen("FunGen");

    // Debug: Show what's currently in BOTH banks before save
    Serial.printf("Before save - Bank 0 first 5 points: ");
    for (int i = 0; i < 5; i++) {
        Serial.printf("%.0f ", PowerSupply.funGenMem.arbitrary_points[i][0]);
    }
    Serial.printf("\nBefore save - Bank 1 first 5 points: ");
    for (int i = 0; i < 5; i++) {
        Serial.printf("%.0f ", PowerSupply.funGenMem.arbitrary_points[i][1]);
    }
    Serial.println();

    // Get the y-array from the chart
    lv_coord_t *y_array = lv_chart_get_y_array(util_Arbit_chart, util_Arbit_chart_series);
    if (!y_array)
    {
        LV_LOG_USER("Error: y_array is NULL.");
        return;
    }

    // Debug: Show what's in the chart
    Serial.printf("Chart data first 5 points: ");
    for (int i = 0; i < 5; i++) {
        Serial.printf("%d ", y_array[i]);
    }
    Serial.println();

    // Update only the specified bank (preserves other banks)
    for (int i = 0; i < CHART_POINTS; i++)
    {
        PowerSupply.funGenMem.arbitrary_points[i][bank] = y_array[i];
    }

    // Debug: Show what's in BOTH banks after update but before flash save
    Serial.printf("After update - Bank 0 first 5 points: ");
    for (int i = 0; i < 5; i++) {
        Serial.printf("%.0f ", PowerSupply.funGenMem.arbitrary_points[i][0]);
    }
    Serial.printf("\nAfter update - Bank 1 first 5 points: ");
    for (int i = 0; i < 5; i++) {
        Serial.printf("%.0f ", PowerSupply.funGenMem.arbitrary_points[i][1]);
    }
    Serial.println();

    // Save the entire FunGen memory structure back to flash
    PowerSupply.SaveMemoryFgen("FunGen", PowerSupply.funGenMem);

    Serial.printf("Bank %d saved successfully to flash!\n", bank);
    Serial.printf("=====================================\n\n");
}

void loadFromBank(int bank)
{
    LV_LOG_USER("Loading from Bank %d", bank);
    Serial.printf("\n========== LOAD FROM BANK %d ==========\n", bank);

    // Load the entire FunGen memory structure
    PowerSupply.funGenMem = PowerSupply.LoadMemoryFgen("FunGen");

    // Debug: Show what's in BOTH banks after loading from flash
    Serial.printf("Loaded from flash - Bank 0 first 5 points: ");
    for (int i = 0; i < 5; i++) {
        Serial.printf("%.0f ", PowerSupply.funGenMem.arbitrary_points[i][0]);
    }
    Serial.printf("\nLoaded from flash - Bank 1 first 5 points: ");
    for (int i = 0; i < 5; i++) {
        Serial.printf("%.0f ", PowerSupply.funGenMem.arbitrary_points[i][1]);
    }
    Serial.println();

    // Debug: Show which bank we're loading
    Serial.printf("Loading Bank %d data to chart (first 5 points): ", bank);
    for (int i = 0; i < 5; i++) {
        Serial.printf("%.0f ", PowerSupply.funGenMem.arbitrary_points[i][bank]);
    }
    Serial.println();

    // Update the chart with data from the specified bank
    for (int i = 0; i < CHART_POINTS; i++)
    {
        lv_chart_set_value_by_id(util_Arbit_chart, util_Arbit_chart_series, i, PowerSupply.funGenMem.arbitrary_points[i][bank]);
    }
    lv_chart_refresh(util_Arbit_chart);

    Serial.printf("Bank %d loaded successfully to chart!\n", bank);
    Serial.printf("========================================\n\n");
}
