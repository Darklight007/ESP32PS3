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
#include <SPIFFS.h>

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
#include "freeze_trace.h"
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
#include "input_handler.h"
#include "battery_charger.h"
#include "ui_helpers.h"

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
  esp_task_wdt_add(NULL);        // Add current task to watchdog

  // Disable Task Watchdog Timer
  // esp_task_wdt_deinit(); // Deinitializes the task watchdog timer

  ESP_LOGI("SETUP", ">>> Starting setup()");
  initializeSerial();
  ESP_LOGI("SETUP", ">>> After initializeSerial()");
  ErrorHandler::init(true, true); // Enable serial and UI error reporting
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
  // Mount SPIFFS ONCE for the whole session. Previously every save/load
  // (graph trace, FunGen tables) called SPIFFS.begin()/end() around itself -
  // mount/unmount is the genuinely expensive part of a SPIFFS operation
  // (confirmed live: one graph-trace autosave took 638ms, causing a visible
  // freeze across every UI element plus a notch in the graph itself), not
  // the actual file read/write. Never unmounting also removes the mount-race
  // between independent auto-save timers (AUDIT_CRASH_WATCHDOG.md Issue 2).
  if (!SPIFFS.begin(true))
    Serial.println("SPIFFS Mount Failed at boot");

  setupPowerSupply();
  LoadGraphData(); // restore the graph trace before Task_ADC starts pushing new points
  setupPreferences();
  setupADC();
  setupDAC();
  setupBuzzer();
  // SetupOVP();  // Create OVP/OCP protection

  setupCalibPage();
  createTasks();
  initialMemory();
  loadEnergyFromStorage(); // Load persistent energy counter
  MemoryMonitor::init();   // Initialize memory monitoring
  scpiParser.init();       // Initialize SCPI command parser

  Serial.printf("\nSetup() run on core: #%i \n\n", xPortGetCoreID());

  myTone(NOTE_A5, 200, true);
  myTone(NOTE_A3, 200, true);

  // pixels.begin();
  // pixels.setPixelColor(0, pixels.Color(0, 0, 0));
  // pixels.show();
  // ADC_INL_Voltage_calibration_cb(nullptr);
}

bool oneTimeCommandDone = false;

void loop()
{
  // static unsigned long loopCounter = 0;
  if (!oneTimeCommandDone)
  {
    esp_task_wdt_init(120, false); // X second timeout
    esp_task_wdt_add(NULL);        // Add current task to watchdog
    oneTimeCommandDone = true;
    // ESP_LOGI("LOOP", "Watchdog initialized - 120s timeout");
  }

  // Feed the watchdog every loop - was never fed, causing a false trip (with
  // a large Serial-printed backtrace dump) every ~120s of uptime, confirmed
  // live earlier this session. AUDIT_CRASH_WATCHDOG.md Issue 1.
  esp_task_wdt_reset();

  // // Debug: Log every 1000 loops
  // if (loopCounter++ % 1000 == 0)
  // {
  //   // ESP_LOGI("LOOP", "Loop %lu - millis: %lu", loopCounter, millis());
  // }

  // pixels.setPixelColor(0, pixels.Color(0, 0, 0)); // Red
  // neopixelWrite(RGB_BUILTIN,0,0,0); // Green

  // Bar graph updates - run every loop iteration for maximum speed
  // // Force LVGL render after bar updates for immediate visual feedback
  // if (Tabs::getCurrentPage() == 2 && !blockAll)
  // {
  //   PowerSupply.Voltage.barUpdate();
  //   PowerSupply.Current.barUpdate();

  //   // Render bars immediately (throttled to ~60 FPS to avoid DMA issues)
  TRACE("loop_top");
  static unsigned long lastBarRender = 0;
  if (millis() - lastBarRender >= 100)
  {
    lvglIsBusy = true;
    TRACE("loop_pre_lvgl");
    lv_timer_handler();
    TRACE("loop_post_lvgl");
    lvglIsBusy = false;
    lastBarRender = millis();
  }
  // }

  freeze_trace_periodic_print();

  // FUN Only mode: skip most processing for cleanest waveforms
  if (lv_obj_has_state(Utility_objs.switch_fun_only, LV_STATE_CHECKED))
  {
    TRACE("loop_fun_return");
    return; // Skip other processing in FUN Only mode (bars already rendered above)
  }

  TRACE("loop_pre_drain");
  // Drain order matters:
  //   1. Page change → does lv_tabview_set_act on Core 1; may set tab-event-pending.
  //   2. View-mode change → runs apply*ViewMode on Core 1 (heavy LVGL work).
  //   3. Tab event → fires LV_EVENT_VALUE_CHANGED → updateObjectPos_cb.
  //   4. Power toggle ('O' key) → Switch::turn → btn_event_cb → setStatus (heavy LVGL).
  drainPendingPageChange();
  drainPendingViewModeChange();
  drainPendingTabEvent();
  drainPendingPowerToggle();
  drainPendingKeyEvents();      // Home, numeric entry, V/v/A/a readback, Z-key avg slider,
                                // memory load/save, chart mode/graph pause toggles, rotary
                                // step buttons, V/A series toggle - was defined but never
                                // called, so all of the above silently did nothing.
  drainBattChemDropdownClose(); // '>' key on Utility page: close the chem dropdown
  TRACE("loop_post_drain");

  // Adaptive encoder response: fast when active, slower when idle
  bool encoderActive = (millis() - encoderTimeStamp) < 500; // 500ms idle threshold

// EXPERIMENTAL DEBUG (2026-07-23): per-section timing to find what causes
// the once/sec ~90-140ms stall found earlier this session (BARDBG). Prints
// once/sec whichever section had the largest single call that second.
// Remove once the stall's cause is found.
#define TIME_SECTION(name, call)                              \
  do                                                          \
  {                                                           \
    unsigned long _t0 = micros();                             \
    call;                                                     \
    unsigned long _dt = micros() - _t0;                       \
    static unsigned long _max = 0;                            \
    if (_dt > _max)                                           \
      _max = _dt;                                             \
    if (millis() - g_sectionPrintTimer >= 1000)               \
    {                                                         \
      if (_max > 1000)                                        \
        Serial.printf("[SECDBG] %s maxUs=%lu\n", name, _max); \
      _max = 0;                                               \
    }                                                         \
  } while (0)
  static unsigned long g_sectionPrintTimer = 0;

  TIME_SECTION("LvglUpd", LvglUpdatesInterval(0, true));
  StatusBarUpdateInterval(300);

  TIME_SECTION("scpi", scpiParser.process());
  TIME_SECTION("pwrMgmt", PowerManagementInterval(500));
  MemoryMonitorInterval(5000); // Memory monitoring every 5 seconds
  TIME_SECTION("recPlay", RecordingPlaybackInterval());
  // TEMP TEST: disabled to check if this causes the periodic ~16-19ms LvglUpd spike
  // TIME_SECTION("p2clean", Page2RightSideCleanup(1000));

  // Chart refresh (Core 1 only - LVGL thread-safe)
  TIME_SECTION("histChart", HistogramChartRefreshInterval(125));
  TIME_SECTION("graphChart", GraphChartRefreshInterval(125));

  // Flush measures - gated by displayReady (NofAvgs-driven, DispObject.cpp) so
  // avg=N updates every Nth sample. BUT: the big V/A label flush physically
  // costs ~10-15ms (confirmed via SECDBG - SPI/DMA bound, not CPU bound), so
  // the display can sustain at most ~50-65 repaints/sec. At avg=1 with FUN
  // mode driving continuous change, displayReady fires far faster than that
  // ceiling and requests pile up into 100ms+ stalls (confirmed live). This
  // floor caps actual repaint attempts at the ceiling regardless of NofAvgs -
  // avg=128 (~128ms between ready events) is unaffected; avg=1 gets capped
  // instead of flooding.
  {
    static unsigned long lastFlushMeasuresMs = 0;
    if (millis() - lastFlushMeasuresMs >= 20) // ~50Hz ceiling, safely under the ~65-100Hz physical limit
    {
      TIME_SECTION("flushMeas", PowerSupply.FlushMeasures());
      lastFlushMeasuresMs = millis();
    }
  }
  if (millis() - g_sectionPrintTimer >= 1000)
    g_sectionPrintTimer = millis();

  // Settings flush: SetUpdate runs on Core 0 (function generator, encoder) and only
  // updates adjValue + adjValueChanged. The actual LVGL writes (setpoint label, bar)
  // must happen on Core 1 — this picks up adjValueChanged every loop and is cheap
  // when there's no change (early return inside Flush()).
  PowerSupply.Voltage.Flush();
  PowerSupply.Current.Flush();

  // Adaptive statistics update: Slow when encoder active for responsive display
  if (encoderActive)
    statisticUpdateInterval(500); // Slow update during encoder activity
  else
    statisticUpdateInterval(333); // Normal update when idle

  // Adaptive VCCC update: slower when encoder active to reduce CPU load
  if (encoderActive)
    VCCCInterval(300); // Slow update during encoder activity
  else
    VCCCInterval(30); // Fast update when idle

  // FFTUpdateInterval(1000);
  EncoderRestartInterval(1000); //--> some bugs?

  // Auto-save FGen settings on Core 1 — covers the case where user changes
  // spinbox values and resets/power-cycles without leaving page 3 (the
  // tab-leave save in tabs.cpp never fires in that path). Debounced: only
  // writes NVS/SPIFFS while dirty, idempotent when clean.
  // Interval bumped 2s -> 15s (2026-07-23): SPIFFS.open()-for-write +
  // close() on an existing file costs ~330ms combined (confirmed live,
  // see SaveGraphDataIfDirty below) - at 2s cadence while actively
  // adjusting FUN parameters, this was very likely the dominant cause of
  // the reported freeze-everything hiccup (settings/digits/bar/graph all
  // pause together because it blocks Core 1 outright). Real fix is
  // migrating this storage off SPIFFS (e.g. LittleFS) - bigger, separate task.
  {
    static unsigned long fgenSaveTimer = 0;
    TIME_SECTION("fgenSave", schedule([]
                                      {
      if (PowerSupply.funGenMemDirty) {
        PowerSupply.SaveMemoryFgen("FunGen", PowerSupply.funGenMem);
        PowerSupply.funGenMemDirty = false;
      } }, 15000, fgenSaveTimer));
  }

  // Auto-save the graph trace snapshot (Core 1). GraphPush() runs at ADC rate
  // on Core 0, so this is throttled hard to protect flash - a power loss can
  // cost up to the last interval's worth of trace, not more.
  // Interval bumped 30s -> 180s (2026-07-23): same SPIFFS open/close cost
  // (~330ms confirmed live) as FunGen above - less frequent while the real
  // fix (move off SPIFFS) is scoped separately.
  {
    static unsigned long graphSaveTimer = 0;
    TIME_SECTION("graphSave", schedule([]
                                       { SaveGraphDataIfDirty(); }, 180000, graphSaveTimer));
  }
  BatteryChargerInterval(250); // Li-ion charge/test state machine (Core 1)
  processDeferredMaToggle();   // Handle mA/A toggle UI updates from Core 0
  updateStatChartSize();       // Safe resize: never call lv_obj_set_size from draw callbacks
  managePageEncoderInteraction();

  // if (lv_obj_has_state(btn_function_gen, LV_STATE_CHECKED))
  //  PowerSupply.toggle();

  //   KeyCheckInterval(400);
  // DACInterval(100);

  // KeyCheckInterval(45);
  // Serial.printf("\nVoltage.encoder.getCount %l",PowerSupply.Voltage.encoder.getCount());
  // KeyCheckInterval(0); // moved to adc taskse
  // delay(10);

  //  Serial.printf("\nADC_loopCounter %l",PowerSupply.adc.ADC_loopCounter);
  //  Serial.printf("\n Current utiltap%i", lv_tabview_get_tab_act(tabview_utility));

  // Second forced render pass per loop iteration: picks up any bar/label
  // invalidations queued by the work above (SCPI, intervals) without waiting
  // for the next full loop pass. Same call already used at line 187 — no new
  // mechanism, just doubles the paint opportunities per iteration.
  LvglUpdatesInterval(0, true);

  // trackLoopExecution(__func__);
}

/*DATA:GRAPH?
//pio device monitor --log-file graph_dump.csv
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