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
                PowerSupply.settingParameters.SetVoltage = PowerSupply.Voltage.adjValue;
                PowerSupply.settingParameters.SetCurrent = PowerSupply.Current.adjValue;
                PowerSupply.SaveSetting();

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

void LvglUpdatesInterval(unsigned long interval)
{
    static unsigned long timer_ = {0};  // Interval in milliseconds
    static unsigned long timer_2 = {0}; // Interval in milliseconds
    static unsigned int lvglIsBlocked = {0};
    // if (lvglChartIsBusy)
    // {
    //     vTaskDelay(1);
    //     // return;
    // }
    schedule([]
             {
                 if (!lvglChartIsBusy && !blockAll && adcDataReady) //&& adcDataReady
                                                                    //  when adcDataReady is set, it means the data is ready and conversion has stoped.
                                                                   /// Best time to run SPI to not generate noise on ADC
                 {
                     // PowerSupply.adc.ads1219->pause();
                     lvglIsBusy = 1;
                     lv_timer_handler();
                     vTaskDelay(pdMS_TO_TICKS(1)); // ~5ms
                     lvglIsBusy = 0;
                     lvglIsBlocked = 0;
                     //  PowerSupply.adc.ads1219->begin();
                      return;
                 }
                else
                  {
                    lvglIsBlocked++;
                    return;
                 } },

             interval, timer_);

    // schedule([]
    //          { lv_obj_invalidate(lv_scr_act()); }, 60000, timer_2);
    if (lvglIsBlocked > 0)
    {
        lvglIsBlocked = 0;
        lv_obj_invalidate(lv_scr_act());
    }
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
