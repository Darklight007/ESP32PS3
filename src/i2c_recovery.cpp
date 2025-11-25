#include "i2c_recovery.h"

// Static member initialization
TwoWire* I2CRecovery::wire = nullptr;
int I2CRecovery::sdaPin = -1;
int I2CRecovery::sclPin = -1;
I2CErrorStats I2CRecovery::stats = {};
uint8_t I2CRecovery::consecutiveErrors = 0;

void I2CRecovery::init(TwoWire* _wire, int _sdaPin, int _sclPin)
{
    wire = _wire;
    sdaPin = _sdaPin;
    sclPin = _sclPin;
    consecutiveErrors = 0;
}

bool I2CRecovery::handleError(uint8_t error)
{
    if (error == 0)
    {
        // Success - reset consecutive error counter
        consecutiveErrors = 0;
        return true;
    }

    // Record error statistics
    stats.totalErrors++;
    stats.lastErrorTime = millis();
    consecutiveErrors++;

    switch (error)
    {
    case 1:
        // Data too long - this is a programming error, not a bus error
        Serial.println("I2C Error: Data too long for buffer");
        consecutiveErrors = 0;  // Don't trigger recovery for this
        return false;

    case 2:
        // NACK on address - slave not responding
        stats.nackErrors++;
        Serial.printf("I2C Error: NACK on address (consecutive: %d)\n", consecutiveErrors);
        break;

    case 3:
        // NACK on data - slave rejected data
        stats.nackErrors++;
        Serial.printf("I2C Error: NACK on data (consecutive: %d)\n", consecutiveErrors);
        break;

    case 4:
        // Other error (timeout, bus error, etc.)
        stats.busErrors++;
        Serial.printf("I2C Error: Bus error (consecutive: %d)\n", consecutiveErrors);
        break;

    default:
        Serial.printf("I2C Error: Unknown error code %d\n", error);
        return false;
    }

    // Attempt recovery if we've had too many consecutive errors
    if (consecutiveErrors >= MAX_ERRORS_BEFORE_RECOVERY)
    {
        Serial.println("I2C: Attempting bus recovery due to consecutive errors...");
        stats.recoveryAttempts++;

        if (recoverBus())
        {
            Serial.println("I2C: Bus recovery successful");
            stats.recoverySuccess++;
            consecutiveErrors = 0;
            return true;
        }
        else
        {
            Serial.println("I2C: Bus recovery failed, attempting full reset...");
            if (resetBus())
            {
                Serial.println("I2C: Bus reset successful");
                stats.recoverySuccess++;
                consecutiveErrors = 0;
                return true;
            }
            else
            {
                Serial.println("I2C: Bus reset failed - manual intervention required");
                return false;
            }
        }
    }

    return false;  // Error occurred but no recovery needed yet
}

bool I2CRecovery::recoverBus()
{
    if (sdaPin < 0 || sclPin < 0)
    {
        Serial.println("I2C Recovery: Pins not configured");
        return false;
    }

    // Clock stretching technique to release stuck SDA line
    // This sends 9 clock pulses to allow the slave to finish its transmission

    // 1. End I2C operation
    wire->end();
    delay(10);

    // 2. Configure pins as GPIO
    pinMode(sdaPin, INPUT_PULLUP);
    pinMode(sclPin, OUTPUT);

    // 3. Check if SDA is stuck low
    if (digitalRead(sdaPin) == HIGH)
    {
        // SDA is high - bus might be OK
        Serial.println("I2C Recovery: SDA already high, reinitializing bus");
        wire->begin(sdaPin, sclPin);
        return isBusHealthy();
    }

    Serial.println("I2C Recovery: SDA stuck low, sending clock pulses");

    // 4. Send 9 clock pulses to release SDA
    for (int i = 0; i < 9; i++)
    {
        digitalWrite(sclPin, LOW);
        delayMicroseconds(5);
        digitalWrite(sclPin, HIGH);
        delayMicroseconds(5);

        // Check if SDA released
        if (digitalRead(sdaPin) == HIGH)
        {
            Serial.printf("I2C Recovery: SDA released after %d pulses\n", i + 1);
            break;
        }
    }

    // 5. Generate STOP condition
    pinMode(sdaPin, OUTPUT);
    digitalWrite(sdaPin, LOW);
    delayMicroseconds(5);
    digitalWrite(sclPin, HIGH);
    delayMicroseconds(5);
    digitalWrite(sdaPin, HIGH);
    delayMicroseconds(5);

    // 6. Reinitialize I2C
    wire->begin(sdaPin, sclPin);
    delay(10);

    // 7. Verify bus health
    bool healthy = isBusHealthy();
    Serial.printf("I2C Recovery: Bus health check: %s\n", healthy ? "PASSED" : "FAILED");

    return healthy;
}

bool I2CRecovery::resetBus()
{
    if (!wire)
    {
        return false;
    }

    // Full I2C peripheral reset
    wire->end();
    delay(50);  // Longer delay for full reset

    // Reinitialize
    if (sdaPin >= 0 && sclPin >= 0)
    {
        wire->begin(sdaPin, sclPin);
    }
    else
    {
        wire->begin();
    }

    delay(10);

    return isBusHealthy();
}

I2CErrorStats& I2CRecovery::getStats()
{
    return stats;
}

void I2CRecovery::resetStats()
{
    stats = {};
    consecutiveErrors = 0;
}

bool I2CRecovery::isBusHealthy()
{
    if (sdaPin < 0 || sclPin < 0)
    {
        // Can't check without pin numbers, assume OK
        return true;
    }

    // Both SDA and SCL should be high when idle
    pinMode(sdaPin, INPUT_PULLUP);
    pinMode(sclPin, INPUT_PULLUP);
    delay(1);

    bool sdaHigh = digitalRead(sdaPin) == HIGH;
    bool sclHigh = digitalRead(sclPin) == HIGH;

    // Restore I2C function
    wire->begin(sdaPin, sclPin);

    return sdaHigh && sclHigh;
}
