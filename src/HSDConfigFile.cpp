#include "HSDConfigFile.h"
#include <FS.h>

HSDConfigFile::HSDConfigFile(String fileName)
:
m_fileName(fileName)
{
}

HSDConfigFile::~HSDConfigFile()
{
}

bool HSDConfigFile::read(char* buffer, size_t bufSize)
{
  bool success = false;

  Serial.print(F("Reading config file ")); 
  Serial.println(m_fileName); 

  if(LittleFS.exists(m_fileName)) 
  {
    File configFile = LittleFS.open(m_fileName, "r");

    if(configFile)
    {
      size_t size = configFile.size();
      Serial.print(F("File size is ")); Serial.println(String(size) + " bytes"); 

      if(size <= bufSize)
      { 
        configFile.readBytes(buffer, size);
        success = true;
      }
      else
      {
        Serial.println(F("File is too big"));
      }
    }
    else
    {
      Serial.println(F("File open failed"));
    }

    configFile.close();
  }
  else
  {
    Serial.println(F("File does not exist"));
  }
        
  return success;
}

bool HSDConfigFile::write(JsonObject* data)
{
  bool success = false;

  Serial.print(F("Writing config file ")); 
  Serial.println(m_fileName); 

  File configFile = LittleFS.open(m_fileName, "w+");

  if(configFile)
  {  
    data->printTo(configFile);
    configFile.close();
    
    success = true;
  }
  else
  {
    Serial.println(F("File open failed"));
  }

  return success;
}

