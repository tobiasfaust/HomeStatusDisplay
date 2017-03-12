#include "HSDMqtt.h"

HSDMqtt::HSDMqtt(const HSDConfig& config, MQTT_CALLBACK_SIGNATURE)
:
m_config(config),
m_pubSubClient(m_wifiClient),
m_lastReconnectAttempt(0)
{
  m_pubSubClient.setCallback(callback);
}

void HSDMqtt::begin()
{
  Serial.println("");
  Serial.print(F("Initializing MQTT connection to "));
  Serial.println(m_config.getMqttServer());
  
  initTopics();
  addTopic(m_config.getMqttStatusTopic());
  addTopic(m_config.getMqttTestTopic());
  
  m_pubSubClient.setServer(m_config.getMqttServer(), 1883);  
}

void HSDMqtt::initTopics()
{
  for(uint32_t index = 0; index < MAX_IN_TOPICS; index++)
  {
    m_inTopics[index] = NULL;
  }

  m_numberOfInTopics = 0;
}

void HSDMqtt::handle()
{
  if (!m_pubSubClient.connected()) 
  {
    long now = millis();
    if (now - m_lastReconnectAttempt > 5000) 
    {
      m_lastReconnectAttempt = now;
      // Attempt to reconnect
      if (reconnect()) 
      {
        m_lastReconnectAttempt = 0;
      }
    }
  }
  else
  {
    m_pubSubClient.loop();
  }
}

bool HSDMqtt::connected() const
{
  return m_pubSubClient.connected();
}

bool HSDMqtt::reconnect()
{
  // Create a random client ID
  String clientId = "ESP8266Client-";
  clientId += String(random(0xffff), HEX);

  Serial.print(F("Connecting to MQTT broker "));
  Serial.print(" with client id " + clientId + "... ");
  
  if(m_pubSubClient.connect(clientId.c_str(), m_config.getMqttWillTopic(), 0, false, "off")) 
  {
    Serial.println(F("connected"));

    publish(m_config.getMqttWillTopic(), "on");

    for(uint32_t index = 0; index < m_numberOfInTopics; index++)
    {
      subscribe(m_inTopics[index]);
    }
  } 
  else 
  {
    Serial.print(F("failed, rc="));
    Serial.println(m_pubSubClient.state());
  }

  return m_pubSubClient.connected();
}

void HSDMqtt::subscribe(const char* topic)
{
  if(topic)
  {
    Serial.print(F("Subscribing to topic "));
    Serial.println(topic);
    
    if(!m_pubSubClient.subscribe(topic))
    {
      Serial.print("Failed to subscribe to topic ");
      Serial.println(topic);
    }
  }
}

void HSDMqtt::publish(String topic, String msg)
{
  if(m_pubSubClient.publish(topic.c_str(), msg.c_str()))
  {
    Serial.println("Published msg " + msg + " for topic " + topic);
  }
  else
  {
    Serial.println("Error publishing msg " + msg + " for topic " + topic);
  }
}

bool HSDMqtt::addTopic(const char* topic)
{
  bool success = false;
  
  if(m_numberOfInTopics < (MAX_IN_TOPICS - 1))
  {
    m_inTopics[m_numberOfInTopics] = topic;
    m_numberOfInTopics++;
    success = true;
  }

  return success;
}
