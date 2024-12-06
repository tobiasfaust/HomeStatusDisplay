#pragma once

#include <ESPAsyncWebServer.h>
//#include <DNSServer.h>
//#include <ESP8266HTTPUpdateServer.h>
#include "HSDConfig.h"
#include "HSDLeds.h"
#include "HSDMqtt.h"
#include "HSDHtmlHelper.h"
#include <ElegantOTA.h>
#include <elegant_html.h>

#ifdef ESP8266
  #define ESP_getChipId() ESP.getChipId() 
  #define ESP_GetFreeHeap() ESP.getMaxFreeBlockSize()
#elif ESP32
  #define ESP_getChipId() (uint32_t)ESP.getEfuseMac()   // Unterschied zu ESP.getFlashChipId() ???
  #define ESP_GetFreeHeap() ESP.getFreeHeap()
#endif

class HSDWebserver
{
  
public:

  HSDWebserver(HSDConfig& config, const HSDLeds& leds, const HSDMqtt& mqtt);

  void begin();
  void handleClient(unsigned long deviceUptime);
  void startWebServer();
  void stopWebServer();

private:

  void deliverRootPage(AsyncWebServerRequest *request);
  void deliverStatusPage(AsyncWebServerRequest *request);
  void deliverColorMappingPage(AsyncWebServerRequest *request);
  void deliverDeviceMappingPage(AsyncWebServerRequest *request);
  void deliverNotFoundPage(AsyncWebServerRequest *request);
  void deliverUpdatePage(AsyncWebServerRequest *request);
  void getDeviceInfo(AsyncWebServerRequest *request);

  void checkReboot(AsyncWebServerRequest *request);

  bool updateMainConfig(AsyncWebServerRequest *request);

  bool needAdd(AsyncWebServerRequest *request);
  bool needDelete(AsyncWebServerRequest *request);
  bool needDeleteAll(AsyncWebServerRequest *request);
  bool needSave(AsyncWebServerRequest *request);
  bool needUndo(AsyncWebServerRequest *request);
    
  bool addColorMappingEntry(AsyncWebServerRequest *request);
  bool deleteColorMappingEntry(AsyncWebServerRequest *request);

  bool addDeviceMappingEntry(AsyncWebServerRequest *request);
  bool deleteDeviceMappingEntry(AsyncWebServerRequest *request);
  
  //bool updateDeviceMappingConfig(AsyncWebServerRequest *request);

  AsyncWebServer m_server;
  
  HSDConfig& m_config;
  const HSDLeds& m_leds;
  const HSDMqtt& m_mqtt;
  unsigned long m_deviceUptimeMinutes;
  const HSDHtmlHelper m_html;
};
