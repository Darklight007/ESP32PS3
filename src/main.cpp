
// FreeRTOS includes
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <EEPROM.h>
#include <lvgl.h>
#include "myFonts.h"
#include <TFT_eSPI.h>
#include "demos/lv_demos.h"
#include "examples/lv_examples.h"
#include "OTA.h"
#include <limits.h>
#include <float.h>
#include <map>
#include "buzzer.h"
#include "config.hpp"
#include "tabs.h"
#include "Preferences.h"
#include "DispObject.h"
#include "device.h"
#include "time.h"
#include "input_device.h"
#include <mutex>
#include <HardwareSerial.h>
#include <complex>
#include <arduinoFFT.h>

arduinoFFT FFT = arduinoFFT();
arduinoFFT FFT_i = arduinoFFT();

const uint16_t samples = 128; // This value MUST ALWAYS be a power of 2
double vReal[samples];
double vImag[samples];
bool FFTSample = false;

double vReal_i[samples];
double vImag_i[samples];
bool FFTSample_i = false;

// #define CONFIG_ESP32S3_SPIRAM_SUPPORT 1

#define EEPROM_SIZE 140
// double GlobalMemoryTest [10000];
static lv_indev_drv_t indev_drv_rotary;
lv_indev_t *my_indev;
lv_group_t *g, *labelGroup;
lv_obj_t *menu;

int8_t pos_index = 0;

lv_obj_t *voltageCurrentCalibration;
lv_obj_t *myTextBox;

static int memory;

#include "setting_menu.h"

Device PowerSupply;
lv_disp_t *lv_disp;

#include "globalFunctions.h"

/* Had to copy lvgl.h into multiple places to make compiling work
=> side by lvgl folder
=> side by root folder */

#if DMA == 1
// Create two sprites for a DMA toggle buffer
TFT_eSprite spr[2] = {TFT_eSprite(&tft), TFT_eSprite(&tft)};

// Toggle buffer selection
bool sprSel{0};

// Pointers to start of Sprites in RAM
static lv_color_t *buf[2];
#else

static lv_color_t buf[2][screenWidth * 10];
#endif

void encoder_read(lv_indev_drv_t *drv, lv_indev_data_t *data);
#define BUFFER_SIZE 655360

void setup()
{
  memory = ESP.getFreeHeap();
  void *buffer = ps_malloc(BUFFER_SIZE);
  // g = lv_group_create();
  // g2 = lv_group_create();

  // configure LED PWM functionalities
  // ledcSetup(lcdBacklightChannel, freq, resolution);
  // attach the channel to the GPIO to be controlled
  // ledcAttachPin(lcdBacklightPin, lcdBacklightChannel);
  // ledcWrite(lcdBacklightChannel, 255);

  // ledcSetup(SOUND_PWM_CHANNEL, 1000, SOUND_RESOLUTION); // Set up PWM channel
  // ledcAttachPin(BUZZER_PIN, SOUND_PWM_CHANNEL);

  // pinMode(PowerSupply.CCCVPin, INPUT);

  Serial.begin(115200); /* prepare for possible serial debug */

  while (!Serial)
  { /*wait*/
  } // for USB serial switching boards

  Serial.printf("\nTotal heap: %d", ESP.getHeapSize());
  Serial.printf("\nFree heap: %d", ESP.getFreeHeap());
  Serial.printf("\nTotal PSRAM: %d", ESP.getPsramSize());
  Serial.printf("\nFree PSRAM: %d", ESP.getFreePsram());

  if (1)
  {
    // setupOTA("Power Supply");
    Wire.begin(18, 15, 1000000UL);

    // Wire.setClock(1250000);
  }
  if (0)
  {
    kpd.begin(); // now does not starts wire library
    kpd.setDebounceTime(30);
    kpd.setHoldTime(700);
  }

  String LVGL_Arduino = "Hello ESP32! ";
  LVGL_Arduino += String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();

  // Serial.printf("\n sizeof(float):%i\n", sizeof(float));

  tft.init();         /* TFT init */
  tft.setRotation(1); /* Landscape orientation, flipped */

  lv_init();
  init_display();
  init_touch();

  Serial.println(LVGL_Arduino);

  //  Serial.print("Initializing EEPROM ... ");
  // while (!EEPROM.begin(EEPROM_SIZE))
  // {
  //   Serial.print(".");
  //   delay(1000);
  // }
  // Serial.println("done.");

  /*Set the touchscreen calibration data*/
  // For rotation(1)
  // touch_calibrate();
  // uint16_t calData[5] = {444, 3403, 285, 3436, 7};
  // uint16_t calData[5] = {379, 3377, 388, 3191, 7};
  // uint16_t calData[5] = {332, 3452, 240, 3553, 7};
  // pinMode(TOUCH_CS, INPUT);
  uint16_t calData[5] = {330, 3475, 216, 3605, 7};
  tft.setTouch(calData);

  Serial.print("Touch Screen Calibrated.");

#if DMA
  // Define sprite colour depth
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
  // Initialize `disp_buf` display buffer with the buffer(s).
  lv_disp_draw_buf_init(&draw_buf, buf[0], buf[1], screenWidth * 10);
#endif

  Serial.print("\nSetup() run on core: ");
  Serial.print(xPortGetCoreID());
  /**************************************************************************/
  // lv_style_t style_unit;
  // lv_style_init(&style_set);
  // Set default screen for PowerSupply object

  PowerSupply.setupDisplay(lv_scr_act());
  PowerSupply.setupPages("Stats", "Graph", "Main", "Utility", "Setting");

  // Setup up on/off touch switch on page 3
  PowerSupply.setupSwitch(PowerSupply.page[2], 0, 240, 160, btn_event_cb);

  // Setup voltage,current and power for page 3
  PowerSupply.Voltage.setup(PowerSupply.page[2], "V-Set:", -14, -8, "V", 32.7675, 5.0, -12, 2000);
  PowerSupply.Current.setup(PowerSupply.page[2], "I-Set:", -14, 74, "A", 6.5535, 1.0, -5.5 /*miliampere*/, 2000);
  PowerSupply.Power.setup(PowerSupply.page[2], "", -14, 144, "W", 0, 0, 0, 0, &dseg_b_24, &Tauri_R_28);

  /*******************************************************************
   * GPIOs 34 to 39 are GPIs for ESP32 – input only pins.
   * These pins don’t have internal pull-up or pull-down resistors.
   * They can’t be used as outputs, so use these pins only as inputs:
   * ESP32Encoder::useInternalWeakPullResistors = UP;
   *******************************************************************/
  // Setup voltage and current's encoder pins
  PowerSupply.Voltage.SetEncoderPins(4, 5, VoltageEnc);
  PowerSupply.Voltage.setLock(false);
  PowerSupply.Voltage.restrict = "%+07.3f";

  // PowerSupply.Voltage.adjValue

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

  /************************************************************
   *  ADC Settings
   *  When DRDY falls low, new conversion data are ready(page 23).
   *  https://www.ti.com/lit/ds/symlink/ads1219.pdf?ts=1668300375705&ref_url=https%253A%252F%252Fwww.google.com%252F
   ************************************************************/

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

  PowerSupply.setupADC(9, ADCPinISR);
  PowerSupply.setupDAC(0x41);

  // LTC2655 Device::DAC(0x41);

  // PowerSupply.LoadCalibrationData();
  // Create OVP/OCP protection
  // SetupOVP();

  // Timer for chart update
  // timerChart = timerBegin(1, 80, true);
  // timerAttachInterrupt(timerChart, &ChartUpdate, true);
  // timerAlarmWrite(timerChart, 10000, true);
  // timerAlarmEnable(timerChart);
  // timerStart(timerChart);

  // timer for adc update?
  // timerADC = timerBegin(0, 80, true);
  // timerAttachInterrupt(timerADC, &ADCinterrupt, true);
  // timerAlarmWrite(timerADC, 50000, true);
  // timerAlarmEnable(timerADC);
  // timerStart(timerADC);

  // create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
      Task1code,   /* Task function. */
      "Task1",     /* name of task. */
      4000 + 4500, /* Stack size of task */
      NULL,        /* parameter of the task */
      0,           /* priority of the task */
      &Task1,      /* Task handle to keep track of created task */
      0);          /* pin task to core 0 */

  if (1)
  {
    xTaskCreatePinnedToCore(
        Task_ADC,                         /* Task function. */
        "Voltage & Current ADC",          /* name of task. */
         2600,                             /* Stack size of task */
        NULL, /* parameter of the task */ /* (void *)&adcDataReady */
        1,                                /* priority of the task */
        &Task_adc,                        /* Task handle to keep track of created task */
        0);                               /* pin task to core 0 */
  }


  // static mutex_t lvgl_mutex;
  // void lvgl_thread(void)
  // {
  //   while (1)
  //   {
  //     mutex_lock(&lvgl_mutex);
  //     lv_task_handler();
  //     mutex_unlock(&lvgl_mutex);
  //     thread_sleep(10); /* sleep for 10 ms */
  //   }
  // }

  // Set default tab
  Tabs::setDefaultPage(2);
  textarea(PowerSupply.page[2]);
  Tabs::setCurrentPage(2);

  Utility_tabview(PowerSupply.page[3]);

  // load setting parameters for page 4
  PowerSupply.LoadSetting();

  // Setting menu on page 4
  SettingMenu(PowerSupply.page[4]);

  // Create Calibration page
  voltage_current_calibration();

  lv_obj_add_flag(voltageCurrentCalibration, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(myTextBox, LV_OBJ_FLAG_HIDDEN);

  myTone(NOTE_A4, 100);
  Serial.println("\nSetup done");
  PowerSupply.turn(SWITCH::ON);

  Preferences fmemory;
  fmemory.begin("param", false);
  fmemory.putUShort("pi", 314);
  Serial.printf("\nPreferences Memory test get:%i", fmemory.getUShort("pi", 0));
  fmemory.end();
}
static unsigned long t = 0;
static int priorityFlag = 1;

void loop()
{
static unsigned long timer_[10] = {0};
  schedule(&lv_timer_handler, 10, timer_[1]);

      int interval = 1000;
    static unsigned long loopCount = 0;
    static unsigned long startTime = 0;
    loopCount++;
    if (true && (millis() - startTime) >= interval)
    {

      Serial.printf("Loop Count:%5.0f at time %07.2f \n", loopCount * 1000.0 / interval, millis() / 1000.0);
      startTime = millis();
      loopCount = 0;
      // Tabs::previousPage();
    }

    
  // Serial.println(digitalRead(TOUCH_CS));
  // uint16_t x = 0, y = 0; // To store the touch coordinates


  // @format
  schedule(&StatusBar, 250, timer_[2]);


  // // Draw a white spot at the detected coordinates
  // if (pressed)
  // {
  //   tft.fillCircle(x, y, 2, TFT_WHITE);
  //   Serial.print("x,y = ");
  //   Serial.print(x);
  //   Serial.print(",");
  //   Serial.println(y);
  // }
  // Serial.printf("Loop check points:");
  if (ismyTextHiddenChange)
  {

    if (priorityFlag != 1 && lv_obj_has_flag(myTextBox, LV_OBJ_FLAG_HIDDEN)) // lv_obj_has_flag(myTextBox, LV_OBJ_FLAG_HIDDEN) || !
    {
      priorityFlag = 1;
      vTaskPrioritySet(Task_adc, priorityFlag);
    }

    else if (priorityFlag != 0 && !lv_obj_has_flag(myTextBox, LV_OBJ_FLAG_HIDDEN))
    {
      priorityFlag = 0;
      vTaskPrioritySet(Task_adc, priorityFlag);
    }

    // Serial.printf("\nPriority changes! %i", priorityFlag);
    ismyTextHiddenChange = false;
  }

  // if (millis() > 1600 && millis() < 1800 && !PowerSupply.settingParameters.isPowerSupplyOn) //&& PowerSupply.settingParameters.isPowerSupplyOn
  // PowerSupply.turn(SWITCH::ON);

  // schedule(&keyCheckLoop, 1, timer_[0]);
  // lv_timer_handler();
  // Serial.printf("0 ");

    schedule([]
             { PowerSupply.FlushMeasures(); },
             4*22 * PowerSupply.Voltage.measured.NofAvgs, timer_[3]); // 2*1000 /(90) SPS = 22.22 / 2*1000 /(80) SPS = 25

    PowerSupply.getPower();

    

  if (1)  {

    // schedule([]
    //          { PowerSupply.FlushMeasures(); },
    //          (1000.0 / PowerSupply.adc.realADCSpeed) * PowerSupply.Voltage.measured.NofAvgs, timer_[2]); // 2*1000 /(90) SPS = 22.22 / 2*1000 /(80) SPS = 25

    // schedule([]
    //          { PowerSupply.FlushSettings(); },
    //          50, timer_[4]);

    schedule([]
             {
//  lv_chart_refresh(PowerSupply.stats.chart);
             PowerSupply.Voltage.statUpdate();
             PowerSupply.Current.statUpdate(); },
             100, timer_[6]);

    schedule([]
             {
              /*FFT*/
              FFT.Windowing(vReal, samples, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
              FFT.Compute(vReal, vImag, samples, FFT_FORWARD);
              FFT.ComplexToMagnitude(vReal, vImag, samples);
              double peak = FFT.MajorPeak(vReal, samples, PowerSupply.adc.realADCSpeed / 2.0); // samplingFrequency  0.911854103*

              // Serial.printf("\nFFT MajorPeak:%5.1f Hz", peak);

              lv_label_set_text_fmt(label_graphMenu_VFFT, "V-FFT:%5.1f Hz", peak);

              FFT_i.Windowing(vReal_i, samples, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
              FFT_i.Compute(vReal_i, vImag_i, samples, FFT_FORWARD);
              FFT_i.ComplexToMagnitude(vReal_i, vImag_i, samples);
              double peak_i = FFT_i.MajorPeak(vReal_i, samples, PowerSupply.adc.realADCSpeed / 2.0); // samplingFrequency  0.911854103*

              lv_label_set_text_fmt(label_statMenu_VFFT, "V-FFT:%5.1f Hz", peak);
              lv_label_set_text_fmt(label_statMenu_IFFT, "I-FFT:%5.1f Hz", peak_i); },
             1000, timer_[7]);

    // schedule([]
    //          {
    //            // spi.end();
    //           //  tft.endWrite();
    //           //  tft.startWrite();
    //           //  tft.init();
    //           // ArduinoOTA.handle();
    //          },
    //          10000, timer_[5]);

    // Serial.printf("1 ");
    // PowerSupply.DACUpdate();
    // PowerSupply.FlushBars();
    // lv_refr_now(NULL);

    // int voltage_bar=PowerSupply.Voltage.measured.value *320/32.768;
    // tft.fillRect(0+voltage_bar, 90, 320-voltage_bar, 5, TFT_BLACK);
    // tft.fillRect(0, 90, voltage_bar, 5, TFT_BLUE);

    // LV_LOG_USER("Check point!");
    // LV_LOG_USER("%i",digitalRead(PowerSupply.CCCVPin));

    if (Serial.available() > 0)
    {
      String received_command = Serial.readStringUntil('\n');
      Serial.print("\nUploading .......");

      // pinMode(PowerSupply.CCCVPin, INPUT_PULLDOWN);
      // digitalWrite(PowerSupply.CCCVPin,INPUT_PULLDOWN);
      // PowerSupply.turn(SWITCH::OFF);
      // PowerSupply.Current.SetUpdate(-0.001- PowerSupply.Current.adjOffset);
    }

    static unsigned long startTime_ = millis();
    if ((startTime_ + 2000) <= millis())
    {
      // Had to add this part to prevent encoder from stop working randomly!?
      int64_t temp = PowerSupply.Voltage.encoder.getCount();
      PowerSupply.Voltage.encoder.clearCount();
      PowerSupply.Voltage.encoder.setCount(temp);
      temp = PowerSupply.Current.encoder.getCount();
      PowerSupply.Current.encoder.clearCount();
      PowerSupply.Current.encoder.setCount(temp);

      startTime_ = millis();
    }
    // Serial.printf("2 ");

    switch (Tabs::getCurrentPage())
    {

    case 4:
      if (lv_obj_is_visible(voltageCurrentCalibration))
      {
        static int32_t encoder1_last_value = 0;
        static int32_t encoder2_last_value = 0;

        if (encoder2_last_value == encoder2_value && encoder1_last_value == encoder1_value)
          break;

        static int32_t cursor_pos = 0;

        if (encoder2_last_value < encoder2_value)
          cursor_pos++;
        else if (encoder2_last_value > encoder2_value)
          cursor_pos--;

        int width = spinboxes.digit_count[spinboxes.id_index] - 1;
        cursor_pos = std::clamp(cursor_pos, 0, width);

        lv_obj_t *spinBox = lv_obj_get_child(voltageCurrentCalibration, spinboxes.current_index);

        lv_spinbox_set_cursor_pos(spinBox, 0);
        lv_spinbox_set_cursor_pos(spinBox, width - cursor_pos);

        encoder2_last_value = encoder2_value;

        /********************************************************************/
        if (encoder1_last_value < encoder1_value)
          lv_event_send(spinboxes.btn_plus[spinboxes.id_index], LV_EVENT_SHORT_CLICKED, NULL);
        else if (encoder1_last_value > encoder1_value)
          lv_event_send(spinboxes.btn_minus[spinboxes.id_index], LV_EVENT_SHORT_CLICKED, NULL);

        PowerSupply.calibrate();
        encoder1_last_value = encoder1_value;
        // SaveCalibrationData();

        // lv_label_set_text(lbl_voltageCalib_m, std::to_string(get_voltageCalib_m()).c_str());
      }

      else
      {
        static int32_t lastValue = 0;

        if (lastValue < encoder2_value)
          lastButton++;
        else if (lastValue > encoder2_value)
          lastButton--;
        else
          break;

        lastButton = std::clamp(int(lastButton), 0, 6);

        lv_obj_t *theMenu = lv_obj_get_child(lv_menu_get_cur_sidebar_page(menu), 0);

        // Serial.printf("\nlastValue:%i  encoder2_value:%i lastButton:%i", lastValue, encoder2_value, lastButton);
        lv_event_send(lv_obj_get_child(theMenu, lastButton), LV_EVENT_CLICKED, NULL);

        lastValue = encoder2_value;
      }
      break;

    case 1:
      static int32_t encoder1_last_value = 0;
      static int32_t encoder2_last_value = 0;

      // if (encoder2_last_value == encoder2_value)
      // break;
      if (encoder2_last_value != encoder2_value)
      {
        static int32_t _posX = lv_slider_get_value(lv_obj_get_child(PowerSupply.page[1], 1));

        if (encoder2_last_value < encoder2_value)
          _posX += 64;
        else if (encoder2_last_value > encoder2_value)
          _posX -= 64;

        // int32_t v = lv_slider_get_value(lv_obj_get_child(PowerSupply.graph.chart,1));
        // lv_coord_t x = lv_obj_get_scroll_x(PowerSupply.graph.chart);
        // lv_coord_t v0 = lv_chart_get_zoom_x(PowerSupply.graph.chart);
        // lv_chart_set_zoom_x(PowerSupply.graph.chart, _posX);
        // lv_obj_scroll_to_x(PowerSupply.graph.chart, 1e10, LV_ANIM_OFF);
        // Serial.println(keyChar);
        if (keyChar == 'W' && msg == " HOLD.")
        {
          _posX = std::clamp(_posX, 0, 57 * (lv_chart_get_zoom_x(PowerSupply.graph.chart) - 256) / 16);
          lv_obj_scroll_to_x(PowerSupply.graph.chart, _posX, LV_ANIM_OFF);
        }
        else
        {
          // Serial.printf("\n*************** %i", millis());
          // lv_obj_scroll_to_x(PowerSupply.graph.chart, _posX, LV_ANIM_OFF);
          _posX = std::clamp(_posX, 256, 5120);
          lv_slider_set_value(lv_obj_get_child(PowerSupply.page[1], 1), _posX, LV_ANIM_OFF);
          lv_event_send(lv_obj_get_child(PowerSupply.page[1], 1), LV_EVENT_VALUE_CHANGED, NULL);
        }

        // Serial.printf("\n lv_obj_get_scroll_x:%i %i", lv_obj_get_scroll_x(PowerSupply.graph.chart), millis());
        // Serial.printf("\n lv_chart_get_zoom_x:%i %i", lv_chart_get_zoom_x(PowerSupply.graph.chart), millis());

        encoder2_last_value = encoder2_value;
      }
      ///*********************************************

      if (encoder1_last_value != encoder1_value)
      {

        static int32_t _posY = lv_slider_get_value(lv_obj_get_child(PowerSupply.page[1], 2));

        if (encoder1_last_value < encoder1_value)
          _posY += 64;
        else if (encoder1_last_value > encoder1_value)
          _posY -= 64;

        // Serial.printf("\n keychar:%c ,msg:%s", keyChar, msg);

        if (keyChar == 'W' && msg == " HOLD.")
        {
          // keyChar = ' ';
          _posY = std::clamp(_posY, 0, int(33.5 * (lv_chart_get_zoom_y(PowerSupply.graph.chart) - 256) / 64));
          lv_obj_scroll_to_y(PowerSupply.graph.chart, _posY, LV_ANIM_OFF);
        }
        else
        {

          // lv_obj_scroll_to_x(PowerSupply.graph.chart, _pos, LV_ANIM_OFF);
          _posY = std::clamp(_posY, 256, 5120);
          lv_slider_set_value(lv_obj_get_child(PowerSupply.page[1], 2), _posY, LV_ANIM_OFF);
          lv_event_send(lv_obj_get_child(PowerSupply.page[1], 2), LV_EVENT_VALUE_CHANGED, NULL);
        }

        encoder1_last_value = encoder1_value;
      }
      // Serial.printf("\nscroll_y:%4i ", lv_obj_get_scroll_y(PowerSupply.graph.chart));
      // Serial.printf("  zoom_y:%4i ", lv_chart_get_zoom_y(PowerSupply.graph.chart));

      // Serial.printf("  scroll_x:%4i ", lv_obj_get_scroll_x(PowerSupply.graph.chart));
      // Serial.printf("  zoom_x:%4i  %4i", lv_chart_get_zoom_x(PowerSupply.graph.chart), millis());

      break;
    case 0:

      if (encoder1_last_value != encoder1_value)
      {

        static int32_t _posY = 0;

        if (encoder1_last_value < encoder1_value)
          _posY = 1;
        else if (encoder1_last_value > encoder1_value)
          _posY = -1;

        if (keyChar == 'W' && msg == " HOLD.")
        {
          if (_posY > 0)
          {
            PowerSupply.Voltage.hist.histWinMin += .025;
            PowerSupply.Voltage.hist.histWinMax += .025;

            PowerSupply.Current.hist.histWinMin += .025;
            PowerSupply.Current.hist.histWinMax += .025;
          }
          if (_posY < 0)
          {
            PowerSupply.Voltage.hist.histWinMin -= .025;
            PowerSupply.Voltage.hist.histWinMax -= .025;

            PowerSupply.Current.hist.histWinMin -= .025;
            PowerSupply.Current.hist.histWinMax -= .025;
          }
        }
        else
        {

          if (_posY > 0)
          {
            double midValue = (PowerSupply.Voltage.hist.histWinMin + PowerSupply.Voltage.hist.histWinMax) / 2;
            PowerSupply.Voltage.hist.histWinMin = PowerSupply.Voltage.hist.histWinMin + (midValue - PowerSupply.Voltage.hist.histWinMin) / 2;
            PowerSupply.Voltage.hist.histWinMax = midValue + (PowerSupply.Voltage.hist.histWinMax - midValue) / 2;

            midValue = (PowerSupply.Current.hist.histWinMin + PowerSupply.Current.hist.histWinMax) / 2;
            PowerSupply.Current.hist.histWinMin = PowerSupply.Current.hist.histWinMin + (midValue - PowerSupply.Current.hist.histWinMin) / 2;
            PowerSupply.Current.hist.histWinMax = midValue + (PowerSupply.Current.hist.histWinMax - midValue) / 2;
          }
          if (_posY < 0)
          {
            double midValue = (PowerSupply.Voltage.hist.histWinMin + PowerSupply.Voltage.hist.histWinMax) / 2;
            PowerSupply.Voltage.hist.histWinMin = midValue - (midValue - PowerSupply.Voltage.hist.histWinMin) * 2;
            PowerSupply.Voltage.hist.histWinMax = PowerSupply.Voltage.hist.histWinMax + (PowerSupply.Voltage.hist.histWinMax - midValue) * 2;

            midValue = (PowerSupply.Current.hist.histWinMin + PowerSupply.Current.hist.histWinMax) / 2;
            PowerSupply.Current.hist.histWinMin = midValue - (midValue - PowerSupply.Current.hist.histWinMin) * 2;
            PowerSupply.Current.hist.histWinMax = PowerSupply.Current.hist.histWinMax + (PowerSupply.Current.hist.histWinMax - midValue) * 2;
          }
        }

        encoder1_last_value = encoder1_value;
        PowerSupply.Voltage.hist.Reset();
        PowerSupply.Current.hist.Reset();
      }

      break;
    }


  }
  // if (g_wifiConnection)
  // ArduinoOTA.handle();
  // Serial.printf("3 \n");
  }

/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{

  tft.startWrite();

#if DMA
  tft.pushImageDMA(area->x1, area->y1, area->x2 - area->x1 + 1, area->y2 - area->y1 + 1, (uint16_t *)&color_p->full);
  sprSel = !sprSel;
  // tft.dmaWait();

#else
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);
  tft.setAddrWindow(area->x1, area->y1, w, h);
  tft.pushColors((uint16_t *)&color_p->full, w * h, true);
#endif

  tft.endWrite();

  lv_disp_flush_ready(disp);
}

// Initialize the display -------------------------------------------
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
