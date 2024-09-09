#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <Wire.h>
#include "ADS1219.h"
    TwoWire *_wire_g =  &Wire1;

static uint8_t i2cRead(void)
{
#if ARDUINO >= 100
  return _wire_g->read();
#else
  return _wire->receive();
#endif
}

static void i2cWrite(uint8_t x)
{
#if ARDUINO >= 100
  _wire_g->write(x);
#else
  _wire->send(x);
#endif
}

ADS1219::ADS1219(int drdy, uint8_t addr)
{
  data_ready = drdy;
  address = addr;
  config = 0x00;
  singleShot = true;
    _wire= _wire_g;
}

void ADS1219::begin()
{

  _wire->begin();
}

void ADS1219::start()
{
  _wire->beginTransmission(address);
  i2cWrite(0x08);
  _wire->endTransmission();
}

void ADS1219::powerDown()
{
  _wire->beginTransmission(address);
  i2cWrite(0x02);
  _wire->endTransmission();
}

uint8_t ADS1219::readRegister(uint8_t reg)
{
  _wire->beginTransmission(address);
  i2cWrite(reg);
  _wire->endTransmission();
  _wire->requestFrom((uint8_t)address, (uint8_t)1);
  return i2cRead();
}

void ADS1219::writeRegister(uint8_t data)
{
  _wire->beginTransmission(address);
  i2cWrite(CONFIG_REGISTER_ADDRESS);
  i2cWrite(data);
  _wire->endTransmission();
}

long ADS1219::readConversionResult()
{
  _wire->beginTransmission(address);
  i2cWrite(0x10);
  _wire->endTransmission();
  _wire->requestFrom((uint8_t)address, (uint8_t)3);
  long data32 = i2cRead();
  data32 <<= 8;
  data32 |= i2cRead();
  data32 <<= 8;
  data32 |= i2cRead();
  return (data32 << 8) >> 8;
}

void ADS1219::resetConfig()
{
  writeRegister(0x00);
}

void ADS1219::readSingleEnded(int channel)
{
  config &= MUX_MASK;
  switch (channel)
  {
  case (0):
    config |= MUX_SINGLE_0;
    break;
  case (1):
    config |= MUX_SINGLE_1;
    break;
  case (2):
    config |= MUX_SINGLE_2;
    break;
  case (3):
    config |= MUX_SINGLE_3;
    break;
  default:
    break;
  }
  writeRegister(config);
  start();
  //  busyChannel = channel;
  // while(digitalRead(data_ready)==1);
  // return readConversionResult();
}

long ADS1219::readDifferential_0_1()
{
  config &= MUX_MASK;
  config |= MUX_DIFF_0_1; //MUX_DIFF_0_1;
  writeRegister(config);
  start();
  // while (digitalRead(data_ready) == 1);
  // return readConversionResult();
  return 0;
}

long ADS1219::readDifferential_2_3()
{
  config &= MUX_MASK;
  config |= MUX_DIFF_2_3;
  writeRegister(config);
  start();
  // while (digitalRead(data_ready) == 1);
  // return readConversionResult();
  return 0;
}

long ADS1219::readDifferential_1_2()
{
  config &= MUX_MASK;
  config |= MUX_DIFF_1_2;
  writeRegister(config);
  start();
  // while (digitalRead(data_ready) == 1);
  // return readConversionResult();
}

long ADS1219::readShorted()
{
  config &= MUX_MASK;
  config |= MUX_SHORTED;
  writeRegister(config);
  while (digitalRead(data_ready) == 1)
    ;
  return readConversionResult();
}

void ADS1219::setGain(adsGain_t gain)
{
  config &= GAIN_MASK;
  config |= gain;
  writeRegister(config);
}

void ADS1219::setDataRate(int rate)
{
  config &= DATA_RATE_MASK;
  switch (rate)
  {
  case (20):
    config |= DATA_RATE_20;
    break;
  case (90):
    config |= DATA_RATE_90;
    break;
  case (330):
    config |= DATA_RATE_330;
    break;
  case (1000):
    config |= DATA_RATE_1000;
    break;
  default:
    break;
  }
  writeRegister(config);
}

void ADS1219::setConversionMode(adsMode_t mode)
{
  config &= MODE_MASK;
  config |= mode;
  writeRegister(config);
  if (mode == CONTINUOUS)
  {
    singleShot = false;
  }
  else
  {
    singleShot = true;
  }
}

void ADS1219::setVoltageReference(adsRef_t vref)
{
  config &= VREF_MASK;
  config |= vref;
  writeRegister(config);
}