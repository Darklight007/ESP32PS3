#include "memory_monitor.h"
#include "config.hpp"

// External task handles from config
extern TaskHandle_t Task_adc, Task1;

// Static member initialization
MemoryStats MemoryMonitor::stats = {};

void MemoryMonitor::init()
{
    // Get baseline memory values
    stats.heapTotal = ESP.getHeapSize();
    stats.psramTotal = ESP.getPsramSize();

    Serial.println("=== Memory Monitor Initialized ===");
    Serial.printf("Total Heap: %u bytes\n", stats.heapTotal);
    Serial.printf("Total PSRAM: %u bytes\n", stats.psramTotal);

    // Initial update
    update();
}

void MemoryMonitor::update()
{
    // Update heap statistics
    stats.heapFree = ESP.getFreeHeap();
    stats.heapUsed = stats.heapTotal - stats.heapFree;
    stats.heapLargestFreeBlock = ESP.getMaxAllocHeap();

    // Track minimum free heap
    if (stats.heapFree < stats.heapMinFree)
    {
        stats.heapMinFree = stats.heapFree;
    }

    // Update PSRAM statistics (if available)
    if (stats.psramTotal > 0)
    {
        stats.psramFree = ESP.getFreePsram();
        stats.psramUsed = stats.psramTotal - stats.psramFree;

        if (stats.psramFree < stats.psramMinFree)
        {
            stats.psramMinFree = stats.psramFree;
        }
    }

    // Update stack high water marks (if task handles are valid)
    if (Task_adc != NULL)
    {
        stats.task1StackHighWater = uxTaskGetStackHighWaterMark(Task_adc);
    }

    if (Task1 != NULL)
    {
        stats.task2StackHighWater = uxTaskGetStackHighWaterMark(Task1);
    }

    // Check warning conditions
    if (stats.heapFree < MIN_FREE_HEAP)
    {
        if (!stats.lowHeapWarning)
        {
            stats.lowHeapWarning = true;
            Serial.printf("WARNING: Low heap! Free: %u bytes\n", stats.heapFree);
        }
    }
    else
    {
        stats.lowHeapWarning = false;
    }

    if (stats.task1StackHighWater < MIN_STACK_HIGHWATER ||
        stats.task2StackHighWater < MIN_STACK_HIGHWATER)
    {
        if (!stats.lowStackWarning)
        {
            stats.lowStackWarning = true;
            Serial.printf("WARNING: Low stack! Task1: %u, Task2: %u bytes\n",
                         stats.task1StackHighWater, stats.task2StackHighWater);
        }
    }
    else
    {
        stats.lowStackWarning = false;
    }

    // Check for heap fragmentation
    if (stats.heapFree > 0)
    {
        float fragmentationRatio = (float)stats.heapLargestFreeBlock / (float)stats.heapFree;
        if (fragmentationRatio < MAX_FRAGMENTATION_RATIO)
        {
            if (!stats.fragmentationWarning)
            {
                stats.fragmentationWarning = true;
                Serial.printf("WARNING: Heap fragmentation detected! Largest block: %u / Free: %u (%.1f%%)\n",
                             stats.heapLargestFreeBlock, stats.heapFree, fragmentationRatio * 100.0f);
            }
        }
        else
        {
            stats.fragmentationWarning = false;
        }
    }

    stats.lastUpdateTime = millis();
}

MemoryStats& MemoryMonitor::getStats()
{
    return stats;
}

void MemoryMonitor::printStats()
{
    Serial.println("\n=== Memory Statistics ===");

    // Heap
    Serial.printf("Heap Total:    %6u bytes\n", stats.heapTotal);
    Serial.printf("Heap Used:     %6u bytes (%.1f%%)\n",
                 stats.heapUsed,
                 (float)stats.heapUsed / (float)stats.heapTotal * 100.0f);
    Serial.printf("Heap Free:     %6u bytes (%.1f%%)\n",
                 stats.heapFree,
                 (float)stats.heapFree / (float)stats.heapTotal * 100.0f);
    Serial.printf("Heap Min Free: %6u bytes\n", stats.heapMinFree);
    Serial.printf("Largest Block: %6u bytes\n", stats.heapLargestFreeBlock);

    // PSRAM (if available)
    if (stats.psramTotal > 0)
    {
        Serial.println();
        Serial.printf("PSRAM Total:    %6u bytes\n", stats.psramTotal);
        Serial.printf("PSRAM Used:     %6u bytes (%.1f%%)\n",
                     stats.psramUsed,
                     (float)stats.psramUsed / (float)stats.psramTotal * 100.0f);
        Serial.printf("PSRAM Free:     %6u bytes (%.1f%%)\n",
                     stats.psramFree,
                     (float)stats.psramFree / (float)stats.psramTotal * 100.0f);
        Serial.printf("PSRAM Min Free: %6u bytes\n", stats.psramMinFree);
    }

    // Stack
    Serial.println();
    Serial.printf("Task_ADC Stack High Water:      %u bytes\n", stats.task1StackHighWater);
    Serial.printf("Task_BarGraph Stack High Water: %u bytes\n", stats.task2StackHighWater);

    // Warnings
    if (stats.lowHeapWarning || stats.lowStackWarning || stats.fragmentationWarning)
    {
        Serial.println();
        Serial.println("WARNINGS:");
        if (stats.lowHeapWarning)
            Serial.println("  - Low heap memory");
        if (stats.lowStackWarning)
            Serial.println("  - Low stack space");
        if (stats.fragmentationWarning)
            Serial.println("  - Heap fragmentation");
    }

    Serial.println("========================\n");
}

bool MemoryMonitor::isCritical()
{
    return stats.lowHeapWarning || stats.lowStackWarning;
}

void MemoryMonitor::clearWarnings()
{
    stats.lowHeapWarning = false;
    stats.lowStackWarning = false;
    stats.fragmentationWarning = false;
}
