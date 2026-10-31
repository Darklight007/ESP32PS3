#pragma once

// FreeRTOS task function declarations

#include <Arduino.h>

void Task_ADC(void *pvParameters);

// Background flash saves (see tasks.cpp)
enum : uint32_t
{
    PERSIST_GRAPH = 1u << 0, // graph trace snapshot (SPIFFS)
    PERSIST_FGEN = 1u << 1,  // function generator settings + tables (NVS + SPIFFS)
};
void createPersistTask();
void persistRequest(uint32_t what);
