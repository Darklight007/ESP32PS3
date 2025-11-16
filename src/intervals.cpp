#include "intervals.h"
#include <Arduino.h>
#include "device.hpp"
#include "globals.h"

// External references
extern Device PowerSupply;
extern bool lvglChartIsBusy;
extern bool blockAll;

// Forward declarations for functions still in globalFunctions.h
void StatusBar();

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
                 PowerSupply.Current.encoder.setCount(temp);
             },
             interval, timer_);
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
