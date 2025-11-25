#include "error_handler.h"
#include "buzzer.h"
#include <Arduino.h>

namespace ErrorHandler {

    // Configuration
    static bool serial_enabled = true;
    static bool ui_enabled = true;

    // Error counters
    static uint32_t error_counts[5] = {0}; // DEBUG, INFO, WARNING, ERROR, CRITICAL

    // Color codes for severity levels
    static const lv_color_t SEVERITY_COLORS[] = {
        lv_color_hex(0x888888),  // DEBUG - Gray
        lv_color_hex(0x0088FF),  // INFO - Blue
        lv_color_hex(0xFFAA00),  // WARNING - Orange
        lv_color_hex(0xFF0000),  // ERROR - Red
        lv_color_hex(0x880000)   // CRITICAL - Dark Red
    };

    // Severity level names
    static const char* SEVERITY_NAMES[] = {
        "DEBUG", "INFO", "WARN", "ERROR", "CRIT"
    };

    // Category names
    static const char* CATEGORY_NAMES[] = {
        "HW", "CALIB", "VALID", "MEM", "SCPI", "SYS", "INPUT"
    };

    void init(bool enable_serial, bool enable_ui)
    {
        serial_enabled = enable_serial;
        ui_enabled = enable_ui;
        clearCounters();

        if (serial_enabled) {
            Serial.println("\n=== Error Handler Initialized ===");
        }
    }

    void report(Severity severity, Category category, const char* message,
                const char* file, int line)
    {
        if (!serial_enabled && !ui_enabled) return;

        // Update counter
        int sev_idx = static_cast<int>(severity);
        if (sev_idx >= 0 && sev_idx < 5) {
            error_counts[sev_idx]++;
        }

        // Format timestamp
        char timestamp[16];
        snprintf(timestamp, sizeof(timestamp), "[%lu]", millis());

        // Serial logging
        if (serial_enabled) {
            Serial.printf("%s [%s:%s] %s",
                         timestamp,
                         SEVERITY_NAMES[sev_idx],
                         CATEGORY_NAMES[static_cast<int>(category)],
                         message);

            if (file && line > 0) {
                // Extract just filename from path
                const char* filename = strrchr(file, '/');
                filename = filename ? filename + 1 : file;
                Serial.printf(" (%s:%d)", filename, line);
            }
            Serial.println();
        }

        // Beep on errors and critical issues
        if (severity == Severity::ERROR || severity == Severity::CRITICAL) {
            myTone(NOTE_C5, 100);
        }

        // Show critical errors to user via UI
        if (ui_enabled && severity == Severity::CRITICAL) {
            showUserMessage("Critical Error", message, severity);
        }
    }

    void hardwareError(const char* device, const char* operation, int error_code)
    {
        char msg[128];
        if (error_code >= 0) {
            snprintf(msg, sizeof(msg), "%s %s failed (code: %d)",
                    device, operation, error_code);
        } else {
            snprintf(msg, sizeof(msg), "%s %s failed", device, operation);
        }
        report(Severity::ERROR, Category::HARDWARE, msg);
    }

    void validationError(const char* param, const char* value, const char* expected)
    {
        char msg[128];
        snprintf(msg, sizeof(msg), "Invalid %s: '%s' (expected: %s)",
                param, value, expected);
        report(Severity::WARNING, Category::VALIDATION, msg);
    }

    void calibrationError(const char* stage, const char* message)
    {
        char msg[128];
        snprintf(msg, sizeof(msg), "Calibration [%s]: %s", stage, message);
        report(Severity::ERROR, Category::CALIBRATION, msg);
    }

    bool isEnabled()
    {
        return serial_enabled || ui_enabled;
    }

    uint32_t getErrorCount(Severity severity)
    {
        int idx = static_cast<int>(severity);
        if (idx >= 0 && idx < 5) {
            return error_counts[idx];
        }
        return 0;
    }

    void clearCounters()
    {
        for (int i = 0; i < 5; i++) {
            error_counts[i] = 0;
        }
    }

    void showUserMessage(const char* title, const char* message, Severity severity)
    {
        // Create a simple message box (this is a placeholder - full implementation
        // would need proper LVGL msgbox setup)

        if (serial_enabled) {
            Serial.println("\n╔════════════════════════════════╗");
            Serial.printf("║ %s\n", title);
            Serial.println("╠════════════════════════════════╣");
            Serial.printf("║ %s\n", message);
            Serial.println("╚════════════════════════════════╝\n");
        }

        // TODO: Implement LVGL msgbox popup
        // This would need to be integrated with the main UI task to avoid
        // thread safety issues with LVGL
    }

} // namespace ErrorHandler
