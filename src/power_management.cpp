// Power Management features implementation
// Features: Timer (#4), Energy Counter (#5), Power-On Duration (#6),
// Output Delay (#7), Auto-save (#13), Voltage/Current Limits

#include "device.hpp"
#include "globals.h"
#include "buzzer.h"
#include "calib_adc.h" // calib_low_noise_active()
#include "waveform_generator.h" // g_funSetPeakV, g_funMeasPeakV
#include "setting_menu.h"       // sirenPlay()
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
            // Any output-on state: while running the status is VC or CC, so
            // checking for ON alone never switched the output off.
            if (PowerSupply.getStatus() != DEVICE::OFF)
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

// Output collapse: the output is on but neither regulation loop holds it - the
// voltage is well under its setpoint and the current well under its limit. In
// CV the voltage should be there, in CC the current; both low means the power
// stage quit, e.g. the SIC461 buck's thermal shutdown after a long 6 A short.
// Runs every 500 ms; 4 bad checks in a row (2 s) turn the output off.
// Nothing measures the input rail, so mains loss looks the same. The cause is
// guessed from the current before the drop: the buck only trips thermally
// after carrying high current for a while (a ~1 min average tracks that).
static char s_faultMsg[200];
static bool s_faultPending = false; // confirmed, siren playing, output not off yet
static bool s_faultSiren = false;   // the siren started at the first bad check

// finished: the first wail ran out (not stopped by Play/Stop), so the alarm
// goes on: 10 more wails after the output is off. Play/Stop ends them.
static void outputFaultShutdown(bool finished)
{
    s_faultPending = false;
    PowerSupply.turn(SWITCH::OFF);
    if (finished && PowerSupply.settingParameters.beeperOnError)
        sirenPlay(10);
    lv_obj_t *mbox = lv_msgbox_create(NULL, "Output Fault", s_faultMsg, NULL, true);
    lv_obj_center(mbox);
}

// Ends of the siren started at the first bad check: shut down only once the
// fault is confirmed (a blip that recovers just stops the siren).
static void faultSirenDone(bool finished)
{
    s_faultSiren = false;
    if (s_faultPending)
        outputFaultShutdown(finished);
}

static void checkOutputCollapse(DEVICE status)
{
    static uint8_t badChecks = 0;
    static double iAvg1min = 0; // A, exponential, tau 60 s
    auto &V = PowerSupply.Voltage;
    auto &I = PowerSupply.Current;

    // FUN sweeps the setpoint: compare the peaks since the last check instead
    double vSet, vMeas;
    if (status == DEVICE::FUN)
    {
        vSet = g_funSetPeakV;
        vMeas = g_funMeasPeakV;
    }
    else
    {
        vSet = (V.adjValue - V.adjOffset) / V.adjFactor;
        vMeas = V.measured.value; // latest sample: the mean lags at high Avg
    }
    g_funSetPeakV = g_funMeasPeakV = -INFINITY;

    if (status == DEVICE::OFF || s_faultPending || PowerSupply.settingParameters.outputFaultProtOff == 1)
    {
        badChecks = 0;
        if (s_faultSiren && !s_faultPending) // output turned off by hand, or protection off
            sirenCancel();
        iAvg1min -= iAvg1min * (0.5 / 60.0); // output off: no current, cools
        return;
    }

    const double iSet = (I.adjValue - I.adjOffset) / I.adjFactor; // A
    double iMeas = I.measured.value;
    if (PowerSupply.mA_Active)
        iMeas /= 1000.0;
    if (badChecks == 0) // keep the pre-collapse average while it's confirmed
        iAvg1min += (std::max(iMeas, 0.0) - iAvg1min) * (0.5 / 60.0);
    // A range switch disturbs both readings: skip 1 s after one
    static bool lastMA = false;
    static unsigned long rangeSwitchAt = 0;
    if (PowerSupply.mA_Active != lastMA)
    {
        lastMA = PowerSupply.mA_Active;
        rangeSwitchAt = millis();
    }
    const bool settling = millis() - rangeSwitchAt < 1000;
    const bool vLow = vSet > 1.0 && vMeas < 0.5 * vSet;
    bool collapsed;
    if (PowerSupply.mA_Active && iSet >= 0.008)
        // the mA range reads only up to ~8 mA, so a limit above that can't be
        // compared: voltage alone, and not in CC, where a low voltage is normal
        collapsed = vLow && status != DEVICE::CC;
    else
        collapsed = vLow && iSet > 0.05 && iMeas < 0.5 * iSet && !I.overRange; // OL: real current unknown
    badChecks = (collapsed && !settling) ? badChecks + 1 : 0;
    // Siren at once, at the first bad check; the shutdown still waits for
    // 4 in a row (2 s), and a recovery before that silences it
    if (badChecks == 1 && PowerSupply.settingParameters.beeperOnError)
    {
        s_faultSiren = true;
        sirenPlay(1, faultSirenDone);
    }
    if (badChecks == 0 && s_faultSiren)
        sirenCancel();
    if (badChecks < 4)
        return;
    badChecks = 0;

    const bool hot = iAvg1min > 2.5;
    snprintf(s_faultMsg, sizeof(s_faultMsg),
             "Output lost: %.3f V of %.3f V set, %.3f A of %.3f A limit.\n"
             "Likely %s (1 min avg %.2f A).\nOutput turned OFF.",
             vMeas, vSet, iMeas, iSet,
             hot ? "thermal shutdown" : "input power lost", iAvg1min);
    Serial.printf("\n[FAULT] %s\n", s_faultMsg);
    // The output goes off when the first wail (~11 s from the first bad check)
    // ends, then 10 more follow (no siren, and off at once, if the error
    // beeper or buzzer is off)
    s_faultPending = true;
    if (!s_faultSiren) // error beeper/buzzer off, or the siren was stopped
        outputFaultShutdown(false);
}

// Check and enforce voltage/current software limits
void enforceSoftwareLimits()
{
    // With OVP on, a voltage setpoint above the OVP level is refused (keys,
    // encoder, SCPI) instead of being accepted and then tripping. Not in FUN,
    // which OVP doesn't cover and where SetUpdate runs at 200-500 Hz.
    // Calibration sweeps to 32 V with nothing connected: neither the ceiling
    // nor OVP/OCP may interfere with it (a refused point would corrupt it).
    const bool calibrating = calib_low_noise_active();
    PowerSupply.Voltage.setCeiling =
        (PowerSupply.settingParameters.ovpEnabled && PowerSupply.getStatus() != DEVICE::FUN && !calibrating)
            ? PowerSupply.settingParameters.voltageLimitMax : INFINITY;
    if (calibrating)
        return;

    // Skip if already triggered (prevents re-triggering loop)
    static unsigned long lastTriggerTime = 0;
    if (millis() - lastTriggerTime < 1000) return;

    // Check if output is active (ON, CC, or VC modes)
    DEVICE status = PowerSupply.getStatus();
    checkOutputCollapse(status);
    if (status != PowerSupply.getStatus())
        return; // it turned the output off
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
                myTone(NOTE_C5, 400); // non-blocking; a delay() here froze input
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
                    myTone(NOTE_C5, 400); // non-blocking; a delay() here froze input
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
