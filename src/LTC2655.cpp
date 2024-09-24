
#include "Arduino.h"
#include "LTC2655.h"
#include <Wire.h>

LTC2655::LTC2655(uint8_t addr)
{
    address = addr;
};

// [Command-Address] 0011_0000  =>Table 1, page 19
// The first byte of the input word consists of the 4-bit command followed by
// the 4-bit address. The next two bytes consist of the 16-bit data word.

void LTC2655::sendCommand(int command, dacChannel_t channel, uint16_t data)
{
  // Start I2C transmission to device at the address specified in the datasheet.
    // Reference: Slave Address Map, Table 2, page 20 of the datasheet.
    Wire.beginTransmission(address); 
    
    // Combine the 4-bit command and 4-bit channel into a single byte.
    Wire.write(((command & 0b00001111) << 4) | channel);
    // Write the high byte of the 16-bit data.
    Wire.write(data >> 8);
    // Write the low byte of the 16-bit data.
    Wire.write(data & 0x00ff);
    
    // End I2C transmission.
    Wire.endTransmission();
    
    // NOPs may be unnecessary unless specifically required by the timing of the I2C hardware.
    // __asm__("nop\n\t");
    // __asm__("nop\n\t");
    
    // Delay can be uncommented if required by the device after sending commands (see datasheet specifications).
    // delay(1);
}

void LTC2655::write(dacChannel_t channel, uint16_t data)
{
    sendCommand(WRITE_REGISTER, channel, data);
}
void LTC2655::update(dacChannel_t channel, uint16_t data)
{
    sendCommand(UPDATE_REGISTER, channel, data);
}
void LTC2655::writeAndPowerAll(dacChannel_t channel, uint16_t data)
{
    sendCommand(WRITE_X_UPDATE_ALL, channel, data);
}
void LTC2655::writeUpdate(dacChannel_t channel, uint16_t data)
{
    sendCommand(WRITE_UPDATE, channel, data);
}
void LTC2655::powerDown(dacChannel_t channel)
{
    // data required in order to complete a full communication cycle
    sendCommand(POWER_DOWN, channel, 0x0000);
}
void LTC2655::powerDownChip()
{
    // data required in order to complete a full communication cycle
    sendCommand(POWER_DOWN_CHIP, ALL_CHANNEL, 0);
}
void LTC2655::setVoltageReference(dacRef_t voltageReferencSource)
{
    sendCommand(voltageReferencSource, ALL_CHANNEL, 0x0000);
}
// void dacSet(dacChannel_t channel, int command, uint16_t data)
// {
//     sendCommand(channel, UPDATE_REGISTER, data);
// }