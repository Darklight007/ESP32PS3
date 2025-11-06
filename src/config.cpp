#include "config.hpp"

#include <TFT_eSPI.h>
#include <Keypad.h>
#include <Keypad_MC17.h>
#include <Wire.h>

#include "device.hpp"
#include "FFTHandler.h"

// ===== Screen =====
uint16_t screenWidth  = 320;
uint16_t screenHeight = 240;

lv_disp_t*         lv_disp = nullptr;
lv_disp_draw_buf_t draw_buf;

// ===== Application singletons / globals =====
Device     PowerSupply;
FFTHandler V;
FFTHandler I;

int  priorityFlag = 1;
int  heepMemory   = 0;

volatile bool adcDataReady = false;

// Init your Calibration as needed by your ctor
Calibration StoreData("", {0}, {0},{0},0);

// TFT & keypad
TFT_eSPI tft;  // default construct

// Keypad matrix & pins are TU-local; only kpd is exported
static const char keys[ROWS][COLS] = {
    {'H','M','j','k','l','T'},
    {'7','8','9','<','-','^'},
    {'4','5','6','V','v','>'},
    {'1','2','3','A','a','p'},
    {'0','.','m','E','+','q'},
    {'W','X','Y','Z','O','r'}
};
static const uint8_t rowPins[ROWS] = {0,1,2,3,4,5};
static const uint8_t colPins[COLS] = {8,9,10,11,12,13};

Keypad_MC17 kpd(makeKeymap(keys), (uint8_t*)rowPins, (uint8_t*)colPins,
                ROWS, COLS, I2CADDR);

// Flags and handles
TaskHandle_t Task_adc = nullptr, Task1 = nullptr;

bool ismyTextHiddenChange = false;

bool lvglIsBusy = false;
bool lvglChartIsBusy = false;
bool emergencyRefresh = false;
bool blockAll = false;

bool wireConnected  = false;
bool wire1Connected = false;

lv_obj_t* Utility = nullptr;
bool Utility_tabview_flag = false;
bool Setting_tabview_flag = false;

// ===== DMA / buffer/sprite setup =====
#if DMA
TFT_eSprite spr[2] = { TFT_eSprite(&tft), TFT_eSprite(&tft) };
bool        sprSel = false;
lv_color_t* buf[2] = { nullptr, nullptr };  // allocate later when you know sizes
#else
// If you really want a static 2D buffer, make it compile-time sized:
lv_color_t buf[2][IWIDTH * 10];  // 10 lines worth
#endif

// ===== Logging helper =====
void my_print(const char *cstr) {
  Serial.printf("%s", cstr);
  Serial.flush();
}
