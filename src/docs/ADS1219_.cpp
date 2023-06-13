#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <Wire.h>
#include "ADS1219_.h"

static uint8_t i2cRead(void)
{
#if ARDUINO >= 100
  return Wire.read();
#else
  return Wire.receive();
#endif
}

static void i2cWrite(uint8_t x)
{
#if ARDUINO >= 100
  Wire.write(x);
#else
  Wire.send(x);
#endif
}

bool ADS1219_::checkDataReady(void)
{

  uint8_t drdy = ADS1219_::readRegister(36);
  if (drdy == 224)
    return 1;
  else
    return 0;

  // return (drdy>>7) && !(drdy&1);
}
uint16_t ADS1219_::getDataRate()
{
  return dataRate;
}

void ADS1219_::setHeartbeat()
{
  heartBeat = millis();
}

bool ADS1219_::isHeartbeatStoped(long period)
{
  if ((millis() - heartBeat) > period)
    return true;
  else
    return false;
}

ADS1219_::ADS1219_(int drdy, uint8_t addr)
{
  data_ready = drdy;
  address = addr;
  config = 0x00;
  singleShot = true;
}

void ADS1219_::begin()
{
  Wire.begin();
}

void ADS1219_::start()
{
  Wire.beginTransmission(address);
  i2cWrite(0x08);
  Wire.endTransmission();
}

void ADS1219_::powerDown()
{
  Wire.beginTransmission(address);
  i2cWrite(0x02);
  Wire.endTransmission();
}

uint8_t ADS1219_::readRegister(uint8_t reg)
{
  Wire.beginTransmission(address);
  i2cWrite(reg);
  Wire.endTransmission();
  Wire.requestFrom((uint8_t)address, (uint8_t)1);
  return i2cRead();
}

void ADS1219_::writeRegister(uint8_t data)
{
  Wire.beginTransmission(address);
  i2cWrite(CONFIG_REGISTER_ADDRESS);
  i2cWrite(data);
  Wire.endTransmission();
}

long ADS1219_::readConversionResult()
{
  Wire.beginTransmission(address);
  i2cWrite(0x10);
  Wire.endTransmission();
  Wire.requestFrom((uint8_t)address, (uint8_t)3);
  long data32 = i2cRead();
  data32 <<= 8;
  data32 |= i2cRead();
  data32 <<= 8;
  data32 |= i2cRead();
  return (data32 << 8) >> 8;
}

void ADS1219_::resetConfig()
{
  writeRegister(0x00);
}

long ADS1219_::readSingleEnded(int channel)
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
  busyChannel = channel;
  writeRegister(config);
  start();
  while (digitalRead(data_ready) == 1)
    ;
  return readConversionResult();
}

long ADS1219_::readDifferential_0_1()
{
  config &= MUX_MASK;
  config |= MUX_DIFF_0_1;
  writeRegister(config);
  start();
  while (digitalRead(data_ready) == 1)
    ;
  return readConversionResult();
}

long ADS1219_::readDifferential_2_3()
{
  config &= MUX_MASK;
  config |= MUX_DIFF_2_3;
  writeRegister(config);
  start();
  while (digitalRead(data_ready) == 1)
    ;
  return readConversionResult();
}

long ADS1219_::readDifferential_1_2()
{
  config &= MUX_MASK;
  config |= MUX_DIFF_1_2;
  writeRegister(config);
  start();
  while (digitalRead(data_ready) == 1)
    ;
  return readConversionResult();
}

long ADS1219_::readShorted()
{
  config &= MUX_MASK;
  config |= MUX_SHORTED;
  writeRegister(config);
  while (digitalRead(data_ready) == 1)
    ;
  return readConversionResult();
}

void ADS1219_::setGain(adsGain_t gain)
{
  config &= GAIN_MASK;
  config |= gain;
  writeRegister(config);
}

void ADS1219_::setDataRate(int rate)
{
  dataRate = rate;
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

void ADS1219_::setConversionMode(adsMode_t mode)
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

void ADS1219_::setVoltageReference(adsRef_t vref)
{
  config &= VREF_MASK;
  config |= vref;
  writeRegister(config);
}