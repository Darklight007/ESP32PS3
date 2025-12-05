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
    constexpr unsigned long DAC_UPDATE_INTERVAL_FUNGEN_MS = 5;  // Back to 5ms - 0ms was too fast
    constexpr unsigned long KEY_CHECK_INTERVAL_ULTRAFAST_MS = 2;  // Ultra-fast when actively typing
    constexpr unsigned long KEY_CHECK_INTERVAL_FAST_MS = 10;
    constexpr unsigned long KEY_CHECK_INTERVAL_SLOW_MS = 105;
    constexpr unsigned long ADC_NOTIFY_TIMEOUT_MS = 1;
    constexpr unsigned long GRAPH_REFRESH_INTERVAL_MS = 125;
    constexpr unsigned long GRAPH_REFRESH_INTERVAL_FUNGEN_MS = 500;  // Slow down graph updates during FUN
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
        // Skip all bar graph updates in FUN Only mode
        if (lv_obj_has_state(Utility_objs.switch_fun_only, LV_STATE_CHECKED))
        {
            vTaskDelay(100);  // Sleep longer when disabled
            continue;
        }

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

        // Check if FUN Only mode is active (minimal processing for clean waveforms)
        bool funOnlyMode = lv_obj_has_state(Utility_objs.switch_fun_only, LV_STATE_CHECKED);

        // Update DAC outputs
        // FUN Only mode: 20ms interval = 50 Hz (smooth waveform)
        // FUN active: 5ms interval = 200 Hz
        // Normal: 100ms interval = 10 Hz
        unsigned long dacInterval;
        if (funOnlyMode)
            dacInterval = 20;  // 50 Hz update rate for smooth waveforms
        else if (lv_obj_has_state(btn_function_gen, LV_STATE_CHECKED))
            dacInterval = TaskTiming::DAC_UPDATE_INTERVAL_FUNGEN_MS;
        else
            dacInterval = TaskTiming::DAC_UPDATE_INTERVAL_NORMAL_MS;

        schedule([&] {
            if (lv_obj_has_state(btn_function_gen, LV_STATE_CHECKED))
                functionGenerator();
            PowerSupply.DACUpdate();
        }, dacInterval, dacUpdateTimer);

        // In FUN Only mode: skip ALL processing except DAC updates (above)
        if (funOnlyMode)
        {
            vTaskDelay(20);  // 20ms delay to match DAC update rate (50 Hz)
            continue;
        }

        // Handle keyboard and encoder input for responsive UI
        if (wireConnected)
        {
            bool textareaVisible = !lv_obj_has_flag(PowerSupply.gui.textarea_set_value, LV_OBJ_FLAG_HIDDEN);

            // Three-tier scanning speed:
            // - ULTRAFAST (2ms): When actively typing numbers
            // - FAST (10ms): When textarea is visible but waiting for first key
            // - SLOW (105ms): When textarea is hidden
            unsigned long keyInterval;
            if (textareaVisible && keyboardInputActive)
                keyInterval = TaskTiming::KEY_CHECK_INTERVAL_ULTRAFAST_MS;
            else if (textareaVisible)
                keyInterval = TaskTiming::KEY_CHECK_INTERVAL_FAST_MS;
            else
                keyInterval = TaskTiming::KEY_CHECK_INTERVAL_SLOW_MS;

            if (textareaVisible || lv_obj_has_state(Utility_objs.switch_keys_scan, LV_STATE_CHECKED))
            {
                KeyCheckInterval(keyInterval);
            }
            else
            {
                // Disable scanning when textarea closed and manual scan switch is OFF
                keyboardInputActive = false;
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

        // Fast recording logic (runs at ADC speed, up to ~300 SPS)
        // UI update strategy: on-the-fly for slow SPS (<=50ms period), batch for fast SPS
        static unsigned long lastRecordTime = 0;
        if (PowerSupply.recordingMem.is_recording)
        {
            if (millis() - lastRecordTime >= PowerSupply.recordingMem.sample_rate_ms)
            {
                lastRecordTime = millis();

                if (PowerSupply.recordingMem.sample_count < RECORDING_TABLE_SIZE)
                {
                    double voltage = PowerSupply.Voltage.measured.Mean();

                    // Normalize voltage to 0-1 range using ADC max voltage from setup
                    double normalized = voltage / PowerSupply.Voltage.adc_maxValue;  // 32.768V
                    // Clamp to valid range
                    if (normalized < 0.0) normalized = 0.0;
                    if (normalized > 1.0) normalized = 1.0;

                    // Store normalized value in table_points array
                    PowerSupply.funGenMem.table_points[PowerSupply.recordingMem.sample_count][0] = normalized;

                    // Update UI on-the-fly if SPS is slow enough (period >= 50ms = 20 SPS or slower)
                    bool slowSampling = (PowerSupply.recordingMem.sample_rate_ms >= 50);
                    if (slowSampling && !lvglIsBusy)
                    {
                        // Update table row
                        if (Utility_objs.table_point_list)
                        {
                            lv_table_set_cell_value_fmt(Utility_objs.table_point_list,
                                                       PowerSupply.recordingMem.sample_count, 1,
                                                       "%+06.4f", normalized);
                        }

                        // Update chart point
                        if (Utility_objs.record_chart && Utility_objs.record_chart_series)
                        {
                            lv_chart_set_value_by_id(Utility_objs.record_chart, Utility_objs.record_chart_series,
                                                    PowerSupply.recordingMem.sample_count,
                                                    (int32_t)(normalized * 100));
                            lv_chart_refresh(Utility_objs.record_chart);
                        }

                        // Update status every 5 samples
                        if (Utility_objs.record_status_label && (PowerSupply.recordingMem.sample_count % 5 == 0))
                        {
                            lv_label_set_text_fmt(Utility_objs.record_status_label, "Recording: %d/%d",
                                                 PowerSupply.recordingMem.sample_count + 1, RECORDING_TABLE_SIZE);
                        }
                    }

                    PowerSupply.recordingMem.sample_count++;
                }
                else
                {
                    // Max samples reached, stop recording and trigger UI update
                    PowerSupply.recordingMem.is_recording = false;
                    PowerSupply.recordingMem.needs_ui_update = true;  // Flag for batch update at end
                }
            }
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
            bool funActive = lv_obj_has_state(btn_function_gen, LV_STATE_CHECKED);
            unsigned long graphInterval = funActive
                ? TaskTiming::GRAPH_REFRESH_INTERVAL_FUNGEN_MS  // 500ms when FUN active
                : TaskTiming::GRAPH_REFRESH_INTERVAL_MS;        // 125ms normally
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
