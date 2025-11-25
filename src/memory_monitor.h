#pragma once

#include <Arduino.h>

/**
 * Memory Monitoring Module
 *
 * Tracks heap and stack usage to detect memory leaks and prevent stack overflows.
 * Provides periodic monitoring and critical threshold warnings.
 */

struct MemoryStats
{
    // Heap statistics
    uint32_t heapTotal = 0;
    uint32_t heapFree = 0;
    uint32_t heapUsed = 0;
    uint32_t heapMinFree = UINT32_MAX;  // Minimum free heap seen
    uint32_t heapLargestFreeBlock = 0;

    // PSRAM statistics (if available)
    uint32_t psramTotal = 0;
    uint32_t psramFree = 0;
    uint32_t psramUsed = 0;
    uint32_t psramMinFree = UINT32_MAX;

    // Stack statistics (per task)
    uint32_t task1StackHighWater = 0;  // Task_ADC
    uint32_t task2StackHighWater = 0;  // Task_BarGraph

    // Warning flags
    bool lowHeapWarning = false;
    bool lowStackWarning = false;
    bool fragmentationWarning = false;

    // Tracking
    unsigned long lastUpdateTime = 0;
};

class MemoryMonitor
{
public:
    /**
     * Initialize memory monitoring
     * Sets baseline values and enables periodic monitoring
     */
    static void init();

    /**
     * Update memory statistics
     * Should be called periodically (e.g., every 1-5 seconds)
     */
    static void update();

    /**
     * Get current memory statistics
     * @return Reference to memory stats structure
     */
    static MemoryStats& getStats();

    /**
     * Print memory statistics to serial
     * Useful for debugging and diagnostics
     */
    static void printStats();

    /**
     * Check for critical memory conditions
     * @return true if memory is critically low
     */
    static bool isCritical();

    /**
     * Reset warning flags
     */
    static void clearWarnings();

private:
    static MemoryStats stats;

    // Warning thresholds
    static constexpr uint32_t MIN_FREE_HEAP = 10000;       // 10KB
    static constexpr uint32_t MIN_STACK_HIGHWATER = 500;   // 500 bytes
    static constexpr float MAX_FRAGMENTATION_RATIO = 0.5;  // 50%
};
