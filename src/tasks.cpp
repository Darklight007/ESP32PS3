#include "tasks.h"
#include "device.hpp"
#include "tabs.h"
#include "functions.h"
#include "globals.h"
#include "buzzer.h"

// Task timing constants
namespace TaskTiming {
    constexpr unsigned long BARGRAPH_DELAY_OFF_PAGE_MS = 10;
    constexpr unsigned long BARGRAPH_DELAY_ACTIVE_MS = 1;
    constexpr unsigned long DAC_UPDATE_INTERVAL_NORMAL_MS = 100;
    constexpr unsigned long DAC_UPDATE_INTERVAL_FUNGEN_MS = 5;
    constexpr unsigned long KEY_CHECK_INTERVAL_FAST_MS = 10;
    constexpr unsigned long KEY_CHECK_INTERVAL_SLOW_MS = 105;
    constexpr unsigned long ADC_NOTIFY_TIMEOUT_MS = 1;
    constexpr unsigned long GRAPH_REFRESH_INTERVAL_MS = 125;
}

// Page indices
namespace Pages {
    constexpr int HISTOGRAM = 0;
    constexpr int GRAPH = 1;
    constexpr int MAIN = 2;
}

// External variables
extern Device PowerSupply;
extern bool lvglIsBusy;
extern bool lvglChartIsBusy;
extern bool blockAll;
extern bool wireConnected;
extern lv_obj_t *btn_function_gen;

// Forward declarations
void schedule(std::function<void(void)> func, unsigned long &&interval, unsigned long &startTime);
void schedule(std::function<void(void)> func, unsigned long &interval, unsigned long &startTime);
void HistPush();
void GraphPush();
void KeyCheckInterval(unsigned long interval);
void getSettingEncoder(lv_indev_drv_t *drv, lv_indev_data_t *data);
bool functionGenerator();

/**
 * @brief Bar graph update task (runs on Core 1)
 *
 * Updates voltage and current bar graphs on the main page.
 * Runs slower when not on main page to save CPU cycles.
 *
 * @param pvParameters Unused FreeRTOS parameter
 */
void Task_BarGraph(void *pvParameters)
{
    for (;;)
    {
        if (Tabs::getCurrentPage() != Pages::MAIN)
        {
            vTaskDelay(TaskTiming::BARGRAPH_DELAY_OFF_PAGE_MS);
            continue;
        }

        if (!lvglIsBusy && !blockAll)
        {
            PowerSupply.Voltage.barUpdate();
            PowerSupply.Current.barUpdate();
        }

        vTaskDelay(TaskTiming::BARGRAPH_DELAY_ACTIVE_MS);
    }
}

/**
 * @brief ADC sampling and processing task (runs on Core 0)
 *
 * High-priority task that:
 * - Handles ADC data acquisition via interrupt notifications
 * - Processes voltage and current measurements
 * - Updates DAC outputs
 * - Handles keyboard and encoder input
 * - Updates charts and histograms
 *
 * Uses hybrid interrupt + polling approach for optimal responsiveness.
 *
 * @param pvParameters Unused FreeRTOS parameter
 */
void Task_ADC(void *pvParameters)
{
    static unsigned long dacUpdateTimer = 0;
    static unsigned long graphRefreshTimer = 0;

    for (;;)
    {
        toneOff();

        // Update DAC outputs (faster when function generator active)
        unsigned long dacInterval = lv_obj_has_state(btn_function_gen, LV_STATE_CHECKED)
            ? TaskTiming::DAC_UPDATE_INTERVAL_FUNGEN_MS
            : TaskTiming::DAC_UPDATE_INTERVAL_NORMAL_MS;

        schedule([&] {
            if (lv_obj_has_state(btn_function_gen, LV_STATE_CHECKED))
                functionGenerator();
            PowerSupply.DACUpdate();
        }, dacInterval, dacUpdateTimer);

        // Handle keyboard and encoder input for responsive UI
        if (wireConnected)
        {
            const unsigned long keyInterval = !lv_obj_has_flag(PowerSupply.gui.textarea_set_value, LV_OBJ_FLAG_HIDDEN)
                ? TaskTiming::KEY_CHECK_INTERVAL_FAST_MS
                : TaskTiming::KEY_CHECK_INTERVAL_SLOW_MS;

            if (!lv_obj_has_flag(PowerSupply.gui.textarea_set_value, LV_OBJ_FLAG_HIDDEN) ||
                lv_obj_has_state(Utility_objs.switch_keys_scan, LV_STATE_CHECKED))
            {
                KeyCheckInterval(keyInterval);
            }
        }
        getSettingEncoder(NULL, NULL);

        // Wait for ADC interrupt notification with short timeout for responsiveness
        if (!adcDataReady)
        {
            ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(TaskTiming::ADC_NOTIFY_TIMEOUT_MS));
        }

        // Always re-check after wait - might have been woken by timeout instead of notification
        if (!adcDataReady)
        {
            continue;
        }

        // Process ADC data
        if (!lvglIsBusy || PowerSupply.settingParameters.adcRate != 0)
        {
            PowerSupply.readVoltage();
            PowerSupply.readCurrent();
        }

        // Update power calculation (convert mA to A if needed)
        const double currentInAmps = PowerSupply.Current.measured.Mean() * (PowerSupply.mA_Active ? 0.001 : 1.0);
        PowerSupply.Power.measureUpdate(currentInAmps * PowerSupply.Voltage.measured.Mean());

        // Track CC/CV status changes
        static bool lastCCCVStatus = false;
        const bool currentCCCVStatus = digitalRead(PowerSupply.CCCVPin);
        if (lastCCCVStatus != currentCCCVStatus)
        {
            lastCCCVStatus = currentCCCVStatus;
        }

        // Update histogram and graph data
        HistPush();
        GraphPush();

        // Refresh histogram chart on page 0
        if (Tabs::getCurrentPage() == Pages::HISTOGRAM && !lvglIsBusy && !lvglChartIsBusy && !blockAll)
        {
            lvglChartIsBusy = true;
            lv_chart_refresh(PowerSupply.stats.chart);
            lvglChartIsBusy = false;
        }

        // Refresh graph chart on page 1 with throttling
        if (Tabs::getCurrentPage() == Pages::GRAPH && !lvglIsBusy)
        {
            unsigned long graphInterval = TaskTiming::GRAPH_REFRESH_INTERVAL_MS;
            schedule([]{
                if (!lvglChartIsBusy && !blockAll)
                {
                    lvglChartIsBusy = true;
                    lv_chart_refresh(PowerSupply.graph.chart);
                    lvglChartIsBusy = false;
                }
            }, graphInterval, graphRefreshTimer);
        }
    }
}
