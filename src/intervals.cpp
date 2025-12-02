#include "intervals.h"
#include <Arduino.h>
#include <lvgl.h>
#include "device.hpp"
#include "globals.h"
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
                     lv_timer_handler();
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

// Recording and Playback Interval
void RecordingPlaybackInterval()
{
    static unsigned long timer_ = 0;
    static unsigned long lastSampleTime = 0;

    // Recording logic
    if (PowerSupply.recordingMem.is_recording)
    {
        if (millis() - lastSampleTime >= PowerSupply.recordingMem.sample_rate_ms)
        {
            lastSampleTime = millis();

            // Record current voltage reading into table
            if (PowerSupply.recordingMem.sample_count < 250)
            {
                double voltage = PowerSupply.Voltage.measured.Mean();

                // Store voltage reading in table_points array
                PowerSupply.funGenMem.table_points[PowerSupply.recordingMem.sample_count][0] = voltage;

                // Update table display
                lv_table_set_cell_value_fmt(Utility_objs.table_point_list,
                                           PowerSupply.recordingMem.sample_count, 1,
                                           "%06.4f",
                                           voltage);

                // Update chart
                if (Utility_objs.record_chart && Utility_objs.record_chart_series)
                {
                    lv_chart_set_value_by_id(Utility_objs.record_chart, Utility_objs.record_chart_series,
                                            PowerSupply.recordingMem.sample_count,
                                            (int32_t)(voltage * 100));
                    lv_chart_refresh(Utility_objs.record_chart);
                }

                PowerSupply.recordingMem.sample_count++;

                // Update status with progress every 10 samples
                if (Utility_objs.record_status_label && (PowerSupply.recordingMem.sample_count % 10 == 0))
                {
                    lv_label_set_text_fmt(Utility_objs.record_status_label, "Recording: %d/%d",
                                         PowerSupply.recordingMem.sample_count - PowerSupply.recordingMem.play_index,
                                         250);
                }
            }
            else
            {
                // Max samples reached (table full), stop recording
                PowerSupply.recordingMem.is_recording = false;
                if (Utility_objs.record_status_label)
                {
                    lv_label_set_text_fmt(Utility_objs.record_status_label, "Complete: %d samples",
                                         PowerSupply.recordingMem.sample_count - PowerSupply.recordingMem.play_index);
                }
            }
        }
    }

    // Playback logic
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
