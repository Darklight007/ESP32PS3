#include "SetupHandlers.h"

void initializeSerial()
{
    Serial.begin(115200);
    delay(1000); // Stabilize serial connection
    Serial.println("Serial Connection Initialized.");
}

void initialMemory()
{
    memory = ESP.getFreeHeap();
    Serial.printf("Total heap: %d\n", ESP.getHeapSize());
    Serial.printf("Free heap: %d\n", ESP.getFreeHeap());
    Serial.printf("Total PSRAM: %d\n", ESP.getPsramSize());
    Serial.printf("Free PSRAM: %d\n", ESP.getFreePsram());

        // Getting information about internal RAM
    multi_heap_info_t info;
    heap_caps_get_info(&info, MALLOC_CAP_INTERNAL);

    size_t total_memory = info.total_free_bytes + info.total_allocated_bytes;
    Serial.printf("\nTotal internal memory: %zu bytes\n", total_memory);
    Serial.printf("Used internal memory: %zu bytes\n", info.total_allocated_bytes);
    Serial.printf("Free internal memory: %zu bytes\n", info.total_free_bytes);

    // If you also need information about PSRAM (external SPI RAM)
    heap_caps_get_info(&info, MALLOC_CAP_SPIRAM);
    // if (info.total_allocated_bytes > 0) 
    { // Check if PSRAM is being used
        total_memory = info.total_free_bytes + info.total_allocated_bytes;
        Serial.printf("Total PSRAM memory: %zu bytes\n", total_memory);
        Serial.printf("Used PSRAM memory: %zu bytes\n", info.total_allocated_bytes);
        Serial.printf("Free PSRAM memory: %zu bytes\n", info.total_free_bytes);
    }


}

#define SDA_1_ADC 17
#define SCL_1_ADC 18

#define SDA_2_KEY 15
#define SCL_2_KEY 16

// High-Speed I^2 C Interface (MCP23017):
// - 100 kHz
// - 400 kHz
// - 1.7 MHz

// 8.5.1.4 Interface Speed
// The ADS1219 supports I2C interface speeds up to 1 Mbps. Standard-mode (Sm) with bit rates up to 100 kbps,
// fast-mode (Fm) with bit rates up to 400 kbps, and fast-mode plus (Fm+) with bit rates up to 1 Mbps are
// supported. High-speed mode (Hs-mode) is not supported.

#define I2C_CLKRATE_400K 400000UL  // Speed of I2C bus 400KHz
#define I2C_CLKRATE_1M 1000000UL   // Speed of I2C bus 1MHz
#define I2C_CLKRATE_1_7M 1700000UL // Speed of I2C bus 1MHz

void initializeI2C()
{
    // setupOTA("Power Supply");

    Wire1.begin(SDA_1_ADC, SCL_1_ADC, I2C_CLKRATE_1_7M); //(18,15), 16,17
    Wire.begin(SDA_2_KEY, SCL_2_KEY, I2C_CLKRATE_1_7M);
    // Wire.setClock(400000UL);

    kpd.begin();             // now does not starts wire library
    kpd.setDebounceTime(33); // no bouncying for this i2C
    kpd.setHoldTime(1000);

    Serial.println("I2C Initialized.");
}

void initializeDisplay()
{
    tft.init();
    tft.setRotation(3); // Landscape orientation
    Serial.println("Display Initialized.");
}

void setupPowerSupply()
{
    // Additional setup code for power supply here
    pinMode(PowerSupply.CCCVPin, INPUT);
    // pinMode(PowerSupply.CCCVPin, INPUT_PULLUP);

    PowerSupply.setupDisplay(lv_scr_act());
    PowerSupply.setupPages("Stats", "Graph", "Main", "Utility", "Setting");
    PowerSupply.setPagesCallback(updateObjectPos_cb);
    // lv_obj_add_event_cb(PowerSupply.tab.tabview, updateObjectPos_cb, LV_EVENT_VALUE_CHANGED, NULL);

    // Setup up on/off touch switch on page 3
    PowerSupply.setupSwitch(PowerSupply.page[2], 0, 240, 160, btn_event_cb);

    // Setup voltage,current and power for page 3
    PowerSupply.Voltage.setup(PowerSupply.page[2], "V-Set:", -14, -8, "V", 32.7675, 5.0, -12 * 0, 2000);
    
    PowerSupply.Voltage.measured.SetWindowSize(MAX_NO_OF_AVG);
    PowerSupply.Voltage.Statistics.SetWindowSize(4096);
    
    PowerSupply.Current.measured.SetWindowSize(MAX_NO_OF_AVG);
    PowerSupply.Current.Statistics.SetWindowSize(4096);
    
    PowerSupply.Power.measured.SetWindowSize(MAX_NO_OF_AVG);
    PowerSupply.Power.Statistics.SetWindowSize(1);


    PowerSupply.Voltage.effectiveResolution.SetWindowSize(32);
    PowerSupply.Current.effectiveResolution.SetWindowSize(32);
    PowerSupply.Power.effectiveResolution.SetWindowSize(1);

    PowerSupply.Current.setup(PowerSupply.page[2], "I-Set:", -14, 74, "A", 6.5535, 1.0, -5.5 /*miliampere*/, 2000);
    PowerSupply.Power.setup(PowerSupply.page[2], "", -14, 144, "W", 0, 0, 0, 0, &dseg_b_24, &Tauri_R_28);

    Serial.println("Power Supply Setup Completed.");

    // Setup voltage and Voltage's encoder pins
    PowerSupply.Voltage.SetEncoderPins(5, 4, VoltageEnc);
    PowerSupply.Voltage.setLock(false);
    PowerSupply.Voltage.restrict = "%+07.3f";

    //   // PowerSupply.Voltage.adjValue
    PowerSupply.Current.SetEncoderPins(7, 6, CurrentEnc);
    PowerSupply.Current.setLock(false);
    PowerSupply.Current.restrict = "%+07.3f";

    // Remove setting for power and setup fonts & ...
    PowerSupply.Power.enableSetting(false);
    PowerSupply.Power.restrict = "%+08.3f";

    lv_obj_align(PowerSupply.Power.label_measureValue, LV_ALIGN_DEFAULT, -10, 161);
    lv_obj_align(PowerSupply.Power.label_unit, LV_ALIGN_DEFAULT, -10 + 140, 144 + 15);

    // Create Chart on page2
    GraphChart(PowerSupply.page[1], 22, -6);

    // Create Histogram on page1
    StatsChart(PowerSupply.page[0], 22, -6);

    // Turn off power supply initially
    PowerSupply.turn(SWITCH::ON);

    Tabs::setDefaultPage(2);
    textarea(PowerSupply.page[2]);
    Tabs::setCurrentPage(2);

    Utility_tabview(PowerSupply.page[3]);

    // load setting parameters for page 4
    PowerSupply.LoadSetting();

    // Setting menu on page 4
    SettingMenu(PowerSupply.page[4]);

    // myTone(NOTE_A4, 100);
    // Serial.println("\nSetup done");
    // PowerSupply.turn(SWITCH::ON);

    // Preferences fmemory;
    // fmemory.begin("param", false);
    // fmemory.putUShort("pi", 314);
    // Serial.printf("\nPreferences Memory test get:%i", fmemory.getUShort("pi", 0));
    // fmemory.end();

    // myTone(NOTE_A4, 100);
    // Serial.println("\nSetup done");
    // PowerSupply.turn(SWITCH::ON);

    // Preferences fmemory;
    // fmemory.begin("param", false);
    // fmemory.putUShort("pi", 314);
    // Serial.printf("\nPreferences Memory test get:%i", fmemory.getUShort("pi", 0));
    // fmemory.end();

    // er(PowerSupply.Voltage.measured.ER(2*32.768));
}
void seupCalibPage()
{
    // Create Calibration page
    voltage_current_calibration();

    lv_obj_add_flag(voltageCurrentCalibration, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(myTextBox, LV_OBJ_FLAG_HIDDEN);
}
void initializeTouch()
{
    // Touchscreen initialization code
    /*Set the touchscreen calibration data*/
    uint16_t calData[5] = {366, 3445, 310, 3406, 1}; // Rotation {3}
    tft.setTouch(calData);
    Serial.println("Touch Screen Calibrated.");
}
void setupLVGL()
{
    lv_init();
    init_display();
    init_touch();

    String LVGL_Arduino = "Hello ESP32! ";
    LVGL_Arduino += String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();
    Serial.println(LVGL_Arduino);
}
void setupDMA()
{
#if DMA
    //     // Define sprite colour depth
    spr[0].setColorDepth(COLOR_DEPTH);
    spr[1].setColorDepth(COLOR_DEPTH);

    // Create the 2 sprites
    buf[0] = (lv_color_t *)spr[0].createSprite(IWIDTH, IHEIGHT / 1);
    buf[1] = (lv_color_t *)spr[1].createSprite(IWIDTH, IHEIGHT / 1);

    // spr[1].setViewport(0, IHEIGHT / 10, IWIDTH, IHEIGHT);

    // Define text datum for each Sprite
    spr[0].setTextDatum(MC_DATUM);
    spr[1].setTextDatum(MC_DATUM);

    // To use SPI DMA you must call initDMA() to setup the DMA engine
    tft.initDMA();
    lv_disp_draw_buf_init(&draw_buf, buf[0], buf[1], IWIDTH * IHEIGHT / 1);

#else
    //     // Initialize `disp_buf` display buffer with the buffer(s).
    lv_disp_draw_buf_init(&draw_buf, buf[0], buf[1], screenWidth * 10);
#endif
}
void setupTasks()
{
    // Task creation for FreeRTOS
    Serial.println("Tasks Setup Completed.");
}


void setupPreferences()
{

    Preferences preferences;

    preferences.begin("param", false);
    preferences.putUShort("pi", 314);
    unsigned short readBack = preferences.getUShort("pi", 0);
    if (314 == readBack)
        // Serial.printf("Preferences Memory test get: %d\n", readBack);
        Serial.print("\nPreferences Memory: OK");
    else
        Serial.print("\nPreferences Memory: NOT OK");
    preferences.end();
}

void setupADC()
{
    // Assuming a method to setup calibration data

    // Bank of calibration data for differecnt devices based on Mac address
    PowerSupply.CalBank = {
        // {"7C:9E:BD:F3:11:B4", {0.005000, 121.4, 32.7503, 3353431}, {0.0000, 124955, 3.000, 1746856}},  // v1.6
        // {"30:AE:A4:97:D1:84", {1.000153, 101997, 32.7535, 3353951}, {0.0000, 124972, 4.000, 2386066}}, // v1.6
        // {"0C:B8:15:C1:21:08", {1.000153, 101997, 32.7535, 3353951}, {0.0000, 124972, 4.000, 2386066}}, // v1.6
        // {"30:AE:A4:97:59:58", {1.000153, 101997, 32.7535, 3353951}, {0.0000, 124972, 4.000, 2386066}}, // v1.6
        {"7C:9E:BD:4D:C7:08", {0.005000, 121, 32.7503, 3353431}, {0.0000, 124955, 3.000, 1746856}}, // v1.6
                                                                                                    // {"7C:9E:BD:39:28:10", {0.099900, 10108, 32.7500, 3354461}, {0.0000, 126945, 3.000, 1667719}},
                                                                                                    //  {"94:B9:7E:D3:01:CC", {0.099009, 10108, 32.7500, 3354461}, {0.0000, 126945, 3.000, 1667719}}
    };
    PowerSupply.setupADC(9, ADCPinISR, &Wire);

    Serial.print("\nADC Setup & Calibration Completed.");
}

void setupDAC()
{
    // Wire.begin(SDA_2_KEY, SCL_2_KEY, 400000UL);
    PowerSupply.setupDAC(0x41);
    PowerSupply.DAC.writeAndPowerAll(DAC_VOLTAGE, 0);
    // Wire.begin(SDA_2_KEY, SCL_2_KEY, I2C_CLKRATE_1_7M);

    // LTC2655 Device::DAC(0x41);
    Serial.print("\nDAC Setup Completed.");
}

void createTasks()
{
    // create a task that will be executed in the Task_BarGraph() function, with priority 1 and executed on core 0
    xTaskCreatePinnedToCore(
        Task_BarGraph, /* Task function. */
        "Task1",       /* name of task. */
        8000,          /* Stack size of task */
        NULL,          /* parameter of the task */
        0,             /* priority of the task */
        &Task1,        /* Task handle to keep track of created task */
        0);            /* pin task to core 0 */

    xTaskCreatePinnedToCore(
        Task_ADC,                         /* Task function. */
        "Voltage & Current ADC",          /* name of task. */
        12000,                            /* Stack size of task */
        NULL, /* parameter of the task */ /* (void *)&adcDataReady */
        1,                                /* priority of the task */
        &Task_adc,                        /* pin task to core x */
        0);

    Serial.print("\nReal-time tasks created and pinned to cores.");
}
// /* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{

    tft.startWrite();

#if DMA
    tft.pushImageDMA(area->x1, area->y1, area->x2 - area->x1 + 1, area->y2 - area->y1 + 1, (uint16_t *)&color_p->full);
    // sprSel = !sprSel;
    tft.dmaWait(); // Remove this will make displaly stuck!

#else
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.pushColors((uint16_t *)&color_p->full, w * h, true);
#endif

    tft.endWrite();

    lv_disp_flush_ready(disp);
}

// // Initialize the display -------------------------------------------
void init_display()
{
    static lv_disp_drv_t disp_drv;     // Descriptor of a display driver
    lv_disp_drv_init(&disp_drv);       // Basic initialization
    disp_drv.flush_cb = my_disp_flush; // Set your driver function
    disp_drv.draw_buf = &draw_buf;     // Set an initialized buffer
    disp_drv.hor_res = screenWidth;    // horizontal resolution
    disp_drv.ver_res = screenHeight;   // vertical resolution
    lv_disp_drv_register(&disp_drv);   // Finally register the driver

    lv_disp = lv_disp_get_default();
}
