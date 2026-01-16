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
extern portMUX_TYPE lvgl_spinlock;  // Shared spinlock for bar updates
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
                // Only save settings if values actually changed
                static uint16_t lastVoltage = 0;
                static uint16_t lastCurrent = 0;
                if (PowerSupply.Voltage.adjValue != lastVoltage ||
                    PowerSupply.Current.adjValue != lastCurrent) {
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
             {
                if (!lvglChartIsBusy && !blockAll)
                     PowerSupply.VCCCStatusUpdate(); },
             interval, timer_);
}

// Internal state for LvglUpdatesInterval - shared between overloads
static unsigned long lvgl_timer_ = {0};
static unsigned int lvglIsBlocked = {0};

void LvglUpdatesInterval(unsigned long interval, bool forceUpdate)
{
    schedule([forceUpdate]
             {
                 // When forceUpdate is true (encoder active), bypass adcDataReady check
                 // for immediate UI responsiveness
                 if (!lvglChartIsBusy && !blockAll && (forceUpdate || adcDataReady))
                 {
                     lvglIsBusy = 1;
                     portENTER_CRITICAL(&lvgl_spinlock);
                     lv_timer_handler();
                     portEXIT_CRITICAL(&lvgl_spinlock);
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
