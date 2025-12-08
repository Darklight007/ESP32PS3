// Power Management features implementation
// Features: Timer (#4), Energy Counter (#5), Power-On Duration (#6),
// Output Delay (#7), Auto-save (#13), Voltage/Current Limits

#include "device.hpp"
#include "globals.h"
#include "buzzer.h"
#include <Arduino.h>
#include <Preferences.h>

extern Device PowerSupply;

// Preferences for persistent energy storage
static Preferences energyPrefs;

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
    // Check if output is active (ON, CC, VC, or FUN modes - all deliver power)
    DEVICE status = PowerSupply.getStatus();
    if (status == DEVICE::OFF)
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
    if (PowerSupply.mA_Active) {
        current /= 1000.0;  // Convert mA to A for power calculation
    }
    double power = voltage * current;

    // Integrate: Energy += Power * Time
    PowerSupply.energyAccumulatedWh += power * deltaHours;

    PowerSupply.lastEnergyUpdateTime = currentTime;
}

// Check and enforce voltage/current software limits
void enforceSoftwareLimits()
{
    // Skip if already triggered (prevents re-triggering loop)
    static unsigned long lastTriggerTime = 0;
    if (millis() - lastTriggerTime < 1000) return;

    // Check if output is active (ON, CC, or VC modes)
    DEVICE status = PowerSupply.getStatus();
    bool outputActive = (status != DEVICE::OFF && status != DEVICE::FUN);

    // Check voltage limit (OVP)
    if (PowerSupply.settingParameters.ovpEnabled &&
        PowerSupply.Voltage.measured.value > PowerSupply.settingParameters.voltageLimitMax)
    {
        if (outputActive)
        {
            lastTriggerTime = millis();
            PowerSupply.turn(SWITCH::OFF);
            PowerSupply.settingParameters.ovpTriggered = true;
            if (PowerSupply.powerSwitch.btn)
                lv_obj_clear_state(PowerSupply.powerSwitch.btn, LV_STATE_CHECKED);
            if (PowerSupply.settingParameters.beeperOnError)
            {
                myTone(NOTE_C5, 200);
                delay(100);
                myTone(NOTE_C5, 200);
            }
            Serial.printf("OVP triggered! Voltage %.3fV exceeded limit %.3fV\n",
                         PowerSupply.Voltage.measured.value,
                         PowerSupply.settingParameters.voltageLimitMax);
        }
    }

    // Check current limit (OCP)
    // Note: currentLimitMax is always in Amps, but Current.measured.value
    // is in mA when mA_Active is true, so we need to convert for comparison
    if (PowerSupply.settingParameters.ocpEnabled)
    {
        float measuredCurrent_A = PowerSupply.Current.measured.value;
        if (PowerSupply.mA_Active) {
            measuredCurrent_A /= 1000.0f;  // Convert mA to A
        }

        if (measuredCurrent_A > PowerSupply.settingParameters.currentLimitMax)
        {
            if (outputActive)
            {
                lastTriggerTime = millis();
                PowerSupply.turn(SWITCH::OFF);
                PowerSupply.settingParameters.ocpTriggered = true;
                if (PowerSupply.powerSwitch.btn)
                    lv_obj_clear_state(PowerSupply.powerSwitch.btn, LV_STATE_CHECKED);
                if (PowerSupply.settingParameters.beeperOnError)
                {
                    myTone(NOTE_C5, 200);
                    delay(100);
                    myTone(NOTE_C5, 200);
                }
                Serial.printf("OCP triggered! Current %.3fA exceeded limit %.3fA\n",
                             measuredCurrent_A,
                             PowerSupply.settingParameters.currentLimitMax);
            }
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
    // Don't reset energyAccumulatedWh - keep accumulating until device restart
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
    // Don't reset energyAccumulatedWh - keep accumulating
    PowerSupply.lastEnergyUpdateTime = 0;
    PowerSupply.timerStartTime = 0;
}

// Load energy from persistent storage (call at startup)
void loadEnergyFromStorage()
{
    energyPrefs.begin("energy", true);  // Read-only
    PowerSupply.energyAccumulatedWh = energyPrefs.getDouble("wh", 0.0);
    energyPrefs.end();

    // Validate loaded value
    if (!std::isfinite(PowerSupply.energyAccumulatedWh) || PowerSupply.energyAccumulatedWh < 0) {
        Serial.println("Energy data invalid, resetting to 0");
        PowerSupply.energyAccumulatedWh = 0.0;
    } else {
        Serial.printf("Loaded energy: %.6f Wh\n", PowerSupply.energyAccumulatedWh);
    }
}

// Save energy to persistent storage (call periodically or on significant change)
void saveEnergyToStorage()
{
    static double lastSavedEnergy = -1.0;
    // Only save if changed by more than 0.001 Wh (1mWh) to reduce flash wear
    if (fabs(PowerSupply.energyAccumulatedWh - lastSavedEnergy) > 0.001)
    {
        energyPrefs.begin("energy", false);  // Read-write
        energyPrefs.putDouble("wh", PowerSupply.energyAccumulatedWh);
        energyPrefs.end();
        lastSavedEnergy = PowerSupply.energyAccumulatedWh;
    }
}

// Reset energy counter (call when user holds AVG button)
void resetEnergyCounter()
{
    PowerSupply.energyAccumulatedWh = 0.0;
    energyPrefs.begin("energy", false);
    energyPrefs.putDouble("wh", 0.0);
    energyPrefs.end();
    Serial.println("Energy counter reset");
}
