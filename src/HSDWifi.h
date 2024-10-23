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

  const HSDConfig& m_config;
  bool m_connectFailure;
  int m_maxConnectRetries;
  int m_numConnectRetriesDone;
  int m_retryDelay;
  unsigned long m_millisLastConnectTry;
  bool m_accessPointActive;
  bool m_lastConnectStatus;
};

