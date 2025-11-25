#pragma once

#include <Arduino.h>
#include <lvgl.h>

/**
 * @file error_handler.h
 * @brief Centralized error handling and logging system
 *
 * Provides consistent error reporting, logging, and user notification
 * for hardware errors, validation failures, and runtime issues.
 */

namespace ErrorHandler {

    /**
     * @brief Error severity levels
     */
    enum class Severity {
        DEBUG,      ///< Debug information (not shown to user)
        INFO,       ///< Informational message
        WARNING,    ///< Warning that doesn't stop operation
        ERROR,      ///< Error requiring user attention
        CRITICAL    ///< Critical error requiring immediate action
    };

    /**
     * @brief Error categories for classification
     */
    enum class Category {
        HARDWARE,       ///< Hardware communication errors (I2C, SPI, ADC, DAC)
        CALIBRATION,    ///< Calibration data errors
        VALIDATION,     ///< Input validation failures
        MEMORY,         ///< Flash/preferences storage errors
        SCPI,           ///< SCPI command parsing errors
        SYSTEM,         ///< General system errors
        USER_INPUT      ///< User input errors
    };

    /**
     * @brief Initialize error handler system
     * @param enable_serial Enable serial logging
     * @param enable_ui Enable on-screen error messages
     */
    void init(bool enable_serial = true, bool enable_ui = true);

    /**
     * @brief Report an error with full details
     * @param severity Error severity level
     * @param category Error category
     * @param message Error message (max 128 chars)
     * @param file Source file (use __FILE__)
     * @param line Line number (use __LINE__)
     */
    void report(Severity severity, Category category, const char* message,
                const char* file = nullptr, int line = 0);

    /**
     * @brief Report a hardware error (I2C, SPI, ADC, DAC)
     * @param device Device name (e.g., "ADS1219", "LTC2655")
     * @param operation Operation that failed (e.g., "read", "write")
     * @param error_code Hardware error code (optional)
     */
    void hardwareError(const char* device, const char* operation, int error_code = -1);

    /**
     * @brief Report a validation error
     * @param param Parameter name that failed validation
     * @param value Value that was invalid
     * @param expected Expected range or format
     */
    void validationError(const char* param, const char* value, const char* expected);

    /**
     * @brief Report a calibration error
     * @param stage Calibration stage (e.g., "ADC", "DAC", "INL")
     * @param message Error description
     */
    void calibrationError(const char* stage, const char* message);

    /**
     * @brief Check if error logging is enabled
     */
    bool isEnabled();

    /**
     * @brief Get error count by severity
     */
    uint32_t getErrorCount(Severity severity);

    /**
     * @brief Clear error counters
     */
    void clearCounters();

    /**
     * @brief Show error message to user via LVGL popup
     * @param title Error title
     * @param message Error message
     * @param severity Severity for color coding
     */
    void showUserMessage(const char* title, const char* message, Severity severity = Severity::ERROR);

} // namespace ErrorHandler

// Convenience macros for easy error reporting
#define ERROR_REPORT(sev, cat, msg) \
    ErrorHandler::report(ErrorHandler::Severity::sev, ErrorHandler::Category::cat, msg, __FILE__, __LINE__)

#define ERROR_HW(dev, op) \
    ErrorHandler::hardwareError(dev, op)

#define ERROR_HW_CODE(dev, op, code) \
    ErrorHandler::hardwareError(dev, op, code)

#define ERROR_VALID(param, val, exp) \
    ErrorHandler::validationError(param, val, exp)

#define ERROR_CALIB(stage, msg) \
    ErrorHandler::calibrationError(stage, msg)
