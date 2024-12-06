#include "HSDMqtt.h"

HSDMqtt::HSDMqtt(const HSDConfig& config, MQTT_CALLBACK_SIGNATURE)
:
m_pubSubClient(m_wifiClient),
m_config(config),
m_numberOfInTopics(0),
m_connectFailure(false),
m_maxConnectRetries(60),
m_numConnectRetriesDone(0),
m_retryDelay(10000),
m_millisLastConnectTry(0)
{
  m_pubSubClient.setCallback(callback);
}

void HSDMqtt::begin()
{  
  initTopics();
  addTopic(m_config.getMqttStatusTopic());
  addTopic(m_config.getMqttTestTopic());

  IPAddress mqttIpAddr;

  if(mqttIpAddr.fromString(m_config.getMqttServer()))
  {
    // valid ip address entered 
    m_pubSubClient.setServer(mqttIpAddr, m_config.getMqttServerPort()); 
  }
  else
  {
    // invalid ip address, try as hostname
    m_pubSubClient.setServer(m_config.getMqttServer(), m_config.getMqttServerPort());  
  }
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
  if(connected()) {
    m_pubSubClient.loop();
  
  } else if(!m_connectFailure) {
    unsigned long currentMillis = millis();

    if( (int)(currentMillis - m_millisLastConnectTry) >= m_retryDelay) {
      m_millisLastConnectTry = currentMillis; 

      if(m_numConnectRetriesDone < m_maxConnectRetries) {
        if(reconnect()) {
          Serial.println("DEBUG: Reconnect successful");
          m_numConnectRetriesDone = 0;
        } else {
          m_numConnectRetriesDone++;
          Serial.println("DEBUG: Reconnect unsuccessful, m_numConnectRetriesDone = " + String(m_numConnectRetriesDone));
        }
      } else {
        Serial.println(F("Failed to connect Mqtt. Give it up, Restart ESP"));      
        m_connectFailure = true;
        ESP.restart();
      } 
    }
  } 
}

bool HSDMqtt::connected() const
{
  return m_pubSubClient.connected();
}

bool HSDMqtt::reconnect()
{
  bool retval = false;
  
  // Create a random client ID
  String clientId = "ESP8266Client-";
  clientId += String(random(0xffff), HEX);
  
  Serial.printf("Connecting to MQTT broker %s:%d with client id %s...\n", m_config.getMqttServer(), m_config.getMqttServerPort(), clientId.c_str());

  const char* willTopic = m_config.getMqttWillTopic();
  const char* mqttAuthUser = m_config.getMqttServerAuthUser();
  const char* mqttAuthPass = m_config.getMqttServerAuthPass();
  bool connected = false;

  if(strlen(mqttAuthUser) != 0 && strlen(mqttAuthPass) != 0){
    Serial.print(" connecting with User and Pass ");
    if(isTopicValid(willTopic)) {
        connected = m_pubSubClient.connect(clientId.c_str(), mqttAuthUser, mqttAuthPass, willTopic, 0, true, "off");
    } else {
        connected = m_pubSubClient.connect(clientId.c_str(), mqttAuthUser, mqttAuthPass);
    }
  } else {
    if(isTopicValid(willTopic)) {
      connected = m_pubSubClient.connect(clientId.c_str(), willTopic, 0, true, "off");
    } else {
      connected = m_pubSubClient.connect(clientId.c_str());
    }
  }
  
  if(connected) 
  {
    Serial.println(F("connected"));

    if(isTopicValid(willTopic))
    {
      publish(willTopic, "on");
    }

    for(uint32_t index = 0; index < m_numberOfInTopics; index++)
    {
      subscribe(m_inTopics[index]);
    }

    retval = true;
  } 
  else 
  {
    Serial.print(F("failed, rc = "));
    Serial.println(m_pubSubClient.state());
  }

  return retval;
}

void HSDMqtt::subscribe(const char* topic)
{
  if(isTopicValid(topic))
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

  if(isTopicValid(topic))
  {  
    if(m_numberOfInTopics < (MAX_IN_TOPICS - 1))
    {
      m_inTopics[m_numberOfInTopics] = topic;
      m_numberOfInTopics++;
      success = true;
    }
  }

  return success;
}

bool HSDMqtt::isTopicValid(const char* topic)
{
  return (strlen(topic) > 0);
}

