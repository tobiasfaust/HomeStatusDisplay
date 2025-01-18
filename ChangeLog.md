Release 0.7:
  - change to PlatformIO
  - fix some bugs
  - change to ESPAsyncWebserver to achieve ESP32 compatibility
  - change max WiFi PSK length to 64 characters
  - change max configurable LED mapping entries for ESP32 to 100
  - change max configurable Color mapping entries for ESP32 to 50
  - use ElegantOta to achieve ESP32 compatibility
  - add Gitpod integration for a custom compiling
  - increase MQTT address length to 50 characters (thanks to JudgeDredd@fhem)
  - add MQTT authentication (thanks to JudgeDredd@fhem)
  - optionally secure WebUI-Config with username/password (thanks to JudgeDredd@fhem)
  - add WebInstaller, remove initial Accesspoint handling
  - change custom wifi handling to ImprovWiFiLibrary via web-installer
  - add custom ElegantOta to select predefined binaries 
  - add automatic Backup&Restore of configfiles when using ElegantOTA
  - migrate from Arduino5 to Arduino7

  **Breaking changes** 
  From release 0.7 onwards, an access point will no longer be opened during an initial installation. The WiFi access data must be entered via the web installer. This is used for both initial installation and entry of WiFi access data. 
  When updating version 0.6 to 0.7, a new installation must also be carried out because the WiFi handling has been switched to the ImprovWiFi Library. 
  If version 0.7 is already installed on the ESP device, an OTA update is sufficient. See “Update” button at ESP-webinterface 

  Detailed instructions can be found in the [WiKi](https://github.com/tobiasfaust/HomeStatusDisplay/wiki)