#include "intervals.h"
#include <Arduino.h>
#include <lvgl.h>
#include "device.hpp"
#include "globals.h"
#include "FFTHandler.h"
#include "power_management.h"

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
