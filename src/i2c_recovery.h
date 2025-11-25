#pragma once

#include <Arduino.h>
#include <Wire.h>

/**
 * I2C Bus Recovery Module
 *
 * Handles I2C bus errors and lockup conditions that can occur when:
 * - Slave device holds SDA low (bus lockup)
 * - NACK errors accumulate
 * - Master loses arbitration
 *
 * Recovery strategies:
 * 1. Clock stretching (send 9 clock pulses to release SDA)
 * 2. Bus reset (stop + restart I2C peripheral)
 * 3. Full hardware reset of I2C bus
 */

// I2C error statistics
struct I2CErrorStats
{
    uint32_t totalErrors = 0;
    uint32_t nackErrors = 0;
    uint32_t timeoutErrors = 0;
    uint32_t busErrors = 0;
    uint32_t recoveryAttempts = 0;
    uint32_t recoverySuccess = 0;
    unsigned long lastErrorTime = 0;
};

class I2CRecovery
{
public:
    /**
     * Initialize I2C recovery system
     * @param wire Pointer to TwoWire instance (Wire or Wire1)
     * @param sdaPin SDA pin number
     * @param sclPin SCL pin number
     */
    static void init(TwoWire* wire, int sdaPin, int sclPin);

    /**
     * Handle I2C error and attempt recovery
     * @param error Error code from Wire.endTransmission()
     *              0 = success
     *              1 = data too long
     *              2 = NACK on address
     *              3 = NACK on data
     *              4 = other error
     * @return true if recovery successful or no recovery needed, false if failed
     */
    static bool handleError(uint8_t error);

    /**
     * Attempt to recover I2C bus from lockup condition
     * Uses clock stretching technique: send 9 clock pulses to release SDA
     * @return true if recovery successful, false otherwise
     */
    static bool recoverBus();

    /**
     * Reset I2C peripheral completely
     * @return true if reset successful
     */
    static bool resetBus();

    /**
     * Get error statistics
     * @return Reference to error stats structure
     */
    static I2CErrorStats& getStats();

    /**
     * Reset error statistics
     */
    static void resetStats();

    /**
     * Check if bus is healthy (SDA and SCL both high)
     * @return true if bus is healthy
     */
    static bool isBusHealthy();

private:
    static TwoWire* wire;
    static int sdaPin;
    static int sclPin;
    static I2CErrorStats stats;

    // Maximum consecutive errors before attempting recovery
    static constexpr uint8_t MAX_ERRORS_BEFORE_RECOVERY = 3;
    static uint8_t consecutiveErrors;
};
