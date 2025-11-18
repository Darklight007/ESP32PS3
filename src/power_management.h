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

// Utility
void formatTime(unsigned long seconds, char *buffer, size_t bufSize);
