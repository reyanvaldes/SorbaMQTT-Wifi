/*
        Author: Reyan Valdes
        email: reyanvaldes@yahoo.com

        A basic example of using SorbaMqttWifi Library - Sending data to SORBA

        Usage and further info:
        https://github.com/reyanvaldes/SorbaMQTT-Wifi
		
 Libraries or dependencies have to be installed
  WiFi         // Wifi (V1.2.7)                  https://docs.arduino.cc/libraries/wifi/
  PubSubClient // for MQTT Messages (V2.8.0)     https://github.com/knolleary/pubsubclient
  ArduinoJson  // For JSON doc handling (V7.3.1) https://arduinojson.org/?utm_source=meta&utm_medium=library.properties
  UUID         // for UUID generator (V0.1.6)    https://github.com/RobTillaart/UUID
  ArduinoQueue // for Queue operations (V1.2.5)  https://github.com/EinarArnason/ArduinoQueue

*/

// Example of how to send basic data
#include <WiFiClient.h>   // For non secure connection include <WifiClient.h> or if using SSL <WiFiClientSecure.h>
#include "sorbamqtt_wifi.h"

// Init communication parameters
 char WIFI_SSID[15]     = "SSID";           // Your Wifi SSID
 char WIFI_PWD[15]      = "PASSWORD";       // Your Password 
 char MQTT_SERVER[25]   = "broker.emqx.io"; // MQTT Server: SORBA Broker u other Public Brokers like "broker.hivemq.com";
 char MQTT_USERNAME[20] = "";               // MQTT User name (if needed)
 char MQTT_PASSWORD[20] = "";               // MQTT Password (if needed)
 uint16_t MQTT_PORT     = 1883;             // MQTT Port
 
 #define  SORBA_GROUP    "PV"                 // Group will used in Sorba structure: <Asset>.<Group>
 #define  MQTT_TOPIC_PUB "sorba/data/Asset1"  // Topic for publish <SORBA_MAIN_TOPIC>/<SORBA_ASSET>;

 WiFiClient wifiClient;            // Create simple WifiClient object

 SorbaMqttWifi sorba(wifiClient); // Create main SORBA object to allow connection,  send or receive messages using MQTT

void setup() {
 // Setup Serial speed for monitoring 
 Serial.begin(115200);   // Set baudrate
 Serial.println("SORBA- sending basic data example with timer control"); 

 // connect to Wifi
 sorba.connectWifi(WIFI_SSID, WIFI_PWD); // It will kep trying until get connection to the Wifi, otherwise cannot do anything
 
 // Connect to MQTT Broker with username & password
 sorba.connect(MQTT_SERVER, MQTT_PORT, MQTT_USERNAME, MQTT_PASSWORD);  // Has a retry of 3 times for the connection to the MQTT broker

 // Show MQTT connection status
 if (sorba.isConnected())
  Serial.println("MQTT connection OK");
 else
  Serial.println("MQTT connection Failed");

 // Set timer to control sending data, by default it is 5 seconds
  sorba.setTimer(3000); // time in ms

}

// main loop
void loop() {
   // Controlling sending data based on Timer. Use setTimer(time_ms) to control when trigger the timer
   if (sorba.timerDone()) {

   // Prepare the message and packing all parameters values
   sorba.msgInit(); // Clear any previous message from memory, this must be called initially to prepare the JSON object correctly
   sorba.msgPack (SORBA_GROUP, "temp",  12.5);       // Will use default decimal places = 2, can change with setFloatDecimals(#)
   sorba.msgPack (SORBA_GROUP, "press", 44.8342, 3); // if want to use custom decimal places different from default, the third parameter indicate total decimals requested
   sorba.msgPack (SORBA_GROUP, "count", 4);          // Adding integer to the JSON message
   sorba.msgPack (SORBA_GROUP, "text", "example");   // Adding char[] to the JSON message

   // sendMsg will publish message, e.g: {"PV":{"temp":12.5,"press":44.834,"count":4,"text":"example"}} 
   // Inside sendMsg include the checking of Wifi and MQTT connections and reconnect if needed
   sorba.sendMsg(MQTT_TOPIC_PUB); 
   }
  // Here we don't need the delay 
}
