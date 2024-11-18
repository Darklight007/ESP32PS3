#pragma once

#include <Arduino.h>
#include <Keypad.h>
#include <Keypad_MC17.h> // I2C i/o library for Keypad
#include <Wire.h>        // I2C library for Keypad_MC17
#include <functional>
#include <ADS1219.h>
#include "device.h"
#include "FFTHandler.h"

void IRAM_ATTR VoltageEnc(void* arg);
void IRAM_ATTR CurrentEnc(void* arg);
void IRAM_ATTR ADCPinISR(void* arg);

// Global variable to manage the timer
// hw_timer_t *timer = NULL;
// volatile SemaphoreHandle_t timerSemaphore;

// // Interrupt Service Routine (ISR) called by the timer
// void IRAM_ATTR onTimer() {
//     // Give the semaphore to unblock the loop task
//     xSemaphoreGiveFromISR(timerSemaphore, NULL);
// }

// /*Change to your screen resolution*/
 uint16_t screenWidth{320};
 uint16_t screenHeight{240};




lv_disp_t *lv_disp;
Device PowerSupply;
FFTHandler V, I;
int priorityFlag = 1;
int heepMemory;

volatile bool adcDataReady{false};


Calibration StoreData("", {0}, {0});
 
// extern Device PowerSupply;

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
  // void *buffer = ps_malloc(BUFFER_SIZE);
 
  // configure LED PWM functionalities
  // ledcSetup(lcdBacklightChannel, freq, resolution);
  // attach the channel to the GPIO to be controlled
  // ledcAttachPin(lcdBacklightPin, lcdBacklightChannel);
  // ledcWrite(lcdBacklightChannel, 255);

  // ledcSetup(SOUND_PWM_CHANNEL, 1000, SOUND_RESOLUTION); // Set up PWM channel
  // ledcAttachPin(BUZZER_PIN, SOUND_PWM_CHANNEL);

  
extern bool buzzerSound;
TaskHandle_t Task_adc,Task1, LVGL;
bool ismyTextHiddenChange = false;

bool lvglIsBusy,lvglChartIsBusy,emergencyRefresh;

  bool wireConnected ;
  bool wire1Connected;
  
// #include "myMenu.h"


#define I2CADDR 0x20 // address of MCP23017 chip on I2C bus

const byte ROWS = 6; // eight rows
const byte COLS = 6; // eight columns
static char keys[ROWS][COLS] = {
    {'H', 'M', 'j', 'k', 'l', 'T'},
    {'7', '8', '9', '<', '-', '^'},
    {'4', '5', '6', 'V', 'v', '>'},
    {'1', '2', '3', 'A', 'a', 'p'},
    {'0', '.', 'm', 'E', '+', 'q'},
    {'W', 'X', 'Y', 'Z', 'O', 'r'}};

static byte rowPins[ROWS] = {0, 1, 2, 3, 4, 5};     // connect to the row pinouts of the kpd
static byte colPins[COLS] = {8, 9, 10, 11, 12, 13}; // connect to the column pinouts of the kpd

// modify constructor for I2C i/o
Keypad_MC17 kpd(makeKeymap(keys), rowPins, colPins, ROWS, COLS, I2CADDR);

#define DMA true

#if DMA == 1
// Create two sprites for a DMA toggle buffer
TFT_eSprite spr[2] = {TFT_eSprite(&tft), TFT_eSprite(&tft)};

// Toggle buffer selection
bool sprSel{0};

// Pointers to start of Sprites in RAM
lv_color_t *buf[2];
#else

static lv_color_t buf[2][screenWidth * 10];
#endif

#define COLOR_DEPTH 16

// Maximum is 181x181 (64Kbytes) for DMA -  restricted by processor design
#define IWIDTH 320
#define IHEIGHT 240

#define LV_USE_LOG 1
#if LV_USE_LOG != 0


lv_disp_draw_buf_t draw_buf;

/* Serial debugging */
void my_print(const char *buf)
{
  Serial.printf(buf);
  Serial.flush();
}



#endif


