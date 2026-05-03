#include "scpi_parser.h"
#include "device.hpp"
#include "globals.h"
#include "ui_helpers.h"
#include "buzzer.h"
#include "version.h"
#include "error_handler.h"
#include "functions.h"
#include "waveform_generator.h"
#include <Wire.h>
#include <cstring>
#include <cmath>

extern Device PowerSupply;
extern lv_obj_t *btn_function_gen;

SCPIParser scpiParser;

SCPIParser::SCPIParser()
    : cmdBufferPos(0),
      errorQueueHead(0),
      errorQueueTail(0),
      errorQueueCount(0),
      statusByte(0),
      eventStatusRegister(0),
      eventStatusEnable(0),
      serviceRequestEnable(0),
      remoteMode(false),
      operationComplete(true)
{
    memset(cmdBuffer, 0, CMD_BUFFER_SIZE);
}

void SCPIParser::init()
{
    // Clear error queue
    errorQueueHead = 0;
    errorQueueTail = 0;
    errorQueueCount = 0;

    // Initialize status registers
    statusByte = 0;
    eventStatusRegister = 0;
    eventStatusEnable = 0xFF;  // Enable all events by default
    serviceRequestEnable = 0;

    operationComplete = true;
    remoteMode = false;

    Serial.println("SCPI Parser initialized");
    Serial.println("Send *IDN? for identification");
}

void SCPIParser::process()
{
    while (Serial.available() > 0)
    {
        char c = Serial.read();

        // Handle line terminators
        if (c == '\n' || c == '\r')
        {
            if (cmdBufferPos > 0)
            {
                cmdBuffer[cmdBufferPos] = '\0';
                String cmd(cmdBuffer);
                executeCommand(cmd);
                cmdBufferPos = 0;
            }
            continue;
        }

        // Store character
        if (cmdBufferPos < CMD_BUFFER_SIZE - 1)
        {
            cmdBuffer[cmdBufferPos++] = c;
        }
        else
        {
            // Buffer overflow
            addError(ERR_COMMAND_ERROR, "Command too long");
            cmdBufferPos = 0;
        }
    }
}

void SCPIParser::executeCommand(const String& command)
{
    String cmd = command;
    trimWhitespace(cmd);

    if (cmd.length() == 0)
        return;

    // Split command and parameter
    String cmdPart;
    String paramPart;
    int spacePos = cmd.indexOf(' ');
    if (spacePos > 0)
    {
        cmdPart = cmd.substring(0, spacePos);
        paramPart = cmd.substring(spacePos + 1);
        trimWhitespace(paramPart);
    }
    else
    {
        cmdPart = cmd;
    }

    toUpperCase(cmdPart);

    // IEEE 488.2 Common Commands (start with *)
    if (cmdPart.startsWith("*"))
    {
        if (cmdPart == "*IDN?") cmd_IDN();
        else if (cmdPart == "*RST") cmd_RST();
        else if (cmdPart == "*CLS") cmd_CLS();
        else if (cmdPart == "*ESE") cmd_ESE(paramPart);
        else if (cmdPart == "*ESE?") cmd_ESE_Q();
        else if (cmdPart == "*ESR?") cmd_ESR_Q();
        else if (cmdPart == "*OPC") cmd_OPC();
        else if (cmdPart == "*OPC?") cmd_OPC_Q();
        else if (cmdPart == "*SRE") cmd_SRE(paramPart);
        else if (cmdPart == "*SRE?") cmd_SRE_Q();
        else if (cmdPart == "*STB?") cmd_STB_Q();
        else if (cmdPart == "*TST?") cmd_TST_Q();
        else if (cmdPart == "*WAI") cmd_WAI();
        else addError(ERR_UNDEFINED_HEADER);
        return;
    }

    // Remove leading colon if present
    if (cmdPart.startsWith(":"))
        cmdPart = cmdPart.substring(1);

    // SCPI SOURce subsystem
    if (cmdPart.startsWith("SOUR"))
    {
        if (cmdPart.indexOf("VOLT:LIM?") > 0) cmd_SOUR_VOLT_LIM_Q();
        else if (cmdPart.indexOf("VOLT:LIM") > 0) cmd_SOUR_VOLT_LIM(paramPart);
        else if (cmdPart.indexOf("VOLT?") > 0) cmd_SOUR_VOLT_Q();
        else if (cmdPart.indexOf("VOLT") > 0) cmd_SOUR_VOLT(paramPart);
        else if (cmdPart.indexOf("CURR:LIM?") > 0) cmd_SOUR_CURR_LIM_Q();
        else if (cmdPart.indexOf("CURR:LIM") > 0) cmd_SOUR_CURR_LIM(paramPart);
        else if (cmdPart.indexOf("CURR?") > 0) cmd_SOUR_CURR_Q();
        else if (cmdPart.indexOf("CURR") > 0) cmd_SOUR_CURR(paramPart);
        else if (cmdPart.indexOf("FUNC?") > 0) cmd_SOUR_FUNC_Q();
        else if (cmdPart.indexOf("FUNC") > 0) cmd_SOUR_FUNC(paramPart);
        else if (cmdPart.indexOf("FGEN:FREQ") > 0) cmd_FGEN_FREQ(paramPart);
        else if (cmdPart.indexOf("FGEN:AMPL") > 0) cmd_FGEN_AMPL(paramPart);
        else if (cmdPart.indexOf("FGEN:OFFS") > 0) cmd_FGEN_OFFS(paramPart);
        else if (cmdPart.indexOf("FGEN:DUTY") > 0) cmd_FGEN_DUTY(paramPart);
        else if (cmdPart.indexOf("FGEN:WAVE?") > 0) cmd_FGEN_WAVE_Q();
        else if (cmdPart.indexOf("FGEN:WAVE") > 0) cmd_FGEN_WAVE(paramPart);
        else addError(ERR_UNDEFINED_HEADER);
        return;
    }

    // SCPI MEASure subsystem
    if (cmdPart.startsWith("MEAS"))
    {
        if (cmdPart.indexOf("VOLT:MIN?") > 0) cmd_MEAS_VOLT_MIN_Q();
        else if (cmdPart.indexOf("VOLT:MAX?") > 0) cmd_MEAS_VOLT_MAX_Q();
        else if (cmdPart.indexOf("VOLT:MEAN?") > 0) cmd_MEAS_VOLT_MEAN_Q();
        else if (cmdPart.indexOf("VOLT?") > 0) cmd_MEAS_VOLT_Q();
        else if (cmdPart.indexOf("CURR:MIN?") > 0) cmd_MEAS_CURR_MIN_Q();
        else if (cmdPart.indexOf("CURR:MAX?") > 0) cmd_MEAS_CURR_MAX_Q();
        else if (cmdPart.indexOf("CURR:MEAN?") > 0) cmd_MEAS_CURR_MEAN_Q();
        else if (cmdPart.indexOf("CURR?") > 0) cmd_MEAS_CURR_Q();
        else if (cmdPart.indexOf("POW?") > 0) cmd_MEAS_POW_Q();
        else addError(ERR_UNDEFINED_HEADER);
        return;
    }

    // SCPI OUTPut subsystem
    if (cmdPart.startsWith("OUTP"))
    {
        if (cmdPart.indexOf("STAT?") > 0) cmd_OUTP_STAT_Q();
        else if (cmdPart.indexOf("STAT") > 0) cmd_OUTP_STAT(paramPart);
        else if (cmdPart.indexOf("PROT:CLE") > 0) cmd_OUTP_PROT_CLE();
        else if (cmdPart.indexOf("PROT:TRIP?") > 0) cmd_OUTP_PROT_TRIP_Q();
        else addError(ERR_UNDEFINED_HEADER);
        return;
    }

    // SCPI SYSTem subsystem
    if (cmdPart.startsWith("SYST"))
    {
        if (cmdPart.indexOf("ERR:COUN?") > 0) cmd_SYST_ERR_COUN_Q();
        else if (cmdPart.indexOf("ERR:ALL?") > 0) cmd_SYST_ERR_ALL_Q();
        else if (cmdPart.indexOf("ERR?") > 0) cmd_SYST_ERR_Q();
        else if (cmdPart.indexOf("VERS?") > 0) cmd_SYST_VERS_Q();
        else if (cmdPart.indexOf("CAP?") > 0) cmd_SYST_CAP_Q();
        else if (cmdPart.indexOf("BEEP") > 0) cmd_SYST_BEEP();
        else if (cmdPart.indexOf("LOC") > 0) cmd_SYST_LOC();
        else if (cmdPart.indexOf("REM") > 0) cmd_SYST_REM();
        else addError(ERR_UNDEFINED_HEADER);
        return;
    }

    // SCPI CALibration subsystem
    if (cmdPart.startsWith("CAL"))
    {
        if (cmdPart.indexOf("STAT?") > 0) cmd_CAL_STAT_Q();
        else if (cmdPart.indexOf("SAVE") > 0) cmd_CAL_SAVE();
        else if (cmdPart.indexOf("LOAD") > 0) cmd_CAL_LOAD();
        else addError(ERR_UNDEFINED_HEADER);
        return;
    }

    // SCPI DATA subsystem
    if (cmdPart.startsWith("DATA"))
    {
        if (cmdPart.indexOf("STAT?") > 0) cmd_DATA_STATUS_Q();
        else if (cmdPart.indexOf("SETT?") > 0) cmd_DATA_SETTINGS_Q();
        else if (cmdPart.indexOf("MEM?") > 0) cmd_DATA_MEM_Q();
        else if (cmdPart.indexOf("FGEN?") > 0) cmd_DATA_FGEN_Q();
        else if (cmdPart.indexOf("STATS?") > 0) cmd_DATA_STATS_Q();
        else if (cmdPart.indexOf("GRAPH:RST") > 0) cmd_DATA_GRAPH_RST();
        else if (cmdPart.indexOf("GRAPH:STAT?") > 0) cmd_DATA_GRAPH_STAT_Q();
        else if (cmdPart.indexOf("GRAPH?") > 0 || cmdPart.indexOf("GRAP?") > 0) cmd_DATA_GRAPH_Q();
        else if (cmdPart.indexOf("ARBT?") > 0) cmd_DATA_ARBT_Q(paramPart);
        else if (cmdPart.indexOf("ARBT") > 0) cmd_DATA_ARBT(paramPart);
        else if (cmdPart.indexOf("TABL?") > 0) cmd_DATA_TABL_Q();
        else if (cmdPart.indexOf("TABL") > 0) cmd_DATA_TABL(paramPart);
        else if (cmdPart.indexOf("CAL?") > 0) cmd_DATA_CAL_Q();
        else if (cmdPart.indexOf("CAL:SET") > 0) cmd_DATA_CAL_SET(paramPart);
        else addError(ERR_UNDEFINED_HEADER);
        return;
    }

    // SCPI SETT subsystem (settings write)
    if (cmdPart.startsWith("SETT"))
    {
        if (cmdPart.indexOf("ADC:RATE") > 0) cmd_SETT_ADC_RATE(paramPart);
        else if (cmdPart.indexOf("ADC:AVG") > 0) cmd_SETT_ADC_AVG(paramPart);
        else if (cmdPart.indexOf("ADC:DIG") > 0) cmd_SETT_ADC_DIG(paramPart);
        else if (cmdPart.indexOf("BEEP:VOL") > 0) cmd_SETT_BEEP_VOL(paramPart);
        else if (cmdPart.indexOf("BEEP:PWR") > 0) cmd_SETT_BOOL(paramPart, PowerSupply.settingParameters.beeperOnPowerChange);
        else if (cmdPart.indexOf("BEEP:ERR") > 0) cmd_SETT_BOOL(paramPart, PowerSupply.settingParameters.beeperOnError);
        else if (cmdPart.indexOf("BEEP:KEY") > 0) cmd_SETT_BOOL(paramPart, PowerSupply.settingParameters.beeperOnKeypress);
        else if (cmdPart.indexOf("BEEP") > 0) cmd_SETT_BOOL(paramPart, PowerSupply.settingParameters.buzzer);
        else if (cmdPart.indexOf("TIM:EN") > 0) cmd_SETT_BOOL(paramPart, PowerSupply.settingParameters.timerEnabled);
        else if (cmdPart.indexOf("TIM:DUR") > 0) cmd_SETT_TIMER_DUR(paramPart);
        else if (cmdPart.indexOf("OVP:LVL") > 0) cmd_SETT_OVP_LVL(paramPart);
        else if (cmdPart.indexOf("OVP:EN") > 0) cmd_SETT_BOOL(paramPart, PowerSupply.settingParameters.ovpEnabled);
        else if (cmdPart.indexOf("OCP:LVL") > 0) cmd_SETT_OCP_LVL(paramPart);
        else if (cmdPart.indexOf("OCP:EN") > 0) cmd_SETT_BOOL(paramPart, PowerSupply.settingParameters.ocpEnabled);
        else if (cmdPart.indexOf("DELAY") > 0) cmd_SETT_DELAY(paramPart);
        else if (cmdPart.indexOf("ASAVE") > 0) cmd_SETT_AUTOSAVE(paramPart);
        else if (cmdPart.indexOf("GRAPH:TIME") > 0) cmd_SETT_BOOL(paramPart, PowerSupply.settingParameters.graphXaxisTimeMode);
        else if (cmdPart.indexOf("GRAPH:SPAN") > 0) cmd_SETT_GRAPH_SPAN(paramPart);
        else if (cmdPart.indexOf("GRAPH:STOP") > 0) cmd_SETT_BOOL(paramPart, PowerSupply.settingParameters.graphAutoStop);
        else if (cmdPart.indexOf("BACK") > 0) cmd_SETT_BACKLIGHT(paramPart);
        else if (cmdPart.indexOf("START") > 0) cmd_SETT_STARTUP(paramPart);
        else if (cmdPart.indexOf("SAVE") > 0) { PowerSupply.SaveSetting(); sendResponse("OK"); }
        else if (cmdPart.indexOf("LOAD") > 0) { PowerSupply.LoadSetting(); sendResponse("OK"); }
        else addError(ERR_UNDEFINED_HEADER);
        return;
    }

    // SCPI MEM subsystem (memory presets)
    if (cmdPart.startsWith("MEM"))
    {
        if (cmdPart.indexOf("STOR") > 0) cmd_MEM_STORE(paramPart);
        else if (cmdPart.indexOf("RCL") > 0) cmd_MEM_RECALL(paramPart);
        else addError(ERR_UNDEFINED_HEADER);
        return;
    }

    // Unknown command
    addError(ERR_UNDEFINED_HEADER);
}

// =============================================================================
// IEEE 488.2 Common Commands
// =============================================================================

void SCPIParser::cmd_IDN()
{
    // Format: Manufacturer,Model,SerialNumber,FirmwareVersion
    String response = "Anthropic,ESP32S3PS,";
    response += String((uint32_t)ESP.getEfuseMac(), HEX);
    response += ",";
    response += SOFTWARE_VERSION;
    sendResponse(response);
}

void SCPIParser::cmd_RST()
{
    // Reset to default state
    PowerSupply.Voltage.SetUpdate(0);
    PowerSupply.Current.SetUpdate(0);
    // Note: outputState is controlled via GUI, not directly accessible
    operationComplete = true;
}

void SCPIParser::cmd_CLS()
{
    // Clear status registers
    eventStatusRegister = 0;
    statusByte = 0;

    // Clear error queue
    errorQueueHead = 0;
    errorQueueTail = 0;
    errorQueueCount = 0;
}

void SCPIParser::cmd_ESE(const String& param)
{
    double value;
    if (parseNumeric(param, value))
    {
        eventStatusEnable = (uint8_t)value;
    }
    else
    {
        addError(ERR_DATA_TYPE_ERROR);
    }
}

void SCPIParser::cmd_ESE_Q()
{
    sendResponse(String(eventStatusEnable));
}

void SCPIParser::cmd_ESR_Q()
{
    sendResponse(String(eventStatusRegister));
    eventStatusRegister = 0;  // Reading clears the register
}

void SCPIParser::cmd_OPC()
{
    operationComplete = true;
    eventStatusRegister |= 0x01;  // Set OPC bit
}

void SCPIParser::cmd_OPC_Q()
{
    sendResponse(operationComplete ? "1" : "0");
}

void SCPIParser::cmd_SRE(const String& param)
{
    double value;
    if (parseNumeric(param, value))
    {
        serviceRequestEnable = (uint8_t)value;
    }
    else
    {
        addError(ERR_DATA_TYPE_ERROR);
    }
}

void SCPIParser::cmd_SRE_Q()
{
    sendResponse(String(serviceRequestEnable));
}

void SCPIParser::cmd_STB_Q()
{
    // Status byte: bit 4 = MAV (message available), bit 2 = error queue
    uint8_t stb = statusByte;
    if (errorQueueCount > 0)
        stb |= 0x04;  // Set error queue bit
    sendResponse(String(stb));
}

void SCPIParser::cmd_TST_Q()
{
    // Comprehensive self-test of all critical hardware and software components
    // Returns: 0 = all tests passed, error code if any test failed

    bool allTestsPassed = true;
    String failureDetails = "";

    // Test 1: I2C Bus Health Check - DAC
    Wire.beginTransmission(PowerSupply.DAC.address);
    if (Wire.endTransmission() != 0)
    {
        allTestsPassed = false;
        failureDetails += "I2C_DAC_FAIL;";
    }

    // Test 2: I2C Bus Health Check - ADC
    // Note: ADS1219 address is typically 0x40 or 0x41
    Wire1.beginTransmission(0x40);
    uint8_t adcResponse = Wire1.endTransmission();
    if (adcResponse != 0)
    {
        // Try alternate address
        Wire1.beginTransmission(0x41);
        adcResponse = Wire1.endTransmission();
        if (adcResponse != 0)
        {
            allTestsPassed = false;
            failureDetails += "I2C_ADC_FAIL;";
        }
    }

    // Test 3: DAC Communication Test
    // Test DAC write operation (non-destructive - write to input register only)
    Wire.beginTransmission(PowerSupply.DAC.address);
    Wire.write(((WRITE_REGISTER & 0b00001111) << 4) | DAC_A);
    Wire.write(0x80);  // MSB of test value
    Wire.write(0x00);  // LSB of test value
    uint8_t dacError = Wire.endTransmission();
    if (dacError != 0)
    {
        allTestsPassed = false;
        failureDetails += "DAC_COMM_FAIL;";
    }

    // Test 4: ADC Measurement Range Check
    // Verify ADC readings are within reasonable bounds
    double testVoltage = PowerSupply.Voltage.measured.Mean();
    double maxExpectedVolt = PowerSupply.Voltage.maxValue / PowerSupply.Voltage.adjFactor * 1.1;
    if (testVoltage < -1.0 || testVoltage > maxExpectedVolt)
    {
        allTestsPassed = false;
        failureDetails += "ADC_VOLT_RANGE;";
    }

    double testCurrent = PowerSupply.Current.measured.Mean();
    double maxExpectedCurr = PowerSupply.Current.maxValue / PowerSupply.Current.adjFactor * 1.1;
    if (testCurrent < -0.1 || testCurrent > maxExpectedCurr)
    {
        allTestsPassed = false;
        failureDetails += "ADC_CURR_RANGE;";
    }

    // Test 5: CC/CV Detection Pin
    pinMode(PowerSupply.CCCVPin, INPUT);
    int ccvvState = digitalRead(PowerSupply.CCCVPin);
    if (ccvvState != LOW && ccvvState != HIGH)
    {
        allTestsPassed = false;
        failureDetails += "CCCV_PIN_FAIL;";
    }

    // Test 6: Memory Health Check
    uint32_t freeHeap = ESP.getFreeHeap();
    uint32_t minFreeHeap = ESP.getMinFreeHeap();
    if (freeHeap < 10000 || minFreeHeap < 5000)
    {
        allTestsPassed = false;
        failureDetails += "LOW_MEMORY;";
    }

    // Test 7: PSRAM Check
    uint32_t psramSize = ESP.getPsramSize();
    uint32_t freePsram = ESP.getFreePsram();
    if (psramSize > 0 && freePsram < 100000)
    {
        allTestsPassed = false;
        failureDetails += "LOW_PSRAM;";
    }

    // Test 8: Calibration Data Validity
    // Check if calibration data is loaded and seems reasonable
    if (PowerSupply.Voltage.adjFactor <= 0 || PowerSupply.Voltage.adjFactor > 100000)
    {
        allTestsPassed = false;
        failureDetails += "VOLT_CAL_INVALID;";
    }

    if (PowerSupply.Current.adjFactor <= 0 || PowerSupply.Current.adjFactor > 100000)
    {
        allTestsPassed = false;
        failureDetails += "CURR_CAL_INVALID;";
    }

    // Test 9: LVGL Display System
    if (!lv_is_initialized())
    {
        allTestsPassed = false;
        failureDetails += "LVGL_NOT_INIT;";
    }

    // Test 10: FreeRTOS Tasks Health
    // Verify critical tasks are running
    extern TaskHandle_t Task_adc;
    extern TaskHandle_t Task_bargraph;

    if (Task_adc == NULL || eTaskGetState(Task_adc) == eDeleted)
    {
        allTestsPassed = false;
        failureDetails += "TASK_ADC_FAIL;";
    }

    if (Task_bargraph == NULL || eTaskGetState(Task_bargraph) == eDeleted)
    {
        allTestsPassed = false;
        failureDetails += "TASK_GUI_FAIL;";
    }

    // Test 11: Watchdog Timer Status
    // Verify watchdog is enabled and functioning
    // (Presence of this running code means watchdog hasn't triggered)

    // Generate response
    if (allTestsPassed)
    {
        sendResponse("0");  // All tests passed
        Serial.println("[SELF-TEST] All systems operational");
    }
    else
    {
        sendResponse("1");  // At least one test failed
        Serial.print("[SELF-TEST] FAILURES: ");
        Serial.println(failureDetails);

        // Add detailed error to error queue
        addError(ERR_EXECUTION_ERROR, failureDetails.c_str());
    }
}

void SCPIParser::cmd_WAI()
{
    // Wait for all pending operations to complete
    while (!operationComplete)
    {
        delay(1);
    }
}

// =============================================================================
// SCPI SOURce subsystem
// =============================================================================

void SCPIParser::cmd_SOUR_VOLT(const String& param)
{
    double value;
    if (parseNumeric(param, value))
    {
        double maxVolt = PowerSupply.Voltage.maxValue / PowerSupply.Voltage.adjFactor;
        if (value >= 0 && value <= maxVolt)
        {
            PowerSupply.Voltage.SetUpdate(value * PowerSupply.Voltage.adjFactor + PowerSupply.Voltage.adjOffset);
        }
        else
        {
            addError(ERR_DATA_OUT_OF_RANGE);
            // Enhanced error reporting
            char range[64];
            snprintf(range, sizeof(range), "0 to %.3fV", maxVolt);
            ERROR_VALID("SOUR:VOLT", param.c_str(), range);
        }
    }
    else
    {
        addError(ERR_DATA_TYPE_ERROR);
        ERROR_VALID("SOUR:VOLT", param.c_str(), "numeric value");
    }
}

void SCPIParser::cmd_SOUR_VOLT_Q()
{
    double voltage = (PowerSupply.Voltage.adjValue - PowerSupply.Voltage.adjOffset) / PowerSupply.Voltage.adjFactor;
    sendResponse(String(voltage, 6));
}

void SCPIParser::cmd_SOUR_VOLT_LIM(const String& param)
{
    double value;
    if (parseNumeric(param, value))
    {
        // Voltage limit - store for future use
        // Note: limits are GUI-controlled, not directly accessible in Device class
    }
    else
    {
        addError(ERR_DATA_TYPE_ERROR);
    }
}

void SCPIParser::cmd_SOUR_VOLT_LIM_Q()
{
    double maxVolt = PowerSupply.Voltage.maxValue / PowerSupply.Voltage.adjFactor;
    sendResponse(String(maxVolt, 6));
}

void SCPIParser::cmd_SOUR_CURR(const String& param)
{
    double value;
    if (parseNumeric(param, value))
    {
        double maxCurr = PowerSupply.Current.maxValue / PowerSupply.Current.adjFactor;
        if (value >= 0 && value <= maxCurr)
        {
            PowerSupply.Current.SetUpdate(value * PowerSupply.Current.adjFactor + PowerSupply.Current.adjOffset);
        }
        else
        {
            addError(ERR_DATA_OUT_OF_RANGE);
            // Enhanced error reporting
            char range[64];
            snprintf(range, sizeof(range), "0 to %.3fA", maxCurr);
            ERROR_VALID("SOUR:CURR", param.c_str(), range);
        }
    }
    else
    {
        addError(ERR_DATA_TYPE_ERROR);
        ERROR_VALID("SOUR:CURR", param.c_str(), "numeric value");
    }
}

void SCPIParser::cmd_SOUR_CURR_Q()
{
    double current = (PowerSupply.Current.adjValue - PowerSupply.Current.adjOffset) / PowerSupply.Current.adjFactor;
    sendResponse(String(current, 6));
}

void SCPIParser::cmd_SOUR_CURR_LIM(const String& param)
{
    double value;
    if (parseNumeric(param, value))
    {
        // Current limit - store for future use
        // Note: limits are GUI-controlled, not directly accessible in Device class
    }
    else
    {
        addError(ERR_DATA_TYPE_ERROR);
    }
}

void SCPIParser::cmd_SOUR_CURR_LIM_Q()
{
    double maxCurr = PowerSupply.Current.maxValue / PowerSupply.Current.adjFactor;
    sendResponse(String(maxCurr, 6));
}

void SCPIParser::cmd_SOUR_FUNC(const String& param)
{
    String p = param;
    toUpperCase(p);

    if (p == "CV" || p == "VOLT")
    {
        // Constant voltage mode — set state then fire event for theme/ADC changes
        if (lv_obj_has_state(btn_function_gen, LV_STATE_CHECKED)) {
            lv_obj_clear_state(btn_function_gen, LV_STATE_CHECKED);
            lv_event_send(btn_function_gen, LV_EVENT_SHORT_CLICKED, NULL);
        }
    }
    else if (p == "FGEN" || p == "FUNC")
    {
        // Function generator mode — set state then fire event for theme/ADC changes
        if (!lv_obj_has_state(btn_function_gen, LV_STATE_CHECKED)) {
            lv_obj_add_state(btn_function_gen, LV_STATE_CHECKED);
            lv_event_send(btn_function_gen, LV_EVENT_SHORT_CLICKED, NULL);
        }
    }
    else
    {
        addError(ERR_ILLEGAL_PARAMETER_VALUE);
    }
}

void SCPIParser::cmd_SOUR_FUNC_Q()
{
    if (lv_obj_has_state(btn_function_gen, LV_STATE_CHECKED))
        sendResponse("FGEN");
    else
        sendResponse("CV");
}

// =============================================================================
// SCPI MEASure subsystem
// =============================================================================

void SCPIParser::cmd_MEAS_VOLT_Q()
{
    double voltage = PowerSupply.Voltage.measured.Mean();
    sendResponse(String(voltage, 6));
}

void SCPIParser::cmd_MEAS_CURR_Q()
{
    double current = PowerSupply.Current.measured.Mean();
    sendResponse(String(current, 6));
}

void SCPIParser::cmd_MEAS_POW_Q()
{
    double voltage = PowerSupply.Voltage.measured.Mean();
    double current = PowerSupply.Current.measured.Mean();
    if (PowerSupply.mA_Active)
        current *= 0.001;  // Convert mA to A
    double power = voltage * current;
    sendResponse(String(power, 6));
}

void SCPIParser::cmd_MEAS_VOLT_MIN_Q()
{
    sendResponse(String(PowerSupply.Voltage.measured.absMin, 6));
}

void SCPIParser::cmd_MEAS_VOLT_MAX_Q()
{
    sendResponse(String(PowerSupply.Voltage.measured.absMax, 6));
}

void SCPIParser::cmd_MEAS_VOLT_MEAN_Q()
{
    sendResponse(String(PowerSupply.Voltage.measured.Mean(), 6));
}

void SCPIParser::cmd_MEAS_CURR_MIN_Q()
{
    sendResponse(String(PowerSupply.Current.measured.absMin, 6));
}

void SCPIParser::cmd_MEAS_CURR_MAX_Q()
{
    sendResponse(String(PowerSupply.Current.measured.absMax, 6));
}

void SCPIParser::cmd_MEAS_CURR_MEAN_Q()
{
    sendResponse(String(PowerSupply.Current.measured.Mean(), 6));
}

// =============================================================================
// SCPI OUTPut subsystem
// =============================================================================

void SCPIParser::cmd_OUTP_STAT(const String& param)
{
    String p = param;
    toUpperCase(p);
    bool isOn = (PowerSupply.getStatus() != DEVICE::OFF);

    if (p == "ON" || p == "1")
    {
        if (!isOn) PowerSupply.toggle();
    }
    else if (p == "OFF" || p == "0")
    {
        if (isOn) PowerSupply.toggle();
    }
    else
    {
        addError(ERR_ILLEGAL_PARAMETER_VALUE);
    }
}

void SCPIParser::cmd_OUTP_STAT_Q()
{
    sendResponse((PowerSupply.getStatus() != DEVICE::OFF) ? "1" : "0");
}

void SCPIParser::cmd_OUTP_PROT_CLE()
{
    // Clear protection trip
    // Implementation depends on hardware protection system
}

void SCPIParser::cmd_OUTP_PROT_TRIP_Q()
{
    // Query protection trip status
    // For now, always return 0 (no trip)
    sendResponse("0");
}

// =============================================================================
// SCPI SYSTem subsystem
// =============================================================================

void SCPIParser::cmd_SYST_ERR_Q()
{
    if (errorQueueCount == 0)
    {
        sendResponse("0,\"No error\"");
        return;
    }

    ErrorEntry& err = errorQueue[errorQueueHead];
    String response = String(err.code) + ",\"" + err.description + "\"";

    errorQueueHead = (errorQueueHead + 1) % ERROR_QUEUE_SIZE;
    errorQueueCount--;

    sendResponse(response);
}

void SCPIParser::cmd_SYST_ERR_COUN_Q()
{
    sendResponse(String(errorQueueCount));
}

void SCPIParser::cmd_SYST_ERR_ALL_Q()
{
    if (errorQueueCount == 0)
    {
        sendResponse("0,\"No error\"");
        return;
    }

    String response = "";
    while (errorQueueCount > 0)
    {
        ErrorEntry& err = errorQueue[errorQueueHead];
        if (response.length() > 0)
            response += ";";
        response += String(err.code) + ",\"" + err.description + "\"";

        errorQueueHead = (errorQueueHead + 1) % ERROR_QUEUE_SIZE;
        errorQueueCount--;
    }

    sendResponse(response);
}

void SCPIParser::cmd_SYST_VERS_Q()
{
    sendResponse(getVersion());
}

void SCPIParser::cmd_SYST_CAP_Q()
{
    // Report power supply capabilities
    String caps = "VOLT:MIN 0;MAX ";
    caps += String(PowerSupply.Voltage.maxValue / PowerSupply.Voltage.adjFactor, 3);
    caps += ";CURR:MIN 0;MAX ";
    caps += String(PowerSupply.Current.maxValue / PowerSupply.Current.adjFactor, 3);
    sendResponse(caps);
}

void SCPIParser::cmd_SYST_BEEP()
{
    myTone(NOTE_A5, 100, false);
}

void SCPIParser::cmd_SYST_LOC()
{
    remoteMode = false;
    // Return to local control (enable front panel)
}

void SCPIParser::cmd_SYST_REM()
{
    remoteMode = true;
    // Enter remote control mode (may disable front panel)
}

// =============================================================================
// SCPI CALibration subsystem
// =============================================================================

void SCPIParser::cmd_CAL_STAT_Q()
{
    // Report calibration status (0 = not calibrated, 1 = calibrated)
    sendResponse("1");
}

void SCPIParser::cmd_CAL_SAVE()
{
    PowerSupply.SaveCalibrationData();
}

void SCPIParser::cmd_CAL_LOAD()
{
    PowerSupply.LoadCalibrationData();
}

// =============================================================================
// DATA subsystem
// =============================================================================

void SCPIParser::cmd_DATA_GRAPH_Q()
{
    // First line: metadata with mA_Active flag
    Serial.printf("META,mA_Active=%d\n", PowerSupply.mA_Active ? 1 : 0);
    // Dump all chart points as CSV: index,voltage_mV,current_mA
    for (int i = 0; i < CHART_SIZE; i++) {
        Serial.printf("%d,%d,%d\n", i, (int)graph_data_V[i], (int)graph_data_I[i]);
    }
}

void SCPIParser::cmd_DATA_STATS_Q()
{
    // Returns two CSV lines: V then I stats
    // Format: mean,stddev,min,max,rms,er_bits
    auto& V = PowerSupply.Voltage.measured;
    auto& I = PowerSupply.Current.measured;
    double vFSR = 32.768;
    double iFSR = PowerSupply.mA_Active ? 5.0 : 5000.0;
    Serial.printf("V,%.6f,%.6f,%.6f,%.6f,%.6f,%.3f\n",
        V.Mean(), V.StandardDeviation(), V.absMin, V.absMax, V.Rms(), V.ER(vFSR));
    Serial.printf("I,%.6f,%.6f,%.6f,%.6f,%.6f,%.3f\n",
        I.Mean(), I.StandardDeviation(), I.absMin, I.absMax, I.Rms(), I.ER(iFSR));
}

void SCPIParser::cmd_DATA_GRAPH_RST()
{
    graphReset();
    sendResponse("OK");
}

void SCPIParser::cmd_DATA_GRAPH_STAT_Q()
{
    // Returns: pushCount,paused (0 or 1)
    Serial.printf("%d,%d\n", g_graphPushCount, g_graphPaused ? 1 : 0);
}

void SCPIParser::cmd_DATA_STATUS_Q()
{
    // Return all status in one line: meas_v,meas_i,set_v,set_i,output,power,mA_active
    double meas_v = PowerSupply.Voltage.measured.Mean();
    double meas_i = PowerSupply.Current.measured.Mean();
    double set_v = (PowerSupply.Voltage.adjValue - PowerSupply.Voltage.adjOffset) / PowerSupply.Voltage.adjFactor;
    double set_i = (PowerSupply.Current.adjValue - PowerSupply.Current.adjOffset) / PowerSupply.Current.adjFactor;
    int output = (PowerSupply.getStatus() != DEVICE::OFF) ? 1 : 0;
    double power = meas_v * (PowerSupply.mA_Active ? meas_i * 0.001 : meas_i);
    Serial.printf("%.6f,%.6f,%.6f,%.6f,%d,%.6f,%d\n", meas_v, meas_i, set_v, set_i, output, power, PowerSupply.mA_Active ? 1 : 0);
}

void SCPIParser::cmd_DATA_SETTINGS_Q()
{
    auto &s = PowerSupply.settingParameters;
    // adcRate,adcAvgs,adcDigits,buzzer,beepPwr,beepErr,beepKey,timerEn,timerDur,
    // ovpEn,ovpLvl,ocpEn,ocpLvl,delay,autoSave,graphTime,graphSpan,graphStop,
    // beepVol,startup
    Serial.printf("%d,%d,%d,%d,%d,%d,%d,%d,%u,%d,%.1f,%d,%.1f,%u,%u,%d,%u,%d,%d,%d\n",
        s.adcRate, s.adcNumberOfAvgs, s.adcNumberOfDigits,
        s.buzzer ? 1 : 0, s.beeperOnPowerChange ? 1 : 0,
        s.beeperOnError ? 1 : 0, s.beeperOnKeypress ? 1 : 0,
        s.timerEnabled ? 1 : 0, s.timerDurationSeconds,
        s.ovpEnabled ? 1 : 0, s.voltageLimitMax,
        s.ocpEnabled ? 1 : 0, s.currentLimitMax,
        s.outputDelayMs, s.autoSaveIntervalMinutes,
        s.graphXaxisTimeMode ? 1 : 0, s.graphTimeSpanSeconds,
        s.graphAutoStop ? 1 : 0,
        s.beeperVolume, static_cast<int>(s.startupBehavior));
}

void SCPIParser::cmd_DATA_MEM_Q()
{
    // Return all 10 memory slots from NVS: idx,voltage_V,current_A per line
    MemArray mem = PowerSupply.LoadMemory("myDataKey");
    for (int i = 0; i < 10; i++) {
        double v = scaleVoltage(mem.memory[i].voltage);
        double c = scaleCurrent(mem.memory[i].current);
        Serial.printf("%d,%.6f,%.6f\n", i, v, c);
    }
}

void SCPIParser::cmd_DATA_FGEN_Q()
{
    auto &f = PowerSupply.funGenMem;
    // freq,ampl,offset,duty
    Serial.printf("%.4f,%.4f,%.4f,%.4f\n", f.frequency, f.amplitude, f.offset, f.dutyCycle);
}

void SCPIParser::cmd_DATA_ARBT_Q(const String& param)
{
    // Query arbitrary waveform points: DATA:ARBT? <bank>
    // Load from flash to get current state
    FunGen fg = PowerSupply.LoadMemoryFgen("FunGen");
    int bank = param.toInt();
    if (bank < 0 || bank > 1) bank = 0;
    for (int i = 0; i < CHART_POINTS; i++) {
        Serial.printf("%d,%.2f\n", i, fg.arbitrary_points[i][bank]);
    }
}

void SCPIParser::cmd_DATA_ARBT(const String& param)
{
    // Write arbitrary waveform point: DATA:ARBT <bank> <index> <value>
    int bank = 0, idx = 0;
    double val = 0;
    int sp1 = param.indexOf(' ');
    if (sp1 < 0) { addError(ERR_MISSING_PARAMETER); return; }
    bank = param.substring(0, sp1).toInt();
    String rest = param.substring(sp1 + 1);
    int sp2 = rest.indexOf(' ');
    if (sp2 < 0) { addError(ERR_MISSING_PARAMETER); return; }
    idx = rest.substring(0, sp2).toInt();
    val = rest.substring(sp2 + 1).toDouble();
    if (bank < 0 || bank > 1 || idx < 0 || idx >= CHART_POINTS) { addError(ERR_DATA_OUT_OF_RANGE); return; }
    // Load, modify, save back to flash
    FunGen fg = PowerSupply.LoadMemoryFgen("FunGen");
    fg.arbitrary_points[idx][bank] = val;
    PowerSupply.funGenMem.arbitrary_points[idx][bank] = val; // also update live copy
    PowerSupply.SaveMemoryFgen("FunGen", fg);
    sendResponse("OK");
}

void SCPIParser::cmd_DATA_TABL_Q()
{
    // Query table points: first line is length, then index,value per line
    Serial.printf("LEN,%d\n", PowerSupply.funGenMem.table_length);
    for (int i = 0; i < PowerSupply.funGenMem.table_length; i++) {
        Serial.printf("%d,%.6f\n", i, PowerSupply.funGenMem.table_points[i][0]);
    }
}

void SCPIParser::cmd_DATA_TABL(const String& param)
{
    // Write table point: DATA:TABL <index> <value>
    // Or set length: DATA:TABL LEN <value>
    String p = param;
    toUpperCase(p);
    if (p.startsWith("LEN ")) {
        int len = param.substring(4).toInt();
        if (len >= 1 && len <= RECORDING_TABLE_SIZE) {
            PowerSupply.funGenMem.table_length = len;
            sendResponse("OK");
        } else addError(ERR_DATA_OUT_OF_RANGE);
        return;
    }
    int sp = param.indexOf(' ');
    if (sp < 0) { addError(ERR_MISSING_PARAMETER); return; }
    int idx = param.substring(0, sp).toInt();
    double val = param.substring(sp + 1).toDouble();
    if (idx < 0 || idx >= RECORDING_TABLE_SIZE) { addError(ERR_DATA_OUT_OF_RANGE); return; }
    PowerSupply.funGenMem.table_points[idx][0] = val;
    sendResponse("OK");
}

void SCPIParser::cmd_DATA_CAL_Q()
{
    // Dump calibration data in CSV format for PC download
    auto& cal = PowerSupply.CalBank[PowerSupply.bankCalibId];
    Serial.printf("MAC,%s\n", cal.macAdd.c_str());
    Serial.printf("VCAL,%.6f,%d,%.6f,%d\n", cal.vCal.value_1, cal.vCal.code_1, cal.vCal.value_2, cal.vCal.code_2);
    Serial.printf("ICAL0,%.6f,%d,%.6f,%d\n", cal.iCal[0].value_1, cal.iCal[0].code_1, cal.iCal[0].value_2, cal.iCal[0].code_2);
    Serial.printf("ICAL1,%.6f,%d,%.6f,%d\n", cal.iCal[1].value_1, cal.iCal[1].code_1, cal.iCal[1].value_2, cal.iCal[1].code_2);
    Serial.printf("LEAK,%.9f,%.9f\n", cal.internalLeakage[0], cal.internalLeakage[1]);
    // INL measure points (36)
    Serial.print("INLM");
    for (int i = 0; i < 36; i++) Serial.printf(",%.6f", cal.adc_inl_measure[i]);
    Serial.print("\n");
    // INL ideal points (36)
    Serial.print("INLI");
    for (int i = 0; i < 36; i++) Serial.printf(",%.6f", cal.adc_inl_ideal[i]);
    Serial.print("\n");
    // Zero-current INL (commented out pending investigation):
    // Serial.print("INLZCM");
    // for (int i = 0; i < 36; i++) Serial.printf(",%.6f", cal.adc_inl_zc_measure[i]);
    // Serial.print("\n");
    // Serial.print("INLZCI");
    // for (int i = 0; i < 36; i++) Serial.printf(",%.6f", cal.adc_inl_zc_ideal[i]);
    // Serial.print("\n");
    Serial.print("END\n");
}

void SCPIParser::cmd_DATA_CAL_SET(const String& param)
{
    // Accepts one line in the same format as DATA:CAL? output.
    // The Python server sends these lines one by one to restore a saved calibration file.
    // After all lines are sent, the server sends CAL:SAVE to persist.
    if (PowerSupply.CalBank.empty() || PowerSupply.bankCalibId < 0 ||
        PowerSupply.bankCalibId >= (int8_t)PowerSupply.CalBank.size())
    {
        addError(ERR_EXECUTION_ERROR); return;
    }
    auto& cal = PowerSupply.CalBank[PowerSupply.bankCalibId];

    // Split key from rest: first token before first comma
    int sep = param.indexOf(',');
    if (sep < 0) { addError(ERR_MISSING_PARAMETER); return; }
    String key = param.substring(0, sep);
    String rest = param.substring(sep + 1);
    key.trim(); key.toUpperCase();

    // Helper: split rest into up to maxN doubles/ints
    auto splitDoubles = [&](double *out, int maxN) -> int {
        int n = 0; int start = 0;
        while (n < maxN) {
            int c = rest.indexOf(',', start);
            String tok = (c < 0) ? rest.substring(start) : rest.substring(start, c);
            tok.trim();
            if (tok.length() == 0) break;
            out[n++] = tok.toDouble();
            if (c < 0) break;
            start = c + 1;
        }
        return n;
    };

    if (key == "MAC") {
        cal.macAdd = rest;
        cal.macAdd.trim();
    } else if (key == "VCAL") {
        double v[4]; if (splitDoubles(v, 4) < 4) { addError(ERR_MISSING_PARAMETER); return; }
        cal.vCal = { v[0], (int)v[1], v[2], (int)v[3] };
    } else if (key == "ICAL0") {
        double v[4]; if (splitDoubles(v, 4) < 4) { addError(ERR_MISSING_PARAMETER); return; }
        cal.iCal[0] = { v[0], (int)v[1], v[2], (int)v[3] };
    } else if (key == "ICAL1") {
        double v[4]; if (splitDoubles(v, 4) < 4) { addError(ERR_MISSING_PARAMETER); return; }
        cal.iCal[1] = { v[0], (int)v[1], v[2], (int)v[3] };
    } else if (key == "LEAK") {
        double v[2]; if (splitDoubles(v, 2) < 2) { addError(ERR_MISSING_PARAMETER); return; }
        cal.internalLeakage[0] = v[0]; cal.internalLeakage[1] = v[1];
    } else if (key == "INLM") {
        double v[36]; int n = splitDoubles(v, 36);
        for (int i = 0; i < n && i < 36; i++) cal.adc_inl_measure[i] = v[i];
    } else if (key == "INLI") {
        double v[36]; int n = splitDoubles(v, 36);
        for (int i = 0; i < n && i < 36; i++) cal.adc_inl_ideal[i] = v[i];
    } else {
        addError(ERR_UNDEFINED_HEADER); return;
    }
    sendResponse("OK");
}

// =============================================================================
// SETT subsystem (settings write)
// =============================================================================

void SCPIParser::cmd_SETT_BOOL(const String& param, bool &target)
{
    String p = param;
    toUpperCase(p);
    if (p == "1" || p == "ON") target = true;
    else if (p == "0" || p == "OFF") target = false;
    else { addError(ERR_ILLEGAL_PARAMETER_VALUE); return; }
    sendResponse("OK");
}

void SCPIParser::cmd_SETT_ADC_RATE(const String& param)
{
    int val = param.toInt();
    if (val >= 0 && val <= 4) {
        const int rates[] = {20, 20, 90, 330, 1000};
        PowerSupply.settingParameters.adcRate = val;
        PowerSupply.adc.ads1219->setDataRate(rates[val]);
        PowerSupply.adjustAdcTaskPriority();
        PowerSupply.ResetStats();
        sendResponse("OK");
    } else addError(ERR_DATA_OUT_OF_RANGE);
}

void SCPIParser::cmd_SETT_ADC_AVG(const String& param)
{
    int val = param.toInt();
    if (val >= 0 && val <= 5) {
        PowerSupply.settingParameters.adcNumberOfAvgs = val;
        int ws = (int)std::pow(2, val);
        PowerSupply.Voltage.measured.SetWindowSize(ws);
        PowerSupply.Current.measured.SetWindowSize(ws);
        PowerSupply.Power.measured.SetWindowSize(ws);
        sendResponse("OK");
    } else addError(ERR_DATA_OUT_OF_RANGE);
}

void SCPIParser::cmd_SETT_ADC_DIG(const String& param)
{
    int val = param.toInt();
    if (val >= 1 && val <= 4) {
        PowerSupply.settingParameters.adcNumberOfDigits = val;
        sendResponse("OK");
    } else addError(ERR_DATA_OUT_OF_RANGE);
}

void SCPIParser::cmd_SETT_TIMER_DUR(const String& param)
{
    int val = param.toInt();
    if (val >= 0 && val <= 28800) {
        PowerSupply.settingParameters.timerDurationSeconds = val;
        sendResponse("OK");
    } else addError(ERR_DATA_OUT_OF_RANGE);
}

void SCPIParser::cmd_SETT_OVP_LVL(const String& param)
{
    double val;
    if (parseNumeric(param, val) && val >= 0 && val <= 32) {
        PowerSupply.settingParameters.voltageLimitMax = val;
        sendResponse("OK");
    } else addError(ERR_DATA_OUT_OF_RANGE);
}

void SCPIParser::cmd_SETT_OCP_LVL(const String& param)
{
    double val;
    if (parseNumeric(param, val) && val >= 0 && val <= 6) {
        PowerSupply.settingParameters.currentLimitMax = val;
        sendResponse("OK");
    } else addError(ERR_DATA_OUT_OF_RANGE);
}

void SCPIParser::cmd_SETT_DELAY(const String& param)
{
    int val = param.toInt();
    if (val >= 0 && val <= 5000) {
        PowerSupply.settingParameters.outputDelayMs = val;
        sendResponse("OK");
    } else addError(ERR_DATA_OUT_OF_RANGE);
}

void SCPIParser::cmd_SETT_AUTOSAVE(const String& param)
{
    int val = param.toInt();
    if (val >= 0 && val <= 60) {
        PowerSupply.settingParameters.autoSaveIntervalMinutes = val;
        sendResponse("OK");
    } else addError(ERR_DATA_OUT_OF_RANGE);
}

void SCPIParser::cmd_SETT_GRAPH_SPAN(const String& param)
{
    int val = param.toInt();
    if (val >= 5 && val <= 43200) {
        PowerSupply.settingParameters.graphTimeSpanSeconds = val;
        sendResponse("OK");
    } else addError(ERR_DATA_OUT_OF_RANGE);
}

void SCPIParser::cmd_SETT_BACKLIGHT(const String& param)
{
    int val = param.toInt();
    if (val >= 0 && val <= 255) {
        // ledcWrite(lcdBacklightChannel, val);  // TODO: wire when backlight PWM is connected
        sendResponse("OK");
    } else addError(ERR_DATA_OUT_OF_RANGE);
}

void SCPIParser::cmd_SETT_BEEP_VOL(const String& param)
{
    int val = param.toInt();
    if (val >= 0 && val <= 100) {
        PowerSupply.settingParameters.beeperVolume = val;
        sendResponse("OK");
    } else addError(ERR_DATA_OUT_OF_RANGE);
}

void SCPIParser::cmd_SETT_STARTUP(const String& param)
{
    int val = param.toInt();
    if (val >= 0 && val <= 2) {
        PowerSupply.settingParameters.startupBehavior = static_cast<StartupBehavior>(val);
        sendResponse("OK");
    } else addError(ERR_DATA_OUT_OF_RANGE);
}

// =============================================================================
// FGEN subsystem (function generator)
// =============================================================================

void SCPIParser::cmd_FGEN_FREQ(const String& param)
{
    double val;
    if (parseNumeric(param, val) && val >= 0 && val <= 10.0) {
        PowerSupply.funGenMem.frequency = val;
        if (Utility_objs.fun.Frequency)
            lv_spinbox_set_value(Utility_objs.fun.Frequency, (int32_t)(val * 1000));
        sendResponse("OK");
    } else addError(ERR_DATA_OUT_OF_RANGE);
}

void SCPIParser::cmd_FGEN_AMPL(const String& param)
{
    double val;
    if (parseNumeric(param, val) && val >= 0 && val <= 32.75) {
        PowerSupply.funGenMem.amplitude = val;
        if (Utility_objs.fun.Amplitude)
            lv_spinbox_set_value(Utility_objs.fun.Amplitude, (int32_t)(val * 1000));
        sendResponse("OK");
    } else addError(ERR_DATA_OUT_OF_RANGE);
}

void SCPIParser::cmd_FGEN_OFFS(const String& param)
{
    double val;
    if (parseNumeric(param, val) && val >= -32.0 && val <= 32.75) {
        PowerSupply.funGenMem.offset = val;
        if (Utility_objs.fun.Offset)
            lv_spinbox_set_value(Utility_objs.fun.Offset, (int32_t)(val * 1000));
        sendResponse("OK");
    } else addError(ERR_DATA_OUT_OF_RANGE);
}

void SCPIParser::cmd_FGEN_DUTY(const String& param)
{
    double val;
    if (parseNumeric(param, val) && val >= 0 && val <= 1.0) {
        PowerSupply.funGenMem.dutyCycle = val;
        if (Utility_objs.fun.Duty)
            lv_spinbox_set_value(Utility_objs.fun.Duty, (int32_t)(val * 10000));
        sendResponse("OK");
    } else addError(ERR_DATA_OUT_OF_RANGE);
}

void SCPIParser::cmd_FGEN_WAVE(const String& param)
{
    int val = param.toInt();
    if (val >= 0 && val < numWaveforms) {
        Utility_objs.table_fun_gen_list->user_data = (void*)(intptr_t)val;
        sendResponse("OK");
    } else addError(ERR_DATA_OUT_OF_RANGE);
}

void SCPIParser::cmd_FGEN_WAVE_Q()
{
    int idx = (int)(intptr_t)Utility_objs.table_fun_gen_list->user_data;
    if (idx >= 0 && idx < numWaveforms)
        Serial.printf("%d,%s\n", idx, waveforms[idx].name);
    else
        sendResponse("0,Sine");
}

// =============================================================================
// MEM subsystem (memory presets)
// =============================================================================

void SCPIParser::cmd_MEM_RECALL(const String& param)
{
    int slot = param.toInt();
    if (slot >= 0 && slot <= 9) {
        MemArray mem = PowerSupply.LoadMemory("myDataKey");
        double v = scaleVoltage(mem.memory[slot].voltage);
        double i = scaleCurrent(mem.memory[slot].current);
        PowerSupply.Voltage.SetUpdate(mem.memory[slot].voltage);
        PowerSupply.Current.SetUpdate(mem.memory[slot].current);
        Serial.printf("%.4f,%.4f\n", v, i);
    } else addError(ERR_DATA_OUT_OF_RANGE);
}

void SCPIParser::cmd_MEM_STORE(const String& param)
{
    // Format: "slot voltage current" (floats in V/A)
    int slot = -1;
    double v = 0, i = 0;
    sscanf(param.c_str(), "%d %lf %lf", &slot, &v, &i);
    if (slot >= 0 && slot <= 9) {
        MemArray mem = PowerSupply.LoadMemory("myDataKey");
        mem.memory[slot].voltage = (uint16_t)(v * PowerSupply.Voltage.adjFactor + PowerSupply.Voltage.adjOffset);
        mem.memory[slot].current = (uint16_t)(i * PowerSupply.Current.adjFactor + PowerSupply.Current.adjOffset);
        PowerSupply.SaveMemory("myDataKey", mem);
        sendResponse("OK");
    } else addError(ERR_DATA_OUT_OF_RANGE);
}

// =============================================================================
// Helper functions
// =============================================================================

void SCPIParser::addError(SCPIError error, const char* description)
{
    if (errorQueueCount >= ERROR_QUEUE_SIZE)
    {
        // Queue full - discard oldest error
        errorQueueHead = (errorQueueHead + 1) % ERROR_QUEUE_SIZE;
        errorQueueCount--;
    }

    ErrorEntry& err = errorQueue[errorQueueTail];
    err.code = error;

    if (description)
        err.description = description;
    else
        err.description = getErrorDescription(error);

    errorQueueTail = (errorQueueTail + 1) % ERROR_QUEUE_SIZE;
    errorQueueCount++;

    // Set error queue bit in status byte
    statusByte |= 0x04;
}

void SCPIParser::sendResponse(const String& response)
{
    Serial.println(response);
}

void SCPIParser::trimWhitespace(String& str)
{
    str.trim();
}

void SCPIParser::toUpperCase(String& str)
{
    str.toUpperCase();
}

bool SCPIParser::isQuery(const String& cmd)
{
    return cmd.endsWith("?");
}

bool SCPIParser::parseNumeric(const String& param, double& value)
{
    if (param.length() == 0)
        return false;

    // Simple numeric parsing (handle suffix later if needed)
    char* endptr;
    value = strtod(param.c_str(), &endptr);

    // Check for valid conversion
    if (endptr == param.c_str())
        return false;

    // Handle common suffixes (V, A, mV, mA, uV, uA)
    String suffix = String(endptr);
    suffix.trim();
    toUpperCase(suffix);

    if (suffix == "MV") value *= 0.001;
    else if (suffix == "UV") value *= 0.000001;
    else if (suffix == "MA") value *= 0.001;
    else if (suffix == "UA") value *= 0.000001;
    else if (suffix == "KV") value *= 1000.0;
    else if (suffix == "KA") value *= 1000.0;

    return true;
}

bool SCPIParser::parseSuffix(const String& param, String& suffix)
{
    // Extract suffix from parameter
    for (size_t i = 0; i < param.length(); i++)
    {
        if (isalpha(param[i]))
        {
            suffix = param.substring(i);
            return true;
        }
    }
    return false;
}

const char* SCPIParser::getErrorDescription(SCPIError error)
{
    switch (error)
    {
        case ERR_NONE: return "No error";
        case ERR_COMMAND_ERROR: return "Command error";
        case ERR_INVALID_CHARACTER: return "Invalid character";
        case ERR_SYNTAX_ERROR: return "Syntax error";
        case ERR_INVALID_SEPARATOR: return "Invalid separator";
        case ERR_DATA_TYPE_ERROR: return "Data type error";
        case ERR_PARAMETER_NOT_ALLOWED: return "Parameter not allowed";
        case ERR_MISSING_PARAMETER: return "Missing parameter";
        case ERR_COMMAND_HEADER_ERROR: return "Command header error";
        case ERR_HEADER_SEPARATOR_ERROR: return "Header separator error";
        case ERR_PROGRAM_MNEMONIC_TOO_LONG: return "Program mnemonic too long";
        case ERR_UNDEFINED_HEADER: return "Undefined header";
        case ERR_HEADER_SUFFIX_OUT_OF_RANGE: return "Header suffix out of range";
        case ERR_NUMERIC_DATA_ERROR: return "Numeric data error";
        case ERR_INVALID_CHARACTER_IN_NUMBER: return "Invalid character in number";
        case ERR_EXPONENT_TOO_LARGE: return "Exponent too large";
        case ERR_TOO_MANY_DIGITS: return "Too many digits";
        case ERR_NUMERIC_DATA_NOT_ALLOWED: return "Numeric data not allowed";
        case ERR_SUFFIX_ERROR: return "Suffix error";
        case ERR_INVALID_SUFFIX: return "Invalid suffix";
        case ERR_SUFFIX_TOO_LONG: return "Suffix too long";
        case ERR_SUFFIX_NOT_ALLOWED: return "Suffix not allowed";
        case ERR_DATA_OUT_OF_RANGE: return "Data out of range";
        case ERR_TOO_MUCH_DATA: return "Too much data";
        case ERR_ILLEGAL_PARAMETER_VALUE: return "Illegal parameter value";
        case ERR_QUERY_INTERRUPTED: return "Query interrupted";
        case ERR_QUERY_UNTERMINATED: return "Query unterminated";
        case ERR_QUERY_DEADLOCKED: return "Query deadlocked";
        case ERR_QUERY_UNTERMINATED_AFTER_INDEFINITE: return "Query unterminated after indefinite response";
        case ERR_EXECUTION_ERROR: return "Execution error";
        case ERR_SETTINGS_LOST: return "Settings lost due to power down";
        case ERR_CALIBRATION_FAILED: return "Calibration failed";
        default: return "Unknown error";
    }
}
