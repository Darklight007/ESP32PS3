#include "esp_task_wdt.h"
#include "device.hpp"
#include "MonotoneCubicCalibrator.h"
#include "setting_menu.h"
#include "error_handler.h"
#include <SPIFFS.h>
#include <FS.h>

extern bool g_voltINL_ready;
extern MonotoneCubicCalibrator g_voltINL;
// extern bool g_voltINL_ZC_ready;
// extern MonotoneCubicCalibrator g_voltINL_ZC;

extern Calibration StoreData;
extern bool lvglIsBusy, lvglChartIsBusy, blockAll;

extern lv_obj_t *btn_function_gen;
extern CalibrationGui Calib_GUI;

void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);

void Device::setupADC(uint8_t pin, void func(void), TwoWire *_awire)
{
    ESP_LOGI("ADC", "=== Starting ADC setup ===");
    ESP_LOGI("ADC", "Setting up DRDY interrupt on pin %d", pin);
    pinMode(pin, INPUT_PULLUP);
    attachInterrupt(pin, func, FALLING);
    LoadCalibrationData();

    ESP_LOGI("ADC", "Resetting ADS1219 config");
    adc.ads1219->resetConfig();

    // Call begin() - needed to initialize ADS1219 library internal state
    // Wire1.begin() was already called in initializeI2C() with correct pins/speed
    // This will just trigger "Bus already started" warning but won't reset pins
    ESP_LOGI("ADC", "Calling ADS1219 begin()");
    adc.ads1219->begin();

    ESP_LOGI("ADC", "Configuring ADS1219: external ref, gain=1, single-shot");
    adc.ads1219->setVoltageReference(REF_EXTERNAL);
    adc.ads1219->setGain(ONE);
    adc.ads1219->setConversionMode(SINGLE_SHOT);

    ESP_LOGI("ADC", "Starting first conversion");
    adc.startConversion(VOLTAGE, REF_EXTERNAL);
    adcDataReady = false;
    adc.dataReady = false;
    ESP_LOGI("ADC", "=== ADC setup complete ===");
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
    // CRITICAL: Validate array bounds before accessing calibration data
    if (CalBank.empty() || bankCalibId < 0 || bankCalibId >= (int8_t)CalBank.size())
    {
        Serial.printf("\nERROR in calibrationUpdate: Invalid bankCalibId=%d, CalBank.size()=%d",
                     bankCalibId, CalBank.size());
        return;
    }

    if (mA_Active < 0 || mA_Active > 1)
    {
        Serial.printf("\nERROR in calibrationUpdate: Invalid mA_Active=%d", mA_Active);
        return;
    }

    Voltage.calib_m = (CalBank[bankCalibId].vCal.code_2 - CalBank[bankCalibId].vCal.code_1) /
                      (CalBank[bankCalibId].vCal.value_2 - CalBank[bankCalibId].vCal.value_1);
    Voltage.calib_b = CalBank[bankCalibId].vCal.code_1 - Voltage.calib_m * CalBank[bankCalibId].vCal.value_1;

    Voltage.calib_1m = 1.0 / Voltage.calib_m;

    Current.calib_m = (CalBank[bankCalibId].iCal[mA_Active].code_2 - CalBank[bankCalibId].iCal[mA_Active].code_1) /
                      (CalBank[bankCalibId].iCal[mA_Active].value_2 - CalBank[bankCalibId].iCal[mA_Active].value_1);
    Current.calib_b = CalBank[bankCalibId].iCal[mA_Active].code_1 - Current.calib_m * CalBank[bankCalibId].iCal[mA_Active].value_1;

    Current.calib_1m = 1.0 / Current.calib_m;

    // Update current ADC full-scale range for correct ENOB calculation
    // A mode: 6.5536A, mA mode: 0.0065536A (1000x smaller)
    Current.adc_maxValue = mA_Active ? 0.0065536 : 6.5536;

    // Reset statistics so stale A-mode samples don't corrupt mA-mode ER (and vice versa)
    Current.Statistics.ResetStats();
    Current.effectiveResolution.ResetStats();
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
    // CRITICAL: Validate array bounds before saving
    if (CalBank.empty() || bankCalibId < 0 || bankCalibId >= (int8_t)CalBank.size())
    {
        Serial.printf("\nERROR in SaveCalibrationData: Invalid bankCalibId=%d, CalBank.size()=%d",
                     bankCalibId, CalBank.size());
        return;
    }

    if (mA_Active < 0 || mA_Active > 1)
    {
        Serial.printf("\nERROR in SaveCalibrationData: Invalid mA_Active=%d", mA_Active);
        return;
    }

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
    // CRITICAL: Validate array bounds before loading
    if (CalBank.empty() || bankCalibId < 0 || bankCalibId >= (int8_t)CalBank.size())
    {
        Serial.printf("\nERROR in LoadCalibrationData: Invalid bankCalibId=%d, CalBank.size()=%d",
                     bankCalibId, CalBank.size());
        return;
    }

    if (mA_Active < 0 || mA_Active > 1)
    {
        Serial.printf("\nERROR in LoadCalibrationData: Invalid mA_Active=%d", mA_Active);
        return;
    }

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
    settingParameters.adcRate = std::clamp((int)settingParameters.adcRate, 0, 4);
    settingParameters.adcNumberOfAvgs = std::clamp((int)settingParameters.adcNumberOfAvgs, 0, 12);
    settingParameters.adcNumberOfDigits = std::clamp((int)settingParameters.adcNumberOfDigits, 1, 4);
    Voltage.adjValue = settingParameters.SetVoltage;
    Current.adjValue = settingParameters.SetCurrent;

    // Restore buzzer state from saved settings
    buzzerSound = settingParameters.buzzer;

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

    ADC_SPS[0] = " ~10";
    ADC_SPS[1] = " ~20";
    ADC_SPS[2] = " ~70";
    ADC_SPS[3] = "~200";
    ADC_SPS[4] = "~500";

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
        adc.ads1219->setDataRate(20);
        break;

    case 2:
        adc.ads1219->setDataRate(90);
        break;
    case 3:
        adc.ads1219->setDataRate(330);
        break;
    case 4:
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

    // Save table_points (4000 bytes) - completely separate from Arbt
    fs::File tableFile = SPIFFS.open("/fgen_table.dat", "w");
    if (tableFile) {
        tableFile.write((uint8_t*)&data.table_points, sizeof(data.table_points));
        tableFile.close();
        Serial.println("Table data saved to SPIFFS");
    }

    // Save arbitrary_points (640 bytes) - completely separate from Table
    fs::File arbFile = SPIFFS.open("/fgen_arb.dat", "w");
    if (arbFile) {
        arbFile.write((uint8_t*)&data.arbitrary_points, sizeof(data.arbitrary_points));
        arbFile.close();
        Serial.println("Arbt bank data saved to SPIFFS");
    }

    SPIFFS.end();
}

FunGen Device::LoadMemoryFgen(const String &key)
{
    FunGen data;

    // Initialize with safe defaults
    data.frequency = 1.0;
    data.amplitude = 5.0;
    data.offset = 0.0;
    data.dutyCycle = 0.5;
    memset(data.table_points, 0, sizeof(data.table_points));
    memset(data.arbitrary_points, 0, sizeof(data.arbitrary_points));

    // Load small settings from NVS
    StoreMem.begin("my-app", false);
    FunGenSettings settings;
    size_t bytesRead = StoreMem.getBytes("fgen", &settings, sizeof(settings));
    if (bytesRead > 0) {
        // Validate loaded data before using it
        if (std::isfinite(settings.frequency) && std::isfinite(settings.amplitude) &&
            std::isfinite(settings.offset) && std::isfinite(settings.dutyCycle) &&
            settings.frequency > 0 && settings.amplitude >= 0 && settings.dutyCycle >= 0 && settings.dutyCycle <= 1.0) {
            data.frequency = settings.frequency;
            data.amplitude = settings.amplitude;
            data.offset = settings.offset;
            data.dutyCycle = settings.dutyCycle;
        } else {
            Serial.println("FunGen NVS data invalid, using defaults");
        }
    }
    StoreMem.end();

    // Load large arrays from SPIFFS
    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS Mount Failed");
        return data;
    }

    // Load table_points (4000 bytes) - independent from Arbt
    fs::File tableFile = SPIFFS.open("/fgen_table.dat", "r");
    if (tableFile) {
        size_t bytesRead = tableFile.read((uint8_t*)&data.table_points, sizeof(data.table_points));
        tableFile.close();
        Serial.printf("Table data loaded from SPIFFS: %d bytes\n", bytesRead);
    } else {
        Serial.println("Table data file not found - using defaults");
    }

    // Load arbitrary_points (640 bytes) - independent from Table
    fs::File arbFile = SPIFFS.open("/fgen_arb.dat", "r");
    if (arbFile) {
        size_t bytesRead = arbFile.read((uint8_t*)&data.arbitrary_points, sizeof(data.arbitrary_points));
        arbFile.close();
        Serial.printf("Arbt bank data loaded from SPIFFS: %d bytes\n", bytesRead);
    } else {
        Serial.println("Arbt bank data file not found - using defaults");
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
  constexpr float adcRateCompensation[5] = {
      1.00000f,            // Index 0: 10 SPS
      1.00000f,            // Index 1: 20 SPS
      32.0000f / 31.9985f, // Index 2: 90 SPS
      32.0000f / 31.9945f, // Index 3: 330 SPS
      32.0000f / 31.9882f  // Index 4: 1000 SPS
  };

        Voltage.rawValue = adc.readConversion();
        adcDataReady = false;
        adc.ads1219->setGain(ONE);
        adc.startConversion(CURRENT, REF_EXTERNAL);

        // OPTIMIZED: Single line calculation for speed
        double v_corrected = (Voltage.rawValue - Voltage.calib_b) * Voltage.calib_1m * adcRateCompensation[settingParameters.adcRate];

        // Apply INL correction if calibration data is available
        if (g_voltINL_ready)
            v_corrected = g_voltINL.apply(v_corrected);
        // Zero-current INL (commented out pending investigation):
        // double i_meas = fabs(Current.measured.Mean());
        // if (g_voltINL_ZC_ready && i_meas < 0.005)
        //     v_corrected = g_voltINL_ZC.apply(v_corrected);
        // else if (g_voltINL_ready)
        //     v_corrected = g_voltINL.apply(v_corrected);

        Voltage.measureUpdate(v_corrected);
        Voltage.rawValueStats(Voltage.rawValue);
        adc.ADC_loopCounter++;

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

        Current.rawValue = adc.readConversion();
        adcDataReady = false;
        adc.ads1219->setGain(ONE);
        adc.startConversion(VOLTAGE, REF_EXTERNAL);

        // OPTIMIZED: Fast path for current calculation with bounds check
        double c = (Current.rawValue - Current.calib_b) * Current.calib_1m;

        // Leakage compensation (with bounds validation and division-by-zero protection)
        if (!CalBank.empty() && bankCalibId >= 0 && bankCalibId < (int8_t)CalBank.size() &&
            mA_Active >= 0 && mA_Active <= 1)
        {
            double leakage_R = CalBank[bankCalibId].internalLeakage[mA_Active];
            // Only apply leakage compensation if resistance is valid (not zero, not FLT_MAX, not NaN)
            if (std::isfinite(leakage_R) && leakage_R > 0.001 && leakage_R < 1e9)
            {
                c -= ((mA_Active ? 1000.0 : 1.0) * (Voltage.measured.Mean() / (leakage_R * 1000.0)));
            }
        }

        // Apply REL offset
        if (mA_Active && currentRelActive_mA) c -= currentRelOffset_mA;
        else if (!mA_Active && currentRelActive_A) c -= currentRelOffset_A;

        Current.measureUpdate(c);
        // Removed direct bar pointer manipulation - barUpdate() handles this now for max refresh rate
        // *Current.Bar.curValuePtr = c / (Current.maxValue / Current.adjFactor) * lv_bar_get_max_value(Current.Bar.bar); // uint16_t(v * 8);

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
    digitalWrite(AmA_Pin, mA_Active);
    // UI updates (LVGL calls) are handled by processDeferredMaToggle() on Core 1
}

void Device::restoreAdcRateFromFUN()
{
    // Restore ADC rate from saved value when exiting FUN mode
    if (settingParameters.adcRateSavedForFUN <= 4)
    {
        settingParameters.adcRate = settingParameters.adcRateSavedForFUN;
        const int rates[] = {20, 20, 90, 330, 1000};
        adc.ads1219->setDataRate(rates[settingParameters.adcRateSavedForFUN]);

        // Clear the saved value to prevent accidental reuse
        settingParameters.adcRateSavedForFUN = 255;  // Invalid value marker

        // Adjust task priority based on restored rate
        adjustAdcTaskPriority();
    }
}

void Device::adjustAdcTaskPriority()
{
    // Adjust Task_ADC priority based on ADC sample rate
    // adcRate: 0=20 SPS, 1=20 SPS, 2=90 SPS, 3=330 SPS, 4=1000 SPS

    UBaseType_t newPriority;
    UBaseType_t newPriority_bargraph;


    switch (settingParameters.adcRate)
    {
        case 0:  // 20 SPS - very slow, low priority (low noise mode)
        case 1:  // 20 SPS - very slow, low priority
        case 2:  // 90 SPS - slow, low priority
            newPriority = 1;
            newPriority_bargraph =0;
            break;

        case 3:  // 330 SPS - fast, higher priority
            newPriority = 1;
            newPriority_bargraph =0;
            break;

        case 4:  // 1000 SPS - very fast, highest priority
            newPriority = 5;
            newPriority_bargraph =0;
            break;

        default:
            newPriority = 1;  // Default to low priority
            newPriority_bargraph =0;
            break;
    }

    // Change the task priority if Task_ADC exists
    if (Task_adc != nullptr)
    {
        vTaskPrioritySet(Task_adc, newPriority);
        Serial.printf("\n[Task Priority] Task_ADC priority set to %d (ADC rate: %d)\n",
                      newPriority, settingParameters.adcRate);
        vTaskPrioritySet(Task_bargraph, newPriority_bargraph);
        Serial.printf("\n[Task Priority] Task_bargraph priority set to %d (ADC rate: %d)\n",
                      newPriority_bargraph, settingParameters.adcRate);

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
    // Fast path: read pin first, bail if unchanged (most common case)
    int currentStatus = digitalRead(CCCVPin);
    if (currentStatus == lastCCCVStatus)
        return;

    // Skip if output is off or in function generator mode
    DEVICE s = status;
    if (s == DEVICE::OFF || s == DEVICE::FUN)
    {
        lastCCCVStatus = -1;  // Reset so we re-check when turning on
        return;
    }

    // Beep on CC/CV transitions (skip on first check after power on)
    if (lastCCCVStatus != -1 && settingParameters.beeperOnPowerChange)
    {
        myTone(currentStatus == false ? NOTE_A3 : NOTE_A5, 10);
    }

    lastCCCVStatus = currentStatus;
    setStatus(currentStatus ? DEVICE::VC : DEVICE::CC);
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

    TaskHandle_t idleTask1 = xTaskGetIdleTaskHandleForCPU(1);
    esp_task_wdt_delete(idleTask1);

    if (status_ == DEVICE::OFF || oldStatus == DEVICE::OFF)
        vTaskDelay(75);

    // Single map lookup — cache the colors
    const deviceColors &c = stateColor[status_];
    const deviceColors &oldC = stateColor[oldStatus];
    bool colorsChanged = (c.measured.full != oldC.measured.full ||
                          c.plotColor1.full != oldC.plotColor1.full ||
                          c.plotColor2.full != oldC.plotColor2.full);
    bool pageColorChanged = (c.pageColor.full != oldC.pageColor.full);

    // Batch all LVGL changes with invalidation disabled
    lv_disp_enable_invalidation(NULL, false);

    if (colorsChanged)
    {
        Voltage.setMeasureColor(c.measured);
        Current.setMeasureColor(c.measured);
        Power.setMeasureColor(c.measured);

        if (gui.label_current_rel)
            lv_obj_set_style_text_color(gui.label_current_rel, c.measured, 0);

        Voltage.setStatsColor(c.plotColor1);
        Current.setStatsColor(c.plotColor2);

        lv_chart_set_series_color(graph.chart, graph.serV, c.plotColor1);
        lv_chart_set_series_color(graph.chart, graph.serI, c.plotColor2);

        lv_style_set_text_color(&graph.style_legend1, c.plotColor1);
        lv_style_set_text_color(&graph.style_legend2, c.plotColor2);
        lv_style_set_text_color(&graph.style_statsVolt, c.plotColor1);
        lv_style_set_text_color(&graph.style_statsCurrent, c.plotColor2);

        lv_chart_set_series_color(stats.chart, stats.serV, c.plotColor1);
        lv_chart_set_series_color(stats.chart, stats.serI, c.plotColor2);

        lv_style_set_text_color(&stats.style_legend1, c.plotColor1);
        lv_style_set_text_color(&stats.style_legend2, c.plotColor2);
        lv_style_set_text_color(&stats.style_statsVolt, c.plotColor1);
        lv_style_set_text_color(&stats.style_statsCurrent, c.plotColor2);

        lv_style_set_text_color(&style_controlMode, c.measured);
        lv_obj_remove_style(controlMode, &style_controlMode, 0);
        lv_obj_add_style(controlMode, &style_controlMode, 0);
    }

    // Skip page/tab recolor for CC↔VC (same pageColor 0x001A3C)
    if (pageColorChanged)
    {
        for (int i = 0; i < 5; i++)
            lv_obj_set_style_bg_color(page[i], c.pageColor, 0);
        lv_obj_set_style_bg_color(lv_obj_get_child(page[3], 0), c.pageColor, 0);

        lv_style_set_bg_color(&style_tabview_df_btn, c.pageColor);
        lv_obj_t *tab_btns = lv_tabview_get_tab_btns(tab.tabview);
        lv_obj_remove_style(tab_btns, &style_tabview_df_btn, LV_PART_ITEMS | LV_STATE_CHECKED);
        lv_obj_add_style(tab_btns, &style_tabview_df_btn, LV_PART_ITEMS | LV_STATE_CHECKED);
    }

    // Control mode label
    const char *modeText = (status_ == DEVICE::FUN) ? "Fun" :
                           (status_ == DEVICE::CC)  ? "CC"  :
                           (status_ == DEVICE::OFF) ? "OFF" : "VC";
    lv_label_set_text(controlMode, modeText);

    if (status_ == DEVICE::OFF)
    {
        myTone(NOTE_A3, 200);
        // Make sure function generator is also off and restore ADC rate
        lv_obj_clear_state(btn_function_gen, LV_STATE_CHECKED);
        lv_obj_add_state(btn_function_gen, LV_STATE_DISABLED);
        lv_label_set_text(lv_obj_get_child(btn_function_gen, 0), "OFF");

        // Only restore ADC rate if transitioning FROM FUN mode
        if (oldStatus == DEVICE::FUN) {
            restoreAdcRateFromFUN();  // Restore ADC rate if it was changed by FUN mode
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

        // If transitioning FROM FUN mode to another state (VC/CC/ON), restore ADC rate
        if (oldStatus == DEVICE::FUN && status_ != DEVICE::FUN) {
            restoreAdcRateFromFUN();
        }

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

        // OPTIMIZATION: Over-range indicator - only execute when actually needed
        // Check mA_Active first to avoid unnecessary Mean() call and comparisons
        if (mA_Active)
        {
            static bool wasOverRange = false;
            bool isOverRange = Current.measured.Mean() >= 3.900;

            if (isOverRange)
            {
                // Over-range blink logic (>= 3.900 mA) - blink like multimeter OL
                static bool blinkState = false;
                static unsigned long lastBlink = 0;
                if (millis() - lastBlink >= 250)  // 4Hz blink
                {
                    blinkState = !blinkState;
                    lastBlink = millis();
                }
                // OPTIMIZATION: Only update flag if state changed
                if (blinkState)
                    lv_obj_add_flag(Current.label_measureValue, LV_OBJ_FLAG_HIDDEN);
                else
                    lv_obj_clear_flag(Current.label_measureValue, LV_OBJ_FLAG_HIDDEN);
                wasOverRange = true;
            }
            else if (wasOverRange)
            {
                // OPTIMIZATION: Only clear flag once when exiting over-range
                lv_obj_clear_flag(Current.label_measureValue, LV_OBJ_FLAG_HIDDEN);
                wasOverRange = false;
            }
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
