#include "functions.h"
#include "device.hpp"

// External reference
extern Device PowerSupply;

double scaleVoltage(uint16_t voltage)
{
    return (voltage - PowerSupply.Voltage.adjOffset) / PowerSupply.Voltage.adjFactor;
}

double scaleCurrent(uint16_t current)
{
    return (current - PowerSupply.Current.adjOffset) / PowerSupply.Current.adjFactor;
}
