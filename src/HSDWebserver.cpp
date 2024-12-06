#include "HSDWebserver.h"

HSDWebserver::HSDWebserver(HSDConfig& config, const HSDLeds& leds, const HSDMqtt& mqtt)
:
m_server(80),
m_config(config),
m_leds(leds),
m_mqtt(mqtt),
m_deviceUptimeMinutes(0),
m_html()
{
  //m_updateServer.setup(&m_server);
}

void HSDWebserver::begin()
{
  Serial.println(F(""));
  Serial.println(F("Starting WebServer."));

  m_server.on("/", std::bind(&HSDWebserver::deliverStatusPage, this, std::placeholders::_1));
  m_server.on("/cfgmain", std::bind(&HSDWebserver::deliverRootPage, this, std::placeholders::_1));
  m_server.on("/cfgcolormapping", std::bind(&HSDWebserver::deliverColorMappingPage, this, std::placeholders::_1));
  m_server.on("/cfgdevicemapping", std::bind(&HSDWebserver::deliverDeviceMappingPage, this, std::placeholders::_1));
  m_server.onNotFound(std::bind(&HSDWebserver::deliverNotFoundPage, this, std::placeholders::_1));

  ElegantOTA.begin(&m_server);    // Start ElegantOTA
  // ElegantOTA callbacks
  //ElegantOTA.onStart(onOTAStart);
  //ElegantOTA.onProgress(onOTAProgress);
  //ElegantOTA.onEnd(onOTAEnd);

  // overwrite standard website of Elegant-Ota, use custom site
  m_server.on("/updateota", HTTP_GET, std::bind(&HSDWebserver::deliverUpdatePage, this, std::placeholders::_1));
  m_server.on("/getdeviceinfo", HTTP_GET, std::bind(&HSDWebserver::getDeviceInfo, this, std::placeholders::_1));

  //m_server.begin(); // handled via ImprovWifiLibrary Callback
}

void HSDWebserver::startWebServer() {
  m_server.begin();
}

void HSDWebserver::stopWebServer() {
  m_server.end();
}

void HSDWebserver::handleClient(unsigned long deviceUptime)
{
  m_deviceUptimeMinutes = deviceUptime;
  ElegantOTA.loop();
}

void HSDWebserver::getDeviceInfo(AsyncWebServerRequest *request) {
  Serial.println(F("Delivering update page."));
  if( strlen( m_config.getGuiUser() ) != 0 ) {
    if( !request->authenticate( m_config.getGuiUser(), m_config.getGuiPass() ) )
      return request->requestAuthentication();
  }
  
  AsyncResponseStream *response = request->beginResponseStream("application/json");
  response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  response->addHeader("Pragma", "no-cache");
  response->addHeader("Expires", "-1");

  String jsonString = "{\"owner\":\"" + String(GIT_OWNER) + 
                    "\",\"repository\":\"" + String(GIT_REPO) + 
                    "\",\"chipfamily\":\"" + m_config.getChipFamilyStr() + 
                    "\",\"withFS\":\"" + String(WITH_FS).toInt() +
                    "\"}";
  response->setContentLength(jsonString.length());
  response->print(jsonString);
  request->send(response);
}

void HSDWebserver::deliverUpdatePage(AsyncWebServerRequest *request) {
  Serial.println(F("Delivering update page."));
  if( strlen( m_config.getGuiUser() ) != 0 ) {
    if( !request->authenticate( m_config.getGuiUser(), m_config.getGuiPass() ) )
      return request->requestAuthentication();
  }
  
  AsyncWebServerResponse *response = request->beginResponse(200, "text/html", ELEGANT_OTA_HTML, ELEGANT_OTA_HTML_len);
  response->addHeader("Content-Encoding", "gzip");
  request->send(response);
}

void HSDWebserver::deliverRootPage(AsyncWebServerRequest *request) {
  if( strlen( m_config.getGuiUser() ) != 0 ) {
    if( !request->authenticate( m_config.getGuiUser(), m_config.getGuiPass() ) )
      return request->requestAuthentication();
  }
  AsyncResponseStream *response = request->beginResponseStream("text/html");
  response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  response->addHeader("Pragma", "no-cache");
  response->addHeader("Expires", "-1");

  bool needSave = updateMainConfig(request);
  
  String html;
  html.reserve(3000);
  
  html = m_html.getHeader("General configuration", m_config.getHost(), m_config.getVersion());

  html += F("<form><font face='Verdana,Arial,Helvetica'>");
  
  html += F(
  "<table width='30%' border='0' cellpadding='0' cellspacing='2'>"
  " <tr>"
  "  <td><b><font size='+1'>General</font></b></td>"
  "  <td></td>"
  " </tr>"
  " <tr>"
  "  <td>Name</td>");
  
  html += F("  <td><input type='text' id='host' name='host' value='");
  html += String(m_config.getHost());
  html += F("' size='30' maxlength='40' placeholder='host'></td></tr>");

  html += F(
  " <tr>"
  "  <td><b><font size='+1'>HTTP</font></b></td>"
  "  <td></td>"
  " </tr>"
  " <tr>"
  "  <td>GUI User</td>"
  "  <td><input type='text' id='guiUser' name='guiUser' value='"); html += String(m_config.getGuiUser()); html += F("' size='30' maxlength='40' placeholder='Username'></td>"
  " </tr>"
  " <tr>"
  "  <td>GUI Passwort</td>"
  "  <td><input type='password' id='guiPass' name='guiPass' value='"); html += String(m_config.getGuiPass()); html += F("' size='30' maxlength='40' placeholder='Password'></td>"
  " </tr>");

  html += F(
  " <tr>"
  "  <td><b><font size='+1'>MQTT</font></b></td>"
  "  <td></td>"
  " </tr>"
  " <tr>"
  "  <td>Server</td>"
  "  <td><input type='text' id='mqttServer' name='mqttServer' value='"); html += String(m_config.getMqttServer()); html += F("' size='30' maxlength='50' placeholder='IP or hostname'></td>"
  " </tr>"
  " <tr>"
  "  <td>Server Auth User</td>"
  "  <td><input type='text' id='mqttServerAuthUser' name='mqttServerAuthUser' value='"); html += String(m_config.getMqttServerAuthUser()); html += F("' size='30' maxlength='50' placeholder='Username'></td>"
  " </tr>"
  " <tr>"
  "  <td>Server Auth Password</td>"
  "  <td><input type='password' id='mqttServerAuthPass' name='mqttServerAuthPass' value='"); html += String(m_config.getMqttServerAuthPass()); html += F("' size='30' maxlength='50' placeholder='Password'></td>"
  " </tr>"

  "<tr><td>Status topic</td>");
  html += F("  <td><input type='text' id='mqttStatusTopic' name='mqttStatusTopic' value='");
  html += String(m_config.getMqttStatusTopic());
  html += F("' size='30' maxlength='50' placeholder='#'></td>"
  " </tr>"
  " <tr>"
  "  <td>Test topic</td>"
  "  <td><input type='text' id='mqttTestTopic' name='mqttTestTopic' value='");
  html += String(m_config.getMqttTestTopic());
  html += F("' size='30' maxlength='50' placeholder='#'></td>"
  " </tr>"
  " <tr>"
  "  <td>Will topic</td>"
  "  <td><input type='text' id='mqttWillTopic' name='mqttWillTopic' value='");
  html += String(m_config.getMqttWillTopic());
  html += F("' size='30' maxlength='50' placeholder='#'></td></tr>");

  html += F(""
  " <tr>"
  "  <td><b><font size='+1'>LEDs</font></b></td>"
  "  <td></td>"
  " </tr>"
  " <tr>"
  "  <td>Number of LEDs</td>");
  html += "  <td><input type='text' id='ledCount' name='ledCount' value='" + String(m_config.getNumberOfLeds()) + "' size='30' maxlength='40' placeholder='0'></td></tr>";
  html += F("<tr><td>LED pin</td>");
  html += F("<td><input type='text' id='ledPin' name='ledPin' value='");
  html += String(m_config.getLedDataPin());
  html += F("' size='30' maxlength='40' placeholder='0'></td></tr>");

  html += F("<tr><td>Brightness</td>");
  html += F("<td><input type='text' id='ledBrightness' name='ledBrightness' value='");
  html += String(m_config.getLedBrightness());
  html += F("' size='30' maxlength='5' placeholder='0-255'></td></tr></table>"); 
  
  html += F("<input type='submit' class='button' value='Save'>");

  html += F("</form></font></body></html>");
  
  Serial.print(F("Page size: "));
  Serial.println(html.length());
  
  response->print(html);
  request->send(response);

  if(needSave)
  {
    Serial.println(F("Main config has changed, storing it."));
    m_config.saveMain();
  }

  checkReboot(request);

  Serial.print(F("Free RAM: ")); Serial.println(ESP.getFreeHeap());
}

void HSDWebserver::deliverStatusPage(AsyncWebServerRequest *request) {
  AsyncResponseStream *response = request->beginResponseStream("text/html");
  response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  response->addHeader("Pragma", "no-cache");
  response->addHeader("Expires", "-1");

  String html;
  html.reserve(3000);
  
  html = m_html.getHeader("Status", m_config.getHost(), m_config.getVersion());

  html += F("ESP Free RAM: ");
  html += ESP_GetFreeHeap();
  html += F(" Bytes</p>");

  html += F("<p>Device uptime: ");
  html += m_html.minutes2Uptime(m_deviceUptimeMinutes);
  html += F("</p>");

  if (WiFi.status() == WL_CONNECTED)
  {
    html += F("<p>Device is connected to WLAN <b>");
    html += WiFi.SSID();
    html += F("</b><br/>IP address is <b>");
    html += m_html.ip2String(WiFi.localIP());
    html += F("</b><br/><p>");
  }
  else
  {
    html += F("<p>Device is not connected to local network<p>");
  }

  if(m_mqtt.connected())
  {
    html += F("<p>Device is connected to  MQTT broker <b>");
    html += m_config.getMqttServer();
    html += F("</b><p>");
  }
  else
  {
    html += F("<p>Device is not connected to an MQTT broker<p>");
  }

  if(m_config.getNumberOfLeds() == 0)
  {
    html += F("<p>No LEDs configured yet<p>");
  }
  else
  {
    int ledOnCount = 0;
    
    html += F("<p>");
    
    for(int ledNr = 0; ledNr < m_config.getNumberOfLeds(); ledNr++)
    {
      HSDConfig::Color color = m_leds.getColor(ledNr);
      HSDConfig::Behavior behavior = m_leds.getBehavior(ledNr);

      if( (HSDConfig::NONE != color) && (HSDConfig::OFF != behavior) )
      {
        html += F("<p><div class='hsdcolor' style='background-color:");
        html += m_html.color2htmlColor(color);
        html += F("';></div>"); 
        html += F("LED number <b>");
        html += ledNr;
        html += F("</b> is <b>");
        html += m_html.behavior2String(behavior);
        html += F("</b> with color <b>");
        html += m_html.color2String(color);      
        html += F("</b><br/></p>");

        ledOnCount++;
      }
    }

    if(ledOnCount == 0)
    {
      html += F("<p>All LEDs are <b>off</b><p>");
    }

    html += F("</p>");
  }

  Serial.print(F("Page size: "));
  Serial.println(html.length());
  
  response->print(html);
  request->send(response);

  checkReboot(request);
}

void HSDWebserver::deliverColorMappingPage(AsyncWebServerRequest *request) {
  if( strlen( m_config.getGuiUser() ) != 0 ) {
    if( !request->authenticate( m_config.getGuiUser(), m_config.getGuiPass() ) )
      return request->requestAuthentication();
  }
  AsyncResponseStream *response = request->beginResponseStream("text/html");
  response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  response->addHeader("Pragma", "no-cache");
  response->addHeader("Expires", "-1");

  if(needUndo(request))
  {
    Serial.println(F("Need to undo changes to color mapping config"));
    m_config.updateColorMapping();
  }
  else if(needAdd(request))
  {
    Serial.println(F("Need to add color mapping config entry"));
    addColorMappingEntry(request);
  }
  else if(needDelete(request))
  {
    Serial.println(F("Need to delete color mapping config entry"));
    deleteColorMappingEntry(request);
  }
  else if(needDeleteAll(request))
  {
    Serial.println(F("Need to delete all color mapping config entries"));
    m_config.deleteAllColorMappingEntries();
  }
  else if(needSave(request))
  {
    Serial.println(F("Need to save color mapping config"));
    m_config.saveColorMapping();
  }
    
  String html;
  html.reserve(8000);
  
  html = m_html.getHeader("Color mapping configuration", m_config.getHost(), m_config.getVersion());

  html += m_html.getColorMappingTableHeader();

  for(uint16_t i = 0; i < m_config.getNumberOfColorMappingEntries(); i++)
  {
    const HSDConfig::ColorMapping* mapping = m_config.getColorMapping(i);
    html += m_html.getColorMappingTableEntry(i, mapping);
  }

  html += m_html.getColorMappingTableFooter();

  if(m_config.isColorMappingFull())
  {
    html += F("</table><p>Edit entry (add not possible, entry limit reached):</p>");
    html += m_html.getColorMappingTableAddEntryForm(m_config.getNumberOfColorMappingEntries(), true);
  }
  else
  {
    html += F("</table><p>Add/edit entry (max ");   
    html += m_config.getColorMappingMaxSize();
    html += F(" items):</p>"); 

    html += m_html.getColorMappingTableAddEntryForm(m_config.getNumberOfColorMappingEntries(), false);
  }

  html += F("<p>Delete Entry:</p>");
  html += m_html.getDeleteForm();

  if(m_config.isColorMappingDirty())
  {
    html += F("<p style='color:red'>Unsaved changes! Press Save to make them permanent, <br/>or Undo to revert to last saved version!</p>");
    html += m_html.getSaveForm();
  }

  html += m_html.getFooter();

  Serial.print(F("Page size: "));
  Serial.println(html.length());
  
  response->print(html);
  request->send(response);

  checkReboot(request);

  Serial.print(F("Free RAM: ")); Serial.println(ESP.getFreeHeap());
}

bool HSDWebserver::needAdd(AsyncWebServerRequest *request)
{
   return (request->hasArg("add"));
}

bool HSDWebserver::needDelete(AsyncWebServerRequest *request)
{
   return (request->hasArg("delete"));
}

bool HSDWebserver::needDeleteAll(AsyncWebServerRequest *request)
{
  return (request->hasArg("deleteall"));
}

bool HSDWebserver::needSave(AsyncWebServerRequest *request)
{
   return (request->hasArg("save"));
}

bool HSDWebserver::needUndo(AsyncWebServerRequest *request)
{
   return (request->hasArg("undo"));
}

bool HSDWebserver::addColorMappingEntry(AsyncWebServerRequest *request)
{
  bool success = false;
  
  if(request->hasArg("i") && request->hasArg("n") && request->hasArg("t") && request->hasArg("c") && request->hasArg("b"))
  {
    if(request->arg("n") != "")
    {
      success = m_config.addColorMappingEntry(request->arg("i").toInt(),
                                              request->arg("n"), 
                                              (HSDConfig::deviceType)(request->arg("t").toInt()), 
                                              (HSDConfig::Color)(HSDConfig::id2color(request->arg("c").toInt())), 
                                              (HSDConfig::Behavior)(request->arg("b").toInt()));
    }
    else
    {
      Serial.print(F("Skipping empty entry"));
    }
  }

  return success;
}

bool HSDWebserver::deleteColorMappingEntry(AsyncWebServerRequest *request)
{
  bool success = false;
  int entryNum = 0;
  
  if(request->hasArg("i"))
  {
    entryNum = request->arg("i").toInt();
// TODO check conversion status
    success = m_config.deleteColorMappingEntry(entryNum);                                  
  }

  return success;
}

void HSDWebserver::deliverDeviceMappingPage(AsyncWebServerRequest *request) {
  if( strlen( m_config.getGuiUser() ) != 0 ) {
    if( !request->authenticate( m_config.getGuiUser(), m_config.getGuiPass() ) )
      return request->requestAuthentication();
  }
  AsyncResponseStream *response = request->beginResponseStream("text/html");
  response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  response->addHeader("Pragma", "no-cache");
  response->addHeader("Expires", "-1");

  if(needUndo(request))
  {
    Serial.println(F("Need to undo changes to device mapping config"));
    m_config.updateDeviceMapping();
  }
  else if(needAdd(request))
  {
    Serial.println(F("Need to add device mapping config entry"));
    addDeviceMappingEntry(request);
  }
  else if(needDelete(request))
  {
    Serial.println(F("Need to delete device mapping config entry"));
    deleteDeviceMappingEntry(request);
  }
  else if(needDeleteAll(request))
  {
    Serial.println(F("Need to delete all device mapping config entries"));
    m_config.deleteAllDeviceMappingEntries();
  }
  else if(needSave(request))
  {
    Serial.println(F("Need to save device mapping config"));
    m_config.saveDeviceMapping();
  }

  String html;
  html.reserve(8000);
    
  html = m_html.getHeader("Device mapping configuration", m_config.getHost(), m_config.getVersion());

  html += m_html.getDeviceMappingTableHeader();
  
  for(uint16_t i = 0; i < m_config.getNumberOfDeviceMappingEntries(); i++)
  {
    const HSDConfig::DeviceMapping* mapping = m_config.getDeviceMapping(i);
    html += m_html.getDeviceMappingTableEntry(i, mapping);
  }

  html += m_html.getDeviceMappingTableFooter();

  if(m_config.isDeviceMappingFull())
  {
    html += F("</table><p>Edit entry (add not possible, entry limit of ");
    html += m_config.getDeviceMaxSize();
    html += F(" reached):</p>");
    html += m_html.getDeviceMappingTableAddEntryForm(m_config.getNumberOfDeviceMappingEntries(), true);
  }
  else
  {
    html += F("</table><p>Add/edit entry (max ");   
    html += m_config.getDeviceMaxSize();
    html += F(" items):</p>"); 

    html += m_html.getDeviceMappingTableAddEntryForm(m_config.getNumberOfDeviceMappingEntries(), false);
  }

  html += F("<br/>Delete Entry:<br/>");
  html += m_html.getDeleteForm();

  if(m_config.isDeviceMappingDirty())
  {
    html += F("<p style='color:red'>Unsaved changes! Press ""Save"" to make them permanent, or they will be lost on next reboot!</p>");
    html += m_html.getSaveForm();
  }

  html += m_html.getFooter();

  Serial.print(F("Page size: "));
  Serial.println(html.length());
  
  response->print(html);
  request->send(response);
  
  checkReboot(request);

  Serial.print(F("Free RAM: ")); Serial.println(ESP.getFreeHeap());
}

bool HSDWebserver::addDeviceMappingEntry(AsyncWebServerRequest *request) {
  bool success = false;

  if(request->hasArg("i") && request->hasArg("n") && request->hasArg("t") && request->hasArg("l"))
  {
    if(request->arg("n") != "")
    {
      success = m_config.addDeviceMappingEntry(request->arg("i").toInt(),
                                               request->arg("n"), 
                                               (HSDConfig::deviceType)(request->arg("t").toInt()), 
                                               request->arg("l").toInt());                                   
    }
    else
    {
      Serial.print(F("Skipping empty entry"));
    }
  }
  
  return success;
}

bool HSDWebserver::deleteDeviceMappingEntry(AsyncWebServerRequest *request)
{
  bool success = false;
  int entryNum = 0;
  
  if(request->hasArg("i"))
  {
    entryNum = request->arg("i").toInt();
// TODO check conversion status
    success = m_config.deleteDeviceMappingEntry(entryNum);                                    
  }

  return success;
}

void HSDWebserver::deliverNotFoundPage(AsyncWebServerRequest *request) {
  AsyncResponseStream *response = request->beginResponseStream("text/html");
  response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  response->addHeader("Pragma", "no-cache");
  response->addHeader("Expires", "-1");

  String html = F("File Not Found\n\n");
  html += F("URI: ");
  html += request->url();
  html += F("\nMethod: ");
  html += (request->method() == HTTP_GET) ? F("GET") : F("POST");
  html += F("\nArguments: ");
  html += request->args();
  html += F("\n");
  
  for (uint8_t i = 0; i < request->args(); i++)
  {
    html += " " + request->argName(i) + ": " + request->arg(i) + "\n";
  }
  
  response->print(html);
  request->send(response);
}

void HSDWebserver::checkReboot(AsyncWebServerRequest *request)
{
  if(request->hasArg(F("reset"))) 
  {
    Serial.println(F("Rebooting ESP."));
    ESP.restart();
  }
}

bool HSDWebserver::updateMainConfig(AsyncWebServerRequest *request)
{
  bool needSave = false;

  if (request->hasArg(JSON_KEY_HOST))
  {
    needSave |= m_config.setHost(request->arg(JSON_KEY_HOST).c_str());
  }
  
  if (request->hasArg(JSON_KEY_GUI_USER))
  {
    needSave |= m_config.setGuiUser(request->arg(JSON_KEY_GUI_USER).c_str());
  }
  
  if (request->hasArg(JSON_KEY_GUI_PASS)) 
  {
    needSave |= m_config.setGuiPass(request->arg(JSON_KEY_GUI_PASS).c_str());
  }

  if (request->hasArg(JSON_KEY_MQTT_SERVER))
  {
    needSave |= m_config.setMqttServer(request->arg(JSON_KEY_MQTT_SERVER).c_str());
  }

  if (request->hasArg(JSON_KEY_MQTT_AUTHUSER))
  {
    needSave |= m_config.setMqttServerAuthUser(request->arg(JSON_KEY_MQTT_AUTHUSER).c_str());
  }

  if (request->hasArg(JSON_KEY_MQTT_AUTHPASS))
  {
    needSave |= m_config.setMqttServerAuthPass(request->arg(JSON_KEY_MQTT_AUTHPASS).c_str());
  }
  
  if (request->hasArg(JSON_KEY_MQTT_STATUS_TOPIC))
  {
    needSave |= m_config.setMqttStatusTopic(request->arg(JSON_KEY_MQTT_STATUS_TOPIC).c_str());
  }
  
  if (request->hasArg(JSON_KEY_MQTT_TEST_TOPIC)) 
  {
    needSave |= m_config.setMqttTestTopic(request->arg(JSON_KEY_MQTT_TEST_TOPIC).c_str());
  }

  if (request->hasArg(JSON_KEY_MQTT_WILL_TOPIC)) 
  {
    needSave |= m_config.setMqttWillTopic(request->arg(JSON_KEY_MQTT_WILL_TOPIC).c_str());
  }

  if (request->hasArg(JSON_KEY_LED_COUNT))
  {
    int ledCount = request->arg(JSON_KEY_LED_COUNT).toInt();
    
    if(ledCount > 0)
    {
      needSave |= m_config.setNumberOfLeds(ledCount);
    }
  }
  
  if (request->hasArg(JSON_KEY_LED_PIN)) 
  {
    int ledPin = request->arg(JSON_KEY_LED_PIN).toInt();
    
    if(ledPin > 0)
    {
      needSave |= m_config.setLedDataPin(ledPin);
    }
  }

  if (request->hasArg(JSON_KEY_LED_BRIGHTNESS)) 
  {
    uint8_t ledBrightness = request->arg(JSON_KEY_LED_BRIGHTNESS).toInt();
    
    if(ledBrightness > 0)
    {
      needSave |= m_config.setLedBrightness(ledBrightness);
    }
  }

  return needSave;
}
