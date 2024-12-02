#include "HomeStatusDisplay.h"

#define WINDOW_STRING (F("/window/"))
#define DOOR_STRING (F("/door/"))
#define LIGHT_STRING (F("/light/"))
#define ALARM_STRING (F("/alarm/"))

#define ONE_MINUTE_MILLIS (60000)

HomeStatusDisplay::HomeStatusDisplay():
  improvSerial(&Serial),
  m_webServer(m_config, m_leds, m_mqttHandler),
  m_mqttHandler(m_config, std::bind(&HomeStatusDisplay::mqttCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)),
  m_leds(m_config),
  m_lastWifiConnectionState(false),
  m_lastMqttConnectionState(false),
  m_oneMinuteTimerLast(0),
  m_uptime(0)
{
}

void HomeStatusDisplay::begin(const char* identifier)
{
  // initialize serial
  Serial.begin(115200);
  Serial.println(F(""));

  m_config.begin(identifier);
  m_webServer.begin();
  m_leds.begin();
  m_mqttHandler.begin(); 

  ImprovTypes::ChipFamily variant = ImprovTypes::ChipFamily::CF_ESP32;
  if (ARDUINO_VARIANT == "esp32s3") {
      variant = ImprovTypes::ChipFamily::CF_ESP32_S3;
  } else if (ARDUINO_VARIANT == "esp32c3") {
      variant = ImprovTypes::ChipFamily::CF_ESP32_C3;
  } else if (ARDUINO_VARIANT == "esp32s2") {
      variant = ImprovTypes::ChipFamily::CF_ESP32_S2;
  } else if (ARDUINO_VARIANT == "esp8266") {
      variant = ImprovTypes::ChipFamily::CF_ESP8266;
  } else {
      variant = ImprovTypes::ChipFamily::CF_ESP32;
  }

  improvSerial.setDeviceInfo(variant, identifier, m_config.getVersion(), m_config.getHost());
  improvSerial.onImprovError(std::bind(&HomeStatusDisplay::onImprovWiFiErrorCb, this, std::placeholders::_1));
  improvSerial.onImprovConnected(std::bind(&HomeStatusDisplay::onImprovWiFiConnectedCb, this, std::placeholders::_1, std::placeholders::_2));
  improvSerial.ConnectToWifi(true);

  Serial.printf("Free RAM: %d\n", ESP.getFreeHeap());
  Serial.println(ARDUINO_BOARD);
}

void HomeStatusDisplay::onImprovWiFiConnectedCb(const char *ssid, const char *password)
{
  // Save ssid and password here
  m_webServer.startWebServer();
}

void HomeStatusDisplay::onImprovWiFiErrorCb(ImprovTypes::Error err)
{
  m_webServer.stopWebServer();
  
  if(err == ImprovTypes::Error::ERROR_WIFI_CONNECT_GIVEUP) {
    Serial.println("Giving up on connecting to WiFi, restart the device");
    ESP.restart();
  }
}

void HomeStatusDisplay::work()
{
  unsigned long uptime = calcUptime();
    
  checkConnections();

  improvSerial.loop();
  m_webServer.handleClient(uptime);

  if(improvSerial.isConnected()) {
    m_mqttHandler.handle();
  }
  
  m_leds.update();

  delay(100);
}

unsigned long HomeStatusDisplay::calcUptime()
{
  unsigned long currentMillis = millis();

  if(currentMillis - m_oneMinuteTimerLast >= ONE_MINUTE_MILLIS)
  {
    m_uptime++;
    m_oneMinuteTimerLast = currentMillis;

    Serial.println("Uptime: " + String(m_uptime) + "min");
  }

  return m_uptime;
}

void HomeStatusDisplay::mqttCallback(char* topic, byte* payload, unsigned int length)
{
  unsigned int i = 0;

  for(i = 0; (i < length) && (i < MQTT_MSG_MAX_LEN); i++) 
  {
    mqttMsgBuffer[i] = payload[i];
  }
  mqttMsgBuffer[i] = '\0';

  String mqttTopicString(topic);
  String mqttMsgString = String(mqttMsgBuffer);
  
  Serial.print(F("Received an MQTT message for topic ")); Serial.println(mqttTopicString + ": " + mqttMsgString);

  if(mqttTopicString.equals(m_config.getMqttTestTopic()))
  {
    handleTest(mqttMsgString);
  }
  else if(isStatusTopic(mqttTopicString))
  {
    HSDConfig::deviceType type = getDeviceType(mqttTopicString);
    String device = getDevice(mqttTopicString);

    handleStatus(device, type, mqttMsgString);
  }
}

bool HomeStatusDisplay::isStatusTopic(String& topic)
{
  String mqttStatusTopic = String(m_config.getMqttStatusTopic());
  int posOfLastSlashInStatusTopic = mqttStatusTopic.lastIndexOf("/");

  return topic.startsWith(mqttStatusTopic.substring(0, posOfLastSlashInStatusTopic)) ? true : false;
}

HSDConfig::deviceType HomeStatusDisplay::getDeviceType(String& statusTopic)
{
  HSDConfig::deviceType type = HSDConfig::TYPE_UNKNOWN;

  if(statusTopic.indexOf(LIGHT_STRING) != -1)
  {
    type = HSDConfig::TYPE_LIGHT;
  }
  else if(statusTopic.indexOf(WINDOW_STRING) != -1)
  {
    type = HSDConfig::TYPE_WINDOW;
  }
  else if(statusTopic.indexOf(DOOR_STRING) != -1)
  {
    type = HSDConfig::TYPE_DOOR;
  }
  else if(statusTopic.indexOf(ALARM_STRING) != -1)
  {
    type = HSDConfig::TYPE_ALARM;
  } 

  return type;
}

String HomeStatusDisplay::getDevice(String& statusTopic)
{
  int posOfLastSlashInStatusTopic = statusTopic.lastIndexOf("/");

  return statusTopic.substring(posOfLastSlashInStatusTopic + 1);
}

void HomeStatusDisplay::handleTest(String msg)
{
  int type = msg.toInt();
  if(type > 0)
  {
    Serial.print(F("Showing testpattern ")); Serial.println(type);
    m_leds.test(type);
  }
  else if(type == 0)
  {
    m_leds.clear();
    m_mqttHandler.reconnect();  // back to normal
  }
}

void HomeStatusDisplay::handleStatus(String device, HSDConfig::deviceType type, String msg)
{ 
  int ledNumber = m_config.getLedNumber(device, type);

  if(ledNumber != -1)
  {
    int colorMapIndex = m_config.getColorMapIndex(type, msg);
    
    if(colorMapIndex != -1)
    {
      HSDConfig::Behavior behavior = m_config.getLedBehavior(colorMapIndex);
      HSDConfig::Color color = m_config.getLedColor(colorMapIndex);

      Serial.println("Set led number " + String(ledNumber) + " to behavior " + String(behavior) + " with color " + String(color, HEX));
      m_leds.set(ledNumber, behavior, color);
    }
    else
    {
      Serial.println("Unknown message " + msg + " for led number " + String(ledNumber) + ", set to OFF");
      m_leds.set(ledNumber, HSDConfig::OFF, HSDConfig::NONE);
    }
  }
  else
  {
    Serial.println("No LED defined for device " + device + " of type " + String(type) + ", ignoring it");
  }
}

void HomeStatusDisplay::checkConnections()
{
  if(!m_lastMqttConnectionState && m_mqttHandler.connected())
  {
    m_leds.clear();
    m_lastMqttConnectionState = true;
  }
  else if(m_lastMqttConnectionState && !m_mqttHandler.connected())
  {
    m_leds.clear();
    m_lastMqttConnectionState = false;
  }

  if(!m_mqttHandler.connected() && improvSerial.isConnected())
  {
    m_leds.setAll(HSDConfig::ON, HSDConfig::YELLOW);
  }
  
  if(!m_lastWifiConnectionState && improvSerial.isConnected())
  {
    m_leds.clear();

    if(!m_mqttHandler.connected())
    {
      m_leds.setAll(HSDConfig::ON, HSDConfig::YELLOW);
    }
    
    m_lastWifiConnectionState = true;
  }
  else if(m_lastWifiConnectionState && !improvSerial.isConnected())
  {
    m_leds.clear();
    m_lastWifiConnectionState = false;
  }

  if(!improvSerial.isConnected())
  {
    m_leds.setAll(HSDConfig::ON, HSDConfig::RED);
  }
}


