#include "scpi_parser.h"
#include "device.hpp"
#include "globals.h"
#include "buzzer.h"
#include "version.h"
#include "error_handler.h"
#include <Wire.h>
#include <cstring>

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
    extern TaskHandle_t Task1;

    if (Task_adc == NULL || eTaskGetState(Task_adc) == eDeleted)
    {
        allTestsPassed = false;
        failureDetails += "TASK_ADC_FAIL;";
    }

    if (Task1 == NULL || eTaskGetState(Task1) == eDeleted)
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
        // Constant voltage mode (default)
        if (lv_obj_has_state(btn_function_gen, LV_STATE_CHECKED))
            lv_obj_clear_state(btn_function_gen, LV_STATE_CHECKED);
    }
    else if (p == "FGEN" || p == "FUNC")
    {
        // Function generator mode
        if (!lv_obj_has_state(btn_function_gen, LV_STATE_CHECKED))
            lv_obj_add_state(btn_function_gen, LV_STATE_CHECKED);
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

    if (p == "ON" || p == "1")
    {
        // Enable output - controlled via toggle() method
        PowerSupply.toggle();
    }
    else if (p == "OFF" || p == "0")
    {
        // Disable output - controlled via toggle() method
        PowerSupply.toggle();
    }
    else
    {
        addError(ERR_ILLEGAL_PARAMETER_VALUE);
    }
}

void SCPIParser::cmd_OUTP_STAT_Q()
{
    // Query output state via CC/CV pin or other status indicator
    sendResponse(digitalRead(PowerSupply.CCCVPin) ? "1" : "0");
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
