#include "tasks.h"
#include "device.hpp"
#include "tabs.h"
#include "functions.h"
#include "globals.h"
#include "buzzer.h"

// External variables (most are already in device.hpp or other headers)
extern Device PowerSupply;
extern bool lvglIsBusy;
extern bool lvglChartIsBusy;
extern bool blockAll;
extern bool wireConnected;
extern lv_obj_t *btn_function_gen;

// Forward declarations for functions from globalFunctions.h
void schedule(std::function<void(void)> func, unsigned long &&interval, unsigned long &startTime);
void schedule(std::function<void(void)> func, unsigned long &interval, unsigned long &startTime);
void HistPush();
void GraphPush();
void KeyCheckInterval(unsigned long interval);
void getSettingEncoder(lv_indev_drv_t *drv, lv_indev_data_t *data);
bool functionGenerator();

void Task_BarGraph(void *pvParameters)
{
    for (;;)
    {
        if (Tabs::getCurrentPage() != 2)
        {
            vTaskDelay(10);  // Longer delay when not on main page
            continue;
        }

        if (!lvglIsBusy && !blockAll)
        {
            PowerSupply.Voltage.barUpdate();
            PowerSupply.Current.barUpdate();
        }

        vTaskDelay(1);  // Yield to other tasks, prevents CPU hogging
    }
}

double t = 0;
void Task_ADC(void *pvParameters)
{
    // ADC task using interrupt-driven approach with task notifications
    // This is more efficient than polling - the task sleeps until the ISR wakes it up

    static unsigned long timer_ = {0};
    static unsigned long lastActivityTime = 0;

    for (;;)
    {
        toneOff();

        // Handle DAC updates based on function generator state
        if (lv_obj_has_state(btn_function_gen, LV_STATE_CHECKED))
        {
            schedule([&]
                     {
                            functionGenerator();
                            PowerSupply.DACUpdate(); },
                     5, timer_);
        }
        else
        {
            schedule([]
                     { PowerSupply.DACUpdate(); },
                     100, timer_);
        }

        // Handle keyboard and encoder input when ADC is idle
        // This happens while waiting for the next ADC interrupt
        if (!adcDataReady)
        {
            if (wireConnected)
            {
                if (!lv_obj_has_flag(PowerSupply.gui.textarea_set_value, LV_OBJ_FLAG_HIDDEN))
                    KeyCheckInterval(10);
                else if (lv_obj_has_state(Utility_objs.switch_keys_scan, LV_STATE_CHECKED))
                    KeyCheckInterval(105);
            }
            getSettingEncoder(NULL, NULL);

            // Wait for ADC interrupt notification with 100ms timeout
            // This blocks the task efficiently instead of busy-waiting
            // Timeout ensures we still process inputs even if no ADC data arrives
            ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(100));
            continue;
        }

        // ADC data is ready - process it
        lastActivityTime = millis();

        if (!lvglIsBusy || PowerSupply.settingParameters.adcRate != 0)
        {
            PowerSupply.readVoltage();
            PowerSupply.readCurrent();
        }

        PowerSupply.Power.measureUpdate(PowerSupply.Current.measured.Mean() * (PowerSupply.mA_Active ? 0.001 : 1.0) * PowerSupply.Voltage.measured.Mean());
        // if (Tabs::getCurrentPage() != 2)
        // moved to measured of Display object
        // {
        // PowerSupply.Voltage.StatisticsUpdate(PowerSupply.Voltage.measured.value);
        // PowerSupply.Current.StatisticsUpdate(PowerSupply.Current.measured.value);
        // }

        static bool lastCCCVStatus = false;
        if (lastCCCVStatus != digitalRead(PowerSupply.CCCVPin))
        {
            lastCCCVStatus = digitalRead(PowerSupply.CCCVPin);
            // myTone(NOTE_A4, 50);
        }
        // Serial.printf("\n%i", digitalRead(PowerSupply.CCCVPin));
        HistPush();
        GraphPush();

        if (Tabs::getCurrentPage() == 0 && !lvglIsBusy)

            if (!lvglChartIsBusy && !blockAll)
            {
                lvglChartIsBusy = true;
                lv_chart_refresh(PowerSupply.stats.chart);
                lvglChartIsBusy = false;
            }

        static unsigned long NoAvgTime;

        if (Tabs::getCurrentPage() == 1 && !lvglIsBusy)
            schedule([]
                     {
                         if (!lvglChartIsBusy && !blockAll)
                         {
                             lvglChartIsBusy = true;
                             lv_chart_refresh(PowerSupply.graph.chart);
                             lvglChartIsBusy = false;
                         } },
                     //  PowerSupply.Voltage.measured.NofAvgs * 100000 / /* pixels per point*/ ((double)PowerSupply.adc.realADCSpeed), NoAvgTime);
                     125, NoAvgTime);

        // if (Tabs::getCurrentPage() == 1)
        // {
        //     if (V.sampleReady)
        //     {
        //         V.push(PowerSupply.Voltage.measured.value);
        //         V.sampleReady = false;
        //     }
        //
        //     if (I.sampleReady)
        //     {
        //         I.push(PowerSupply.Current.measured.value);
        //         I.sampleReady = false;
        //     }
        // }
    }
}
