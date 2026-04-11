#include "intervals.h"
#include <Arduino.h>
#include <lvgl.h>
#include "device.hpp"
#include "globals.h"
#include "tabs.h"
#include "FFTHandler.h"
#include "power_management.h"
#include "memory_monitor.h"

// External references
extern Device PowerSupply;
extern bool lvglChartIsBusy;
extern bool blockAll;
extern volatile bool adcDataReady;
extern bool lvglIsBusy;
extern FFTHandler V;
extern FFTHandler I;

// Forward declarations for functions still in globalFunctions.h
void StatusBar();
void keyCheckLoop();

void schedule(std::function<void(void)> func, unsigned long &&interval, unsigned long &startTime)
{
    if ((millis() - startTime) < interval)
        return;

    startTime = millis();
    func();
}

void schedule(std::function<void(void)> func, unsigned long &interval, unsigned long &startTime)
{
    if ((millis() - startTime) < interval)
        return;
    startTime = millis();
    func();
}

void StatusBarUpdateInterval(unsigned long interval)
{
    static unsigned long timer_ = {0};
    schedule(&StatusBar, interval, timer_);
}

void FlushMeasuresInterval(unsigned long interval)
{
    static unsigned long timer_ = {0};
    schedule([]
             { PowerSupply.FlushMeasures(); }, interval, timer_);
}

void statisticUpdateInterval(unsigned long interval)
{
    static unsigned long timer_ = {0};
    schedule([]
             {
                // Only save settings if values actually changed AND not in FUN mode
                // (FUN mode changes adjValue at 200-500Hz — saving to NVS that fast
                // causes flash GC stalls that freeze the display)
                static uint16_t lastVoltage = 0;
                static uint16_t lastCurrent = 0;
                if (PowerSupply.getStatus() != DEVICE::FUN &&
                    (PowerSupply.Voltage.adjValue != lastVoltage ||
                     PowerSupply.Current.adjValue != lastCurrent)) {
                    PowerSupply.settingParameters.SetVoltage = PowerSupply.Voltage.adjValue;
                    PowerSupply.settingParameters.SetCurrent = PowerSupply.Current.adjValue;
                    PowerSupply.SaveSetting();
                    lastVoltage = PowerSupply.Voltage.adjValue;
                    lastCurrent = PowerSupply.Current.adjValue;
                }

                PowerSupply.Voltage.statUpdate();
                PowerSupply.Current.statUpdate(); },
             interval, timer_);
}

void EncoderRestartInterval(unsigned long interval)
{
    static unsigned long timer_ = {0};
    schedule([]
             {
                 int64_t temp = PowerSupply.Voltage.encoder.getCount();
                 PowerSupply.Voltage.encoder.clearCount();
                 PowerSupply.Voltage.encoder.setCount(temp);

                 temp = PowerSupply.Current.encoder.getCount();
                 PowerSupply.Current.encoder.clearCount();
                 PowerSupply.Current.encoder.setCount(temp); },
             interval, timer_);
}

void KeyCheckInterval(unsigned long interval)
{
    static unsigned long timer_2 = {0};
    schedule([]
             { keyCheckLoop(); },
             interval, timer_2);
}

void LvglFullUpdates(unsigned long interval)
{
    static unsigned long timer_ = {0};
    schedule([]
             { lv_obj_invalidate(lv_scr_act()); }, interval, timer_);
}

// Clean right side of page 2 to fix dirty pixels
void Page2RightSideCleanup(unsigned long interval)
{
    static unsigned long timer_ = {0};
    schedule([]
             {
                 // Only clean when on page 2 (Main page)
                 if (Tabs::getCurrentPage() != 2) return;

                 // Invalidate current display elements (including mA/A prefix)
                 if (PowerSupply.Current.label_si_prefix)
                     lv_obj_invalidate(PowerSupply.Current.label_si_prefix);
                 if (PowerSupply.Current.label_unit)
                     lv_obj_invalidate(PowerSupply.Current.label_unit);
                 if (PowerSupply.Current.label_measureValue)
                     lv_obj_invalidate(PowerSupply.Current.label_measureValue);

                 // Invalidate voltage display elements
                 if (PowerSupply.Voltage.label_unit)
                     lv_obj_invalidate(PowerSupply.Voltage.label_unit);
                 if (PowerSupply.Voltage.label_measureValue)
                     lv_obj_invalidate(PowerSupply.Voltage.label_measureValue);

                 // Invalidate power and other right side elements
                 if (PowerSupply.Power.label_measureValue)
                     lv_obj_invalidate(PowerSupply.Power.label_measureValue);
                 if (PowerSupply.Power.label_unit)
                     lv_obj_invalidate(PowerSupply.Power.label_unit);
                 if (PowerSupply.gui.label_energy_counter)
                     lv_obj_invalidate(PowerSupply.gui.label_energy_counter);
                 if (PowerSupply.gui.label_power_on_time)
                     lv_obj_invalidate(PowerSupply.gui.label_power_on_time);
                 if (PowerSupply.gui.label_timer_remaining)
                     lv_obj_invalidate(PowerSupply.gui.label_timer_remaining);
                 if (PowerSupply.powerSwitch.btn)
                     lv_obj_invalidate(PowerSupply.powerSwitch.btn);
             }, interval, timer_);
}

void VCCCInterval(unsigned long interval)
{
    static unsigned long timer_ = {0};
    schedule([]
             { PowerSupply.VCCCStatusUpdate(); },
             interval, timer_);
}

// Internal state for LvglUpdatesInterval - shared between overloads
static unsigned long lvgl_timer_ = {0};
static unsigned int lvglIsBlocked = {0};

void LvglUpdatesInterval(unsigned long interval, bool forceUpdate)
{
    // Safety: ensure invalidation is always enabled (could get stuck disabled
    // if setStatus() is interrupted or exits abnormally)
    if (!lv_disp_is_invalidation_enabled(NULL))
        lv_disp_enable_invalidation(NULL, true);

    schedule([forceUpdate]
             {
                 // When forceUpdate is true (encoder active), bypass adcDataReady check
                 // for immediate UI responsiveness
                 if (!lvglChartIsBusy && !blockAll && (forceUpdate || adcDataReady))
                 {
                     lvglIsBusy = 1;
                     lv_refr_now(NULL);
                     lvglIsBusy = 0;
                     lvglIsBlocked = 0;
                     return;
                 }
                 else
                 {
                     lvglIsBlocked++;
                     return;
                 } },
             interval, lvgl_timer_);

    if (lvglIsBlocked > 0)
    {
        lvglIsBlocked = 0;
        lv_obj_invalidate(lv_scr_act());
    }
}

void LvglUpdatesInterval(unsigned long interval)
{
    LvglUpdatesInterval(interval, false);
}

void FFTUpdateInterval(unsigned long interval)
{
    static unsigned long timer_ = {0};
    schedule([]
             {
                 /*FFT_v*/

                 V.computeFFT(PowerSupply.adc.realADCSpeed /* 1/2.02 */);
                 I.computeFFT(PowerSupply.adc.realADCSpeed /**/);

                 lv_label_set_text_fmt(PowerSupply.Voltage.statLabels.label_fft, "V-FFT:%5.1f Hz", V.peak);
                 lv_label_set_text_fmt(PowerSupply.Current.statLabels.label_fft, "I-FFT:%5.1f Hz", I.peak);

                 //   lv_label_set_text_fmt(label_statMenu_VFFT, "V-FFT_v:%5.1f Hz", V.peak);
                 //   lv_label_set_text_fmt(label_statMenu_IFFT, "I-FFT_i:%5.1f Hz", I.peak);
             },
             interval, timer_);
}

// Power Management Interval (Timer, Energy, Duration, Limits, Auto-save)
void PowerManagementInterval(unsigned long interval)
{
    static unsigned long timer_ = 0;
    schedule(
        []
        {
            // Update energy integration
            integrateEnergy();

            // Save energy to flash periodically (only writes if changed)
            saveEnergyToStorage();

            // Update displays
            updateEnergyAndTimeDisplays();
            updateTimerDisplay();

            // Enforce software limits (OVP/OCP)
            enforceSoftwareLimits();

            // Auto-save check
            autoSaveCheck();
        },
        interval, timer_);
}

// Memory Monitoring Interval
void MemoryMonitorInterval(unsigned long interval)
{
    static unsigned long timer_ = 0;
    schedule(
        []
        {
            MemoryMonitor::update();
        },
        interval, timer_);
}

// Update table and chart after recording completes or incrementally during slow recording
void UpdateRecordingUI()
{
    if (!PowerSupply.recordingMem.needs_ui_update)
        return;

    PowerSupply.recordingMem.needs_ui_update = false;

    // Determine update mode: incremental (slow SPS) or full batch (fast SPS or completed)
    bool slowSampling = (PowerSupply.recordingMem.sample_rate_ms >= 50);
    bool isRecording = PowerSupply.recordingMem.is_recording;

    static uint16_t lastUpdatedIndex = 0;

    if (isRecording && slowSampling)
    {
        // Incremental update: Only update new samples since last update
        for (uint16_t i = lastUpdatedIndex; i < PowerSupply.recordingMem.sample_count; i++)
        {
            if (Utility_objs.table_point_list)
            {
                lv_table_set_cell_value_fmt(Utility_objs.table_point_list, i, 1,
                                           "%+06.4f",
                                           PowerSupply.funGenMem.table_points[i][0]);
            }

            // Update chart
            if (Utility_objs.record_chart && Utility_objs.record_chart_series)
            {
                lv_chart_set_value_by_id(Utility_objs.record_chart, Utility_objs.record_chart_series,
                                        i, (int32_t)(PowerSupply.funGenMem.table_points[i][0] * 100));
            }
        }
        lastUpdatedIndex = PowerSupply.recordingMem.sample_count;

        // Update status
        if (Utility_objs.record_status_label)
        {
            lv_label_set_text_fmt(Utility_objs.record_status_label, "Recording: %d/%d",
                                 PowerSupply.recordingMem.sample_count, RECORDING_TABLE_SIZE);
            lv_obj_set_style_text_color(Utility_objs.record_status_label, lv_palette_main(LV_PALETTE_RED), 0);
        }
    }
    else
    {
        // Full batch update: Update all samples at once (fast SPS or recording completed)
        for (uint16_t i = 0; i < PowerSupply.recordingMem.sample_count; i++)
        {
            if (Utility_objs.table_point_list)
            {
                lv_table_set_cell_value_fmt(Utility_objs.table_point_list, i, 1,
                                           "%+06.4f",
                                           PowerSupply.funGenMem.table_points[i][0]);
            }

            // Update chart
            if (Utility_objs.record_chart && Utility_objs.record_chart_series)
            {
                lv_chart_set_value_by_id(Utility_objs.record_chart, Utility_objs.record_chart_series,
                                        i, (int32_t)(PowerSupply.funGenMem.table_points[i][0] * 100));
            }
        }
        lastUpdatedIndex = 0;  // Reset for next recording

        // Update status label - recording completed
        if (Utility_objs.record_status_label)
        {
            lv_label_set_text_fmt(Utility_objs.record_status_label, "Complete: %d samples",
                                 PowerSupply.recordingMem.sample_count);
            lv_obj_set_style_text_color(Utility_objs.record_status_label, lv_palette_main(LV_PALETTE_GREEN), 0);
        }
    }

    // Refresh chart
    if (Utility_objs.record_chart)
    {
        lv_chart_refresh(Utility_objs.record_chart);
    }
}

// Playback Interval (Recording moved to Task_ADC for faster sampling)
void RecordingPlaybackInterval()
{
    static unsigned long lastSampleTime = 0;

    // Check if recording UI needs update
    UpdateRecordingUI();

    // Playback logic (kept in main loop, doesn't need high speed)
    if (PowerSupply.recordingMem.is_playing)
    {
        if (millis() - lastSampleTime >= PowerSupply.recordingMem.sample_rate_ms)
        {
            lastSampleTime = millis();

            if (PowerSupply.recordingMem.play_index < PowerSupply.recordingMem.sample_count)
            {
                // Set voltage to recorded value
                double voltage = PowerSupply.recordingMem.samples[PowerSupply.recordingMem.play_index];
                PowerSupply.Voltage.SetUpdate(voltage * PowerSupply.Voltage.adjFactor + PowerSupply.Voltage.adjOffset);

                // Turn on output if not already on
                if (PowerSupply.status != DEVICE::ON && PowerSupply.status != DEVICE::VC && PowerSupply.status != DEVICE::CC)
                {
                    PowerSupply.setStatus(DEVICE::ON);
                }

                PowerSupply.recordingMem.play_index++;

                // Update status
                if (Utility_objs.record_status_label)
                {
                    lv_label_set_text_fmt(Utility_objs.record_status_label, "Playing: %d/%d",
                                         PowerSupply.recordingMem.play_index,
                                         PowerSupply.recordingMem.sample_count);
                }
            }
            else
            {
                // Playback complete, stop
                PowerSupply.recordingMem.is_playing = false;
                PowerSupply.recordingMem.play_index = 0;
                if (Utility_objs.record_status_label)
                {
                    lv_label_set_text(Utility_objs.record_status_label, "Playback complete");
                }
                // Turn off output
                PowerSupply.setStatus(DEVICE::OFF);
            }
        }
    }
}

// Refresh histogram chart on page 0 (Core 1 only - LVGL thread-safe)
void HistogramChartRefreshInterval(unsigned long interval)
{
    static unsigned long timer_ = 0;
    schedule([]
             {
                 // Only refresh when on histogram page (page 0)
                 if (Tabs::getCurrentPage() == 0 && !lvglChartIsBusy && !blockAll)
                 {
                     lvglChartIsBusy = true;
                     lv_chart_refresh(PowerSupply.stats.chart);
                     lvglChartIsBusy = false;
                 } },
             interval, timer_);
}

// Refresh graph chart on page 1 (Core 1 only - LVGL thread-safe)
void GraphChartRefreshInterval(unsigned long interval)
{
    static unsigned long timer_ = 0;
    schedule([]
             {
                 // Only refresh when on graph page (page 1)
                 if (Tabs::getCurrentPage() == 1 && !lvglChartIsBusy && !blockAll)
                 {
                     lvglChartIsBusy = true;
                     lv_chart_refresh(PowerSupply.graph.chart);
                     lvglChartIsBusy = false;
                 } },
             interval, timer_);
}

// Custom FPS-only monitor (replaces LVGL's built-in CPU+FPS monitor)
void CustomFPSMonitor()
{
    static lv_obj_t *fps_label = nullptr;
    static unsigned long last_time = 0;
    static unsigned long frame_count = 0;
    static unsigned long elaps_sum = 0;

    // Create label on first call
    if (fps_label == nullptr)
    {
        // Create on layer_top to ensure it's always visible above all content
        fps_label = lv_label_create(lv_layer_top());
        lv_obj_set_style_bg_opa(fps_label, LV_OPA_50, 0);
        lv_obj_set_style_bg_color(fps_label, lv_color_black(), 0);
        lv_obj_set_style_text_color(fps_label, lv_color_white(), 0);
        lv_obj_set_style_pad_all(fps_label, 3, 0);
        lv_obj_align(fps_label, LV_ALIGN_BOTTOM_LEFT, 0, 0);
        lv_label_set_text(fps_label, "-- FPS");
        last_time = millis();
    }

    // Track frame rendering
    frame_count++;
    unsigned long now = millis();
    unsigned long elaps = now - last_time;
    elaps_sum += elaps;
    last_time = now;

    // Update display every ~500ms
    if (elaps_sum >= 500)
    {
        uint32_t fps = (frame_count * 1000) / elaps_sum;
        lv_label_set_text_fmt(fps_label, "%lu FPS", fps);
        frame_count = 0;
        elaps_sum = 0;
    }
}
