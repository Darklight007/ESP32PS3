#include "SetupHandlers.h" // Include the setup handlers header
#include "display.h"       // DISPLAY_DRIVER_NAME
#include "i2c_recovery.h"  // Include I2C bus recovery module
#include "calib_inl.h"     // Include INL calibration for startup rebuild
#include "esp_heap_caps.h" // heap_caps_malloc for the internal DMA band buffer
#include "error_handler.h" // startup failure reporting (createTasks)
#include "disp_perf.h"     // DISP:FPS frame-rate meter
#include "calib_cc_limit.h"
#include "calib_rate_match.h"  // ccLimitLoad()
// #include "calib_inl_zc.h"  // Include zero-current INL calibration for startup rebuild

// Internal (DMA-capable) display buffer, 320 x 48 px = 30 KB. Two jobs:
// - small updates are drawn straight into it and sent from it (pickDrawBuffer)
// - larger ones are drawn in the full-screen PSRAM buffer, which SPI cannot
//   DMA from, and my_disp_flush() copies them through it in bands.
// Internal RAM has room since LVGL's heap moved to PSRAM (lv_mem_psram.cpp).
static constexpr size_t DMA_BAND_ROWS = 48;
static constexpr size_t DMA_BAND_PIXELS = (size_t)IWIDTH * DMA_BAND_ROWS;
static uint16_t *dmaBandBuf = nullptr;
static uint16_t *dmaBandBuf2 = nullptr; // second strip: LVGL draws one while DMA sends the other
// 1 = every frame drawn in the two internal strips (default); 0 = small frames
// in one strip, large ones in the PSRAM full-screen buffer. DISP:BUF n.
int g_drawMode = 1;

// **Pin Definitions**
#define SDA_1_ADC 17 // SDA pin for I2C bus 1 (ADC)
#define SCL_1_ADC 18 // SCL pin for I2C bus 1 (ADC)

#define SDA_2_KEY 15 // SDA pin for I2C bus 2 (Keypad)
#define SCL_2_KEY 16 // SCL pin for I2C bus 2 (Keypad)

// **I2C Interface Speeds**
// High-Speed I^2C Interface (MCP23017):
// - 100 kHz
// - 400 kHz
// - 1.7 MHz

// ADS1219 Interface Speed:
// - Supports I2C speeds up to 1 Mbps
// - Standard-mode (Sm) up to 100 kbps
// - Fast-mode (Fm) up to 400 kbps
// - Fast-mode plus (Fm+) up to 1 Mbps
// - High-speed mode (Hs-mode) is not supported

// **I2C Clock Rate Definitions**
#define I2C_CLKRATE_400K 400'000UL   // Speed of I2C bus 400 KHz
#define I2C_CLKRATE_1M 1'000'000UL   // Speed of I2C bus 1 MHz
#define I2C_CLKRATE_1_7M 1'700'000UL // Speed of I2C bus 1.7 MHz

// **Global Variables and External Declarations**
extern bool wireConnected;  // Flag indicating if device is connected on Wire
extern bool wire1Connected; // Flag indicating if device is connected on Wire1
// size_t memory;               // Global variable to hold free heap memory size
extern int32_t barGraph_V[1];
// extern DAC_codes dac_data_g;

// **Initialization Functions**

// Initialize serial communication
void initializeSerial()
{
    Serial.begin(115200);
    // Native USB CDC: once plugged in, the core's TX timeout becomes 100 ms. With
    // the cable in but no program reading the port, the TX buffer fills and every
    // print then blocks 100 ms - a once-per-second debug print froze the digits
    // and bars like clockwork. 20 ms bounds that while still letting a reading
    // host (SCPI responses) drain without truncation.
    Serial.setTxTimeoutMs(20);
    delay(100); // let the serial port settle

    // Use ESP_LOGI for debugging since Serial.print may not work
    ESP_LOGI("SETUP", "=== ESP32-S3 BOOT ===");
    ESP_LOGI("SETUP", "CPU Frequency: %d MHz", ESP.getCpuFreqMHz());

    Serial.printf("CPU Frequency: %d MHz\n", ESP.getCpuFreqMHz());
    Serial.println("Serial connection initialized.");

    ESP_LOGI("SETUP", "Serial connection initialized");
}

// Initialize and display memory usage information
void initialMemory()
{
    heepMemory = ESP.getFreeHeap(); // Get free heap memory
    Serial.printf("Total heap: %d\n", ESP.getHeapSize());
    Serial.printf("Free heap: %d\n", ESP.getFreeHeap());
    Serial.printf("Total PSRAM: %d\n", ESP.getPsramSize());
    Serial.printf("Free PSRAM: %d\n", ESP.getFreePsram());

    // Get information about internal RAM
    multi_heap_info_t info;
    heap_caps_get_info(&info, MALLOC_CAP_INTERNAL);

    size_t total_memory = info.total_free_bytes + info.total_allocated_bytes;
    Serial.printf("\nTotal internal memory: %zu bytes\n", total_memory);
    Serial.printf("Used internal memory: %zu bytes\n", info.total_allocated_bytes);
    Serial.printf("Free internal memory: %zu bytes\n", info.total_free_bytes);

    // Get information about PSRAM (external SPI RAM)
    heap_caps_get_info(&info, MALLOC_CAP_SPIRAM);
    total_memory = info.total_free_bytes + info.total_allocated_bytes;
    Serial.printf("Total PSRAM memory: %zu bytes\n", total_memory);
    Serial.printf("Used PSRAM memory: %zu bytes\n", info.total_allocated_bytes);
    Serial.printf("Free PSRAM memory: %zu bytes\n", info.total_free_bytes);
}

// Initialize I2C buses and scan for devices
void initializeI2C()
{
    // Initialize I2C buses at 1MHz first, then switch to 1.7MHz
    // This fixes cold-boot timing issues where MCP23017 may not respond
    // at high speeds until the bus has been properly synchronized
    Wire1.begin(SDA_1_ADC, SCL_1_ADC, I2C_CLKRATE_1_7M);  // Start ADC bus at 1MHz
    Wire.begin(SDA_2_KEY, SCL_2_KEY, I2C_CLKRATE_1_7M);   // Start keypad/DAC bus at 1MHz
    delay(100);  // Give peripherals time to sync at slower speed

    // Now switch to full 1.7MHz speed
    Wire1.setClock(I2C_CLKRATE_1_7M);
    Wire.setClock(I2C_CLKRATE_400K);
    // Scan the default I2C bus (Wire) 
    wireConnected = scanI2CBus(Wire, 0x20);

    // Scan the secondary I2C bus (Wire1)
    wire1Connected = scanI2CBus(Wire1, 0x40);

    // Check if devices were found on the default I2C bus
    if (!wireConnected)
        Serial.println("I2C_1 device NOT OK.");
    else
        Serial.println("I2C_1 device OK.");

    // Check if devices were found on the secondary I2C bus
    if (!wire1Connected)
        Serial.println("I2C_2 device NOT OK.");
    else
        Serial.println("I2C_2 device OK.");

    // NOTE: I2CRecovery is deliberately NOT armed here (no init() call), so it
    // stays inert exactly as it has been: Keypad_MC17/LTC2655 still call
    // handleError() on failed transfers, but with wire/sdaPin unset both
    // recoverBus() and resetBus() bail immediately and nothing is touched.
    // Arming it (init + a kpd.begin() recovery callback) was tried on
    // 2026-09-20 and coincided with V/I/mV entry closing the textarea
    // immediately instead of waiting for input - recovery tears the bus down
    // (wire->end(), pin re-muxing, ~70ms of delays) and re-runs kpd.begin()
    // mid-keypress, which can scramble the key state machine into a second
    // RELEASED event. Causation was never proven (the board disconnected
    // before logs could be captured). Re-arm only with a serial monitor
    // attached, watching for "I2C: Attempting bus recovery".

    // Initialize keypad (does not start Wire library now)
    kpd.begin();
    // Keypad::getKeys() won't rescan sooner than this, so at 66 ms it silently
    // overrode the 2/10 ms typing tiers in tasks.cpp. 20 ms still rides out
    // switch bounce (typically <5-10 ms) and lets those tiers take effect.
    kpd.setDebounceTime(20);
    kpd.setHoldTime(1000);       // Set hold time

    Serial.println("I2C Initialized.");
}

// Initialize the display
void initializeDisplay()
{
    tft.init();         // Initialize the TFT display
    tft.setRotation(3); // Set landscape orientation
    Serial.println("Display Initialized (" DISPLAY_DRIVER_NAME ").");
}

// Initialize the touch screen
void initializeTouch()
{
    // Touchscreen calibration data (rotation 3): the one saved by Settings >
    // Calibration > Touch Screen, else the factory values
    uint16_t calData[5] = {366, 3445, 310, 3406, 1};
    Preferences prefs;
    prefs.begin("my-app", true);
    uint16_t saved[5];
    const bool haveSaved = prefs.getBytes("touch_cal", saved, sizeof(saved)) == sizeof(saved) &&
                           saved[1] > 1000 && saved[3] > 1000 && saved[4] < 8;
    prefs.end();
    if (haveSaved)
        memcpy(calData, saved, sizeof(calData));
    tft.setTouch(calData);
    Serial.printf("Touch Screen Calibrated (%s).\n", haveSaved ? "saved" : "factory");
}

// Initialize LVGL graphics library
void setupLVGL()
{
    lv_init();      // Initialize LVGL library
    init_display(); // Initialize the display driver for LVGL
    init_touch();   // Initialize the touch driver for LVGL

    // Display LVGL version
    String LVGL_Arduino = "Hello ESP32! ";
    LVGL_Arduino += String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();
    Serial.println(LVGL_Arduino);
}

// Initialize DMA (Direct Memory Access) for display
void setupDMA()
{
#if DMA
    // Full-screen LVGL draw buffer; the sprite only allocates it (in PSRAM).
    // One buffer: my_disp_flush() waits for every DMA band, so a second one
    // never overlapped anything and only cost 150 KB. (Drawing into the small
    // internal band buffer instead was tried: slower, 84 vs 65 ms per full
    // repaint - LVGL re-walks the page for each of the 20 bands.)
    spr[0].setColorDepth(COLOR_DEPTH);
    buf[0] = (lv_color_t *)spr[0].createSprite(IWIDTH, IHEIGHT);

    // Initialize DMA
    tft.initDMA();
    lv_disp_draw_buf_init(&draw_buf, buf[0], nullptr, IWIDTH * IHEIGHT);

    // DMA staging buffer in INTERNAL (DMA-capable) RAM.
    //
    // The LVGL draw buffers above are TFT_eSprites of IWIDTH*IHEIGHT, which
    // createSprite() puts in PSRAM (2 x 150KB). PSRAM is not DMA-capable for
    // SPI master, so when pushImageDMA() handed such a pointer straight to
    // spi_device_queue_trans() the driver tried to allocate its own internal
    // bounce buffer for the whole transfer and failed:
    //     [TFTDMA] ret=0x101 (ESP_ERR_NO_MEM) len=16287 buf=0x3d800908 dmaCap=0
    // Upstream then did assert(ret == ESP_OK), which abort()s and REBOOTED the
    // unit - the random restarts during full-auto calibration, where INL's
    // repaints are large and frequent.
    //
    // pushImageDMA()'s 6th argument is exactly the hook for this: given a
    // buffer, it copies the image into it and DMAs from THAT. So stage each
    // band through this internal buffer and the driver never needs to bounce.
    // One fixed allocation at boot - no per-frame malloc, so no fragmentation.
    dmaBandBuf = (uint16_t *)heap_caps_malloc(DMA_BAND_PIXELS * sizeof(uint16_t),
                                              MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    Serial.printf("\nDMA band buffer: %u px (%u bytes) @ %p\n",
                  (unsigned)DMA_BAND_PIXELS, (unsigned)(DMA_BAND_PIXELS * sizeof(uint16_t)),
                  (void *)dmaBandBuf);
    if (!dmaBandBuf)
        Serial.println("WARNING: DMA band buffer alloc failed - flushing direct from PSRAM (may drop frames)");
    dmaBandBuf2 = (uint16_t *)heap_caps_malloc(DMA_BAND_PIXELS * sizeof(uint16_t),
                                               MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);

#else
    // Initialize display buffer without DMA
    lv_disp_draw_buf_init(&draw_buf, buf[0], buf[1], screenWidth * 10);
#endif
}

// **Setup Functions for Hardware Components**

// Setup the power supply
void setupPowerSupply()
{
#define LV_EVENT_REFR_EXT_DRAW_SIZE

    // Setup code for power supply

    // Initialize the power supply display and pages
    PowerSupply.setupDisplay(lv_scr_act());
    PowerSupply.setupPages("Stats", "Graph", "Main", "Utility", "Setting");
    PowerSupply.setPagesCallback(updateObjectPos_cb);

    pinMode(PowerSupply.CCCVPin, INPUT); // Configure CCCV pin as input

    pinMode(PowerSupply.AmA_Pin, OUTPUT);
    digitalWrite(PowerSupply.AmA_Pin, LOW);

    // pinMode(PowerSupply.CCCVPin, INPUT_PULLUP);  // Alternative configuration

    // Setup on/off touch switch on page 3
    PowerSupply.setupSwitch(PowerSupply.page[2], 0, 240, 160, btn_event_cb);

    PowerSupply.dac_data = PowerSupply.LoadDACdata("dac_data_");
    ccLimitLoad();
    rateMatchLoad();
    PowerSupply.Voltage.adjOffset = PowerSupply.dac_data.zero_voltage;
    PowerSupply.Voltage.minValue = (-PowerSupply.dac_data.zero_voltage);                                   // 2000.0;
    PowerSupply.Voltage.maxValue = (PowerSupply.dac_data.max_voltage - PowerSupply.dac_data.zero_voltage); // 2000.0;
    PowerSupply.Voltage.adc_maxValue = 32.768;

    PowerSupply.Current.adjOffset = PowerSupply.dac_data.zero_current;
    PowerSupply.Current.minValue = (-PowerSupply.dac_data.zero_current);                                   // 10000.0;
    PowerSupply.Current.maxValue = (PowerSupply.dac_data.max_current - PowerSupply.dac_data.zero_current); // 10000.0;
    PowerSupply.Current.adc_maxValue = 6.5536;

    // Setup voltage, current, and power parameters for page 3
    PowerSupply.Voltage.setup(PowerSupply.page[2], "V-Set:", -14, -8, "V",PowerSupply.Voltage.maxValue, PowerSupply.Voltage.minValue,
                              5.0, PowerSupply.dac_data.zero_voltage, 2000,"");

    // Set window sizes for measurements and statistics
    PowerSupply.Voltage.measured.SetWindowSize(MAX_NO_OF_AVG);
    PowerSupply.Voltage.Statistics.SetWindowSize(512);

    PowerSupply.Current.measured.SetWindowSize(MAX_NO_OF_AVG);
    PowerSupply.Current.Statistics.SetWindowSize(512);

    PowerSupply.Power.measured.SetWindowSize(MAX_NO_OF_AVG);
    PowerSupply.Power.Statistics.SetWindowSize(1);

    PowerSupply.Voltage.effectiveResolution.SetWindowSize(32);
    PowerSupply.Current.effectiveResolution.SetWindowSize(32);
    PowerSupply.Power.effectiveResolution.SetWindowSize(1);

    // Fixed window, independent of the ADC Avgs setting (which only sizes the
    // calibrated `measured` averages above).
    PowerSupply.Voltage.rawValueStats.SetWindowSize(2048);
    PowerSupply.Current.rawValueStats.SetWindowSize(2048);
    PowerSupply.Current.rawValueStats_mA.SetWindowSize(2048);
    // Setup current parameters
    PowerSupply.Current.setup(PowerSupply.page[2], "I-Set:", -14, 74, "A", PowerSupply.Current.maxValue, PowerSupply.Current.minValue,
                              1.0, PowerSupply.dac_data.zero_current, 10000,"m");

    // REL indicator label (hidden by default) - same level as "m" prefix
    PowerSupply.gui.label_current_rel = lv_label_create(PowerSupply.page[2]);
    lv_label_set_text(PowerSupply.gui.label_current_rel, "R");
    lv_obj_set_style_text_color(PowerSupply.gui.label_current_rel, lv_color_hex(0xFFFF00), 0);
    lv_obj_set_style_text_font(PowerSupply.gui.label_current_rel, &lv_font_montserrat_12, 0);
    lv_obj_align(PowerSupply.gui.label_current_rel, LV_ALIGN_TOP_RIGHT, -2, 74);
    lv_obj_add_flag(PowerSupply.gui.label_current_rel, LV_OBJ_FLAG_HIDDEN);

    // "OVL" overload indicator - shown once the current reading approaches the
    // active range's full scale (threshold scales with the PGA; see FlushMeasures).
    PowerSupply.gui.label_current_ovl = lv_label_create(PowerSupply.page[2]);
    lv_label_set_text(PowerSupply.gui.label_current_ovl, "OVL");
    lv_obj_set_style_text_color(PowerSupply.gui.label_current_ovl, lv_color_hex(0xFF0000), 0);
    lv_obj_set_style_text_font(PowerSupply.gui.label_current_ovl, &lv_font_montserrat_12, 0);
    lv_obj_align(PowerSupply.gui.label_current_ovl, LV_ALIGN_TOP_RIGHT, -24, 74);
    lv_obj_add_flag(PowerSupply.gui.label_current_ovl, LV_OBJ_FLAG_HIDDEN);

    // Setup power display parameters
    PowerSupply.Power.setup(PowerSupply.page[2], "", -14, 144, "W", 0, 0, 0, 0, 0,"", &dseg_b_24, &Tauri_R_28);

    Serial.println("Power Supply Setup Completed.");

    // Setup voltage and current encoder pins and settings
    PowerSupply.Voltage.SetEncoderPins(5, 4, VoltageEnc);
    PowerSupply.Voltage.setLock(false);
    PowerSupply.Voltage.restrict = "%+07.3f";

    PowerSupply.Current.SetEncoderPins(7, 6, CurrentEnc);
    PowerSupply.Current.setLock(false);
    PowerSupply.Current.restrict = "%+07.3f";

    // Disable settings for power and adjust fonts
    PowerSupply.Power.enableSetting(false);
    PowerSupply.Power.restrict = "%+08.3f";

    lv_obj_align(PowerSupply.Power.label_measureValue, LV_ALIGN_DEFAULT, -10, 152);
    lv_obj_align(PowerSupply.Power.label_unit, LV_ALIGN_DEFAULT, 130, 150);

    // Energy counter and timer countdown, between the "W" and the ON/OFF button
    // (button: x=240, y=160, 54x30). Both right-aligned to the button's left
    // edge. The timer line shows only while the timer runs; without it the
    // energy sits centred on the power digits (see updateEnergyAndTimeDisplays).
    // Output-on time is in the status bar, bottom left.
    auto sideLabel = [](const char *txt, lv_coord_t y)
    {
        lv_obj_t *l = lv_label_create(PowerSupply.page[2]);
        lv_label_set_text(l, txt);
        lv_obj_set_style_text_font(l, &lv_font_montserrat_10, 0);
        lv_obj_set_width(l, 72);
        lv_obj_set_style_text_align(l, LV_TEXT_ALIGN_RIGHT, 0);
        lv_obj_set_pos(l, 157, y);
        return l;
    };
    PowerSupply.gui.label_timer_remaining = sideLabel("", 162);
    lv_obj_add_flag(PowerSupply.gui.label_timer_remaining, LV_OBJ_FLAG_HIDDEN); // Hidden by default
    // Energy (Wh) and charge (Ah) under the power reading: digits in bold
    // DSEG, units in Tauri, power colour (recoloured with CC/CV in device.cpp).
    {
        const lv_color_t c = lv_obj_get_style_text_color(PowerSupply.Power.label_measureValue, 0);
        auto mk = [&](const char *txt, const lv_font_t *f, lv_coord_t x, lv_coord_t y, lv_coord_t w)
        {
            lv_obj_t *l = lv_label_create(PowerSupply.page[2]);
            lv_label_set_text(l, txt);
            lv_obj_set_style_text_font(l, f, 0);
            lv_obj_set_style_text_color(l, c, 0);
            if (w)
            {
                lv_obj_set_width(l, w);
                lv_obj_set_style_text_align(l, LV_TEXT_ALIGN_RIGHT, 0);
            }
            lv_obj_set_pos(l, x, y);
            return l;
        };
        PowerSupply.gui.label_energy_counter = mk("0.000", &dseg_b_11, 30, 185, 90);
        PowerSupply.gui.label_energy_unit = mk("Wh", &Tauri_R_16, 123, 180, 0);
        PowerSupply.gui.label_charge_counter = mk("0.000", &dseg_b_11, 135, 185, 58);
        PowerSupply.gui.label_charge_unit = mk("Ah", &Tauri_R_16, 196, 180, 0);
    }

    // Create charts on pages
    GraphChart(PowerSupply.page[1], 22, -6); // Page 2: Graph chart
    StatsChart(PowerSupply.page[0], 22, -6); // Page 1: Statistics chart

    stat_measure(PowerSupply.page[0], 10, 167);

    // Setup tabs and pages
    Tabs::setDefaultPage(2);
    textarea(PowerSupply.page[2]);
    Tabs::setCurrentPage(2);
    // return;

    // Load settings parameters for page 4
    PowerSupply.LoadSetting();

    // Setup utility tab view on page 3
    Utility_tabview(PowerSupply.page[3]);

    // Setting menu on page 4
    SettingMenu(PowerSupply.page[4]);

    // myTone(NOTE_A4, 100);

    PowerSupply.Voltage.SetUpdate(PowerSupply.Voltage.adjValue);
    PowerSupply.Current.SetUpdate(PowerSupply.Current.adjValue);

    // Apply startup behavior setting AFTER all UI is initialized
    bool powerOnAtStartup = false;
    switch (PowerSupply.settingParameters.startupBehavior)
    {
    case StartupBehavior::ALWAYS_OFF:
        powerOnAtStartup = false;
        Serial.println("Startup behavior: Always OFF");
        break;
    case StartupBehavior::ALWAYS_ON:
        powerOnAtStartup = true;
        Serial.println("Startup behavior: Always ON");
        break;
    case StartupBehavior::LAST_STATUS:
    default:
        powerOnAtStartup = PowerSupply.settingParameters.isPowerSupplyOn;
        Serial.printf("Startup behavior: Last Status (%s)\n", powerOnAtStartup ? "ON" : "OFF");
        break;
    }

    if (powerOnAtStartup)
    {
        lv_obj_add_state(PowerSupply.powerSwitch.btn, LV_STATE_CHECKED);
        PowerSupply.turn(SWITCH::ON);
    }
    else
    {
        lv_obj_clear_state(PowerSupply.powerSwitch.btn, LV_STATE_CHECKED);
        PowerSupply.turn(SWITCH::OFF);
    }

    // MemArray mem;
    // for (int i = 0; i < 20; ++i)
    // {
    //     // myData.doubles[i] = i;
    //  mem.memory[i].voltage=0;
    //  mem.memory[i].current=0;

    PowerSupply.Voltage.Bar.bar;
    lv_bar_t *bar = (lv_bar_t *)PowerSupply.Voltage.Bar.bar;

    // Assuming 'bar' is a pointer to lv_bar_t
    // lv_bar_t *bar = /* your bar object initialization */;

    // Get the address of cur_value
    int32_t *curValuePtr = &bar->cur_value;
    curValuePtr = barGraph_V;
    // }
    // PowerSupply.SaveMemory("myDataKey", mem);

    // btn_calibration_ADC_voltage_event_cb(NULL);
    // btn_calibration_ADC_current_event_cb(NULL);

    // NOTE: rebuildINLFromCalibration() moved to setupADC() after CalBank is initialized
}

// Setup the calibration page
void setupCalibPage()
{
    // Create Calibration page
    // voltage_current_calibration();
    // voltageCurrentCalibration = lv_obj_create(lv_scr_act());

    // lv_obj_add_flag(voltageCurrentCalibration, LV_OBJ_FLAG_HIDDEN);
    // hide(win_ADC_voltage_calibration);
    // hide(win_ADC_current_calibration);
    // hide(win_DAC_calibration);
    btn_calibration_ADC_voltage_event_cb(NULL);
    btn_calibration_ADC_current_event_cb(NULL);
    open_dac_calibration_cb(NULL);

    lv_obj_add_flag(PowerSupply.gui.calibration.win_ADC_voltage_calibration, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(PowerSupply.gui.calibration.win_ADC_current_calibration, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(PowerSupply.gui.calibration.win_DAC_calibration, LV_OBJ_FLAG_HIDDEN);

    lv_obj_add_flag(PowerSupply.gui.textarea_set_value, LV_OBJ_FLAG_HIDDEN);
}

// Setup the ADC (Analog-to-Digital Converter)
void setupADC()
{
    // Bank of calibration data for different devices based on MAC address
    PowerSupply.CalBank = {
        {"7C:9E:BD:4D:C7:08", {0.005000, 121, 32.7503, 3353431}, {0.0000, 124955, 3.000, 1746856} // v1.6
        ,
        {0.0000, 124955, 3.000, 1746856}
         ,
         40'000.0} // Internal current consumption in Amperes;
    };

    // Setup ADC with READY pin number, ISR function, and I2C bus (Wire1)
    PowerSupply.setupADC(9, ADCPinISR, &Wire1);  // FIXED: ADC is on Wire1, not Wire!

    // Populate Current.calib_m/b, adc_maxValue, and restrict (mA vs A decimal
    // format) for the current range now that CalBank exists. LoadSetting() ran
    // earlier in setup() but couldn't do this - CalBank was still empty then.
    PowerSupply.calibrationUpdate();
    Serial.print("\nADC Setup & Calibration Completed.");

    // Rebuild INL interpolators from saved calibration data (must be after CalBank init)
    rebuildINLFromCalibration();
    // rebuildINLZCFromCalibration();
}

// Setup the DAC (Digital-to-Analog Converter)
void setupDAC()
{
    PowerSupply.setupDAC(0x41);                       // Setup DAC with I2C address
    PowerSupply.DAC.writeAndPowerAll(DAC_VOLTAGE, 0); // Initialize DAC outputs
    Serial.print("\nDAC Setup Completed.");
}

// Setup the Buzzer (LEDC PWM)
void setupBuzzer()
{
    initBuzzer(); // Initialize LEDC peripheral for buzzer
    Serial.print("\nBuzzer Setup Completed.");
}
// void startTasks(){
//   xTaskCreatePinnedToCore(adc_task, "ADC_TASK", 4096, nullptr, 4, &adcTaskHandle, 0);
// }

// Create tasks for FreeRTOS
void createTasks()
{
    // No bar-graph task: the bars are updated on Core 1 right before each
    // render (updateBarsForFrame() in intervals.cpp), so every frame carries the
    // newest value. The old Core 0 task polled every 1 ms, skipped whenever
    // Core 1 was rendering, and its LVGL writes raced the renderer.

    // Create a task for ADC reading and FUN generation
    // Priority 5 - balanced (was 1 too low, 20 was too high)
    const BaseType_t adcOk = xTaskCreatePinnedToCore(
        Task_ADC,                /* Task function */
        "Voltage & Current ADC", /* Name of task */
        14000,                   /* Stack size of task */
        NULL,                    /* Parameter of the task */
        12,                       /* Priority of the task */
        &Task_adc,               /* Task handle */
        0                        /* Pin task to core 0 */
    );
    // Task_ADC is created after the UI has taken most internal SRAM. If its
    // 14 KB stack doesn't fit, creation fails and the unit boots looking normal
    // but with no measurements and a dead keypad - make that impossible to miss.
    // (setup() runs on Core 1, so the LVGL calls here are safe.)
    if (adcOk != pdPASS)
    {
        Task_adc = nullptr;
        char msg[96];
        snprintf(msg, sizeof(msg), "Task_ADC not started: %u B internal RAM free, largest block %u B",
                 (unsigned)heap_caps_get_free_size(MALLOC_CAP_INTERNAL),
                 (unsigned)heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL));
        ErrorHandler::report(ErrorHandler::Severity::CRITICAL, ErrorHandler::Category::MEMORY, msg, __FILE__, __LINE__);
        lv_obj_t *mb = lv_msgbox_create(NULL, "STARTUP FAILURE",
                                        "ADC/keypad task could not start (out of internal RAM).\n"
                                        "Measurements and keypad are disabled.", NULL, false);
        lv_obj_center(mb);
    }

    createPersistTask(); // background flash saves (tasks.cpp)

 
    // Create a task for ADC reading, with priority 1, pinned to core 0
    // xTaskCreatePinnedToCore(
    //     Task_DAC,                /* Task function */
    //     "DAC set", /* Name of task */
    //     4000,                   /* Stack size of task */
    //     NULL,                    /* Parameter of the task */
    //     10,                       /* Priority of the task */
    //     &Task_dac,               /* Task handle */
    //     0                        /* Pin task to core 0 */
    // );

    Serial.print("\nReal-time tasks created and pinned to cores.");

    // Set initial task priority based on loaded ADC rate setting
    PowerSupply.adjustAdcTaskPriority();
}

// **Utility Functions**

// Scan an I2C bus for a specific device address
// Returns true if a device is found at the given address
bool scanI2CBus(TwoWire &i2cBus, uint8_t address)
{
    i2cBus.beginTransmission(address);
    uint8_t error = i2cBus.endTransmission();

    if (error == 0)
        return true; // Device found
    else
        return false; // No device found
}

// LVGL display flush function
void touchSample();

static bool lastFlushDirect = false;
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    // Low-noise mode: wait here for the ADC to finish its V+I pair (the frame
    // was already rendered into the buffer while it converted). No-op otherwise.
    spiFrameClaim();
    dispPerfFlushBegin();
    tft.startWrite();

#if DMA
    // Push in horizontal bands of at most 0x4000 (16384) pixels.
    //
    // pushImageDMA() can only DMA 64KB at once, and for anything larger it falls
    // back to a `while (len > 0x4000) pushPixels(...)` loop - that is BLOCKING
    // (polling) SPI, after which it still queues a DMA transaction on the same
    // device. Mixing a polling transfer with spi_device_queue_trans() makes the
    // queue call return ESP_ERR_INVALID_STATE, and the library does
    // `assert(ret == ESP_OK)` - which abort()s and REBOOTS the unit.
    //
    // Our LVGL draw buffer is a full-screen sprite (IWIDTH*IHEIGHT = 76800 px),
    // so every large invalidation took that path. It only bit on big repaints
    // (INL opening its window during full-auto calibration) and only sometimes,
    // depending on whether the polling transfer had drained - which is exactly
    // how it presented: random reboots at step 6/6.
    // TFT_eSPI's own comment says the caller should split into <=32768 px
    // blocks; staying under 0x4000 keeps us clear of the fallback entirely.
    //
    // No throughput cost: the original code called dmaWait() immediately after
    // the single push, so transfers were never overlapped anyway.
    {
        const int32_t x = area->x1;
        const int32_t w = area->x2 - area->x1 + 1;
        const int32_t h = area->y2 - area->y1 + 1;
        uint16_t *src = (uint16_t *)&color_p->full;
        // Drawn straight into an internal strip: DMA from it as is
        const bool direct = src == dmaBandBuf || (dmaBandBuf2 && src == dmaBandBuf2);
        lastFlushDirect = direct;

        // Rows per band. Bounded by the internal staging buffer when we have
        // one (that is what makes the source DMA-capable), otherwise by the
        // library's 0x4000-pixel fallback threshold.
        const size_t bandLimitPx = dmaBandBuf ? DMA_BAND_PIXELS : (size_t)0x4000;
        int32_t maxRows = (w > 0) ? (int32_t)(bandLimitPx / (size_t)w) : h;
        if (maxRows < 1)
            maxRows = 1;

        int32_t yy = area->y1;
        int32_t rowsLeft = h;
        while (rowsLeft > 0)
        {
            const int32_t rows = (rowsLeft > maxRows) ? maxRows : rowsLeft;
            // 6th arg = staging buffer: pushImageDMA() copies this band into it
            // and DMAs from THERE, so the SPI driver gets DMA-capable internal
            // memory instead of a PSRAM pointer it has to bounce (and fail on
            // with ESP_ERR_NO_MEM). nullptr falls back to DMA straight from the
            // source, i.e. the old behaviour, if the alloc ever failed.
            // Drawn straight into the internal buffer (see pickDrawBuffer): send
            // it as is, no staging copy.
            tft.pushImageDMA(x, yy, w, rows, src, direct ? nullptr : dmaBandBuf);
            if (!direct)
                tft.dmaWait(); // the next band overwrites the staging buffer
            // direct: no wait. The transfer runs while LVGL draws the next
            // strip into the OTHER buffer; pushImageDMA() waits for it before
            // queueing the next one, and the frame's last flush waits below.
            src += (size_t)rows * (size_t)w;
            yy += rows;
            rowsLeft -= rows;
        }
    }

#else
    // Push image to display without DMA
    uint32_t w = area->x2 - area->x1 + 1;
    uint32_t h = area->y2 - area->y1 + 1;
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.pushColors((uint16_t *)&color_p->full, w * h, true);
#endif

    // Chip-select is driven by the library, not the DMA: keep the write open
    // while a direct transfer is still running, close it once the frame's
    // last one is done.
    if (!lastFlushDirect || lv_disp_flush_is_last(disp))
    {
        tft.dmaWait();
        tft.endWrite();
    }
    dispPerfFlushEnd();

    if (lv_disp_flush_is_last(disp) && SpiQuiet::frameClaimed)
    {
        if (SpiQuiet::lowNoise.load())
            touchSample(); // touch shares this slot; my_touchpad_read uses the cache
        spiFrameRelease();
    }

    lv_disp_flush_ready(disp); // Indicate that flushing is done
}

// Per frame: small updates (digits, bar strips) are drawn in the internal DMA
// buffer and sent from it directly; anything larger uses the full-screen PSRAM
// buffer, which LVGL fills in one pass.
static void (*prevRenderStart)(lv_disp_drv_t *) = nullptr;
static void pickDrawBuffer(lv_disp_drv_t *drv)
{
    lv_disp_draw_buf_t *b = drv->draw_buf;
    if (g_drawMode == 1 && dmaBandBuf && dmaBandBuf2)
    {
        // Two internal strips, LVGL alternates between them (it swaps after
        // each flush). Large areas are drawn strip by strip.
        b->buf1 = dmaBandBuf;
        b->buf2 = dmaBandBuf2;
        if (b->buf_act != b->buf1 && b->buf_act != b->buf2)
            b->buf_act = b->buf1;
        b->size = DMA_BAND_PIXELS;
    }
    else
    {
        lv_disp_t *d = lv_disp_get_default();
        bool small = dmaBandBuf && d;
        for (uint16_t i = 0; small && i < d->inv_p; i++)
            if (!d->inv_area_joined[i] && lv_area_get_size(&d->inv_areas[i]) > DMA_BAND_PIXELS)
                small = false;
        b->buf2 = nullptr;
        b->buf1 = b->buf_act = small ? (void *)dmaBandBuf : (void *)buf[0];
        b->size = small ? DMA_BAND_PIXELS : (uint32_t)IWIDTH * IHEIGHT;
    }
    if (prevRenderStart)
        prevRenderStart(drv);
}

// Initialize the display driver for LVGL
void init_display()
{
    static lv_disp_drv_t disp_drv;     // Descriptor of a display driver
    lv_disp_drv_init(&disp_drv);       // Basic initialization
    disp_drv.flush_cb = my_disp_flush; // Set your driver function
    disp_drv.draw_buf = &draw_buf;     // Set an initialized buffer
    disp_drv.hor_res = screenWidth;    // Set horizontal resolution
    disp_drv.ver_res = screenHeight;   // Set vertical resolution
    dispPerfAttach(&disp_drv);         // frame-rate meter (DISP:FPS?)
    prevRenderStart = disp_drv.render_start_cb;
    disp_drv.render_start_cb = pickDrawBuffer;
    lv_disp_drv_register(&disp_drv);   // Finally register the driver

    lv_disp = lv_disp_get_default(); // Get the default display
}
