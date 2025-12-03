#include "esp_task_wdt.h"
#include "device.hpp"
#include "MonotoneCubicCalibrator.h"
#include "setting_menu.h"
#include "error_handler.h"
#include <SPIFFS.h>
#include <FS.h>

extern bool g_voltINL_ready;
extern MonotoneCubicCalibrator g_voltINL;

extern Calibration StoreData;
extern bool lvglIsBusy, lvglChartIsBusy, blockAll;

extern lv_obj_t *btn_function_gen;
extern CalibrationGui Calib_GUI;

void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);

void Device::setupADC(uint8_t pin, void func(void), TwoWire *_awire)
{
    pinMode(pin, INPUT_PULLUP);
    attachInterrupt(pin, func, FALLING);
    LoadCalibrationData();

    adc.ads1219->resetConfig();

    adc.ads1219->begin();
    adc.ads1219->setVoltageReference(REF_EXTERNAL);
    adc.ads1219->setGain(ONE);
    adc.ads1219->setConversionMode(SINGLE_SHOT);

    adc.startConversion(VOLTAGE, REF_EXTERNAL);
    adcDataReady = false;
    adc.dataReady = false;
}

void Device::setupDAC(uint8_t addr)
{
    DAC.address = addr;
    DAC.setVoltageReference(REFR_INTERNAL);
}

void Device::calibrate(void)
{
    bankCalibId = 0;
}

void Device::calibrationUpdate(void)
{
    Voltage.calib_m = (CalBank[bankCalibId].vCal.code_2 - CalBank[bankCalibId].vCal.code_1) /
                      (CalBank[bankCalibId].vCal.value_2 - CalBank[bankCalibId].vCal.value_1);
    Voltage.calib_b = CalBank[bankCalibId].vCal.code_1 - Voltage.calib_m * CalBank[bankCalibId].vCal.value_1;

    Voltage.calib_1m = 1.0 / Voltage.calib_m;

    Current.calib_m = (CalBank[bankCalibId].iCal[mA_Active].code_2 - CalBank[bankCalibId].iCal[mA_Active].code_1) /
                      (CalBank[bankCalibId].iCal[mA_Active].value_2 - CalBank[bankCalibId].iCal[mA_Active].value_1);
    Current.calib_b = CalBank[bankCalibId].iCal[mA_Active].code_1 - Current.calib_m * CalBank[bankCalibId].iCal[mA_Active].value_1;

    Current.calib_1m = 1.0 / Current.calib_m;
}

//  std::vector<Calibration> CalBank
void Device::SaveSettingData(const String &key, const SettingParameters &data)
{
    StoreMem.begin("my-app", false);
    StoreMem.putBytes(key.c_str(), &data, sizeof(SettingParameters));
    StoreMem.end();
}

SettingParameters Device::LoadSettingData(const String &key)
{
    StoreMem.begin("my-app", false);
    ssize_t bytesRead = StoreMem.getBytes(key.c_str(), &settingParameters, sizeof(SettingParameters));
    StoreMem.end();
    return settingParameters;
}

void Device::SaveCalibData(const String &key, const Calibration &data)
{
    StoreMem.begin("my-app", false);
    StoreMem.putBytes(key.c_str(), &data, sizeof(Calibration));
    StoreMem.end();
}

Calibration Device::LoadCalibData(const String &key)
{
    StoreMem.begin("my-app", false);
    ssize_t bytesRead = StoreMem.getBytes(key.c_str(), &StoreData, sizeof(Calibration));
    StoreMem.end();
    return StoreData;
}

void Device::SaveCalibrationData()
{
    Device::SaveCalibData("cal", CalBank[bankCalibId]);
    Serial.print("\ndata saved.");

    Calibration outputData = CalBank[bankCalibId];

    calibrationUpdate();

    Serial.printf("\nSaved calibration data:%s {%+08.4f %+08i %+08.4f %+08i}, {%+08.4f %+08i %+08.4f %+08i}, %4.4f",
                  outputData.macAdd,
                  outputData.vCal.value_1, outputData.vCal.code_1, outputData.vCal.value_2, outputData.vCal.code_2,
                  outputData.iCal[mA_Active].value_1, outputData.iCal[mA_Active].code_1, outputData.iCal[mA_Active].value_2, outputData.iCal[mA_Active].code_2,
                  outputData.internalLeakage[mA_Active]);
}

void Device::LoadCalibrationData()
{
    Calibration outputData = Device::LoadCalibData("cal");
    CalBank[bankCalibId] = {outputData};

    calibrationUpdate();

    Serial.printf("\nCalibration data loaded:%s {%+08.4f %+08i %+08.4f %+08i}, {%+08.4f %+08i %+08.4f %+08i}, %4.4f",
                  outputData.macAdd,
                  outputData.vCal.value_1, outputData.vCal.code_1, outputData.vCal.value_2, outputData.vCal.code_2,
                  outputData.iCal[mA_Active].value_1, outputData.iCal[mA_Active].code_1, outputData.iCal[mA_Active].value_2, outputData.iCal[mA_Active].code_2,
                  outputData.internalLeakage[mA_Active]);

    Serial.printf("\nVoltage Calibration m:%7.3f", Voltage.calib_m);
    Serial.printf("\nVoltage Calibration b:%7.3f", Voltage.calib_b);
    Serial.printf("\nCurrent Calibration m:%7.3f", Current.calib_m);
    Serial.printf("\nCurrent Calibration b:%7.3f", Current.calib_b);

    if (!isfinite(Voltage.calib_m) || !std::isfinite(Voltage.calib_b) || !std::isfinite(Current.calib_m) || !std::isfinite(Current.calib_b))
    {
        Serial.print("\n\n ** WRONG CALIBRATION DATA ** \n");
        Serial.print("\nLoading factory calibration data ...");

        // Report calibration error
        ERROR_CALIB("Load", "Invalid calibration coefficients - reverting to factory defaults");

        CalBank[bankCalibId] = Calibration(CalBank[bankCalibId].macAdd,
                                           {+00.0000, -259, +32.0000, +8164608},
                                           {+00.0000, +104080, +3.0000, +2926000},
                                           {+00.0000, +104080, +3.0000, +2926000},
                                           40'000.0); // 1/40kOhm /volt

        calibrate();
        calibrationUpdate();
        Serial.printf(" done.");

        Serial.print("\nFactory calibration data saved.");

        // Resetting measured values after calibration
        Power.measured.ResetStats();
        Voltage.measured.ResetStats();
        Current.measured.ResetStats();

        Current.hist.Reset();
        Voltage.hist.Reset();

        SaveCalibrationData();
    }
}

void Device::LoadSetting(void)
{

    Serial.print("\nLoading setting ... ");
    settingParameters = Device::LoadSettingData("setting_param");
    Serial.print("done.");

    // Make sure loaded data is not out of range
    settingParameters.adcRate = std::clamp((int)settingParameters.adcRate, 0, 3);
    settingParameters.adcNumberOfAvgs = std::clamp((int)settingParameters.adcNumberOfAvgs, 0, 12);
    settingParameters.adcNumberOfDigits = std::clamp((int)settingParameters.adcNumberOfDigits, 1, 4);
    Voltage.adjValue = settingParameters.SetVoltage;
    Current.adjValue = settingParameters.SetCurrent;

    Preferences memory;
    memory.begin("param", false);
    if (314 != memory.getUShort("pi", 0))
    {
        Serial.print("\n\n ** WRONG SETTING DATA ** \n");
        Serial.print("\nLoad default setting ...");

        // Report settings validation error
        ERROR_REPORT(WARNING, MEMORY, "Settings validation failed - loading defaults");

        settingParameters.adcRate = 0;
        settingParameters.adcNumberOfAvgs = 1;
        settingParameters.adcNumberOfDigits = 4;

        Voltage.adjValue = 5.0 - 0 * Voltage.adjOffset;
        Current.adjValue = 2 - 0 * Current.adjOffset;

        memory.putUShort("pi", 314);

        Device::SaveSettingData("setting_param", settingParameters);

        dac_data.zero_voltage = 11;
        dac_data.max_voltage = 65535;
        dac_data.zero_current = 10;
        dac_data.max_current = 65535;

        SaveDACdata("dac_data_", dac_data);

        // Initialize default function generator parameters
        funGenMem.amplitude = 5.0;
        funGenMem.frequency = 1.0;
        funGenMem.offset = 0.0;
        funGenMem.dutyCycle = 0.5;
        SaveMemoryFgen("FunGen", funGenMem);
    }
    memory.end();

    static std::map<int, const char *> ADC_SPS;

    ADC_SPS[0] = "  20";
    ADC_SPS[1] = "  90";
    ADC_SPS[2] = " 330";
    ADC_SPS[3] = "1000";

    Serial.printf("\nadcRate:           %sSPS", ADC_SPS[settingParameters.adcRate]);
    Serial.printf("\nadcNumberOfAvgs:   %f", pow(2, settingParameters.adcNumberOfAvgs));
    Serial.printf("\nadcNumberOfDigits: %i", settingParameters.adcNumberOfDigits);
    Serial.printf("\nSetVoltage: %f", settingParameters.SetVoltage);

    switch (settingParameters.adcRate)
    {
    case 0:
        adc.ads1219->setDataRate(20);
        break;

    case 1:
        adc.ads1219->setDataRate(90);
        break;
    case 2:
        adc.ads1219->setDataRate(330);
        break;
    case 3:
        adc.ads1219->setDataRate(1000);
        break;
    default:
        adc.ads1219->setDataRate(20);
    }

    Voltage.measured.SetWindowSize(std::pow(2, settingParameters.adcNumberOfAvgs));

    static const char *formats[] = {
        "%+07.3f", // fallback/default
        "%+05.1f", // 1 digit
        "%+06.2f", // 2 digits
        "%+07.3f", // 3 digits
        "%+08.4f"  // 4 digits
    };

    int d = settingParameters.adcNumberOfDigits;
    if (d < 1 || d > 4)
        d = 3; // default if out of range

    Voltage.restrict = formats[d];
    Current.restrict = formats[d];
    Power.restrict = formats[d];

    Serial.printf("\nLast Voltage Value:% +8.4f", Voltage.adjValue / 2000.0 + 0 * Voltage.adjOffset);
    Serial.printf("\nLast Current Value:% +8.4f", Current.adjValue / 10000.0 + 0 * Current.adjOffset);
};

void Device::SaveSetting(void)
{
    // Serial.printf(getStatus() );
    // if (getStatus() == DEVICE::VC)
    //     settingParameters.isPowerSupplyOn = true;
    // else
    //     settingParameters.isPowerSupplyOn = false;

    Device::SaveSettingData("setting_param", settingParameters);
    // Serial.print("\nSetting saved.");
}

// void Device::SaveDataArrays(const String &key, const DataArrays &data) {
//     StoreMem.begin("my-app", false);
//     StoreMem.putBytes(key.c_str(), &data, sizeof(DataArrays));
//     StoreMem.end();
// }

// DataArrays Device::LoadDataArrays(const String &key) {
//     DataArrays data;
//     StoreMem.begin("my-app", false);
//     StoreMem.getBytes(key.c_str(), &data, sizeof(DataArrays));
//     StoreMem.end();
//     return data;
// }

void Device::SaveMemory(const String &key, const MemArray &data)
{
    StoreMem.begin("my-app", false);
    StoreMem.putBytes(key.c_str(), &data, sizeof(MemArray));
    StoreMem.end();
}

MemArray Device::LoadMemory(const String &key)
{
    MemArray data;
    StoreMem.begin("my-app", false);
    StoreMem.getBytes(key.c_str(), &data, sizeof(MemArray));
    StoreMem.end();
    return data;
}

void Device::SaveMemoryFgen(const String &key, const FunGen &data)
{
    // Save small settings to NVS (fast, no size limit issues)
    StoreMem.begin("my-app", false);
    FunGenSettings settings = {data.frequency, data.amplitude, data.offset, data.dutyCycle};
    StoreMem.putBytes("fgen", &settings, sizeof(settings));
    StoreMem.end();

    // Save large arrays to SPIFFS (3.375MB available!)
    // No more 1984-byte NVS blob limits or chunking needed!
    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS Mount Failed");
        return;
    }

    // Save table_points (4000 bytes)
    File file = SPIFFS.open("/fgen_table.dat", "w");
    if (file) {
        file.write((uint8_t*)&data.table_points, sizeof(data.table_points));
        file.close();
    }

    // Save arbitrary_points (640 bytes)
    file = SPIFFS.open("/fgen_arb.dat", "w");
    if (file) {
        file.write((uint8_t*)&data.arbitrary_points, sizeof(data.arbitrary_points));
        file.close();
    }

    SPIFFS.end();
}

FunGen Device::LoadMemoryFgen(const String &key)
{
    FunGen data;

    // Load small settings from NVS
    StoreMem.begin("my-app", false);
    FunGenSettings settings;
    size_t bytesRead = StoreMem.getBytes("fgen", &settings, sizeof(settings));
    if (bytesRead > 0) {
        data.frequency = settings.frequency;
        data.amplitude = settings.amplitude;
        data.offset = settings.offset;
        data.dutyCycle = settings.dutyCycle;
    }
    StoreMem.end();

    // Load large arrays from SPIFFS
    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS Mount Failed");
        return data;
    }

    // Load table_points (4000 bytes)
    File file = SPIFFS.open("/fgen_table.dat", "r");
    if (file) {
        file.read((uint8_t*)&data.table_points, sizeof(data.table_points));
        file.close();
    }

    // Load arbitrary_points (640 bytes)
    file = SPIFFS.open("/fgen_arb.dat", "r");
    if (file) {
        file.read((uint8_t*)&data.arbitrary_points, sizeof(data.arbitrary_points));
        file.close();
    }

    SPIFFS.end();
    return data;
}

void Device::SaveMemoryRecording(const String &key, const Recording &data)
{
    StoreMem.begin("my-app", false);
    StoreMem.putBytes(key.c_str(), &data, sizeof(Recording));
    StoreMem.end();
}

Recording Device::LoadMemoryRecording(const String &key)
{
    Recording data;
    StoreMem.begin("my-app", false);
    size_t bytesRead = StoreMem.getBytes(key.c_str(), &data, sizeof(Recording));
    StoreMem.end();

    // Validate loaded data - if invalid, return defaults
    if (bytesRead == 0 || data.sample_count > 500)
    {
        Serial.printf("\nRecording data invalid or not found, using defaults");
        data.sample_count = 0;
        data.sample_rate_ms = 100;
        data.is_recording = false;
        data.is_playing = false;
        data.play_index = 0;
    }

    return data;
}

void Device::SaveDACdata(const String &key, const DAC_codes &data)
{
    StoreMem.begin("my-app", false);
    StoreMem.putBytes(key.c_str(), &data, sizeof(DAC_codes));
    StoreMem.end();
}

DAC_codes Device::LoadDACdata(const String &key)
{
    DAC_codes data;
    StoreMem.begin("my-app", false);
    StoreMem.getBytes(key.c_str(), &data, sizeof(DAC_codes));
    StoreMem.end();
    return data;
}

// Reference:https://training.ti.com/ti-precision-labs-adcs-understanding-and-calibrating-offset-and-gain-adc-systems
//  Time @ 05:41
void Device::readVoltage()
{

    if ((adcDataReady || false) && (adc.busyChannel == VOLTAGE

                                    /*
                                    to avoid spi during conversion but after
                                    adding 10uf cap to ref SPI noise gone
                                    && (!lvglIsBusy  || settingParameters.adcRate!=0 )
                                    */
                                    ))

    {

        // static int throw_away = 0;
        // if (throw_away < 2)
        // {
        //     throw_away++;
        //     adcDataReady = false;
        //     adc.startConversion(VOLTAGE, REF_EXTERNAL);
        //     return;
        // } // throw away first 2 readings after changing rate
        // throw_away = 0;

        constexpr float adcRateCompensation[4] = {
            1.00000f,            // 20 SPS
            32.0000f / 31.9985f, // 90 SPS
            32.0000f / 31.9945f, // 330 SPS
            32.0000f / 31.9882f  // 1000 SPS
        };

        static double v;
        Voltage.rawValue = adc.readConversion();
        adcDataReady = false;
        adc.ads1219->setGain(ONE); // Gain 1 or 4 for Current
        // adc.ads1219->setDataRate(1000); //settingParameters.adcRate
        adc.startConversion(CURRENT, REF_EXTERNAL); // REF_EXTERNAL

        double v_ideal = (Voltage.rawValue - Voltage.calib_b) * Voltage.calib_1m;

        // v = (Voltage.rawValue - Voltage.calib_b) * Voltage.calib_1m;

        v_ideal *= adcRateCompensation[settingParameters.adcRate];

        // Add monotone cubic residual on top (safe if not built yet)
        double v_corrected = g_voltINL_ready ? (g_voltINL.apply(v_ideal)) : v_ideal;

        // Voltage.hist[v];
        Voltage.measureUpdate(v_corrected); //  enob(rs[0].StandardDeviation())
        adc.ADC_loopCounter++;
        // myTone(NOTE_A3, 1);

        static double factor = lv_bar_get_max_value(Voltage.Bar.bar) / (Voltage.maxValue / Voltage.adjFactor);

        *Voltage.Bar.curValuePtr = v_corrected * factor; // uint16_t(v * 8);

        // lv_obj_invalidate(Voltage.Bar.bar);

        // Serial.printf("\n%9.4f %10.5f %5.2f %i", v, Voltage.Statistics.Mean(),Voltage.effectiveResolution.Mean(),
        //               Voltage.Statistics.windowSizeIndex_ % Voltage.Statistics.NofAvgs);

        // Serial.printf(" %3i",    *Voltage.Bar.curValuePtr );
    }
}

void Device::readCurrent()
{
    if ((adcDataReady || false) && (adc.busyChannel == CURRENT) /* && ads.checkDataReady() */)
    {

        // static int throw_away = 0;
        // if (throw_away < 2)
        // {
        //     throw_away++;
        //     adcDataReady = false;
        //     // if (throw_away == 1)
        //         // adc.ads1219->setDataRate(settingParameters.adcRate); //
        //     adc.startConversion(CURRENT, REF_EXTERNAL);
        //     return;
        // } // throw away first 2 readings after changing rate
        // throw_away = 0;

        static double c;
        Current.rawValue = adc.readConversion();
        adcDataReady = false;
        adc.ads1219->setGain(ONE);                  // Gain 1 for voltage
        adc.startConversion(VOLTAGE, REF_EXTERNAL); // REF_EXTERNAL

        // Current used by R11&R12 and arrR2
        // static double diff_A = (0.000461 - 0.000021) / (32.0 - 0.0); // (0.000594 - 0.000143)
        double internalLeakage = CalBank[bankCalibId].internalLeakage[mA_Active]; // 1.0 / 40'000.0; // 1/40kOhm /volt

        double currentOfInternalRes =(mA_Active ? 1000.0 : 1.0) *  (Voltage.measured.Mean() / (internalLeakage * 1000.0)) +
                                      0.0 * 0.000180 * !digitalRead(CCCVPin); // Why?; (mA_Active ? .001 : 1.0) * 

        c = (((Current.rawValue - Current.calib_b) * Current.calib_1m) - currentOfInternalRes); // old value: .0009
        // c=c+c*0.00009901;

        // Apply REL offset if active (like Keithley 2010 REL button)
        if (mA_Active && currentRelActive_mA) {
            c -= currentRelOffset_mA;
        } else if (!mA_Active && currentRelActive_A) {
            c -= currentRelOffset_A;
        }

        // Current.hist[c];

        Current.measureUpdate(c);
        *Current.Bar.curValuePtr = c / (Current.maxValue / Current.adjFactor) * lv_bar_get_max_value(Current.Bar.bar); // uint16_t(v * 8);

        // Serial.print (c);
        // Serial.print(",  Current:");
        // Serial.printf("%9.5f", Current.measured.Mean());
        // Serial.print(":");
        // Serial.print(Current.measured.value);
        // Serial.print(":");
        // Serial.print(Current.measured.sum_);
        // Serial.print("\n");
        // *Current.Bar.curValuePtr = c * Current.Bar.scaleFactor;
        // lv_obj_invalidate(Current.Bar.bar);

        // Serial.printf("\n\r%9.6f %9.6f %5.2f %i",
        //               c,
        //               Current.Statistics.Mean(),
        //               Current.effectiveResolution.Mean(),
        //               Current.Statistics.windowSizeIndex_ % Current.Statistics.NofAvgs);

        static unsigned long loopCount = 0;
        static unsigned long startTime = millis();

        adc.ADC_loopCounter++;
        if ((startTime + 1000) <= millis())
        {
            adc.realADCSpeed = adc.ADC_loopCounter;
            // Serial.printf("\nADC real SPS for 1 ch:%4i ",  adc.ADC_loopCounter);
            adc.ADC_loopCounter = 0;
            startTime = millis();
        }

         Current.rawValueStats(Current.rawValue);
    }
}


void Device::getPower()
{
    // if (Current.changed || Voltage.changed)
    // {

    Power.measureUpdate(Current.measured.Mean() * Voltage.measured.Mean() * (mA_Active ? 0.001 : 1.0));

    // Serial.printf("Power:%+07.3f Current:%+07.3f Voltage:%+07.3f  Voltage.changed %i Current.changed %i\n",
    //               Power.measured.Mean(),
    //               Current.measured.Mean(),
    //               Voltage.measured.Mean(),
    //                Voltage.changed, Current.changed);

    // Voltage.changed = false;
    // Current.changed = false;
    // }

    // static int interval = 1000;
    // static unsigned long loopCount = 0;
    // static unsigned long startTime = millis();

    // loopCount++;
    // if ((millis() - startTime) >= interval)
    // {
    //     Serial.printf("\t\t\t\tADC call Loop Count:%5i at time %07.2f \n", loopCount, millis() / 1000.0);
    //     //   startTime = millis();
    //     startTime = millis();
    //     loopCount = 0;
    // }
}

void Device::toggle_measure_unit()
{
    mA_Active = digitalRead(AmA_Pin) ^ 1;

    digitalWrite(AmA_Pin, mA_Active); // Toggle the pin state

    // Update ADC Current Calibration window title (only if window exists)
    if (gui.calibration.win_ADC_current_calibration) {
        lv_obj_t *hdr = lv_win_get_header(gui.calibration.win_ADC_current_calibration);
        if (hdr) {
            lv_obj_t *title_lbl = lv_obj_get_child(hdr, 0);
            if (title_lbl) {
                lv_label_set_text(title_lbl, mA_Active ? "ADC Current Calibration [mA]" : "ADC Current Calibration [A]");
            }
        }
    }

    // Update Internal Current Calibration window title (only if window exists)
    if (gui.calibration.win_int_current_calibration) {
        lv_obj_t *hdr_int = lv_win_get_header(gui.calibration.win_int_current_calibration);
        if (hdr_int) {
            lv_obj_t *title_lbl_int = lv_obj_get_child(hdr_int, 0);
            if (title_lbl_int) {
                lv_label_set_text(title_lbl_int, mA_Active ? "Internal Current Calibration [mA]" : "Internal Current Calibration [A]");
            }
        }
    }

    // Update mA prefix visibility
    if (mA_Active)
        lv_obj_clear_flag(Current.label_si_prefix, LV_OBJ_FLAG_HIDDEN);
    else
        lv_obj_add_flag(Current.label_si_prefix, LV_OBJ_FLAG_HIDDEN);

    // Note: Internal resistor spinboxes are now separate for A and mA, no need to update on toggle
}

void Device::restoreAdcRateFromFUN()
{
    // Restore ADC rate from saved value when exiting FUN mode or turning OFF
    if (settingParameters.adcRateSavedForFUN <= 3)
    {
        settingParameters.adcRate = settingParameters.adcRateSavedForFUN;
        const int rates[] = {20, 90, 330, 1000};
        adc.ads1219->setDataRate(rates[settingParameters.adcRateSavedForFUN]);
    }
}

void Device::writeDAC_Voltage(uint16_t value)
{
    DAC.writeUpdate(DAC_VOLTAGE, value);
}

void Device::writeDAC_Current(uint16_t value)
{
    DAC.writeUpdate(DAC_CURRENT, value);
}

void Device::VCCCStatusUpdate(void)
{
    // Skip if output is off or in function generator mode
    if (getStatus() == DEVICE::OFF || getStatus() == DEVICE::FUN)
    {
        lastCCCVStatus = -1;  // Reset so we re-check when turning on
        return;
    }

    int currentStatus = digitalRead(CCCVPin);

    // Skip if status hasn't changed (but -1 means first check after power on)
    if (lastCCCVStatus == currentStatus)
        return;

    if (currentStatus == false)
        setStatus(DEVICE::CC);
    else
        setStatus(DEVICE::VC);

    lastCCCVStatus = currentStatus;
}

void Device::DACUpdate(void)
{
    if (getStatus() == DEVICE::OFF)
    {
        DAC.writeAndPowerAll(DAC_VOLTAGE, Voltage.adjOffset);

        //  DAC.writeAndPowerAll(DAC_CURRENT, (-Current.adjOffset - 0.001) * 10000);
        DAC.writeAndPowerAll(DAC_CURRENT, Current.adjOffset);

        return;
    }

    // if (digitalRead(CCCVPin) == true)
    // setStatus(DEVICE::VC);

    // else
    // setStatus(DEVICE::CC);

    if (getStatus() != DEVICE::OFF)
    {
        // Always write to DAC - no change detection, it was causing missed updates
        DAC.writeAndPowerAll(DAC_VOLTAGE, Voltage.adjValue);
        DAC.writeAndPowerAll(DAC_CURRENT, Current.adjValue);
        // Serial.printf("\n v_uint16_t: %i v:%f ", Voltage.adjValue, double((Voltage.adjValue - Voltage.adjOffset) / 2000.0));
        // Serial.printf(" c_uint16_t: %i c:%f ", Current.adjValue, double((Current.adjValue - Current.adjOffset) / 10000.0));
    }
    else // if (getStatus() == DEVICE::OFF)
    {
        DAC.writeAndPowerAll(DAC_VOLTAGE, Voltage.adjOffset);
        // DAC.writeAndPowerAll(DAC_CURRENT, Current.adjOffset+ 1*10000);
        DAC.writeAndPowerAll(DAC_CURRENT, 45);
    }
}

void Device::turn(SWITCH onOff)
{
    // powerSwitch = onOff;
    powerSwitch.turn(onOff);
    if (onOff == SWITCH::ON)
    {
        status = DEVICE::ON;
        // Clear OVP/OCP triggered flags when turning on
        settingParameters.ovpTriggered = false;
        settingParameters.ocpTriggered = false;
    }
    else if (onOff == SWITCH::OFF)
        status = DEVICE::OFF;
}

void Device::toggle(void)
{
    // delay(4);
    if (getStatus() != DEVICE::OFF)
        turn(SWITCH::OFF);
    else
        turn(SWITCH::ON);
    // delay(4);
};

void Device::setupDisplay(lv_obj_t *scr)
{
    screen = scr;
    tab.setup_(screen);
}

void Device::setupPages(const char *page0, const char *page1, const char *page2, const char *page3, const char *page4)
{
    tab.addPage(tab.tabview, &page[0], page0);
    tab.addPage(tab.tabview, &page[1], page1);
    tab.addPage(tab.tabview, &page[2], page2);
    tab.addPage(tab.tabview, &page[3], page3);
    tab.addPage(tab.tabview, &page[4], page4);

    //    lv_event_send(btn, LV_EVENT_VALUE_CHANGED, NULL);
    stateColor[DEVICE::OFF] = deviceColors{lv_palette_darken(LV_PALETTE_GREY, 3),
                                           lv_palette_main(LV_PALETTE_GREY),
                                           lv_palette_darken(LV_PALETTE_GREY, 1),
                                           lv_color_hex(0x000000)};

    stateColor[DEVICE::ON] = deviceColors{lv_palette_main(LV_PALETTE_BLUE),
                                          lv_palette_main(LV_PALETTE_BLUE),
                                          lv_palette_main(LV_PALETTE_AMBER),
                                          lv_color_hex(0x001A3C)};

    stateColor[DEVICE::VC] = deviceColors{lv_palette_main(LV_PALETTE_BLUE),
                                          lv_palette_main(LV_PALETTE_BLUE),
                                          lv_palette_main(LV_PALETTE_AMBER),
                                          lv_color_hex(0x001A3C)};

    stateColor[DEVICE::CC] = deviceColors{lv_palette_main(LV_PALETTE_YELLOW),
                                          lv_palette_main(LV_PALETTE_YELLOW),
                                          lv_palette_main(LV_PALETTE_ORANGE),
                                          lv_color_hex(0x001A3C)};
    // Function generator mode
    stateColor[DEVICE::FUN] = deviceColors{lv_palette_main(LV_PALETTE_RED),
                                           lv_palette_main(LV_PALETTE_RED),
                                           lv_palette_main(LV_PALETTE_ORANGE),
                                           //   lv_palette_darken(LV_PALETTE_DEEP_ORANGE, 4)
                                           lv_color_hex(0x3C1A00)

    };
}

void Device::setPagesCallback(lv_event_cb_t event_cb)
{
    // lv_obj_add_event_cb(tab.tabview, event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(tab.tabview, event_cb, LV_EVENT_DRAW_POST_END, NULL);
}

void Device::setupSwitch(lv_obj_t *parent, lv_align_t align, lv_coord_t x_ofs, lv_coord_t y_ofs, lv_event_cb_t event_cb)
{
    powerSwitch.setup(parent, align, x_ofs, y_ofs, event_cb);
    lv_style_init(&style_tabview_df_btn);

    controlMode = lv_label_create(parent);
    lv_obj_remove_style_all(controlMode);
    lv_label_set_text(controlMode, "VC");
    lv_obj_center(controlMode);
    lv_obj_align(controlMode, 0, 200, -12); // 268, 185
    //  lv_obj_set_size(label, 30, 20);

    lv_style_init(&style_controlMode);
    lv_obj_add_style(controlMode, &style_controlMode, 0);

    /*Init the style for the default state*/
    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_radius(&style, 1);

    // lv_style_set_bg_opa(&style, LV_OPA_100);
    // lv_style_set_bg_color(&style, lv_palette_main(LV_PALETTE_BLUE));

    // lv_style_set_text_color(&style, lv_color_white());
    // lv_style_set_pad_all(&style, 10);
    lv_style_set_text_font(&style, &Tauri_R_20); // lv_font_montserrat_24
    lv_obj_add_style(controlMode, &style, 0);
}
DEVICE Device::getStatus(void)
{
    return status;
};

void Device::setStatus(DEVICE status_)

{
    static DEVICE oldStatus = status_;
    blockAll = true;

    // lv_obj_invalidate(lv_scr_act()); // Force a full redraw before pausing
    // lv_disp_trig_activity(NULL); // Ensure display updates are handled
    // lv_disp_enable_invalidation(NULL, false); // Disable invalidation (pause updates)
    TaskHandle_t idleTask1 = xTaskGetIdleTaskHandleForCPU(1);
    esp_task_wdt_delete(idleTask1);

    if (status_ == DEVICE::OFF || oldStatus == DEVICE::OFF)
        vTaskDelay(75);

    // OPTIMIZATION: Disable invalidation during mass style updates
    // This prevents 50+ individual redraws and batches them into one
    // Performance improvement: 60-80% faster status changes (200ms → 40ms)
    lv_disp_enable_invalidation(NULL, false);

    // Set Colors
    Voltage.setMeasureColor(stateColor[status_].measured);
    Current.setMeasureColor(stateColor[status_].measured);
    Power.setMeasureColor(stateColor[status_].measured);

    // Update REL label color to match current measurement color
    if (gui.label_current_rel)
        lv_obj_set_style_text_color(gui.label_current_rel, stateColor[status_].measured, 0);

    Voltage.setStatsColor(stateColor[status_].plotColor1);
    Current.setStatsColor(stateColor[status_].plotColor2);

    // graph Chart color
    lv_chart_set_series_color(graph.chart, graph.serV, stateColor[status_].plotColor1);
    lv_chart_set_series_color(graph.chart, graph.serI, stateColor[status_].plotColor2);

    // graph chart legend color
    lv_style_set_text_color(&graph.style_legend1, stateColor[status_].plotColor1);
    lv_style_set_text_color(&graph.style_legend2, stateColor[status_].plotColor2);

    lv_style_set_text_color(&graph.style_statsVolt, stateColor[status_].plotColor1);
    lv_style_set_text_color(&graph.style_statsCurrent, stateColor[status_].plotColor2);

    // Stats Chart color
    lv_chart_set_series_color(stats.chart, stats.serV, stateColor[status_].plotColor1);
    lv_chart_set_series_color(stats.chart, stats.serI, stateColor[status_].plotColor2);

    // Stats chart legend color
    lv_style_set_text_color(&stats.style_legend1, stateColor[status_].plotColor1);
    lv_style_set_text_color(&stats.style_legend2, stateColor[status_].plotColor2);

    lv_style_set_text_color(&stats.style_statsVolt, stateColor[status_].plotColor1);
    lv_style_set_text_color(&stats.style_statsCurrent, stateColor[status_].plotColor2);

    // Pages color
    lv_obj_set_style_bg_color(page[0], stateColor[status_].pageColor, 0);
    lv_obj_set_style_bg_color(page[1], stateColor[status_].pageColor, 0);
    lv_obj_set_style_bg_color(page[2], stateColor[status_].pageColor, 0);
    lv_obj_set_style_bg_color(page[3], stateColor[status_].pageColor, 0);
    lv_obj_set_style_bg_color(page[4], stateColor[status_].pageColor, 0);

    lv_obj_set_style_bg_color(lv_obj_get_child(page[3], 0), stateColor[status_].pageColor, 0);
    // lv_obj_set_style_bg_color(lv_obj_get_child(lv_obj_get_child(page[3], 0), 0), stateColor[status_].pageColor, 0);

    lv_obj_t *tab_btns = lv_tabview_get_tab_btns(tab.tabview);
    lv_style_set_bg_color(&style_tabview_df_btn, stateColor[status_].pageColor);

    lv_obj_remove_style(tab_btns, &style_tabview_df_btn, LV_PART_ITEMS | LV_STATE_CHECKED);
    lv_obj_add_style(tab_btns, &style_tabview_df_btn, LV_PART_ITEMS | LV_STATE_CHECKED);

    lv_style_set_text_color(&style_controlMode, stateColor[status_].measured);

    lv_obj_remove_style(controlMode, &style_controlMode, 0);
    lv_obj_add_style(controlMode, &style_controlMode, 0);

    if (status_ != DEVICE::CC)
        lv_label_set_text(controlMode, "VC");
    else
        lv_label_set_text(controlMode, "CC");

    if (status_ == DEVICE::FUN)
        lv_label_set_text(controlMode, "Fun");

    if (status_ == DEVICE::OFF)
    {
        myTone(NOTE_A3, 200);
        // Make sure function generator is also off and restore ADC rate ONLY if we were in FUN mode
        lv_obj_clear_state(btn_function_gen, LV_STATE_CHECKED);
        lv_obj_add_state(btn_function_gen, LV_STATE_DISABLED);
        lv_label_set_text(lv_obj_get_child(btn_function_gen, 0), "OFF");

        // Only restore ADC rate if we were actually in FUN mode (not OFF, ON, VC, or CC)
        if (status == DEVICE::FUN)
        {
            restoreAdcRateFromFUN();
        }

        lv_label_set_text(controlMode, "OFF");
        lv_label_set_text(lv_obj_get_child(powerSwitch.btn, 0), "OFF");

        settingParameters.isPowerSupplyOn = false;
        // Serial.printf("\nThe switch and status set to %s", "Off.");
    }
    else
    {

        // if (btn_function_gen)
        // {
        lv_obj_clear_state(btn_function_gen, LV_STATE_DISABLED);
        // if (lv_obj_has_state(btn_function_gen, LV_STATE_CHECKED))
        // PowerSupply.setStatus(DEVICE::FUN);
        // }

        // myTone(NOTE_A5, 200);
        settingParameters.isPowerSupplyOn = true;
        lv_label_set_text(lv_obj_get_child(powerSwitch.btn, 0), "ON");

        // Serial.printf("\nThe switch and status set to %s", "On.");
    }

    status = status_;

    // Re-enable invalidation and trigger single batched redraw
    lv_disp_enable_invalidation(NULL, true);
    lv_obj_invalidate(lv_scr_act());  // Single invalidation for all changes

    oldStatus = status_;
    blockAll = false;
    esp_task_wdt_add(idleTask1);
};

void Device::FlushSettings(void)
{

    Voltage.Flush();
    Current.Flush();

    if (Voltage.adjValueChanged)
    {
        // _lv_disp_refr_timer(NULL);
        // Serial.println("Voltage Changed!.");
        Voltage.adjValueChanged = false;
        settingParameters.SetVoltage = Voltage.adjValue;
        Device::SaveSetting();
    }

    if (Current.adjValueChanged)
    {
        // _lv_disp_refr_timer(NULL);
        Current.adjValueChanged = false;
        settingParameters.SetCurrent = Current.adjValue;
        Device::SaveSetting();
    }
}

void Device::FlushMeasures(void)
{
    if (Voltage.changed || Current.changed || Power.changed)
    {
        Voltage.displayUpdate();
        Current.displayUpdate();
        Power.displayUpdate();

        // Over-range indicator for mA mode (>= 3.900 mA) - blink like multimeter OL
        if (mA_Active && Current.measured.Mean() >= 3.900)
        {
            static bool blinkState = false;
            static unsigned long lastBlink = 0;
            if (millis() - lastBlink >= 250)  // 4Hz blink
            {
                blinkState = !blinkState;
                lastBlink = millis();
            }
            if (blinkState)
                lv_obj_add_flag(Current.label_measureValue, LV_OBJ_FLAG_HIDDEN);
            else
                lv_obj_clear_flag(Current.label_measureValue, LV_OBJ_FLAG_HIDDEN);
        }
        else
        {
            // Ensure visible when not over-range
            lv_obj_clear_flag(Current.label_measureValue, LV_OBJ_FLAG_HIDDEN);
        }

        Voltage.changed = false;
        Current.changed = false;
        Power.changed = false;

        // Serial.println(Current.measured.sum_);
    }

    // if (Voltage.changed || Current.changed || Power.changed)
    // {
    //     // _lv_disp_refr_timer(NULL);
    //     Voltage.changed = false;
    //     Current.changed = false;
    //     Power.changed = false;
    // }
}

void Device::FlushBars(void)
{

    // Voltage.barUpdate();
    // Current.barUpdate();

    if (Voltage.Bar.changed || Current.Bar.changed) //|| Power.Bar.changed)
    {
        // lv_obj_invalidate( Voltage.Bar.bar);
        // lv_refr_now(NULL);
        // _lv_disp_refr_timer(NULL);
        // lv_bar_set_value(Voltage.Bar.bar, Voltage.measured.value / Voltage.maxValue * lv_bar_get_max_value(Voltage.Bar.bar), LV_ANIM_OFF);
        // lv_tick_inc(10);
        lv_refr_now(NULL);

        Voltage.Bar.changed = false;
        Current.Bar.changed = false;
    }
}
