// Standard libraries
#include <map>
#include <mutex>
#include <complex>
#include <limits.h>
#include <float.h>
#include <time.h>

// FreeRTOS includes
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// EEPROM and Device configurations
#include <Preferences.h>

// Hardware specific
#include <HardwareSerial.h>

// TFT & LVGL includes
#include <lvgl.h>
#include "demos/lv_demos.h"
// #include <../lib/TFT_eSPI/Processors/TFT_eSPI_ESP32_S3.h>
#include <TFT_eSPI.h>

// Custom components
#include "myFonts.h"
#include "OTA.h"
#include "buzzer.h"
#include "config.hpp"
#include "tabs.h"
#include "DispObject.h"
#include "globalFunctions.h"
#include "device.h"
#include "input_device.h"
#include <Keypad_MC17.h>
#include "FFTHandler.h"
#include "setting_menu.h"
#include "SetupHandlers.h"

void setup()
{

  initializeSerial();
  initialMemory();
  initializeI2C();
  initializeDisplay();
  initializeTouch();
  setupLVGL();
  setupDMA();

  // lv_demo_music();
  // lv_demo_benchmark();
  // lv_demo_stress();
  // return;

  /**************************************************************************/
  setupPowerSupply();
  setupADC();
  setupDAC();
  // SetupOVP();  // Create OVP/OCP protection

  createTasks();
  seupCalibPage();

  Serial.print("\nSetup() run on core: ");
  Serial.print(xPortGetCoreID());
}

void loop()
{

  LvglUpdatesInterval(30);
  trackLoopExecution();
  StatusBarUupdateInterval(250);
  FlushMeasuresInterval(100 * PowerSupply.Voltage.measured.NofAvgs);
  statisticUpdateInterval(100);
  FFTUpdateInterval(1000);
  EncoderUpdateInterval(2000);
  managePageInteraction();
}
