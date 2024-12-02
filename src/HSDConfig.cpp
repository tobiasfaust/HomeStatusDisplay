#include "HSDConfig.h"
#include <FS.h>
#include <ArduinoJson.h>

const constexpr HSDConfig::ColorTranslator HSDConfig::colorTranslator[8];

HSDConfig::HSDConfig()
:
m_cfgColorMapping(MAX_COLOR_MAP_ENTRIES),
m_cfgDeviceMapping(MAX_DEVICE_MAP_ENTRIES),
m_mainConfigFile(String("/config.json")),
m_colorMappingConfigFile(String("/colormapping.json")),
m_deviceMappingConfigFile(String("/devicemapping.json"))
{  
  // reset non-configurable members
  setHost("");
  this->version = (String(Release) + "@" + String(GIT_BRANCH));

  // reset configurable members
  resetMainConfigData();
  resetColorMappingConfigData();
  resetDeviceMappingConfigData();
}

void HSDConfig::begin(const char* defaultIdentifier)
{
  Serial.println(F(""));
  Serial.println(F("Initializing config."));

  setHost(defaultIdentifier);
  
  if(startLittleFS)
  {
    Serial.println(F("Mounted file system."));

    readMainConfigFile();
    readColorMappingConfigFile();
    readDeviceMappingConfigFile();
  }
  else
  {
    Serial.println(F("Failed to mount file system"));
  }
}

void HSDConfig::resetMainConfigData()
{
  Serial.println(F("Deleting main config data."));
    
  setGuiUser("admin");
  setGuiPass("admin");
  setMqttServer("");
  setMqttServerAuthUser("");
  setMqttServerAuthPass("");
  setMqttStatusTopic("");
  setMqttTestTopic("");  
  setMqttWillTopic(""); 

  setNumberOfLeds(0);
  setLedDataPin(0);
  setLedBrightness(DEFAULT_LED_BRIGHTNESS);
}

void HSDConfig::resetColorMappingConfigData()
{
  Serial.println(F("Deleting color mapping config data."));
  
  m_cfgColorMapping.clear();
  m_cfgColorMappingDirty = true;
}

void HSDConfig::resetDeviceMappingConfigData()
{
  Serial.println(F("Deleting device mapping config data."));
  
  m_cfgDeviceMapping.clear();
  m_cfgDeviceMappingDirty = true;
}

bool HSDConfig::readMainConfigFile()
{
  bool success = false;

  char fileBuffer[MAX_SIZE_MAIN_CONFIG_FILE];

  if(m_mainConfigFile.read(fileBuffer, MAX_SIZE_MAIN_CONFIG_FILE))
  {
    DynamicJsonBuffer jsonBuffer(JSON_BUFFER_MAIN_CONFIG_FILE);
    JsonObject& json = jsonBuffer.parseObject(fileBuffer);

    if (json.success()) 
    {
      Serial.println(F("Main config data successfully parsed."));
      Serial.print(F("JSON length is ")); Serial.println(json.measureLength());     
      printMainConfigFile(json);
      Serial.println(F(""));

      if(json.containsKey(JSON_KEY_HOST) && 
         json.containsKey(JSON_KEY_GUI_USER) && 
         json.containsKey(JSON_KEY_GUI_PASS) && 
         json.containsKey(JSON_KEY_MQTT_SERVER) && 
         json.containsKey(JSON_KEY_MQTT_AUTHUSER) && 
         json.containsKey(JSON_KEY_MQTT_AUTHPASS) &&          
         json.containsKey(JSON_KEY_MQTT_STATUS_TOPIC) && 
         json.containsKey(JSON_KEY_MQTT_TEST_TOPIC) && 
         json.containsKey(JSON_KEY_MQTT_WILL_TOPIC) &&
         json.containsKey(JSON_KEY_LED_COUNT) && 
         json.containsKey(JSON_KEY_LED_PIN))
      {
        Serial.println(F("Config data is complete."));

        setHost(json[JSON_KEY_HOST]);
        setGuiUser(json[JSON_KEY_GUI_USER]);
        setGuiPass(json[JSON_KEY_GUI_PASS]);
        setMqttServer(json[JSON_KEY_MQTT_SERVER]);
        setMqttServerAuthUser(json[JSON_KEY_MQTT_AUTHUSER]);
        setMqttServerAuthPass(json[JSON_KEY_MQTT_AUTHPASS]);
        setMqttStatusTopic(json[JSON_KEY_MQTT_STATUS_TOPIC]);
        setMqttTestTopic(json[JSON_KEY_MQTT_TEST_TOPIC]);
        setMqttWillTopic(json[JSON_KEY_MQTT_WILL_TOPIC]);
        setNumberOfLeds(json[JSON_KEY_LED_COUNT]);
        setLedDataPin(json[JSON_KEY_LED_PIN]);
        setLedBrightness(json[JSON_KEY_LED_BRIGHTNESS]);

        success = true;
      }
    } 
    else 
    {
      Serial.println(F("Could not parse config data."));
    }
  }
  else
  {
    Serial.println(F("Creating default main config file."));
    resetMainConfigData();
    writeMainConfigFile();
  }

  return success;
}

void HSDConfig::printMainConfigFile(JsonObject& json)
{
  Serial.print  (F("  • version         : ")); Serial.println(getVersion());
  Serial.print  (F("  • host            : ")); Serial.println((const char*)(json[JSON_KEY_HOST]));
  Serial.print  (F("  • guiUser         : ")); Serial.println((const char*)(json[JSON_KEY_GUI_USER]));
  Serial.println(F("  • guiPass         : not shown"));
  Serial.print  (F("  • mqttServer      : ")); Serial.println((const char*)(json[JSON_KEY_MQTT_SERVER]));
  Serial.print  (F("  • mqttServerUser  : ")); Serial.println((const char*)(json[JSON_KEY_MQTT_AUTHUSER]));
  Serial.println(F("  • mqttServerPass  : not shown"));
  Serial.print  (F("  • mqttStatusTopic : ")); Serial.println((const char*)(json[JSON_KEY_MQTT_STATUS_TOPIC]));
  Serial.print  (F("  • mqttTestTopic   : ")); Serial.println((const char*)(json[JSON_KEY_MQTT_TEST_TOPIC]));
  Serial.print  (F("  • mqttWillTopic   : ")); Serial.println((const char*)(json[JSON_KEY_MQTT_WILL_TOPIC]));
  Serial.print  (F("  • ledCount        : ")); Serial.println((const char*)(json[JSON_KEY_LED_COUNT]));
  Serial.print  (F("  • ledPin          : ")); Serial.println((const char*)(json[JSON_KEY_LED_PIN]));
  Serial.print  (F("  • ledBrightness   : ")); Serial.println((const char*)(json[JSON_KEY_LED_BRIGHTNESS]));
}

bool HSDConfig::readColorMappingConfigFile()
{
  bool success = false;

  char fileBuffer[MAX_SIZE_COLOR_MAPPING_CONFIG_FILE];
  memset(fileBuffer, 0, MAX_SIZE_COLOR_MAPPING_CONFIG_FILE);
  resetColorMappingConfigData();

  if(m_colorMappingConfigFile.read(fileBuffer, MAX_SIZE_COLOR_MAPPING_CONFIG_FILE))
  {
    DynamicJsonBuffer jsonBuffer(JSON_BUFFER_COLOR_MAPPING_CONFIG_FILE);
    JsonObject& json = jsonBuffer.parseObject(fileBuffer);

    if (json.success()) 
    {
      Serial.println(F("Color mapping config data successfully parsed."));
      Serial.print(F("JSON length is ")); Serial.println(json.measureLength());  
      //json.prettyPrintTo(Serial);
      Serial.println(F(""));

      success = true;
      int index = 0;
      
      for(JsonObject::iterator it = json.begin(); it != json.end(); ++it)
      {
        JsonObject& entry = json[it->key]; 

        if(entry.containsKey(JSON_KEY_COLORMAPPING_MSG) && entry.containsKey(JSON_KEY_COLORMAPPING_TYPE) &&
           entry.containsKey(JSON_KEY_COLORMAPPING_COLOR) && entry.containsKey(JSON_KEY_COLORMAPPING_BEHAVIOR) )
        {
          addColorMappingEntry(index,
                               entry[JSON_KEY_COLORMAPPING_MSG].as<char*>(), 
                               (deviceType)(entry[JSON_KEY_COLORMAPPING_TYPE].as<int>()), 
                               (Color)(id2color(entry[JSON_KEY_COLORMAPPING_COLOR].as<int>())), 
                               (Behavior)(entry[JSON_KEY_COLORMAPPING_BEHAVIOR].as<int>())); 

          index++;
        }
      }
    }
    else
    {
      Serial.println(F("Could not parse config data."));
    }
  }
  else
  {
    Serial.println(F("Creating default color mapping config file."));
    resetColorMappingConfigData();
    writeColorMappingConfigFile();
  }

  m_cfgColorMappingDirty = false;
  return success;
}

bool HSDConfig::readDeviceMappingConfigFile()
{
  bool success = false;

  char fileBuffer[MAX_SIZE_DEVICE_MAPPING_CONFIG_FILE];
  memset(fileBuffer, 0, MAX_SIZE_DEVICE_MAPPING_CONFIG_FILE);
  resetDeviceMappingConfigData();

  if(m_deviceMappingConfigFile.read(fileBuffer, MAX_SIZE_DEVICE_MAPPING_CONFIG_FILE))
  {
    DynamicJsonBuffer jsonBuffer(JSON_BUFFER_DEVICE_MAPPING_CONFIG_FILE);
    JsonObject& json = jsonBuffer.parseObject(fileBuffer);

    if (json.success()) 
    {
      Serial.println(F("Device mapping config data successfully parsed."));
      Serial.print(F("JSON length is ")); Serial.println(json.measureLength());  
      //json.prettyPrintTo(Serial);
      Serial.println(F(""));

      success = true;
      int index = 0;
      
      for(JsonObject::iterator it = json.begin(); it != json.end(); ++it)
      {
        JsonObject& entry = json[it->key]; 

        if(entry.containsKey(JSON_KEY_DEVICEMAPPING_NAME) && entry.containsKey(JSON_KEY_DEVICEMAPPING_TYPE) &&
           entry.containsKey(JSON_KEY_DEVICEMAPPING_LED) )
        {
          addDeviceMappingEntry(index,
                                entry[JSON_KEY_DEVICEMAPPING_NAME].as<char*>(), 
                                (deviceType)(entry[JSON_KEY_DEVICEMAPPING_TYPE].as<int>()), 
                                entry[JSON_KEY_DEVICEMAPPING_LED].as<int>());

           index++;                               
        }
      }
    }
    else
    {
      Serial.println(F("Could not parse config data."));
    }
  }
  else
  {
    Serial.println(F("Creating default device mapping config file."));
    resetDeviceMappingConfigData();
    writeDeviceMappingConfigFile();
  }

  m_cfgDeviceMappingDirty = false;
  return success;
}

void HSDConfig::writeMainConfigFile()
{
  DynamicJsonBuffer jsonBuffer(JSON_BUFFER_MAIN_CONFIG_FILE);
  JsonObject& json = jsonBuffer.createObject();

  json[JSON_KEY_HOST] = m_cfgHost;
  json[JSON_KEY_GUI_USER] = m_cfgGuiUser;
  json[JSON_KEY_GUI_PASS] = m_cfgGuiPass;
  json[JSON_KEY_MQTT_SERVER] = m_cfgMqttServer;
  json[JSON_KEY_MQTT_AUTHUSER] = m_cfgMqttServerAuthUser;
  json[JSON_KEY_MQTT_AUTHPASS] = m_cfgMqttServerAuthPass;
  json[JSON_KEY_MQTT_STATUS_TOPIC] = m_cfgMqttStatusTopic;
  json[JSON_KEY_MQTT_TEST_TOPIC] = m_cfgMqttTestTopic;
  json[JSON_KEY_MQTT_WILL_TOPIC] = m_cfgMqttWillTopic;
  json[JSON_KEY_LED_COUNT] = m_cfgNumberOfLeds;
  json[JSON_KEY_LED_PIN] = m_cfgLedDataPin;
  json[JSON_KEY_LED_BRIGHTNESS] = m_cfgLedBrightness;

  if(!m_mainConfigFile.write(&json))
  {
    onFileWriteError();
  }
}

void HSDConfig::writeColorMappingConfigFile()
{
  DynamicJsonBuffer jsonBuffer(JSON_BUFFER_COLOR_MAPPING_CONFIG_FILE);
  JsonObject& json = jsonBuffer.createObject();

  for(int index = 0; index < m_cfgColorMapping.size(); index++)
  { 
    const ColorMapping* mapping = m_cfgColorMapping.get(index);
    
    if(strlen(mapping->msg) != 0)
    {
      Serial.print(F("Preparing to write color mapping config file index "));
      Serial.print(String(index));
      Serial.print(F(", msg="));
      Serial.println(String(mapping->msg));
      
      JsonObject& colorMappingEntry = json.createNestedObject(String(index));
  
      colorMappingEntry[JSON_KEY_COLORMAPPING_MSG] = mapping->msg;
      colorMappingEntry[JSON_KEY_COLORMAPPING_TYPE] = (int)mapping->type;
      colorMappingEntry[JSON_KEY_COLORMAPPING_COLOR] = (int)color2id(mapping->color);
      colorMappingEntry[JSON_KEY_COLORMAPPING_BEHAVIOR] = (int)mapping->behavior;
    }
    else
    {
      Serial.print(F("Removing color mapping config file index "));
      Serial.println(String(index));
    }
  }

  if(!m_colorMappingConfigFile.write(&json))
  {
    onFileWriteError();
  }
  else
  {
    m_cfgColorMappingDirty = false;
  }
}

void HSDConfig::writeDeviceMappingConfigFile()
{
  DynamicJsonBuffer jsonBuffer(JSON_BUFFER_DEVICE_MAPPING_CONFIG_FILE);
  JsonObject& json = jsonBuffer.createObject();

  for(int index = 0; index < m_cfgDeviceMapping.size(); index++)
  {
    const DeviceMapping* mapping = m_cfgDeviceMapping.get(index);
        
    if(strlen(mapping->name) != 0)
    {
      Serial.print(F("Preparing to write device mapping config file index "));
      Serial.println(String(index));
        
      JsonObject& deviceMappingEntry = json.createNestedObject(String(index));
  
      deviceMappingEntry[JSON_KEY_DEVICEMAPPING_NAME] = mapping->name;
      deviceMappingEntry[JSON_KEY_DEVICEMAPPING_TYPE] = (int)mapping->type;
      deviceMappingEntry[JSON_KEY_DEVICEMAPPING_LED]  = (int)mapping->ledNumber;
    }
    else
    {
      Serial.print(F("Removing device mapping config file index "));
      Serial.println(String(index));
    }
  }
  
  if(!m_deviceMappingConfigFile.write(&json))
  {
    onFileWriteError();
  } 
  else
  {
    m_cfgDeviceMappingDirty = false;
  }
}

void HSDConfig::saveMain()
{
  writeMainConfigFile();
}

void HSDConfig::saveColorMapping()
{
  writeColorMappingConfigFile();
}

void HSDConfig::updateColorMapping()
{
  readColorMappingConfigFile();
}

void HSDConfig::saveDeviceMapping()
{
  writeDeviceMappingConfigFile();
}

void HSDConfig::updateDeviceMapping()
{
  readDeviceMappingConfigFile();
}

void HSDConfig::onFileWriteError()
{
  Serial.println(F("Failed to write file, formatting file system."));
  LittleFS.format();
  Serial.println(F("Done.")); 
}

bool HSDConfig::addDeviceMappingEntry(int entryNum, String name, deviceType type, int ledNumber)
{
  bool success = false;

  Serial.print(F("Adding or editing device mapping entry at index ")); 
  Serial.println(String(entryNum) + " with name " + name + ", type " + String(type) + ", LED number " + String(ledNumber));

  DeviceMapping mapping(name, type, ledNumber);

  if(m_cfgDeviceMapping.set(entryNum, mapping))
  {
    m_cfgDeviceMappingDirty = true;
    success = true;
  }
  else
  {
    Serial.println(F("Cannot add/edit device mapping entry")); 
  }

  return success;
}

bool HSDConfig::deleteDeviceMappingEntry(int entryNum)
{
  bool removed = m_cfgDeviceMapping.remove(entryNum); 
  
  if(removed)
  {
    m_cfgDeviceMappingDirty = true;
  }
  
  return removed;
}

bool HSDConfig::deleteAllDeviceMappingEntries()
{
  m_cfgDeviceMapping.clear();
  m_cfgDeviceMappingDirty = true; 
  return true;
}

bool HSDConfig::isDeviceMappingDirty() const
{
  return m_cfgDeviceMappingDirty;
}

bool HSDConfig::isDeviceMappingFull() const
{
  return m_cfgDeviceMapping.isFull();
}

int HSDConfig::getDeviceMaxSize() const
{
  return m_cfgDeviceMapping.maxSize();
}

bool HSDConfig::addColorMappingEntry(int entryNum, String msg, deviceType type, Color color, Behavior behavior)
{
  bool success = false;

  Serial.print(F("Adding or editing color mapping entry at index ")); 
  Serial.println(String(entryNum) + ", new values: name " + msg + ", type " + String(type) + ", color " + String(color) + ", behavior " + String(behavior));

  ColorMapping mapping(msg, type, color, behavior);

  if(m_cfgColorMapping.set(entryNum, mapping))
  {
    m_cfgColorMappingDirty = true;
    success = true;
  }
  else
  {
    Serial.println(F("Cannot add/edit device mapping entry")); 
  }

  return success;  
}

bool HSDConfig::deleteColorMappingEntry(int entryNum)
{
  bool removed = m_cfgColorMapping.remove(entryNum);
  
  if(removed)
  {
    m_cfgColorMappingDirty = true;
  }
  
  return removed;
}

bool HSDConfig::deleteAllColorMappingEntries()
{
  m_cfgColorMapping.clear();
  m_cfgColorMappingDirty = true; 
  return true;
}

int HSDConfig::getColorMappingMaxSize() const
{
  return m_cfgColorMapping.maxSize();
}

bool HSDConfig::isColorMappingDirty() const
{
  return m_cfgColorMappingDirty;
}

bool HSDConfig::isColorMappingFull() const
{
  return m_cfgColorMapping.isFull();
}

const char* HSDConfig::getHost() const
{
  return m_cfgHost;
}

bool HSDConfig::setHost(const char* host)
{
  strncpy(m_cfgHost, host, MAX_HOST_LEN);
  m_cfgHost[MAX_HOST_LEN] = '\0';
  return true;
}

const char* HSDConfig::getVersion() const
{
  return this->version.c_str();
}

const char* HSDConfig::getGuiUser() const
{
  return m_cfgGuiUser;
}

bool HSDConfig::setGuiUser(const char* guiuser)
{
  strncpy(m_cfgGuiUser, guiuser, MAX_GUI_USER_LEN);
  m_cfgGuiUser[MAX_GUI_USER_LEN] = '\0';
  return true;
}

const char* HSDConfig::getGuiPass() const
{
  return m_cfgGuiPass;
}

bool HSDConfig::setGuiPass(const char* guipass)
{
  strncpy(m_cfgGuiPass, guipass, MAX_GUI_PASS_LEN);
  m_cfgGuiPass[MAX_GUI_PASS_LEN] = '\0';
  return true;
}

const char* HSDConfig::getMqttServer() const
{
  return m_cfgMqttServer;
}

bool HSDConfig::setMqttServer(const char* ip)
{
  strncpy(m_cfgMqttServer, ip, MAX_MQTT_SERVER_LEN);
  m_cfgMqttServer[MAX_MQTT_SERVER_LEN] = '\0';
  return true;
}

const char* HSDConfig::getMqttServerAuthUser() const
{
  return m_cfgMqttServerAuthUser;
}

bool HSDConfig::setMqttServerAuthUser(const char* mqttauthuser)
{
  strncpy(m_cfgMqttServerAuthUser, mqttauthuser, MAX_MQTT_SERVER_USER_LEN);
  m_cfgMqttServerAuthUser[MAX_MQTT_SERVER_USER_LEN] = '\0';
  return true;
}

const char* HSDConfig::getMqttServerAuthPass() const
{
  return m_cfgMqttServerAuthPass;
}

bool HSDConfig::setMqttServerAuthPass(const char* mqttauthpass)
{
  strncpy(m_cfgMqttServerAuthPass, mqttauthpass, MAX_MQTT_SERVER_PASS_LEN);
  m_cfgMqttServerAuthPass[MAX_MQTT_SERVER_PASS_LEN] = '\0';
  return true;
}

const char* HSDConfig::getMqttStatusTopic() const
{
  return m_cfgMqttStatusTopic;
}

bool HSDConfig::setMqttStatusTopic(const char* topic)
{
  strncpy(m_cfgMqttStatusTopic, topic, MAX_MQTT_STATUS_TOPIC_LEN);
  m_cfgMqttStatusTopic[MAX_MQTT_STATUS_TOPIC_LEN] = '\0';
  return true;
}

const char* HSDConfig::getMqttTestTopic() const
{
  return m_cfgMqttTestTopic;
}

bool HSDConfig::setMqttTestTopic(const char* topic)
{
  strncpy(m_cfgMqttTestTopic, topic, MAX_MQTT_TEST_TOPIC_LEN);
  m_cfgMqttTestTopic[MAX_MQTT_TEST_TOPIC_LEN] = '\0';
  return true;
}

int HSDConfig::getNumberOfLeds() const
{
  return m_cfgNumberOfLeds;
}

const char* HSDConfig::getMqttWillTopic() const
{
  return m_cfgMqttWillTopic;
}

bool HSDConfig::setMqttWillTopic(const char* topic)
{
  strncpy(m_cfgMqttWillTopic, topic, MAX_MQTT_WILL_TOPIC_LEN);
  m_cfgMqttWillTopic[MAX_MQTT_WILL_TOPIC_LEN] = '\0';
  return true;
}

bool HSDConfig::setNumberOfLeds(uint32_t numberOfLeds)
{
  m_cfgNumberOfLeds = numberOfLeds;
  return true;
}

int HSDConfig::getLedDataPin() const
{
  return m_cfgLedDataPin;
}

bool HSDConfig::setLedDataPin(int dataPin)
{
  m_cfgLedDataPin = dataPin;
  return true;
}

uint8_t HSDConfig::getLedBrightness() const
{
  return m_cfgLedBrightness;
}

bool HSDConfig::setLedBrightness(uint8_t brightness)
{
  m_cfgLedBrightness = brightness;
  return true;
}

int HSDConfig::getNumberOfColorMappingEntries()
{
  return m_cfgColorMapping.size();
}

const HSDConfig::ColorMapping* HSDConfig::getColorMapping(int index)
{
  return m_cfgColorMapping.get(index);
}

int HSDConfig::getNumberOfDeviceMappingEntries() const
{
  return m_cfgDeviceMapping.size();
}

const HSDConfig::DeviceMapping* HSDConfig::getDeviceMapping(int index) const
{
    return m_cfgDeviceMapping.get(index);
}

int HSDConfig::getLedNumber(String deviceName, deviceType deviceType)
{
  int number = -1;

  for(int i = 0; i < m_cfgDeviceMapping.size(); i++)
  {
    const DeviceMapping* mapping = m_cfgDeviceMapping.get(i);
    
    if(deviceName.equals(mapping->name) && (deviceType == mapping->type))
    {
      number = mapping->ledNumber;
      break;
    }
  }

  return number;
}

int HSDConfig::getColorMapIndex(deviceType deviceType, String msg)
{
  int index = -1;

  for(int i = 0; i < m_cfgColorMapping.size(); i++)
  {
    const ColorMapping* mapping = m_cfgColorMapping.get(i);
    
    if(msg.equals(mapping->msg) && (deviceType == mapping->type))
    {
      index = i;
      break;
    }
  }
  
  return index;
}

HSDConfig::Behavior HSDConfig::getLedBehavior(int colorMapIndex)
{
  return m_cfgColorMapping.get(colorMapIndex)->behavior;
}

HSDConfig::Color HSDConfig::getLedColor(int colorMapIndex)
{
  return m_cfgColorMapping.get(colorMapIndex)->color;
}
