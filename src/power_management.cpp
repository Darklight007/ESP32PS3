// Power Management features implementation
// Features: Timer (#4), Energy Counter (#5), Power-On Duration (#6),
// Output Delay (#7), Auto-save (#13), Voltage/Current Limits

#include "device.hpp"
#include "globals.h"
#include "buzzer.h"
#include <Arduino.h>

extern Device PowerSupply;

// Format time as HH:MM:SS
void formatTime(unsigned long seconds, char *buffer, size_t bufSize)
{
    unsigned long hours = seconds / 3600;
    unsigned long minutes = (seconds % 3600) / 60;
    unsigned long secs = seconds % 60;
    snprintf(buffer, bufSize, "%02lu:%02lu:%02lu", hours, minutes, secs);
}

// Update Energy Counter and Power-On Duration displays
void updateEnergyAndTimeDisplays()
{
    if (!PowerSupply.gui.label_energy_counter || !PowerSupply.gui.label_power_on_time)
        return;

    // Update energy counter
    if (PowerSupply.energyAccumulatedWh < 1.0)
    {
        // Display in mWh for values < 1Wh
        lv_label_set_text_fmt(PowerSupply.gui.label_energy_counter, "%.1fmWh",
                             PowerSupply.energyAccumulatedWh * 1000.0);
    }
    else
    {
        // Display in Wh
        lv_label_set_text_fmt(PowerSupply.gui.label_energy_counter, "%.3fWh",
                             PowerSupply.energyAccumulatedWh);
    }

    // Update power-on duration
    if (PowerSupply.powerOnStartTime > 0)
    {
        unsigned long elapsedSeconds = (millis() - PowerSupply.powerOnStartTime) / 1000;
        char timeStr[16];
        formatTime(elapsedSeconds, timeStr, sizeof(timeStr));
        lv_label_set_text(PowerSupply.gui.label_power_on_time, timeStr);
    }
    else
    {
        lv_label_set_text(PowerSupply.gui.label_power_on_time, "00:00:00");
    }
}

// Update timer countdown display
void updateTimerDisplay()
{
    if (!PowerSupply.gui.label_timer_remaining)
        return;

    if (PowerSupply.settingParameters.timerEnabled && PowerSupply.timerStartTime > 0)
    {
        unsigned long elapsedSeconds = (millis() - PowerSupply.timerStartTime) / 1000;
        unsigned long totalSeconds = PowerSupply.settingParameters.timerDurationSeconds;

        if (elapsedSeconds < totalSeconds)
        {
            unsigned long remainingSeconds = totalSeconds - elapsedSeconds;
            char timeStr[16];
            formatTime(remainingSeconds, timeStr, sizeof(timeStr));
            lv_label_set_text_fmt(PowerSupply.gui.label_timer_remaining, "T:%s", timeStr);
            lv_obj_clear_flag(PowerSupply.gui.label_timer_remaining, LV_OBJ_FLAG_HIDDEN);
        }
        else
        {
            // Timer expired - shutdown output
            lv_obj_add_flag(PowerSupply.gui.label_timer_remaining, LV_OBJ_FLAG_HIDDEN);
            if (PowerSupply.getStatus() == DEVICE::ON)
            {
                PowerSupply.turn(SWITCH::OFF);
                lv_obj_clear_state(PowerSupply.powerSwitch.btn, LV_STATE_CHECKED);
                if (PowerSupply.settingParameters.beeperOnPowerChange)
                {
                    myTone(NOTE_A3, 500);  // Long beep for timer expiration
                }
                Serial.println("Timer expired - output turned OFF");
            }
            PowerSupply.timerStartTime = 0;  // Reset timer
        }
    }
    else
    {
        lv_obj_add_flag(PowerSupply.gui.label_timer_remaining, LV_OBJ_FLAG_HIDDEN);
    }
}

// Integrate energy (Power * Time)
void integrateEnergy()
{
    if (PowerSupply.getStatus() != DEVICE::ON)
    {
        PowerSupply.lastEnergyUpdateTime = 0;
        return;
    }

    unsigned long currentTime = millis();
    if (PowerSupply.lastEnergyUpdateTime == 0)
    {
        PowerSupply.lastEnergyUpdateTime = currentTime;
        return;
    }

    // Calculate time delta in hours
    unsigned long deltaMs = currentTime - PowerSupply.lastEnergyUpdateTime;
    double deltaHours = deltaMs / 3600000.0;  // ms to hours

    // Get current power in watts
    double voltage = PowerSupply.Voltage.measured.value;
    double current = PowerSupply.Current.measured.value;
    double power = voltage * current;

    // Integrate: Energy += Power * Time
    PowerSupply.energyAccumulatedWh += power * deltaHours;

    PowerSupply.lastEnergyUpdateTime = currentTime;
}

// Check and enforce voltage/current software limits
void enforceSoftwareLimits()
{
    // Check voltage limit
    if (PowerSupply.Voltage.measured.value > PowerSupply.settingParameters.voltageLimitMax)
    {
        if (PowerSupply.getStatus() == DEVICE::ON)
        {
            PowerSupply.turn(SWITCH::OFF);
            lv_obj_clear_state(PowerSupply.powerSwitch.btn, LV_STATE_CHECKED);
            if (PowerSupply.settingParameters.beeperOnError)
            {
                myTone(NOTE_C8, 200);
                delay(100);
                myTone(NOTE_C8, 200);
            }
            Serial.printf("OVP triggered! Voltage %.3fV exceeded limit %.3fV\n",
                         PowerSupply.Voltage.measured.value,
                         PowerSupply.settingParameters.voltageLimitMax);
        }
    }

    // Check current limit
    if (PowerSupply.Current.measured.value > PowerSupply.settingParameters.currentLimitMax)
    {
        if (PowerSupply.getStatus() == DEVICE::ON)
        {
            PowerSupply.turn(SWITCH::OFF);
            lv_obj_clear_state(PowerSupply.powerSwitch.btn, LV_STATE_CHECKED);
            if (PowerSupply.settingParameters.beeperOnError)
            {
                myTone(NOTE_C8, 200);
                delay(100);
                myTone(NOTE_C8, 200);
            }
            Serial.printf("OCP triggered! Current %.3fA exceeded limit %.3fA\n",
                         PowerSupply.Current.measured.value,
                         PowerSupply.settingParameters.currentLimitMax);
        }
    }
}

// Auto-save functionality
void autoSaveCheck()
{
    if (PowerSupply.settingParameters.autoSaveIntervalMinutes == 0)
        return;  // Auto-save disabled

    unsigned long currentTime = millis();
    unsigned long intervalMs = PowerSupply.settingParameters.autoSaveIntervalMinutes * 60000UL;

    if (PowerSupply.lastAutoSaveTime == 0)
    {
        PowerSupply.lastAutoSaveTime = currentTime;
        return;
    }

    if ((currentTime - PowerSupply.lastAutoSaveTime) >= intervalMs)
    {
        PowerSupply.SaveSetting();
        PowerSupply.lastAutoSaveTime = currentTime;
        Serial.printf("Auto-save triggered (interval: %d minutes)\n",
                     PowerSupply.settingParameters.autoSaveIntervalMinutes);
    }
}

// Initialize power management when output turns ON
void initializePowerManagement()
{
    PowerSupply.powerOnStartTime = millis();
    PowerSupply.energyAccumulatedWh = 0.0;
    PowerSupply.lastEnergyUpdateTime = millis();

    if (PowerSupply.settingParameters.timerEnabled)
    {
        PowerSupply.timerStartTime = millis();
        Serial.printf("Timer started: %lu seconds\n", PowerSupply.settingParameters.timerDurationSeconds);
    }
    else
    {
        PowerSupply.timerStartTime = 0;
    }
}

// Reset power management when output turns OFF
void resetPowerManagement()
{
    PowerSupply.powerOnStartTime = 0;
    PowerSupply.energyAccumulatedWh = 0.0;
    PowerSupply.lastEnergyUpdateTime = 0;
    PowerSupply.timerStartTime = 0;
}
