#include "SetupHandlers.h"

extern Device PowerSupply; // Assuming PowerSupply is a class or struct

extern TFT_eSPI tft; // Global TFT instance
extern Preferences preferences;

extern Keypad_MC17 kpd;
extern lv_disp_draw_buf_t draw_buf;
extern const uint16_t screenWidth;
extern lv_color_t *buf[2];
extern void btn_event_cb(lv_event_t *e);

extern void IRAM_ATTR VoltageEnc(void *arg);
extern void IRAM_ATTR CurrentEnc(void *arg);
extern void IRAM_ATTR ADCPinISR(void *arg);
extern void GraphChart(lv_obj_t *parent, lv_coord_t x, lv_coord_t y);
extern void StatsChart(lv_obj_t *parent, lv_coord_t x, lv_coord_t y);
extern void textarea(lv_obj_t *parent);
extern void Utility_tabview(lv_obj_t *parent);
extern void SettingMenu(lv_obj_t *parent);
extern void voltage_current_calibration(void);

extern lv_obj_t *menu;
extern lv_obj_t *voltageCurrentCalibration;
extern lv_obj_t *myTextBox;

extern lv_obj_t *dd_calibration;
extern lv_obj_t *lbl_voltageCalib_m;
extern lv_obj_t *lbl_voltageCalib_b;
extern lv_obj_t *lbl_rawCode;
extern lv_obj_t *lbl_calibratedValue;
extern lv_obj_t *lbl_rawAVG_;
extern lv_obj_t *lbl_calibValueAVG_;
extern lv_obj_t *lbl_ER_;


void initializeSerial()
{
    Serial.begin(115200);
    delay(1000); // Stabilize serial connection
    Serial.println("Serial Connection Initialized.");
}

void initialMemory()
{
    Serial.printf("Total heap: %d\n", ESP.getHeapSize());
    Serial.printf("Free heap: %d\n", ESP.getFreeHeap());
    Serial.printf("Total PSRAM: %d\n", ESP.getPsramSize());
    Serial.printf("Free PSRAM: %d\n", ESP.getFreePsram());
}

void initializeI2C()
{
    // setupOTA("Power Supply");
    Wire.begin(18, 15, 1000000UL);
    // Wire.setClock(1250000);
    Serial.println("I2C Initialized.");
    if (0)
    {
        kpd.begin(); // now does not starts wire library
        kpd.setDebounceTime(30);
        kpd.setHoldTime(700);
    }
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
    // pinMode(PowerSupply.CCCVPin, INPUT);
    PowerSupply.setupDisplay(lv_scr_act());
    PowerSupply.setupPages("Stats", "Graph", "Main", "Utility", "Setting");

    // Setup up on/off touch switch on page 3
    PowerSupply.setupSwitch(PowerSupply.page[2], 0, 240, 160, btn_event_cb);

    // Setup voltage,current and power for page 3
    PowerSupply.Voltage.setup(PowerSupply.page[2], "V-Set:", -14, -8, "V", 32.7675, 5.0, -12, 2000);
    PowerSupply.Current.setup(PowerSupply.page[2], "I-Set:", -14, 74, "A", 6.5535, 1.0, -5.5 /*miliampere*/, 2000);
    PowerSupply.Power.setup(PowerSupply.page[2], "", -14, 144, "W", 0, 0, 0, 0, &dseg_b_24, &Tauri_R_28);
    Serial.println("Power Supply Setup Completed.");

    // Setup voltage and current's encoder pins
    PowerSupply.Voltage.SetEncoderPins(4, 5, VoltageEnc);
    PowerSupply.Voltage.setLock(false);
    PowerSupply.Voltage.restrict = "%+07.3f";

    //   // PowerSupply.Voltage.adjValue
    PowerSupply.Current.SetEncoderPins(6, 7, CurrentEnc);
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

    // Create Calibration page
    // voltage_current_calibration();

    // lv_obj_add_flag(voltageCurrentCalibration, LV_OBJ_FLAG_HIDDEN);
    // lv_obj_add_flag(myTextBox, LV_OBJ_FLAG_HIDDEN);

    // myTone(NOTE_A4, 100);
    // Serial.println("\nSetup done");
    // PowerSupply.turn(SWITCH::ON);

    // Preferences fmemory;
    // fmemory.begin("param", false);
    // fmemory.putUShort("pi", 314);
    // Serial.printf("\nPreferences Memory test get:%i", fmemory.getUShort("pi", 0));
    // fmemory.end();
      myTone(NOTE_A4, 100);
  Serial.println("\nSetup done");
  PowerSupply.turn(SWITCH::ON);
    Preferences fmemory;
  fmemory.begin("param", false);
  fmemory.putUShort("pi", 314);
  Serial.printf("\nPreferences Memory test get:%i", fmemory.getUShort("pi", 0));
  fmemory.end();
}

void initializeTouch()
{
    // Touchscreen initialization code
    /*Set the touchscreen calibration data*/
    uint16_t calData[5] = {366, 3445, 310, 3406, 1}; // Rotation {3}
    tft.setTouch(calData);
    Serial.println("Touch Screen Calibrated.");
}

void setupTasks()
{
    // Task creation for FreeRTOS
    Serial.println("Tasks Setup Completed.");
}

void setupPreferences()
{
    preferences.begin("param", false);
    preferences.putUShort("pi", 314);
    unsigned short readBack = preferences.getUShort("pi", 0);
    Serial.printf("Preferences Memory test get: %d\n", readBack);
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

    Serial.println("ADC Calibration Completed.");
}

void setupDAC()
{
    PowerSupply.setupADC(9, ADCPinISR);
    PowerSupply.setupDAC(0x41);

    // LTC2655 Device::DAC(0x41);
}

void createTasks()
{
    // Example task creation
    // xTaskCreatePinnedToCore(Task1code, "Task1", 4000 + 4500, NULL, 0, &Task1, 0);
    // xTaskCreatePinnedToCore(Task_ADC, "Voltage & Current ADC", 2600, NULL, 1, &Task_adc, 0);
    // Add other task creations as needed
    Serial.println("Real-time tasks created and pinned to cores.");
}