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
            vTaskDelay(1);
            continue;
        }

        if (!lvglIsBusy && !blockAll)
        {

            // lv_obj_invalidate(PowerSupply.Voltage.Bar.bar);
            // lv_obj_invalidate(PowerSupply.Current.Bar.bar);
            PowerSupply.Voltage.barUpdate();
            PowerSupply.Current.barUpdate();
        }

        // trackLoopExecution(__func__);
    }
}

double t = 0;
void Task_ADC(void *pvParameters)
{

    // ************************ Temperature DS18B20 Sensor ********************************************************
    for (;;)
    {

        // if (lvglIsBusy)
        // {

        //  toneOff();
        // vTaskDelay(1);

        // }
        // Serial.printf("\nMeasured adcDataReady :%i Channel:%i", adcDataReady, PowerSupply.adc.busyChannel);

        toneOff();
        static unsigned long timer_ = {0};
        static bool trigger = false;
        if (lv_obj_has_state(btn_function_gen, LV_STATE_CHECKED))

            schedule([&]
                     {
                            functionGenerator();
                            PowerSupply.DACUpdate(); },
                     5, timer_);

        else
            schedule([]
                     { PowerSupply.DACUpdate(); },
                     100, timer_);

        if (!adcDataReady)
        {
            if (wireConnected)
            {
                // isPowerSupply.gui.textarea_set_valueHidden = lv_obj_has_flag(PowerSupply.gui.textarea_set_value, LV_OBJ_FLAG_HIDDEN);
                if (!lv_obj_has_flag(PowerSupply.gui.textarea_set_value, LV_OBJ_FLAG_HIDDEN))
                    KeyCheckInterval(10);

                else if (!lv_obj_has_state(Utility_objs.switch_keys_scan, LV_STATE_CHECKED))
                {
                    // if (trigger)
                    // {
                    //     KeyCheckInterval(105);
                    //     Serial.printf("\n Triggered *******************!");
                    // }
                    // else
                    // KeyCheckInterval(105);
                }
                else
                    KeyCheckInterval(105);
            }
            getSettingEncoder(NULL, NULL);

            // DACInterval(49);

            // trackLoopExecution(__func__);

            // if (Tabs::getCurrentPage() == 2)
            //     vTaskDelay(1);
            // else if (Tabs::getCurrentPage() == 1)
            //     vTaskDelay(5);
            //  vTaskDelay(1);
            continue;
        }
        // if (xSemaphoreTake(timerSemaphore, portMAX_DELAY) != pdTRUE)
        // {
        //     vTaskDelay(1);
        //     continue;
        // }

        // New ADC sample is ready
        // if (Tabs::getCurrentPage() == 1)
        // {
        //     if (adcDataReady && PowerSupply.adc.busyChannel == VOLTAGE)
        //         V.shift(); // Shift for new sample
        //
        //     if (adcDataReady && PowerSupply.adc.busyChannel == CURRENT)
        //         I.shift(); // Shift for new sample
        // }
        // ulTaskNotifyTake(pdTRUE, portMAX_DELAY);   // wait until ready
        if (!lvglIsBusy || PowerSupply.settingParameters.adcRate != 0) // avid conversion when spi is working!
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
