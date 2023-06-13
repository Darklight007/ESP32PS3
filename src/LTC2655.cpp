
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
    Wire.beginTransmission(address); // transmit to device #65, Table 2. Slave Address Map, page 20th data sheet (FLOAT FLOAT FLOAT)
    Wire.write((command << 4) | channel);
    Wire.write(data >> 8);
    Wire.write(data & 0xff);
    Wire.endTransmission();
    __asm__("nop\n\t");
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