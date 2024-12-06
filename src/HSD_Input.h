#ifndef HSD_INPUT_H
#define HSD_INPUT_H

#include <PicoMQTT.h>
 #define Inputpin A0

unsigned int warten = 5000;
unsigned long loopTime;
int ANALOG_EINGANG0 = 0;
//String topic2 = "ledrahmen/light/mute/#";
String antwort = "INIT";


  
    PicoMQTT::Client mqtt1(
    "192.168.178.189",    // broker address (or IP)
    1883,                   // broker port (defaults to 1883)
    "HSD-Klingel",           // Client ID
    "mqvipertt",             // MQTT username
    "mqahb36wdtt"              // MQTT password
);
/*
 mqtt1.subscribe("ledrahmen/light/mute", [](const char * topic, const char * payload) {
        // payload might be binary, but PicoMQTT guarantees that it's zero-terminated
        Serial.printf("Received message in topic '%s': %s\n", topic, payload);
    });

    mqtt1.begin(); 
   
*/




  /*
  Serial.println("In Funktion getmqtt!");
  Serial.println("Topic: ");
  Serial.print(topic1);
  */


 
void In(){

ANALOG_EINGANG0 = analogRead(Inputpin);
//Serial.println(ANALOG_EINGANG0);

 if((ANALOG_EINGANG0 > 800) & ((loopTime + warten) < millis())){
   Serial.println("Klingel");
   loopTime = millis();
   /*
   HTTPClient http;    //Declare object of class HTTPClient
   http.begin("http://192.168.178.21:1880/blinklicht/9");      //Specify request destination
   http.addHeader("Content-Type", "json");  //Specify content-type header
   int httpCode = http.GET();   //Send the request
   String payload = http.getString();                  //Get the response payload

   Serial.println(httpCode);   //Print HTTP return code
   Serial.println(payload);    //Print request response payload

   http.end();  //Close connection
   */
  mqtt1.publish("ledrahmen/light/blink", "9");
  Serial.println("Published");
}

}
/*
HTTPClient http;

http.begin("http://viper:ahb36wd@192.168.178.190/api/device/blinklicht/");

http.addHeader("Content-Type", "text/plain");

if (Start != 1)
{
    Start = 1;

int httpCode = http.POST("turnOn");
String payload = http.getString()
Serial.println(httpCode);   //Print HTTP return code
Serial.println(payload);    //Print request response payload
http.end();
}
/*

unsigned int Start = 0;
#include "HSDMqtt.hpp"
//#include "HomeStatusDisplay.hpp"
//#include "HSDWifi.cpp"

unsigned int warten = 5000;
unsigned long loopTime;
String msg = "1";
String topic = "ledrahmen/light/blink";

//void handleMqttMessage(String topic, String msg);

//HomeStatusDisplay::HomeStatusDisplay()

//Mqtt1::HomeStatusDisplay()
//class Mqtt1 : HomeStatusDisplay
HSDMqtt::HSDMqtt(const HSDConfig& config, MQTT_CALLBACK_SIGNATURE)
:
m_config(config),
m_pubSubClient(m_wifiClient),
m_maxConnectRetries(3),
m_numConnectRetriesDone(0),
m_retryDelay(5000),
m_millisLastConnectTry(0),
m_numberOfInTopics(0)
{
  m_pubSubClient.setCallback(callback);
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
 //{
void In(){

// HomeStatusDisplay HSDMqtt;
 // Mqtt1 :: Mqtt1(String);
 // Mqtt1.publish();
//void Mqtt1::publish(String topic, String msg)
if (Start != 1)
{
    publish(topic, msg);
 //   m_mqttHandler.publish(topic, msg);
 // if(m_pubSubClient.publish(topic.c_str(), msg.c_str()))
    Start = 1;

    Serial.println("Published msg " + msg + " for topic " + topic);
  }

}
//}

/*
//#include <PubSubClient.h>                         		//-- moved to arduinoGlue.h
//#include <ESP8266WiFi.h>                          		//-- moved to arduinoGlue.h
	//-- moved to arduinoGlue.h // #define Inputpin D2
//WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int msg_blink = 0;
int value = 0;
long now = 0;
unsigned int warten = 5000;
unsigned long loopTime;
String tmsg = "&";
String topic = "ledrahmen/light/blink";

//client.setServer(mqtt_server, 1883);
//client.setCallback(callback);
/*
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.subscribe(blinktopic);
      client.publish(blinktopic, "0");
      // ... and resubscribe
      // Once connected, publish an announcement...
      client.subscribe(pimatictopic);
      client.publish(pimatictopic, "0");
      // ... and resubscribe      
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }

}

void In() {
  pinMode(Inputpin, INPUT);
  if(digitalRead(Inputpin == HIGH)) {
  //&((loopTime + warten) < millis())){
   Serial.println("Klingel");
   loopTime = millis();
//     if (!client.connected()) {
 //   reconnect();
//  }
      Serial.print("Publish message: ");
    Serial.println("&");
    client.publish(String topic, String tmsg);
   }
}
*/
//============ Includes ====================


//============ Added by Convertor ==========


#endif // HSD_INPUT_H