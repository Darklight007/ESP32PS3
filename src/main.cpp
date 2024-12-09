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

#include "esp_task_wdt.h"

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

#include "esp_heap_caps.h"
#include "esp_log.h"

#include "esp_int_wdt.h"
#include "esp_system.h"

void setup()
{

  // Disable Task Watchdog Timer
  // esp_task_wdt_deinit(); // Deinitializes the task watchdog timer

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
  setupPreferences();
  setupADC();
  setupDAC();
  // SetupOVP();  // Create OVP/OCP protection
  setupCalibPage();
  createTasks();
  initialMemory();
  Serial.printf("\nSetup() run on core: #%i \n\n", xPortGetCoreID());
}

void loop()
{

  LvglUpdatesInterval(33);
  trackLoopExecution(__func__);
  StatusBarUpdateInterval(333);

  if ((millis() - encoderTimeStamp) > 33)
    FlushMeasuresInterval(75 + 60 * PowerSupply.Voltage.measured.NofAvgs); // PowerSupply.Voltage.measured.NofAvgs

  statisticUpdateInterval(333);
  // FFTUpdateInterval(1000);
  EncoderRestartInterval(1000); //--> some bugs?
  managePageEncoderInteraction();

     
  // DACInterval(100);
  // VCCCInterval(250);
  // KeyCheckInterval(45);
  // Serial.printf("\nVoltage.encoder.getCount %l",PowerSupply.Voltage.encoder.getCount());
  // KeyCheckInterval(0); // moved to adc taskse
  // delay(10);
  
  


  //  Serial.printf("\nADC_loopCounter %l",PowerSupply.adc.ADC_loopCounter);
}
