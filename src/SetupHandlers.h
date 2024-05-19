// SetupHandlers.h
#ifndef SETUP_HANDLERS_H
#define SETUP_HANDLERS_H

#include <Arduino.h>
#include <Wire.h>
#include "TFT_eSPI.h"
#include "lvgl.h"
// #include "config.hpp" // Assuming this contains configuration like pin numbers and settings
#include "Preferences.h"

// #include "config.hpp"
// #include "DispObject.h"
#include "device.h"

// #include "input_device.h"
#include <Keypad_MC17.h> 
// #include "buzzer.h"
// #include "setting_menu.h"
// #include "globalFunctions.h"

// Declarations of setup and utility functions
void initializeSerial();
void initialMemory();
void initializeI2C();
void initializeDisplay();
void setupPowerSupply();
void initializeTouch();
void setupTasks();
void setupPreferences();
void setupADC();
void setupDAC();
void createTasks();

// Add other necessary function declarations here

#endif // SETUP_HANDLERS_H
