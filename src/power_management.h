#pragma once

// Power Management feature functions

// Initialize/reset tracking when power supply turns on/off
void initializePowerManagement();
void resetPowerManagement();

// Update displays
void updateEnergyAndTimeDisplays();
void updateTimerDisplay();

// Runtime checks
void integrateEnergy();
void enforceSoftwareLimits();
void autoSaveCheck();

// Energy persistence (mWh counter)
void loadEnergyFromStorage();
void saveEnergyToStorage();
void resetEnergyCounter();

// Utility
void formatTime(unsigned long seconds, char *buffer, size_t bufSize);
