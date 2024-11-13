#include "SetupHandlers.h" // Include the setup handlers header

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
#define I2C_CLKRATE_400K 400000UL  // Speed of I2C bus 400 KHz
#define I2C_CLKRATE_1M 1000000UL   // Speed of I2C bus 1 MHz
#define I2C_CLKRATE_1_7M 1700000UL // Speed of I2C bus 1.7 MHz

// **Global Variables and External Declarations**
extern bool wireConnected;  // Flag indicating if device is connected on Wire
extern bool wire1Connected; // Flag indicating if device is connected on Wire1
// size_t memory;               // Global variable to hold free heap memory size

// **Initialization Functions**

// Initialize serial communication
void initializeSerial()
{
    Serial.begin(115200);
    delay(1000); // Wait for serial connection to stabilize
    Serial.println("Serial Connection Initialized.");
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
    // Initialize I2C buses with appropriate clock rates
    Wire1.begin(SDA_1_ADC, SCL_1_ADC, I2C_CLKRATE_1_7M); // For ADS1219 (supports up to 1 Mbps)
    Wire.begin(SDA_2_KEY, SCL_2_KEY, I2C_CLKRATE_1_7M);  // For other devices (supports up to 1.7 Mbps)

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

    // Initialize keypad (does not start Wire library now)
    kpd.begin();
    kpd.setDebounceTime(33); // Set debounce time (no bouncing for this I2C)
    kpd.setHoldTime(1000);   // Set hold time

    Serial.println("I2C Initialized.");
}

// Initialize the display
void initializeDisplay()
{
    tft.init();         // Initialize the TFT display
    tft.setRotation(3); // Set landscape orientation
    Serial.println("Display Initialized.");
}

// Initialize the touch screen
void initializeTouch()
{
    // Touchscreen calibration data (rotation 3)
    uint16_t calData[5] = {366, 3445, 310, 3406, 1};
    tft.setTouch(calData);
    Serial.println("Touch Screen Calibrated.");
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
    // Define sprite color depth
    spr[0].setColorDepth(COLOR_DEPTH);
    spr[1].setColorDepth(COLOR_DEPTH);

    // Create the 2 sprites
    buf[0] = (lv_color_t *)spr[0].createSprite(IWIDTH, IHEIGHT);
    buf[1] = (lv_color_t *)spr[1].createSprite(IWIDTH, IHEIGHT);

    // Define text datum for each sprite
    spr[0].setTextDatum(MC_DATUM);
    spr[1].setTextDatum(MC_DATUM);

    // Initialize DMA
    tft.initDMA();
    lv_disp_draw_buf_init(&draw_buf, buf[0], buf[1], IWIDTH * IHEIGHT);

#else
    // Initialize display buffer without DMA
    lv_disp_draw_buf_init(&draw_buf, buf[0], buf[1], screenWidth * 10);
#endif
}

// Setup tasks for FreeRTOS
void setupTasks()
{
    // Task creation for FreeRTOS would be done in createTasks()
    Serial.println("Tasks Setup Completed.");
}

// Setup preferences (non-volatile storage)
void setupPreferences()
{
    Preferences preferences;

    preferences.begin("param", false);
    preferences.putUShort("pi", 314);
    unsigned short readBack = preferences.getUShort("pi", 0);

    if (314 == readBack)
        Serial.print("\nPreferences Memory: OK");
    else
        Serial.print("\nPreferences Memory: NOT OK");

    preferences.end();
}

// **Setup Functions for Hardware Components**

// Setup the power supply
void setupPowerSupply()
{
#define LV_EVENT_REFR_EXT_DRAW_SIZE

    // Setup code for power supply
    pinMode(PowerSupply.CCCVPin, INPUT); // Configure CCCV pin as input
    // pinMode(PowerSupply.CCCVPin, INPUT_PULLUP);  // Alternative configuration

    // Initialize the power supply display and pages
    PowerSupply.setupDisplay(lv_scr_act());
    PowerSupply.setupPages("Stats", "Graph", "Main", "Utility", "Setting");
    PowerSupply.setPagesCallback(updateObjectPos_cb);

    // Setup on/off touch switch on page 3
    PowerSupply.setupSwitch(PowerSupply.page[2], 0, 240, 160, btn_event_cb);

    // Setup voltage, current, and power parameters for page 3
    PowerSupply.Voltage.setup(PowerSupply.page[2], "V-Set:", -14, -8, "V", 32.7675, 5.0, 0, 2000);

    // Set window sizes for measurements and statistics
    PowerSupply.Voltage.measured.SetWindowSize(MAX_NO_OF_AVG);
    PowerSupply.Voltage.Statistics.SetWindowSize(4096);

    PowerSupply.Current.measured.SetWindowSize(MAX_NO_OF_AVG);
    PowerSupply.Current.Statistics.SetWindowSize(4096);

    PowerSupply.Power.measured.SetWindowSize(MAX_NO_OF_AVG);
    PowerSupply.Power.Statistics.SetWindowSize(1);

    PowerSupply.Voltage.effectiveResolution.SetWindowSize(32);
    PowerSupply.Current.effectiveResolution.SetWindowSize(32);
    PowerSupply.Power.effectiveResolution.SetWindowSize(1);

    // Setup current parameters
    PowerSupply.Current.setup(PowerSupply.page[2], "I-Set:", -14, 74, "A", 6.5535, 1.0, -5.5, 2000);

    // Setup power display parameters
    PowerSupply.Power.setup(PowerSupply.page[2], "", -14, 144, "W", 0, 0, 0, 0, &dseg_b_24, &Tauri_R_28);

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

    lv_obj_align(PowerSupply.Power.label_measureValue, LV_ALIGN_DEFAULT, -10, 161);
    lv_obj_align(PowerSupply.Power.label_unit, LV_ALIGN_DEFAULT, 130, 159);

    // Create charts on pages
    GraphChart(PowerSupply.page[1], 22, -6); // Page 2: Graph chart
    StatsChart(PowerSupply.page[0], 22, -6); // Page 1: Statistics chart

    stat_measure(PowerSupply.page[0], 10, 167);

    // Turn on power supply initially
    PowerSupply.turn(SWITCH::ON);

    // Setup tabs and pages
    Tabs::setDefaultPage(2);
    textarea(PowerSupply.page[2]);
    Tabs::setCurrentPage(2);

    // Setup utility tab view on page 3
    Utility_tabview(PowerSupply.page[3]);

    // Load settings parameters for page 4
    PowerSupply.LoadSetting();

    // Setting menu on page 4
    SettingMenu(PowerSupply.page[4]);

    // Uncomment for testing
    // myTone(NOTE_A4, 100);
    // Serial.println("\nSetup done");
    // PowerSupply.turn(SWITCH::ON);

 

    PowerSupply.Voltage.SetUpdate(PowerSupply.Voltage.adjValue);
    PowerSupply.Current.SetUpdate(PowerSupply.Current.adjValue);
    DataArrays myData;
    // for (int i = 0; i < 20; ++i)
        // myData.doubles[i] = i ; // Example values

    // PowerSupply.SaveDataArrays("myDataKey", myData);
}

// Setup the calibration page
void setupCalibPage()
{
    // Create Calibration page
    voltage_current_calibration();

    // Hide the calibration objects initially
    lv_obj_add_flag(voltageCurrentCalibration, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(myTextBox, LV_OBJ_FLAG_HIDDEN);
}

// Setup the ADC (Analog-to-Digital Converter)
void setupADC()
{
    // Bank of calibration data for different devices based on MAC address
    PowerSupply.CalBank = {
        {"7C:9E:BD:4D:C7:08", {0.005000, 121, 32.7503, 3353431}, {0.0000, 124955, 3.000, 1746856}} // v1.6
    };

    // Setup ADC with pin number, ISR function, and I2C bus
    PowerSupply.setupADC(9, ADCPinISR, &Wire);

    Serial.print("\nADC Setup & Calibration Completed.");
}

// Setup the DAC (Digital-to-Analog Converter)
void setupDAC()
{
    PowerSupply.setupDAC(0x41);                       // Setup DAC with I2C address
    PowerSupply.DAC.writeAndPowerAll(DAC_VOLTAGE, 0); // Initialize DAC outputs

    Serial.print("\nDAC Setup Completed.");
}

// Create tasks for FreeRTOS
void createTasks()
{
    // Create a task for Bar Graph display, with priority 0, pinned to core 0
    xTaskCreatePinnedToCore(
        Task_BarGraph, /* Task function */
        "Task1",       /* Name of task */
        8000,          /* Stack size of task */
        NULL,          /* Parameter of the task */
        0,             /* Priority of the task */
        &Task1,        /* Task handle to keep track of created task */
        0              /* Pin task to core 0 */
    );

    // Create a task for ADC reading, with priority 1, pinned to core 0
    xTaskCreatePinnedToCore(
        Task_ADC,                /* Task function */
        "Voltage & Current ADC", /* Name of task */
        18000,                   /* Stack size of task */
        NULL,                    /* Parameter of the task */
        1,                       /* Priority of the task */
        &Task_adc,               /* Task handle */
        0                        /* Pin task to core 0 */
    );

    Serial.print("\nReal-time tasks created and pinned to cores.");
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
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    tft.startWrite();

#if DMA
    // Use DMA to push image to display
    tft.pushImageDMA(area->x1, area->y1,
                     area->x2 - area->x1 + 1,
                     area->y2 - area->y1 + 1,
                     (uint16_t *)&color_p->full);
    tft.dmaWait(); // Wait for DMA transfer to complete

#else
    // Push image to display without DMA
    uint32_t w = area->x2 - area->x1 + 1;
    uint32_t h = area->y2 - area->y1 + 1;
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.pushColors((uint16_t *)&color_p->full, w * h, true);
#endif

    tft.endWrite();

    lv_disp_flush_ready(disp); // Indicate that flushing is done
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
    lv_disp_drv_register(&disp_drv);   // Finally register the driver

    lv_disp = lv_disp_get_default(); // Get the default display
}
