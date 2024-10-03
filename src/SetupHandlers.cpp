#include "SetupHandlers.h"

// Pin Definitions
constexpr uint8_t SDA_1_ADC = 17;
constexpr uint8_t SCL_1_ADC = 18;
constexpr uint8_t SDA_2_KEY = 15;
constexpr uint8_t SCL_2_KEY = 16;

// I2C Clock Rates
constexpr uint32_t I2C_CLKRATE_400K = 400000UL;
constexpr uint32_t I2C_CLKRATE_1M = 1000000UL;
constexpr uint32_t I2C_CLKRATE_1_7M = 1700000UL; // 1.7 MHz

void initializeSerial() {
    Serial.begin(115200);
    delay(1000); // Stabilize serial connection
    Serial.println("Serial Connection Initialized.");
}

void initialMemory() {
    memory = ESP.getFreeHeap();
    Serial.printf("Total heap: %d\nFree heap: %d\nTotal PSRAM: %d\nFree PSRAM: %d\n",
                  ESP.getHeapSize(), ESP.getFreeHeap(), ESP.getPsramSize(), ESP.getFreePsram());

    // Internal RAM Info
    multi_heap_info_t info;
    heap_caps_get_info(&info, MALLOC_CAP_INTERNAL);

    size_t total_memory = info.total_free_bytes + info.total_allocated_bytes;
    Serial.printf("\nTotal internal memory: %zu bytes\nUsed internal memory: %zu bytes\nFree internal memory: %zu bytes\n",
                  total_memory, info.total_allocated_bytes, info.total_free_bytes);

    // PSRAM Info
    heap_caps_get_info(&info, MALLOC_CAP_SPIRAM);
    total_memory = info.total_free_bytes + info.total_allocated_bytes;
    Serial.printf("Total PSRAM memory: %zu bytes\nUsed PSRAM memory: %zu bytes\nFree PSRAM memory: %zu bytes\n",
                  total_memory, info.total_allocated_bytes, info.total_free_bytes);
}

void initializeI2C() {
    // Initialize I2C buses with appropriate clock rates
    Wire1.begin(SDA_1_ADC, SCL_1_ADC, I2C_CLKRATE_1M); // For ADS1219
    Wire.begin(SDA_2_KEY, SCL_2_KEY, I2C_CLKRATE_1_7M); // For other devices

    kpd.begin();             // Initialize keypad
    kpd.setDebounceTime(33); // Debounce time in ms
    kpd.setHoldTime(1000);   // Hold time in ms

    Serial.println("I2C Initialized.");
}

void initializeDisplay() {
    tft.init();
    tft.setRotation(3); // Landscape orientation
    Serial.println("Display Initialized.");
}

void setupPowerSupply() {
    pinMode(PowerSupply.CCCVPin, INPUT);

    PowerSupply.setupDisplay(lv_scr_act());
    PowerSupply.setupPages("Stats", "Graph", "Main", "Utility", "Setting");
    PowerSupply.setPagesCallback(updateObjectPos_cb);

    // Setup on/off touch switch on page 3
    PowerSupply.setupSwitch(PowerSupply.page[2], 0, 240, 160, btn_event_cb);

    // Setup voltage, current, and power for page 3
    PowerSupply.Voltage.setup(PowerSupply.page[2], "V-Set:", -14, -8, "V", 32.7675, 5.0, 0, 2000);
    PowerSupply.Current.setup(PowerSupply.page[2], "I-Set:", -14, 74, "A", 6.5535, 1.0, -5.5, 2000);
    PowerSupply.Power.setup(PowerSupply.page[2], "", -14, 144, "W", 0, 0, 0, 0, &dseg_b_24, &Tauri_R_28);

    // Configure measurement window sizes
    auto configureMeasurement = [](auto& measurement, uint16_t avgSize, uint16_t statSize, uint16_t effResSize) {
        measurement.measured.SetWindowSize(avgSize);
        measurement.Statistics.SetWindowSize(statSize);
        measurement.effectiveResolution.SetWindowSize(effResSize);
    };

    constexpr uint16_t MAX_NO_OF_AVG = 64; // Set appropriate value
    configureMeasurement(PowerSupply.Voltage, MAX_NO_OF_AVG, 4096, 32);
    configureMeasurement(PowerSupply.Current, MAX_NO_OF_AVG, 4096, 32);
    configureMeasurement(PowerSupply.Power, MAX_NO_OF_AVG, 1, 1);

    // Setup encoders
    PowerSupply.Voltage.SetEncoderPins(5, 4, VoltageEnc);
    PowerSupply.Voltage.setLock(false);
    PowerSupply.Voltage.restrict = "%+07.3f";

    PowerSupply.Current.SetEncoderPins(7, 6, CurrentEnc);
    PowerSupply.Current.setLock(false);
    PowerSupply.Current.restrict = "%+07.3f";

    PowerSupply.Power.enableSetting(false);
    PowerSupply.Power.restrict = "%+08.3f";

    // Adjust power display alignment
    lv_obj_align(PowerSupply.Power.label_measureValue, LV_ALIGN_DEFAULT, -10, 161);
    lv_obj_align(PowerSupply.Power.label_unit, LV_ALIGN_DEFAULT, 130, 159);

    // Create charts
    GraphChart(PowerSupply.page[1], 22, -6);
    StatsChart(PowerSupply.page[0], 22, -6);

    // Initialize power supply
    PowerSupply.turn(SWITCH::ON);

    Tabs::setDefaultPage(2);
    textarea(PowerSupply.page[2]);
    Tabs::setCurrentPage(2);

    Utility_tabview(PowerSupply.page[3]);

    // Load settings
    PowerSupply.LoadSetting();

    // Setup settings menu
    SettingMenu(PowerSupply.page[4]);

    Serial.println("Power Supply Setup Completed.");
}

void setupCalibPage() {
    // Create Calibration page
    voltage_current_calibration();

    lv_obj_add_flag(voltageCurrentCalibration, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(myTextBox, LV_OBJ_FLAG_HIDDEN);
}

void initializeTouch() {
    // Touchscreen calibration data
    uint16_t calData[5] = {366, 3445, 310, 3406, 1}; // Rotation {3}
    tft.setTouch(calData);
    Serial.println("Touch Screen Calibrated.");
}

void setupLVGL() {
    lv_init();
    init_display();
    init_touch();

    String LVGL_Arduino = "Hello ESP32! V";
    LVGL_Arduino += lv_version_major();
    LVGL_Arduino += ".";
    LVGL_Arduino += lv_version_minor();
    LVGL_Arduino += ".";
    LVGL_Arduino += lv_version_patch();
    Serial.println(LVGL_Arduino);
}

void setupDMA() {
#if DMA
    // Define sprite color depth
    spr[0].setColorDepth(COLOR_DEPTH);
    spr[1].setColorDepth(COLOR_DEPTH);

    // Create sprites
    buf[0] = (lv_color_t*)spr[0].createSprite(IWIDTH, IHEIGHT);
    buf[1] = (lv_color_t*)spr[1].createSprite(IWIDTH, IHEIGHT);

    // Initialize DMA
    tft.initDMA();
    lv_disp_draw_buf_init(&draw_buf, buf[0], buf[1], IWIDTH * IHEIGHT);
#else
    // Initialize display buffer
    lv_disp_draw_buf_init(&draw_buf, buf[0], buf[1], screenWidth * 10);
#endif
}

void setupTasks() {
    Serial.println("Tasks Setup Completed.");
}

void setupPreferences() {
    Preferences preferences;
    preferences.begin("param", false);
    preferences.putUShort("pi", 314);
    unsigned short readBack = preferences.getUShort("pi", 0);
    Serial.println(readBack == 314 ? "\nPreferences Memory: OK" : "\nPreferences Memory: NOT OK");
    preferences.end();
}

void setupADC() {
    // Calibration data based on MAC address
    PowerSupply.CalBank = {
        {"7C:9E:BD:4D:C7:08", {0.005000, 121, 32.7503, 3353431}, {0.0000, 124955, 3.000, 1746856}}
    };
    PowerSupply.setupADC(9, ADCPinISR, &Wire);

    Serial.println("\nADC Setup & Calibration Completed.");
}

void setupDAC() {
    PowerSupply.setupDAC(0x41);
    PowerSupply.DAC.writeAndPowerAll(DAC_VOLTAGE, 0);

    Serial.println("\nDAC Setup Completed.");
}

void createTasks() {
    xTaskCreatePinnedToCore(Task_BarGraph, "Task1", 8000, NULL, 0, &Task1, 0);
    xTaskCreatePinnedToCore(Task_ADC, "Voltage & Current ADC", 12000, NULL, 1, &Task_adc, 0);

    Serial.println("\nReal-time tasks created and pinned to cores.");
}

void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
    tft.startWrite();

#if DMA
    tft.pushImageDMA(area->x1, area->y1, area->x2 - area->x1 + 1, area->y2 - area->y1 + 1, (uint16_t*)&color_p->full);
    tft.dmaWait(); // Wait for DMA to complete
#else
    uint32_t w = area->x2 - area->x1 + 1;
    uint32_t h = area->y2 - area->y1 + 1;
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.pushColors((uint16_t*)&color_p->full, w * h, true);
#endif

    tft.endWrite();
    lv_disp_flush_ready(disp);
}

void init_display() {
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    disp_drv.hor_res = screenWidth;
    disp_drv.ver_res = screenHeight;
    lv_disp_drv_register(&disp_drv);

    lv_disp = lv_disp_get_default();
}
