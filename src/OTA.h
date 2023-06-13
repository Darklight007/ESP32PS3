

#ifndef OTA_H
#define OTA_H

#ifdef ESP32
#include <WiFi.h>
#include <ESPmDNS.h>
#else
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#endif

#include <WiFiUdp.h>
#include <ArduinoOTA.h>
//#include <credentials.h>

// const char* ssid = "Tenda_27CB10";
// const char* password = "aj5QALnZ";

const char *ssid = "Homayoun2";
const char *password = "2229packard";
bool g_wifiConnection = true;

#if defined(ESP32_RTOS) && defined(ESP32)
void taskOne(void *parameter)
{
  ArduinoOTA.handle();
  //  delay(3500);
}
#endif

void setupOTA(const char *nameprefix)
{
  const int maxlen = 40;
  char fullhostname[maxlen];
  static int wifiFail = 0;
  uint8_t mac[6];
  WiFi.macAddress(mac);
  snprintf(fullhostname, maxlen, "%s-%02x%02x%02x", nameprefix, mac[3], mac[4], mac[5]);
  ArduinoOTA.setHostname(fullhostname);
    Serial.println("Connecting to WiFi ... ");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.println("Connection Failed! Trying again ...");
    delay(2000);
    // ESP.restart();
    wifiFail++;

    if (wifiFail > 2)
    {
      g_wifiConnection = false;
      Serial.printf("\nStart booting without WIFI & OTA ...");
      return;
      //  break;
    }
  }
 Serial.println("Connection Succeeded.");
  // Port defaults to 3232
  // ArduinoOTA.setPort(3232);

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA
      .onStart([]()
               {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type); })
      .onEnd([]()
             { Serial.println("\nEnd"); })
      .onProgress([](unsigned int progress, unsigned int total)
                  { Serial.printf("Progress: %u%%\r", (progress / (total / 100))); })
      .onError([](ota_error_t error)
               {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed"); });

  ArduinoOTA.begin();

  Serial.print("OTA Initialized, ");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

#if defined(ESP32_RTOS) && defined(ESP32)
  xTaskCreate(
      ota_handle,   /* Task function. */
      "OTA_HANDLE", /* String with name of task. */
      10000,        /* Stack size in bytes. */
      NULL,         /* Parameter passed as input of the task */
      1,            /* Priority of the task. */
      NULL);        /* Task handle. */
#endif
}
#endif
