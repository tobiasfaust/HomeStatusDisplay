#include <HomeStatusDisplay.h>
#include "HSD_Input.h"
#include "HSD_Switch.h"

static const char* IDENTIFIER = "HomeStatusDisplay";

HomeStatusDisplay display; //HSD

void setup() { 
  mqtt1.subscribe("ledrahmen/light/#", [](const char * topic, const char * payload) {
        // payload might be binary, but PicoMQTT guarantees that it's zero-terminated
        Serial.printf("Received message in topic '%s': %s\n", topic, payload);
        antwort = payload;
        mqtt1.begin();
        Serial.println("TopicAntwort: ");
        Serial.print(antwort);
             // return(payload);
    });
  display.begin(IDENTIFIER);
}

void loop() {     
  
  mqtt1.loop();
  display.work();
 // ArduinoOTA.handle();//check for OTA 
  In();
  delay (100);
  button ();
  delay (100);

  
}

