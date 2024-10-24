#include "HSDWifi.h"

#define SOFT_AP_SSID  (F("StatusDisplay"))
#define SOFT_AP_PSK   (F("statusdisplay"))

HSDWifi::HSDWifi(const HSDConfig& config)
:
m_config(config),
m_connectFailure(false),
m_maxConnectRetries(120),
m_numConnectRetriesDone(0),
m_retryDelay(1000),
m_millisLastConnectTry(0),
m_accessPointActive(false),
m_accessPointStartTime(0),
m_lastConnectStatus(false)
{ 
  
}

void HSDWifi::begin() {
  // nothing to do right now
  WiFi.persistent(false);

  if(String(m_config.getWifiSSID())=="") {
    Serial.println(F("No WiFi credentials detected"));
    this->startAccessPoint();
  } else {
    this->handleConnection(true);
  }
}

void HSDWifi::handleConnection(bool firstRun) {
  bool isConnected = connected();

  if (this->m_accessPointActive && (uint)(millis() - m_accessPointStartTime) > (5*60*1000)) {
    Serial.println(F("Inactivive Accesspoint since 5min, Rebooting ESP."));
    ESP.restart();
  }

  if(isConnected != m_lastConnectStatus) {
    if(isConnected) {
      Serial.print(F("WiFi connected with IP: "));
      Serial.println(WiFi.localIP());
      
      m_numConnectRetriesDone = 0;
    } else {
      Serial.println(F("WiFi connection lost."));
    }
    
    m_lastConnectStatus = isConnected;
  }
  
  if(!isConnected && !m_accessPointActive) {

    if(m_connectFailure) {
      Serial.printf("Connection failure detected after %d tries and %d minutes, start Accesspoint\n", 
        m_numConnectRetriesDone,
        m_numConnectRetriesDone*m_retryDelay);

      this->startAccessPoint();
    } else {
      unsigned long currentMillis = millis();
      
      if ( firstRun || (int)(currentMillis - m_millisLastConnectTry) >= m_retryDelay) {
        m_millisLastConnectTry = currentMillis; 
        Serial.println(F("Try to connect..."));

        if(m_numConnectRetriesDone == 0) {
          Serial.print(F("Starting Wifi connection to "));
          Serial.print(m_config.getWifiSSID());
          Serial.println(F("..."));

          WiFi.disconnect(true);
          WiFi.softAPdisconnect(true);
          if (WiFi.getMode() != WIFI_STA) WiFi.mode(WIFI_STA);

          WiFi.begin(m_config.getWifiSSID(), m_config.getWifiPSK());
      
          m_numConnectRetriesDone++;
        } else if (m_numConnectRetriesDone < m_maxConnectRetries) {
          Serial.printf("Waiting %d/%dsec\n", m_numConnectRetriesDone*m_retryDelay/1000, m_maxConnectRetries*m_retryDelay/1000);
          m_numConnectRetriesDone++;
        } else {
          Serial.println(F("Failed to connect WiFi."));
          m_connectFailure = true;
        }   
      }   
    }
  }
}

bool HSDWifi::connected() {
  return (WiFi.status() == WL_CONNECTED);
}

void HSDWifi::startAccessPoint() {
  Serial.println(F(""));
  Serial.println(F("Starting access point."));

  WiFi.mode(WIFI_AP);

  if(WiFi.softAP(String(SOFT_AP_SSID).c_str(), String(SOFT_AP_PSK).c_str())) {
    IPAddress ip = WiFi.softAPIP();
 
    Serial.print(F("AccessPoint SSID is ")); Serial.println(SOFT_AP_SSID); 
    Serial.print(F("IP: ")); Serial.println(ip);

    this->m_accessPointActive = true;
    this->m_accessPointStartTime = millis();
  } else {
    Serial.println(F("Error starting access point."));
  }
}

