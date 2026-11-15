#pragma once
#include <Arduino.h>

// Local weather for the splash (weather.cpp)
enum WxKind { WX_CLEAR, WX_CLOUD, WX_RAIN, WX_SNOW, WX_FOG, WX_THUNDER, WX_COUNT };

WxKind weatherKind();     // forced kind, else the last fetched (NVS cache)
void weatherFetchStart(); // background: WiFi, IP location, weather, WiFi off
void weatherForce(int kind); // -1 = live
String weatherInfo();     // kind,code,temp,city,status,forced
void weatherWait(uint32_t untilMs); // boot: wait for the fetch (millis deadline)
bool weatherBusy();
bool weatherHasData();
float weatherTemp();
const char *weatherCity();
void weatherSetAuto(bool on); // fetch at boot (NVS wx_auto, default on)
bool weatherAuto();
void weatherSetCity(const String &name); // fixed city, "" = IP location
String weatherUserCity();
