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

// EEPROM and Device configurations
#include <Preferences.h>

// Hardware specific
#include <HardwareSerial.h>

// TFT & LVGL includes
#include <lvgl.h>
// #include "demos/lv_demos.h"
// #include <../lib/TFT_eSPI/Processors/TFT_eSPI_ESP32_S3.h>
#include <TFT_eSPI.h>

// Custom components
#include "myFonts.h"
#include "OTA.h"
#include "buzzer.h"
#include "config.hpp"
#include "tabs.h"
#include "DispObject.h"
#include "device.h"
#include "input_device.h"
#include <Keypad_MC17.h> 
#include "FFTHandler.h"
#include "setting_menu.h"
#include "globalFunctions.h"
#include "SetupHandlers.h"

lv_disp_t *lv_disp;
Device PowerSupply;

// FFTHandler V, I; // Signal Processing

void setup()
{
  memory = ESP.getFreeHeap();

  initializeSerial();
  initialMemory();
  initializeI2C();
  initializeDisplay();
  initializeTouch();
  
  String LVGL_Arduino = "Hello ESP32! ";
  LVGL_Arduino += String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();
  Serial.println(LVGL_Arduino);

  lv_init();
  init_display();
  init_touch();

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

  // lv_demo_music();
  // lv_demo_benchmark();
  // lv_demo_stress();
  // return;

  /**************************************************************************/
setupPowerSupply();
setupADC();
setupDAC();

   // Create OVP/OCP protection
  // SetupOVP();


  // create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
      Task1code, /* Task function. */
      "Task1",   /* name of task. */
      4000,      /* Stack size of task */
      NULL,      /* parameter of the task */
      0,         /* priority of the task */
      &Task1,    /* Task handle to keep track of created task */
      0);        /* pin task to core 0 */

  xTaskCreatePinnedToCore(
      Task_ADC,                         /* Task function. */
      "Voltage & Current ADC",          /* name of task. */
      2600,                             /* Stack size of task */
      NULL, /* parameter of the task */ /* (void *)&adcDataReady */
      1,                                /* priority of the task */
      &Task_adc,                        /* Task handle to keep track of created task */
      0);                               /* pin task to core 0 */

  // Set default tab
  // Tabs::setDefaultPage(2);
  // textarea(PowerSupply.page[2]);
  // Tabs::setCurrentPage(2);

  // Utility_tabview(PowerSupply.page[3]);

  // load setting parameters for page 4
  // PowerSupply.LoadSetting();

  // Setting menu on page 4
  // SettingMenu(PowerSupply.page[4]);

  // Create Calibration page
  voltage_current_calibration();

  lv_obj_add_flag(voltageCurrentCalibration, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(myTextBox, LV_OBJ_FLAG_HIDDEN);

  // myTone(NOTE_A4, 100);
  // Serial.println("\nSetup done");
  // PowerSupply.turn(SWITCH::ON);


  // Preferences fmemory;
  // fmemory.begin("param", false);
  // fmemory.putUShort("pi", 314);
  // Serial.printf("\nPreferences Memory test get:%i", fmemory.getUShort("pi", 0));
  // fmemory.end();
}

static unsigned long t = 0;
static int priorityFlag = 1;
//  bool lvglIsBusy=0;

void loop()
{
  static unsigned long timer_[10] = {0};
  // if(!lvglIsBusy)

  schedule([]
           {lvglIsBusy=1;
            lv_timer_handler();
            lvglIsBusy=0; },
           50, timer_[1]);

  // lv_timer_handler();
  // delay(1);
  // return;

  int interval = 1000;
  static unsigned long loopCount = 0;
  static unsigned long startTime = 0;
  loopCount++;
  if (true && (millis() - startTime) >= interval)
  {
    Serial.printf("Loop Count:%5.0f at time %07.2f \n", loopCount * 1000.0 / interval, millis() / 1000.0);
    // Serial.printf("DMA: %i",tft.DMA_Enabled);
    startTime = millis();
    loopCount = 0;
    // Tabs::previousPage();
  }

  // Serial.println(digitalRead(TOUCH_CS));
  // uint16_t x = 0, y = 0; // To store the touch coordinates

  // @format
  schedule(&StatusBar, 250, timer_[2]);

  if (ismyTextHiddenChange && false)
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
           4 * 22 * PowerSupply.Voltage.measured.NofAvgs, timer_[3]); // 2*1000 /(90) SPS = 22.22 / 2*1000 /(80) SPS = 25

  PowerSupply.getPower();

  if (1)
  {

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
              /*FFT_v*/

              V.computeFFT( PowerSupply.adc.realADCSpeed / 2.0);
              I.computeFFT( PowerSupply.adc.realADCSpeed / 2.0);


              lv_label_set_text_fmt(label_graphMenu_VFFT, "V-FFT_v:%5.1f Hz",V.peak);
              lv_label_set_text_fmt(label_graphMenu_IFFT, "I-FFT_v:%5.1f Hz",I.peak);

              lv_label_set_text_fmt(label_statMenu_VFFT, "V-FFT_v:%5.1f Hz", V.peak);
              lv_label_set_text_fmt(label_statMenu_IFFT, "I-FFT_i:%5.1f Hz", I.peak); }, 1000, timer_[7]);

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
  // sprSel = !sprSel;
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
