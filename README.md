# HomeStatusDisplay
#### original code forked from https://github.com/MTJoker/HomeStatusDisplay

**This fork refactures the original code and improved some technicals**

Show status information sent via MQTT (e.g. from FHEM or any other MQTT-speaking system) using RGB LEDs.
![alt tag](http://www.bernd-schubart.de/downloads/fhem/fhemstatusdisplay/homestatusdisplay_37.jpg)

## Requirements
### Hardware
This code was tested on a Wemos D1 mini ESP8266 and ESP32 board. It *should* run on any ESP8266 and ESP32 board. You need a number of WS2812B LEDs (e.g. a NeoPixel stripe) connected to `5V`, `GND` and a `DATA` pin of the ESP. Please notice the wiring tips from Adafruit- use a large capacitor (around 1000uF) between the `5V` and `GND` of the LEDs and put a 300-500 Ohm resistor into the `DATA` line connection to the LEDs. Here is a overview of the wiring - it is the standard wiring recommended for WS2812B stripes. The board can be powered via a the 5V power supply via the USB port. Of course it should be powerful enough to drive all the LEDs used:
![alt tag](http://www.bernd-schubart.de/downloads/fhem/fhemstatusdisplay/homestatusdisplay_board.png)

### Software
The code was developed using PlatformIO. You dont need to compile it by yourself, please us the [web-installer](https://tobiasfaust.github.io/HomeStatusDisplay/)

You also need a running MQTT broker (e.g. https://mosquitto.org), to which the system you want to monitor (e.g. FHEM) pushes its status information. You can also use a public accessible MQTT-Broker, but its not recommend due security reasons. For example, in FHEM the status information which you want to display, can be configured using the FHEM modules `MQTT` and `MQTT_BRIDGE`.

## How to use
Please use the [web-installer](https://tobiasfaust.github.io/HomeStatusDisplay/) to upload the firmware. After it you will request entering Wifi Credentials. After sucessfully WiFiConnect, the installer tells you the Device-IP.

Now, your HomeStatusDisplay is ready to use.
Upon first usage, you should configure your Device at some configuration pages to set up:
 - MQTT setup
 - LED setup

Als intitialer user ist "admin" und als Passwort ist ebenfalls "admin" voreingestellt. Bitte auf der Seite "General" einen eigenen User und Passwort eintragen.

You can find more detailed information in the tutorial on my website (it's in German, unfortunately...):
http://www.bernd-schubart.de/homestatusdisplay-smart-home-status-immer-im-blick/
