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

// ===== Keypad Matrix Layout =====
// Physical keypad button mappings (6 rows x 6 columns)
//
// Function Key Legend (non-numeric keys):
// Row 0: H=Home/Restart, M=Settings, j=Stats-reset, k/l=Page nav, T=Unit/REL toggle
// Row 1: <=Backspace/Delete, -=Decrease 0.1V/A (Page2)/Zoom out (Page0), ^=Lock voltage
// Row 2: V/v=Voltage show/input, >=Lock current
// Row 3: A/a=Current show/input, p=(reserved)
// Row 4: m=Utility/Save, E=Textarea toggle, +=Increase 0.1V/A (Page2)/Zoom in (Page0), q=(reserved)
// Row 5: W/X/Y=Step size (fine/med/coarse), Z=Cycle averaging, O=Output on/off, r=(reserved)
//
static const char keys[ROWS][COLS] = {
    {'H','M','j','k','l','T'},  // Row 0: Home, Settings, Reset stats, Page prev/next, Toggle unit/REL
    {'7','8','9','<','-','^'},  // Row 1: Numbers 7-9, Delete, Zoom out, Lock voltage
    {'4','5','6','V','v','>'},  // Row 2: Numbers 4-6, Voltage display/input, Lock current
    {'1','2','3','A','a','p'},  // Row 3: Numbers 1-3, Current display/input, (reserved)
    {'0','.','m','E','+','q'},  // Row 4: Zero, Decimal, Utility/Save, Textarea, Zoom in, (reserved)
    {'W','X','Y','Z','O','r'}   // Row 5: Step fine/med/coarse, Avg cycle, Output, (reserved)
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
