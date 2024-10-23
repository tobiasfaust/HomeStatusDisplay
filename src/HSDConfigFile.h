#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>

class HSDConfigFile
{
  
public:

  HSDConfigFile(String fileName);
  virtual ~HSDConfigFile();

  bool read(char* buffer, int bufSize);
  bool write(JsonObject* data);
  
private:
  
  String m_fileName;
};

