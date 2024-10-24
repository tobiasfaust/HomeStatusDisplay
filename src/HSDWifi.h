#pragma once

#ifdef ESP8266
  #include <ESP8266WiFi.h>
  #include <ESPAsyncTCP.h>
#elif ESP32
  #include <WiFi.h> 
  #include <AsyncTCP.h>
#endif

#include "HSDConfig.h"

class HSDWifi
{
public:

  HSDWifi(const HSDConfig& config);

  void begin();
  void handleConnection(bool firstRun);
  bool connected();

private:

  void startAccessPoint();

  const     HSDConfig& m_config;
  bool      m_connectFailure;
  uint16_t  m_maxConnectRetries;
  uint16_t  m_numConnectRetriesDone;
  uint16_t  m_retryDelay;
  uint32_t  m_millisLastConnectTry;
  bool      m_accessPointActive;
  uint32_t  m_accessPointStartTime;
  bool      m_lastConnectStatus;
};

