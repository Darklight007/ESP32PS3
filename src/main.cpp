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
#include "lv_gui_helper.h"
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
#include "device.hpp"
#include "input_device.h"
#include <Keypad_MC17.h>
#include "FFTHandler.h"
#include "setting_menu.h"
// #include "setting_menu2.h"
#include "SetupHandlers.h"
#include "power_management.h"
#include "memory_monitor.h"

#include "esp_heap_caps.h"
#include "esp_log.h"

#include "esp_int_wdt.h"
#include "esp_system.h"
// #include <Adafruit_NeoPixel.h>
// #define LED_PIN 48
// Adafruit_NeoPixel pixels(1, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup()
{
  esp_task_wdt_init(120, false); // X second timeout
  esp_task_wdt_add(NULL);       // Add current task to watchdog

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
  loadEnergyFromStorage();  // Load persistent energy counter
  MemoryMonitor::init();    // Initialize memory monitoring

  Serial.printf("\nSetup() run on core: #%i \n\n", xPortGetCoreID());

  myTone(NOTE_A5, 200, true);
  myTone(NOTE_A3, 200, true);

  // pixels.begin();
  // pixels.setPixelColor(0, pixels.Color(0, 0, 0));
  // pixels.show();
  // ADC_INL_Voltage_calibration_cb(nullptr);
}

bool oneTimeCommandDone = false;

void loop() {
  if (!oneTimeCommandDone) {
  esp_task_wdt_init(120, false); // X second timeout
  esp_task_wdt_add(NULL);       // Add current task to watchdog
    oneTimeCommandDone = true;
  }
  // pixels.setPixelColor(0, pixels.Color(0, 0, 0)); // Red
  // neopixelWrite(RGB_BUILTIN,0,0,0); // Green

  // Adaptive encoder response: fast when active, slower when idle
  // This makes encoder feel more responsive while saving CPU when idle
  bool encoderActive = (millis() - encoderTimeStamp) < 500;  // 500ms idle threshold

  StatusBarUpdateInterval(443);
  LvglUpdatesInterval(0, encoderActive);  // Force update when encoder active for immediate response
  PowerManagementInterval(500);  // Timer, Energy, Auto-save, Limits
  MemoryMonitorInterval(5000);   // Memory monitoring every 5 seconds

  // trackLoopExecution(__func__);

  // Flush measures - faster when encoder active for immediate visual feedback
  if (encoderActive)
    FlushMeasuresInterval(10);  // Very fast update during encoder activity
  else
    FlushMeasuresInterval(50);  // Fast update even when idle for responsive display

  // Adaptive statistics update: faster when encoder active for responsive display
  if (encoderActive)
    statisticUpdateInterval(100);  // Fast update during encoder activity
  else
    statisticUpdateInterval(333);  // Normal update when idle

  // FFTUpdateInterval(1000);
  EncoderRestartInterval(1000); //--> some bugs?
  managePageEncoderInteraction();

  // if (lv_obj_has_state(btn_function_gen, LV_STATE_CHECKED))
  //  PowerSupply.toggle();

  //   KeyCheckInterval(400);
  // DACInterval(100);

  // Adaptive VCCC update: faster when encoder active
  if (encoderActive)
    VCCCInterval(10);   // Fast update during encoder activity
  else
    VCCCInterval(33);   // Normal update when idle
  // KeyCheckInterval(45);
  // Serial.printf("\nVoltage.encoder.getCount %l",PowerSupply.Voltage.encoder.getCount());
  // KeyCheckInterval(0); // moved to adc taskse
  // delay(10);

  //  Serial.printf("\nADC_loopCounter %l",PowerSupply.adc.ADC_loopCounter);
  //  Serial.printf("\n Current utiltap%i", lv_tabview_get_tab_act(tabview_utility));
}

/*
Noise analysis
80Mhz page 2: ER:18.09
80Mhz page 4: ER:19.64

80Mhz page 2: ER:19.09 add 3x10uf capacitors
80Mhz page 4: ER:19.78

80Mhz page 2: No LCD ER:18.29
80Mhz page 4: No LCD ER:19.30

80Mhz page 2: No LCD ER:17.58 if spi work during ADC conversion
80Mhz page 2: No LCD ER:18.29 if limit spi work during ADC conversion

10Mhz page 2: ER:18.60
 1Mhz page 2: ER:19.62
*/