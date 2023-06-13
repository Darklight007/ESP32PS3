// #ifndef CONFIG_H
// #define CONFIG_H

#include <Arduino.h>
#include <Keypad.h>
#include <Keypad_MC17.h> // I2C i/o library for Keypad
#include <Wire.h>        // I2C library for Keypad_MC17
#include <functional>
// #include "ADC.h"
#include <ADS1219.h>
#include "device.h"

static void IRAM_ATTR VoltageEnc();
static void IRAM_ATTR CurrentEnc();
static void IRAM_ATTR ADCPinISR();

volatile bool adcDataReady{false};

Calibration StoreData("", {0}, {0});
 

extern Device PowerSupply;

TFT_eSPI tft = TFT_eSPI();

const byte lcdBacklightPin = 26; // 26 corresponds to GPIO26 which is pin number 10
// setting PWM properties
const int freq = 5000;
const byte lcdBacklightChannel = 1;
const byte resolution = 8;

static bool Utility_tabview_flag = false;
static bool Setting_tabview_flag = false;
lv_obj_t *Utility;

/*******************************************
 * tone(int frequency, int duration)
 * this is non blocking tone generator
 *********************************************/
// uint8_t
// buzzerOn = true,
// avgMode = true,
// calMode = true,
// toneIsOn = true;
// digitalRead_19 = true;

extern bool buzzerSound;
static TaskHandle_t Task_adc,Task1;
bool ismyTextHiddenChange = false;
 


// #include "myMenu.h"
// /*Change to your screen resolution*/
static const uint16_t screenWidth{320};
static const uint16_t screenHeight{240};

#define I2CADDR 0x20 // address of MCP23017 chip on I2C bus

const byte ROWS = 6; // eight rows
const byte COLS = 6; // eight columns
static char keys[ROWS][COLS] = {
    {'H', 'M', 'j', 'k', 'l', 'T'},
    {'7', '8', '9', '<', '-', '^'},
    {'4', '5', '6', 'V', 'v', '>'},
    {'1', '2', '3', 'A', 'a', ' '},
    {'0', '.', 'm', 'E', '+', ' '},
    {'W', 'X', 'Y', 'Z', 'O', ' '}};

static byte rowPins[ROWS] = {0, 1, 2, 3, 4, 5};     // connect to the row pinouts of the kpd
static byte colPins[COLS] = {8, 9, 10, 11, 12, 13}; // connect to the column pinouts of the kpd

// modify constructor for I2C i/o
static Keypad_MC17 kpd(makeKeymap(keys), rowPins, colPins, ROWS, COLS, I2CADDR);

#define DMA true
#define COLOR_DEPTH 16

// Maximum is 181x181 (64Kbytes) for DMA -  restricted by processor design
#define IWIDTH 320
#define IHEIGHT 240

#define LV_USE_LOG 1
#if LV_USE_LOG != 0

/* Serial debugging */
void my_print(const char *buf)
{
  Serial.printf(buf);
  Serial.flush();
}
#endif

/*Initialize the display*/
// static lv_disp_drv_t disp_drv;
static lv_disp_draw_buf_t draw_buf;

// lv_style_t style_unit;
//  lv_style_t style_set;

// static void IRAM_ATTR ADCPinISR()
// {
//     adcDataReady = true;

//     if (PowerSupply.adc.busyChannel == VOLTAGE) /*&& ads.checkDataReady()*/
//         {
//             static double v;
//             PowerSupply.Voltage.rawValue = PowerSupply.adc.readConversion();
//             adcDataReady = false;
//             PowerSupply.adc.startConversion(CURRENT);
//             // Reference:https://training.ti.com/ti-precision-labs-adcs-understanding-and-calibrating-offset-and-gain-adc-systems
//             //  Time @ 05:41
//             v = (PowerSupply.Voltage.rawValue - PowerSupply.Voltage.calib_b) / PowerSupply.Voltage.calib_m;
//             PowerSupply.Voltage.measureUpdate(v); //  enob(rs[0].StandardDeviation())

//             // PowerSupply.Voltage.hist[v];

//             // lv_chart_set_next_value(graph.chart, graph.serV, PowerSupply.Voltage.measured.value * 1000.0);
//         }

// }
 