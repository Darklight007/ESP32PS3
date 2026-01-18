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
#include "scpi_parser.h"
#include "error_handler.h"

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

  ESP_LOGI("SETUP", ">>> Starting setup()");
  initializeSerial();
  ESP_LOGI("SETUP", ">>> After initializeSerial()");
  ErrorHandler::init(true, true);  // Enable serial and UI error reporting
  ESP_LOGI("SETUP", ">>> After ErrorHandler::init()");
  initialMemory();
  ESP_LOGI("SETUP", ">>> After initialMemory()");
  initializeI2C();
  ESP_LOGI("SETUP", ">>> After initializeI2C()");
  initializeDisplay();
  ESP_LOGI("SETUP", ">>> After initializeDisplay()");
  initializeTouch();
  ESP_LOGI("SETUP", ">>> After initializeTouch()");
  setupLVGL();
  ESP_LOGI("SETUP", ">>> After setupLVGL()");
  setupDMA();
  ESP_LOGI("SETUP", ">>> After setupDMA()");

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
  scpiParser.init();        // Initialize SCPI command parser

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
  static unsigned long loopCounter = 0;
  if (!oneTimeCommandDone) {
  esp_task_wdt_init(120, false); // X second timeout
  esp_task_wdt_add(NULL);       // Add current task to watchdog
    oneTimeCommandDone = true;
    ESP_LOGI("LOOP", "Watchdog initialized - 120s timeout");
  }

  // Debug: Log every 1000 loops
  if (loopCounter++ % 1000 == 0) {
    ESP_LOGI("LOOP", "Loop %lu - millis: %lu", loopCounter, millis());
  }

  // pixels.setPixelColor(0, pixels.Color(0, 0, 0)); // Red
  // neopixelWrite(RGB_BUILTIN,0,0,0); // Green

  // FUN Only mode: skip ALL main loop processing for cleanest waveforms
  // LVGL timer handler called every 50ms for responsive touch control
  if (lv_obj_has_state(Utility_objs.switch_fun_only, LV_STATE_CHECKED))
  {
    static unsigned long lastTouchCheck = 0;
    // Check touch every 50ms for responsive control
    if (millis() - lastTouchCheck >= 50)
    {
      lv_timer_handler();  // Enable touch for FUN/FUN Only controls
      lastTouchCheck = millis();

      // Re-check state after touch processing - if user disabled FUN Only, exit immediately
      if (!lv_obj_has_state(Utility_objs.switch_fun_only, LV_STATE_CHECKED))
      {
        // FUN Only was just disabled, continue to normal processing
        // Fall through to normal loop below
      }
      else
      {
        return;  // Still in FUN Only mode
      }
    }
    else
    {
      return;  // Skip other processing
    }
  }

  // Adaptive encoder response: fast when active, slower when idle
  // This makes encoder feel more responsive while saving CPU when idle
  bool encoderActive = (millis() - encoderTimeStamp) < 500;  // 500ms idle threshold

  scpiParser.process();          // Process SCPI commands from Serial
  StatusBarUpdateInterval(443);

  // Fast bar refresh on main page when FUN active or high averaging
  bool funActive = lv_obj_has_state(btn_function_gen, LV_STATE_CHECKED);
  bool highAvg = PowerSupply.Voltage.measured.NofAvgs > 32;
  bool onMainPage = Tabs::getCurrentPage() == 2;

  if (onMainPage && (funActive || highAvg))
  {
    lv_timer_handler();  // Direct call for fast bar updates
  }
  else
  {
    LvglUpdatesInterval(0, true);  // Normal update path
  }
  PowerManagementInterval(500);  // Timer, Energy, Auto-save, Limits
  MemoryMonitorInterval(5000);   // Memory monitoring every 5 seconds
  RecordingPlaybackInterval();   // Voltage recording and playback
  Page2RightSideCleanup(1000);   // Clean dirty pixels on right side of page 2

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