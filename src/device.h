#ifndef DEVICE_H
#define DEVICE_H

// #include "gLobalVariables.h"
#include <vector>
#include <lvgl.h>
#include <map>
#include <DispObject.h>
#include <tabs.h>
#include <ADS1219.h>
#include <WiFi.h>
#include <functional>
#include <Preferences.h>
#include <EEPROM.h>
#include "LTC2655.h"

#define VOLTAGE 1
#define CURRENT 3
#define DAC_VOLTAGE CHANNEL_D
#define DAC_CURRENT CHANNEL_B

// #pragma pack(push, 1)
struct DataArrays
{
    double doubles[20];
    bool bools[20];
};
// #pragma pack(pop)

// #pragma pack(push, 1)
struct mem
{
    uint16_t voltage;
    uint16_t current;
    bool V_isLocked;
    bool I_isLocked;
};

struct MemArray
{
    mem memory[10];
};

struct FunGen
{
    double frequency;
    double amplitude;
    double offset;
    double dutyCycle;
    double table_points[100][2];
    double arbitrary_points[40][2];
};
struct DAC_codes
{
    int32_t zero_voltage;
    int32_t max_voltage;
    int32_t zero_current;
    int32_t max_current;
};

// extern FunGen funGen; // Declaration

// #pragma pack(pop)

extern lv_disp_t *lv_disp;
extern volatile bool adcDataReady;

extern bool blockAll;
// extern volatile bool dataReady;

static void IRAM_ATTR ADCPinISR()
{
    adcDataReady = true;
    // dataReady = true;
    // Device::readVoltage();
    // Device::readCurrent();
}

struct SettingParameters
{
    uint8_t adcRate;
    uint16_t adcNumberOfAvgs;
    uint8_t adcNumberOfDigits;
    uint16_t SetVoltage;
    uint16_t SetCurrent;
    bool buzzer = false;
    bool isPowerSupplyOn = true;
};

struct GUI
{
    lv_obj_t *slider_Avgs = nullptr;
    lv_obj_t *setting_menu = nullptr;
    lv_obj_t *win_ADC_voltage_calibration= nullptr;
    lv_obj_t *win_ADC_current_calibration= nullptr;
    lv_obj_t *win_DAC_calibration= nullptr;
    lv_obj_t *textarea_set_value = nullptr;
};

struct Graph_
{
public:
    lv_obj_t *chart;
    lv_chart_series_t *serV;
    lv_chart_series_t *serI;
    lv_chart_series_t *serT;
    lv_style_t style_legend1;
    lv_style_t style_legend2;
    lv_style_t style_statsVolt;
    lv_style_t style_statsCurrent;
    int y_min = 0.0;
    int y_max = 30.0;
};
struct Stats_
{
public:
    lv_obj_t *chart;
    lv_chart_series_t *serV;
    lv_chart_series_t *serI;
    lv_chart_series_t *serT;
    lv_style_t style_legend1;
    lv_style_t style_legend2;
    lv_style_t style_statsVolt;
    lv_style_t style_statsCurrent;
};
struct deviceColors
{
    lv_color_t measured;
    lv_color_t plotColor1;
    lv_color_t plotColor2;
    lv_color_t pageColor;
};

enum class DEVICE : int
{
    OFF = 0x0000,
    ON = 0x0001,
    VC = 0x0002,
    CC = 0x0004,
    FUN = 0x0005,
    INFO = 0x0008,

    OVP = 0x0010,
    OCP = 0x0020,
    OTP = 0x0040,

    WARNING = 0x0080,
    ERROR = 0x0100,
    FAILURE = 0x0200,

    SAFEMODE = 0x0400,
};

enum class SWITCH
{
    OFF = 0x00,
    ON = 0x01,
};

struct Switch
{
public:
    SWITCH status = {SWITCH::OFF};

    lv_obj_t *label;
    lv_obj_t *btn; // Front push button switch = lv_btn_create(lv_scr_act());

    void setup(lv_obj_t *parent, lv_align_t align, lv_coord_t x_ofs, lv_coord_t y_ofs, lv_event_cb_t event_cb)
    {

        btn = lv_btn_create(parent);

        lv_obj_add_event_cb(btn, event_cb, LV_EVENT_VALUE_CHANGED, NULL);
        lv_obj_align(btn, align, x_ofs, y_ofs);
        lv_obj_add_flag(btn, LV_OBJ_FLAG_CHECKABLE);

        lv_obj_set_style_radius(btn, 2, LV_PART_MAIN);
        lv_obj_set_size(btn, 54, 30);

        label = lv_label_create(btn);
        lv_label_set_text(label, "---");
        lv_obj_center(label);

        static lv_style_t style_btn_toggled;
        lv_style_init(&style_btn_toggled);
        lv_style_set_text_font(&style_btn_toggled, &lv_font_montserrat_12);
        lv_style_set_text_color(&style_btn_toggled, lv_palette_lighten(LV_PALETTE_RED, 4));

        lv_style_set_bg_color(&style_btn_toggled, lv_palette_main(LV_PALETTE_BLUE));
        lv_style_set_shadow_width(&style_btn_toggled, 20);
        lv_style_set_shadow_spread(&style_btn_toggled, 2);
        lv_style_set_shadow_color(&style_btn_toggled, lv_palette_main(LV_PALETTE_BLUE));
        lv_style_set_border_width(&style_btn_toggled, 2);
        lv_style_set_border_color(&style_btn_toggled, lv_palette_darken(LV_PALETTE_GREY, 4));

        lv_obj_remove_style(btn, &style_btn_toggled, LV_STATE_DEFAULT);

        lv_obj_add_style(btn, &style_btn_toggled, LV_STATE_CHECKED);

        static lv_style_t style_btn_toggledOff;
        lv_style_init(&style_btn_toggledOff);
        lv_style_set_bg_color(&style_btn_toggledOff, lv_palette_darken(LV_PALETTE_GREY, 4));

        lv_style_set_shadow_width(&style_btn_toggledOff, 0);
        lv_style_set_shadow_spread(&style_btn_toggledOff, 0);
        lv_style_set_text_font(&style_btn_toggledOff, &lv_font_montserrat_12);
        lv_style_set_text_color(&style_btn_toggledOff, lv_palette_darken(LV_PALETTE_GREY, 1));
        lv_style_set_border_width(&style_btn_toggledOff, 2);
        lv_style_set_border_color(&style_btn_toggledOff, lv_palette_darken(LV_PALETTE_GREY, 3));

        lv_obj_remove_style(btn, &style_btn_toggledOff, LV_STATE_DEFAULT);
        lv_obj_add_style(btn, &style_btn_toggledOff, LV_STATE_DEFAULT);
    }
    void turn(SWITCH onOff)
    {
        // powerSwitch = onOff;
        if (onOff == SWITCH::ON)
        {
            lv_obj_add_state(btn, LV_STATE_CHECKED);
            status = SWITCH::ON;
        }
        else if (onOff == SWITCH::OFF)
        {
            lv_obj_clear_state(btn, LV_STATE_CHECKED);
            status = SWITCH::OFF;
        }
        lv_event_send(btn, LV_EVENT_VALUE_CHANGED, NULL);
    }
};

struct calibPoints
{
    double value_1; // Real Voltage
    int code_1;     // Code for the Real Voltage
    double value_2; // Real Current
    int code_2;     // Code for the Real Current

    // uint16_t value1EEPROMAddress;
    // uint16_t code1EEPROMAddress;
    // uint16_t value2EEPROMAddress;
    // uint16_t code2EEPROMAddress;
};

class Calibration
{
public:
    String macAdd;
    calibPoints vCal;
    calibPoints iCal;
    Calibration(
        String t_macAdd,
        calibPoints t_vcalib,
        calibPoints t_icalib) : macAdd(t_macAdd),
                                vCal(t_vcalib),
                                iCal(t_icalib)
    {
    }

    ~Calibration() {};
};

class ADC
{
public:
    // ADS1219 ADC(int dataReady, uint8_t addrs);
    ADS1219 *ads1219;
    int busyChannel{0};
    int drdy{-1};
    uint8_t addr{0x40};
    uint8_t config;
    uint16_t realADCSpeed{0};
    unsigned long ADC_loopCounter{0};
    TwoWire *_wire;
    volatile bool dataReady{false};

    // ADC(int drdy, uint8_t addr = 0x40)
    ADC()
    {
        ads1219 = new ADS1219(drdy, addr);
        _wire = &Wire1;
        // ads1219->setVoltageReference(REF_EXTERNAL);
    }

    void startConversion(int channel, adsRef_t vref = REF_INTERNAL, adsGain_t gain = ONE)
    {

        // ads1219->setVoltageReference(REF_INTERNAL);

        // ads1219->setVoltageReference(vref);
        // ads1219->setGain(gain);

        // if (channel == VOLTAGE)
        //     // ads1219->readDifferential_0_1();
        //     ads1219->readSingleEnded(1);
        // else if (channel == CURRENT)
        //     ads1219->readDifferential_2_3();
        //     // ads1219->readSingleEnded(3);

        ads1219->readSingleEnded(channel);
        busyChannel = channel;
    }

    int address = 0x40;
    int32_t data32;
    long readConversion()
    {
        _wire->beginTransmission(addr);
        _wire->write(0x10);
        _wire->endTransmission();
        _wire->requestFrom((uint8_t)addr, (uint8_t)3);
        data32 = _wire->read();
        data32 <<= 8;
        data32 |= _wire->read();
        data32 <<= 8;
        data32 |= _wire->read();
        return (data32 << 8) >> 8;
        // return ads1219->readSingleEnded(channel); // readConversionResult();
    }

    bool checkDataReady(void)
    {

        _wire->beginTransmission(addr);
        _wire->write(36);
        _wire->endTransmission();
        _wire->requestFrom((uint8_t)addr, (uint8_t)1);
        uint8_t drdy = _wire->read();

        // uint8_t drdy = ads1219->readRegister(36);
        if (drdy == 224)
            return 1;
        else
            return 0;

        // return (drdy>>7) && !(drdy&1);
    }
};

class Device
{

public:
    // lv_obj_t * screen =  lv_obj_create(NULL);
    lv_obj_t *screen;

    DispObjects Voltage;
    DispObjects Current;
    DispObjects Power;

    Switch powerSwitch;
    DEVICE status;
    lv_obj_t *controlMode; // Voltage control or Current control
    lv_style_t style_controlMode;
    lv_style_t style_tabview_df_btn;
    Tabs tab;
    Tabs utility_tab;
    lv_obj_t *page[5];
    std::map<DEVICE, deviceColors> stateColor;

    ADC adc;
    LTC2655 DAC;

    std::vector<Calibration> CalBank;
    int8_t bankCalibId;
    Graph_ graph;
    Stats_ stats;
    const byte CCCVPin = 12;

    SettingParameters settingParameters;
    GUI gui;
    mem memory[10];

    DAC_codes dac_data;

    FunGen funGenMem;
    FunGen funGenArbitraryMem;

    /***************
     * @param pin MCU interrupt pin for ADC drdy
     * @param func interrupt routine
     ****************/
    void setupADC(uint8_t pin, void func(void), TwoWire *_awire);
    void setupDAC(uint8_t);
    void calibrate(void);
    void calibrationUpdate(void);
    void readVoltage(void);
    void readCurrent(void);
    void writeDAC_Voltage(uint16_t value);
    void writeDAC_Current(uint16_t value);
    void VCCCStatusUpdate(void);
    void DACUpdate(void);
    void getPower(void);
    void turn(SWITCH onOff);
    void toggle(void);
    void setupDisplay(lv_obj_t *scr);
    void setupPages(const char *page0, const char *page1, const char *page2, const char *page3, const char *page4);
    void setupSwitch(lv_obj_t *parent, lv_align_t align, lv_coord_t x_ofs, lv_coord_t y_ofs, lv_event_cb_t event_cb);
    void setPagesCallback(lv_event_cb_t event_cb);
    DEVICE getStatus(void);
    void setStatus(DEVICE status_);
    void FlushSettings(void);
    void FlushMeasures(void);
    void FlushBars(void);
    void SaveCalibrationData(void);
    void LoadCalibrationData(void);
    void LoadSetting(void);
    void SaveSetting(void);

    void SaveCalibData(const String &key, const Calibration &data);
    Calibration LoadCalibData(const String &key);
    Preferences StoreMem;

    void SaveSettingData(const String &key, const SettingParameters &data);
    SettingParameters LoadSettingData(const String &key);

    // void SaveDataArrays(const String &key, const DataArrays &data);
    void SaveMemory(const String &key, const MemArray &data);
    // DataArrays LoadDataArrays(const String &key);
    MemArray LoadMemory(const String &key);

    void SaveMemoryFgen(const String &key, const FunGen &data);
    FunGen LoadMemoryFgen(const String &key);

    void SaveDACdata(const String &key, const DAC_codes &data);
    DAC_codes LoadDACdata(const String &key);

    void ResetStats(void)
    {
        Power.measured.ResetStats();
        Voltage.measured.ResetStats();
        Current.measured.ResetStats();

        Voltage.Statistics.ResetStats();
        Current.Statistics.ResetStats();

        Voltage.effectiveResolution.ResetStats();
        Current.effectiveResolution.ResetStats();

        Voltage.Bar.changed = true;
        Current.Bar.changed = true;

        Current.hist.Reset();
        Voltage.hist.Reset();
    }

    bool eepromWriteFlag = false;
    template <class T>
    int EEPROMwrite(int ee, const T &value)
    {
        const byte *p = (const byte *)(const void *)&value;
        unsigned int i;
        for (i = 0; i < sizeof(value); i++)
            EEPROM.write(ee++, *p++);
        eepromWriteFlag = true;
        // EEPROM.commit();
        return i;
    }

    template <class T>
    int EEPROMread(int ee, T &value)
    {
        byte *p = (byte *)(void *)&value;
        unsigned int i;
        for (i = 0; i < sizeof(value); i++)
            *p++ = EEPROM.read(ee++);
        return i;
    }
};

#endif // DEVICE_H
