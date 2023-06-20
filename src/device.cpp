#include "device.h"

extern Calibration StoreData;
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);

void Device::setupADC(uint8_t pin, void func(void))

{
    // mem.begin("param", false);
    pinMode(pin, INPUT_PULLUP);
    attachInterrupt(pin, func, FALLING);
    // calibrate();
    LoadCalibrationData();

    adc.ads1219->setDataRate(90);
    adc.ads1219->begin();
    adc.ads1219->setVoltageReference(REF_INTERNAL);
    adc.ads1219->setGain(ONE);
    adc.ads1219->setConversionMode(SINGLE_SHOT); // SINGLE_SHOT
    adc.ads1219->readSingleEnded(VOLTAGE);
    adc.busyChannel = VOLTAGE;
}
void Device::setupDAC(uint8_t addr)
{
    DAC.address = addr;
    DAC.setVoltageReference(REFR_INTERNAL);
}

void Device::calibrate(void)
{
    // id = 1;
    // bankCalibId = -1;
    bankCalibId = 0;
    // for (int j = 0; j < CalBank.size(); j++)
    // {
    //     // Serial.print(WiFi.macAddress());
    //     // Serial.print("  ");
    //     // Serial.print(CalBank[j].macAdd);
    //     // Serial.print("  ");
    //     // Serial.println(WiFi.macAddress() == CalBank[j].macAdd);
    //     if (WiFi.macAddress() == CalBank[j].macAdd)
    //     {
    //         bankCalibId = j;
    //         // Serial.printf("Calibration bank index:%i\n", bankCalibId);
    //         break;
    //     }
    // }

    // calibrationUpdate();
}

void Device::calibrationUpdate(void)
{

    Voltage.calib_m = (CalBank[bankCalibId].vCal.code_2 - CalBank[bankCalibId].vCal.code_1) /
                      (CalBank[bankCalibId].vCal.value_2 - CalBank[bankCalibId].vCal.value_1);
    Voltage.calib_b = CalBank[bankCalibId].vCal.code_1 - Voltage.calib_m * CalBank[bankCalibId].vCal.value_1;

    Current.calib_m = (CalBank[bankCalibId].iCal.code_2 - CalBank[bankCalibId].iCal.code_1) /
                      (CalBank[bankCalibId].iCal.value_2 - CalBank[bankCalibId].iCal.value_1);
    Current.calib_b = CalBank[bankCalibId].iCal.code_1 - Current.calib_m * CalBank[bankCalibId].iCal.value_1;

    Power.measured.ResetStats();
    Voltage.measured.ResetStats();
    Current.measured.ResetStats();

    Current.hist.Reset();
    Voltage.hist.Reset();
    // SaveCalibrationData();
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
    // Calibration data("", {0}, {0});
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
    // Calibration data("", {0}, {0});
    ssize_t bytesRead = StoreMem.getBytes(key.c_str(), &StoreData, sizeof(Calibration));
    StoreMem.end();
    return StoreData;
}

void Device::SaveCalibrationData()
{
    Device::SaveCalibData("cal", CalBank[bankCalibId]);
    Serial.print("\ndata saved.");
}

void Device::LoadCalibrationData()
{
    Calibration outputData = Device::LoadCalibData("cal");
    CalBank[bankCalibId] = {outputData};

    calibrationUpdate();

    Serial.printf("\nLoaded data:%s {%+08.4f %+08i %+08.4f %+08i}, {%+08.4f %+08i %+08.4f %+08i}",
                  outputData.macAdd,
                  outputData.vCal.value_1, outputData.vCal.code_1, outputData.vCal.value_2, outputData.vCal.code_2,
                  outputData.iCal.value_1, outputData.iCal.code_1, outputData.iCal.value_2, outputData.iCal.code_2);

    // CalBank[bankCalibId].vCal.value_1 = outputData.vCal.value_1;
    // CalBank[bankCalibId].vCal.value_2 = outputData.vCal.value_2;
    // CalBank[bankCalibId].vCal.code_1 = outputData.vCal.code_1;
    // CalBank[bankCalibId].vCal.code_2 = outputData.vCal.code_2;

    // Serial.printf("\nVoltage vCalvalue_1:%7.3f", vCalvalue_1);
    // Serial.printf("\nVoltage vCalvalue_2:%7.3f", vCalvalue_2);
    // Serial.printf("\nVoltage vCalcode_1:%i", vCalcode_1);
    // Serial.printf("\nVoltage vCalcode_2:%i",vCalcode_2);

    // EEPROMread(CalBank[bankCalibId].vCal.value1EEPROMAddress, CalBank[bankCalibId]);

    // Voltage.calib_m = (vCalcode_2 - vCalcode_1) / (vCalvalue_2 - vCalvalue_1);
    // Voltage.calib_b = vCalcode_1 - Voltage.calib_m * vCalvalue_1;

    // Current.calib_m = (iCalcode_2 - iCalcode_1) / (iCalvalue_2 - iCalvalue_1);
    // Current.calib_b = iCalcode_1 - Current.calib_m * iCalvalue_1;

    Serial.printf("\nVoltage Calibration m:%7.3f", Voltage.calib_m);
    Serial.printf("\nVoltage Calibration b:%7.3f", Voltage.calib_b);
    Serial.printf("\nCurrent Calibration m:%7.3f", Current.calib_m);
    Serial.printf("\nCurrent Calibration b:%7.3f", Current.calib_b);

    if (!isfinite(Voltage.calib_m) || !std::isfinite(Voltage.calib_b) || !std::isfinite(Current.calib_m) || !std::isfinite(Current.calib_b))
    {
        Serial.print("\n\n ** WRONG CALIBRATION DATA ** \n");
        Serial.print("\nLoading factory calibration data ...");

        CalBank[bankCalibId] = Calibration(CalBank[bankCalibId].macAdd, {+00.0000, -259, +32.0000, +8164608}, {+00.0000, +104080, +3.0000, +2926000});
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

        //  Voltage.SetUpdate(5.0);
        //  Current.SetUpdate(.1);
        SaveCalibrationData();
    }

    // CalBank[bankCalibId].vCal.value_1 = vCalvalue_1;
    // CalBank[bankCalibId].vCal.value_2 = vCalvalue_2;
    // CalBank[bankCalibId].vCal.code_1 = vCalcode_1;
    // CalBank[bankCalibId].vCal.code_2 = vCalcode_2;

    // CalBank[bankCalibId].iCal.value_1 = iCalvalue_1;
    // CalBank[bankCalibId].iCal.value_2 = iCalvalue_2;
    // CalBank[bankCalibId].iCal.code_1 = iCalcode_1;
    // CalBank[bankCalibId].iCal.code_2 = iCalcode_2;
}

void Device::LoadSetting(void)
{

    Serial.print("\nLoading setting ... ");
    settingParameters = Device::LoadSettingData("setting_param");
    Serial.print("done.");

    // Make sure loaded data is not out of range
    settingParameters.adcRate = std::clamp((int)settingParameters.adcRate, 0, 3);
    settingParameters.adcNumberOfAvgs = std::clamp((int)settingParameters.adcNumberOfAvgs, 0, 7);
    settingParameters.adcNumberOfDigits = std::clamp((int)settingParameters.adcNumberOfDigits, 1, 4);
    Voltage.adjValue = settingParameters.SetVoltage;
    Current.adjValue = settingParameters.SetCurrent;
    Serial.printf("\n isPowerOn?:%i", settingParameters.isPowerSupplyOn);

    Preferences memory;
    memory.begin("param", false);
    //  memory.putUShort("pi", 314);
    if (314 != memory.getUShort("pi", 0))
    {
        Serial.print("\n\n ** WRONG SETTING DATA ** \n");
        Serial.print("\nLoad default setting ...");
        settingParameters.adcRate = 0;
        settingParameters.adcNumberOfAvgs = 1;
        settingParameters.adcNumberOfDigits = 4;

        Voltage.adjValue = 5.0 - Voltage.adjOffset;
        Current.adjValue = .2 - Current.adjOffset;

        memory.putUShort("pi", 314);

        Device::SaveSettingData("setting_param", settingParameters);
    }
    // memory.putUShort("pi", 0);
    memory.end();

    Serial.printf("\nadcRate:           %i", settingParameters.adcRate);
    Serial.printf("\nadcNumberOfAvgs:   %f", pow(2, settingParameters.adcNumberOfAvgs));
    Serial.printf("\nadcNumberOfDigits: %i", settingParameters.adcNumberOfDigits);

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

    switch (settingParameters.adcNumberOfDigits)
    {
    case 1:
        Voltage.restrict = "%+05.1f";
        Current.restrict = "%+05.1f";
        Power.restrict = "%+05.1f";
        break;
    case 2:
        Voltage.restrict = "%+06.2f";
        Current.restrict = "%+06.2f";
        Power.restrict = "%+06.2f";
        break;
    case 3:
        Voltage.restrict = "%+07.3f";
        Current.restrict = "%+07.3f";
        Power.restrict = "%+07.3f";
        break;
    case 4:
        Voltage.restrict = "%+08.4f";
        Current.restrict = "%+08.4f";
        Power.restrict = "%+08.4f";
        break;

    default:
        Voltage.restrict = "%+07.3f";
        Current.restrict = "%+07.3f";
        Power.restrict = "%+07.3f";
    }

    // EEPROMread(Voltage.adjEEPROMAddress + 8 * 0, Voltage.adjValue); // PowerSupply.Voltage.adjValue
    // EEPROMread(Current.adjEEPROMAddress + 8 * 1, Current.adjValue); // PowerSupply.Voltage.adjValue

    Serial.printf("\nLast Voltage Value:% +8.4f", Voltage.adjValue+Voltage.adjOffset);
    Serial.printf("\nLast Current Value:% +8.4f", Current.adjValue+Current.adjOffset);
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

// Reference:https://training.ti.com/ti-precision-labs-adcs-understanding-and-calibrating-offset-and-gain-adc-systems
//  Time @ 05:41
void Device::readVoltage()
{

    if ((adcDataReady) && (adc.busyChannel == VOLTAGE) /*&& ads.checkDataReady()*/) // ||  adc.checkDataReady()
    {

        static double v;
        Voltage.rawValue = adc.readConversion();
        adc.startConversion(CURRENT);
        adcDataReady = false;

        v = (Voltage.rawValue - Voltage.calib_b) / Voltage.calib_m;

        // Voltage.hist[v];
        Voltage.measureUpdate(v); //  enob(rs[0].StandardDeviation())
        adc.ADC_loopCounter++;


        // Serial.print("Measured Raw Voltage:");
        // Serial.println(v);
        // Serial.print("Voltage:");
        // Serial.printf("%9.5f", Voltage.measured.Mean());
    }
}
double internalCurrentConsumption;
void Device::readCurrent()
{
    if ((adcDataReady) && (adc.busyChannel == CURRENT) /* && ads.checkDataReady() */)
    {
        static double c;
        Current.rawValue = adc.readConversion();
        adc.startConversion(VOLTAGE);
        adcDataReady = false;

        // double currentOfR11R12 = 0.0 * Voltage.measured.Mean() / 80.0e3; // Current consuming by R11 & R12
        // double currentOfUnknowSource = 1.0 * Voltage.measured.Mean() / (32.0 / (0.0019 - 0.0012));

        // Current used by R11&R12 and maybe others?
        double currentOfUnknowSource = 1.0 * Voltage.measured.Mean() / (32.0 / (0.0037 - 0.0034));
        internalCurrentConsumption = currentOfUnknowSource +
                                     1.0 * 0.000180 * !digitalRead(CCCVPin);                         // Why?
        c = (((Current.rawValue - Current.calib_b) / Current.calib_m) - internalCurrentConsumption); // old value: .0009

        // Current.hist[c];

        Current.measureUpdate(c);

        // Serial.print (c);
        // Serial.print(",  Current:");
        // Serial.printf("%9.5f", Current.measured.Mean());
        // Serial.print(":");
        // Serial.print(Current.measured.value);
        // Serial.print(":");
        // Serial.print(Current.measured.sum_);
        // Serial.print("\n");

        static unsigned long loopCount = 0;
        static unsigned long startTime = millis();

        adc.ADC_loopCounter++;
        if ((startTime + 1000) <= millis())
        {
            adc.realADCSpeed = adc.ADC_loopCounter;
            // Serial.printf("ADC real SPS for 1 ch:%4i \n",  adc.ADC_loopCounter);
            adc.ADC_loopCounter = 0;
            startTime = millis();
        }
    }
    static unsigned long loopCount = 0;
    static unsigned long startTime = millis();

    static unsigned ADC_loopCheckCounter;
    ADC_loopCheckCounter++;
    if ((startTime + 1000) <= millis())
    {

        // Serial.printf("ADC loop check for 1 ch:%4i %+6.3f\n", ADC_loopCheckCounter, Current.measured.value);
        ADC_loopCheckCounter = 0;
        startTime = millis();
    }
}
void Device::getPower()
{
    // if (Current.changed || Voltage.changed)
    // {

    Power.measureUpdate(Current.measured.Mean() * Voltage.measured.Mean());

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
void Device::writeDAC_Voltage(uint16_t value)
{
    DAC.writeUpdate(DAC_VOLTAGE, value);
}
void Device::writeDAC_Current(uint16_t value)
{
    DAC.writeUpdate(DAC_CURRENT, value);
}

void Device::DACUpdate(void)
{
    if (getStatus() == DEVICE::OFF)
    {
        DAC.writeAndPowerAll(DAC_VOLTAGE, Voltage.adjOffset);

        // DAC.writeAndPowerAll(DAC_CURRENT, Current.adjOffset+ 1*10000);
        DAC.writeAndPowerAll(DAC_CURRENT, (-Current.adjOffset - 0.001) * 10000);
        return;
    }

    // if (digitalRead(CCCVPin) == true)
    // setStatus(DEVICE::VC);

    // else
    // setStatus(DEVICE::CC);

    if (digitalRead(CCCVPin) == false)
        setStatus(DEVICE::CC);
    else
        setStatus(DEVICE::VC);

    if (getStatus() != DEVICE::OFF)
    {
        double v = Voltage.adjValue * 2000;
        /*
        This current obtain experimentally by measurement.
        It is proportional  to voltage and it's measure at 32V
        */
        double otherInternalCurrentConsumer = 1.0 * Voltage.measured.Mean() / (32.0 / (0.00098 - 0.00025)); // -2.1 mm
        double c = (Current.adjValue + 0 * internalCurrentConsumption + otherInternalCurrentConsumer) * 10000;

        static double dac_last_Voltage = -1, dac_last_Current = -1;
        // if (dac_last_Voltage != v || dac_last_Current != c)
        {
            DAC.writeAndPowerAll(DAC_VOLTAGE, v);
            DAC.writeAndPowerAll(DAC_CURRENT, uint16_t(c));
            dac_last_Voltage = v;
            dac_last_Current = c;
        }
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
        status = DEVICE::ON;

    else if (onOff == SWITCH::OFF)
        status = DEVICE::OFF;
}

void Device::toggle(void)
{
    if (getStatus() != DEVICE::OFF)
        turn(SWITCH::OFF);
    else
        turn(SWITCH::ON);
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
    // Set Colors
    Voltage.setMeasureColor(stateColor[status_].measured);
    Current.setMeasureColor(stateColor[status_].measured);
    Power.setMeasureColor(stateColor[status_].measured);

    Voltage.setStatsColor(stateColor[status_].plotColor1);
    Current.setStatsColor(stateColor[status_].plotColor2);

    // Chart color
    lv_chart_set_series_color(graph.chart, graph.serV, stateColor[status_].plotColor1);
    lv_chart_set_series_color(graph.chart, graph.serI, stateColor[status_].plotColor2);

    // Chart legend color
    lv_style_set_text_color(&graph.style_legend1, stateColor[status_].plotColor1);
    lv_style_set_text_color(&graph.style_legend2, stateColor[status_].plotColor2);

    lv_style_set_text_color(&graph.style_statsVolt, stateColor[status_].plotColor1);
    lv_style_set_text_color(&graph.style_statsCurrent, stateColor[status_].plotColor2);

    // Chart color
    lv_chart_set_series_color(stats.chart, stats.serV, stateColor[status_].plotColor1);
    lv_chart_set_series_color(stats.chart, stats.serI, stateColor[status_].plotColor2);

    // Chart legend color
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
    if (status_ == DEVICE::OFF)
        lv_label_set_text(controlMode, "OFF");

    status = status_;
};

void Device::FlushSettings(void)
{

    Voltage.Flush();
    Current.Flush();

    if (Voltage.adjValueChanged)
    {
        // _lv_disp_refr_timer(NULL);
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
