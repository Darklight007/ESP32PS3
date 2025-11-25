#pragma once

#include <Arduino.h>
#include <vector>
#include <functional>

/**
 * @brief SCPI Parser implementing IEEE 488.2 and SCPI-1999 standards
 *
 * This class provides a complete SCPI command parser for programmable power supplies.
 * It implements:
 * - IEEE 488.2 mandatory common commands (*IDN?, *RST, *CLS, etc.)
 * - SCPI power supply subsystems (SOURce, MEASure, OUTPut, STATus, SYSTem)
 * - Error queue with standard SCPI error codes
 * - Query response formatting
 *
 * Command format: [:subsystem]:command[:subcommand] [parameter]
 * Example: :SOUR:VOLT 5.0
 *          :MEAS:VOLT?
 *          *IDN?
 */

// SCPI Error codes (IEEE 488.2 standard)
enum SCPIError {
    ERR_NONE = 0,
    ERR_COMMAND_ERROR = -100,
    ERR_INVALID_CHARACTER = -101,
    ERR_SYNTAX_ERROR = -102,
    ERR_INVALID_SEPARATOR = -103,
    ERR_DATA_TYPE_ERROR = -104,
    ERR_PARAMETER_NOT_ALLOWED = -108,
    ERR_MISSING_PARAMETER = -109,
    ERR_COMMAND_HEADER_ERROR = -110,
    ERR_HEADER_SEPARATOR_ERROR = -111,
    ERR_PROGRAM_MNEMONIC_TOO_LONG = -112,
    ERR_UNDEFINED_HEADER = -113,
    ERR_HEADER_SUFFIX_OUT_OF_RANGE = -114,
    ERR_NUMERIC_DATA_ERROR = -120,
    ERR_INVALID_CHARACTER_IN_NUMBER = -121,
    ERR_EXPONENT_TOO_LARGE = -123,
    ERR_TOO_MANY_DIGITS = -124,
    ERR_NUMERIC_DATA_NOT_ALLOWED = -128,
    ERR_SUFFIX_ERROR = -130,
    ERR_INVALID_SUFFIX = -131,
    ERR_SUFFIX_TOO_LONG = -134,
    ERR_SUFFIX_NOT_ALLOWED = -138,
    ERR_DATA_OUT_OF_RANGE = -222,
    ERR_TOO_MUCH_DATA = -223,
    ERR_ILLEGAL_PARAMETER_VALUE = -224,
    ERR_QUERY_INTERRUPTED = -410,
    ERR_QUERY_UNTERMINATED = -420,
    ERR_QUERY_DEADLOCKED = -430,
    ERR_QUERY_UNTERMINATED_AFTER_INDEFINITE = -440,
    ERR_EXECUTION_ERROR = -200,
    ERR_SETTINGS_LOST = -310,
    ERR_CALIBRATION_FAILED = -311
};

class SCPIParser {
public:
    SCPIParser();

    /**
     * @brief Initialize SCPI parser
     * Sets up command tree and error queue
     */
    void init();

    /**
     * @brief Process incoming serial data
     * Call this frequently in main loop to check for SCPI commands
     */
    void process();

    /**
     * @brief Parse and execute a single SCPI command
     * @param command Complete command string (terminated by \n)
     */
    void executeCommand(const String& command);

    /**
     * @brief Add error to error queue
     * @param error Error code from SCPIError enum
     * @param description Optional custom error description
     */
    void addError(SCPIError error, const char* description = nullptr);

    /**
     * @brief Send response to host
     * @param response Response string (will be terminated with \n)
     */
    void sendResponse(const String& response);

    /**
     * @brief Get SCPI version string
     */
    const char* getVersion() const { return "1999.0"; }

private:
    // Command buffer
    static constexpr size_t CMD_BUFFER_SIZE = 256;
    char cmdBuffer[CMD_BUFFER_SIZE];
    size_t cmdBufferPos;

    // Error queue (SCPI requires minimum 1, we implement 10)
    static constexpr size_t ERROR_QUEUE_SIZE = 10;
    struct ErrorEntry {
        SCPIError code;
        String description;
    };
    ErrorEntry errorQueue[ERROR_QUEUE_SIZE];
    size_t errorQueueHead;
    size_t errorQueueTail;
    size_t errorQueueCount;

    // Helper functions
    bool isQuery(const String& cmd);
    void trimWhitespace(String& str);
    void toUpperCase(String& str);
    bool parseNumeric(const String& param, double& value);
    bool parseSuffix(const String& param, String& suffix);

    // IEEE 488.2 Common Commands (mandatory)
    void cmd_IDN();           // *IDN? - Identification query
    void cmd_RST();           // *RST - Reset
    void cmd_CLS();           // *CLS - Clear status
    void cmd_ESE(const String& param);  // *ESE - Event status enable
    void cmd_ESE_Q();         // *ESE? - Event status enable query
    void cmd_ESR_Q();         // *ESR? - Event status register query
    void cmd_OPC();           // *OPC - Operation complete
    void cmd_OPC_Q();         // *OPC? - Operation complete query
    void cmd_SRE(const String& param);  // *SRE - Service request enable
    void cmd_SRE_Q();         // *SRE? - Service request enable query
    void cmd_STB_Q();         // *STB? - Status byte query
    void cmd_TST_Q();         // *TST? - Self test query
    void cmd_WAI();           // *WAI - Wait to continue

    // SCPI SOURce subsystem (power supply output control)
    void cmd_SOUR_VOLT(const String& param);      // :SOURce:VOLTage
    void cmd_SOUR_VOLT_Q();                       // :SOURce:VOLTage?
    void cmd_SOUR_VOLT_LIM(const String& param);  // :SOURce:VOLTage:LIMit
    void cmd_SOUR_VOLT_LIM_Q();                   // :SOURce:VOLTage:LIMit?
    void cmd_SOUR_CURR(const String& param);      // :SOURce:CURRent
    void cmd_SOUR_CURR_Q();                       // :SOURce:CURRent?
    void cmd_SOUR_CURR_LIM(const String& param);  // :SOURce:CURRent:LIMit
    void cmd_SOUR_CURR_LIM_Q();                   // :SOURce:CURRent:LIMit?
    void cmd_SOUR_FUNC(const String& param);      // :SOURce:FUNCtion (CV/CC/FGEN)
    void cmd_SOUR_FUNC_Q();                       // :SOURce:FUNCtion?

    // SCPI MEASure subsystem (measurement queries)
    void cmd_MEAS_VOLT_Q();                       // :MEASure:VOLTage?
    void cmd_MEAS_CURR_Q();                       // :MEASure:CURRent?
    void cmd_MEAS_POW_Q();                        // :MEASure:POWer?
    void cmd_MEAS_VOLT_MIN_Q();                   // :MEASure:VOLTage:MIN?
    void cmd_MEAS_VOLT_MAX_Q();                   // :MEASure:VOLTage:MAX?
    void cmd_MEAS_VOLT_MEAN_Q();                  // :MEASure:VOLTage:MEAN?
    void cmd_MEAS_CURR_MIN_Q();                   // :MEASure:CURRent:MIN?
    void cmd_MEAS_CURR_MAX_Q();                   // :MEASure:CURRent:MAX?
    void cmd_MEAS_CURR_MEAN_Q();                  // :MEASure:CURRent:MEAN?

    // SCPI OUTPut subsystem (output control)
    void cmd_OUTP_STAT(const String& param);      // :OUTPut:STATe
    void cmd_OUTP_STAT_Q();                       // :OUTPut:STATe?
    void cmd_OUTP_PROT_CLE();                     // :OUTPut:PROTection:CLEar
    void cmd_OUTP_PROT_TRIP_Q();                  // :OUTPut:PROTection:TRIPped?

    // SCPI SYSTem subsystem (system control)
    void cmd_SYST_ERR_Q();                        // :SYSTem:ERRor?
    void cmd_SYST_ERR_COUN_Q();                   // :SYSTem:ERRor:COUNt?
    void cmd_SYST_ERR_ALL_Q();                    // :SYSTem:ERRor:ALL?
    void cmd_SYST_VERS_Q();                       // :SYSTem:VERSion?
    void cmd_SYST_CAP_Q();                        // :SYSTem:CAPability?
    void cmd_SYST_BEEP();                         // :SYSTem:BEEPer
    void cmd_SYST_LOC();                          // :SYSTem:LOCal (return to local control)
    void cmd_SYST_REM();                          // :SYSTem:REMote (remote control mode)

    // SCPI CALibration subsystem
    void cmd_CAL_STAT_Q();                        // :CALibration:STATe?
    void cmd_CAL_SAVE();                          // :CALibration:SAVE
    void cmd_CAL_LOAD();                          // :CALibration:LOAD

    // Status registers (IEEE 488.2)
    uint8_t statusByte;
    uint8_t eventStatusRegister;
    uint8_t eventStatusEnable;
    uint8_t serviceRequestEnable;

    // SCPI state
    bool remoteMode;
    bool operationComplete;

    // Error descriptions
    const char* getErrorDescription(SCPIError error);
};

extern SCPIParser scpiParser;
