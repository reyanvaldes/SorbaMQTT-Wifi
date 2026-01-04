/*
        Author: Reyan Valdes
        email: reyanvaldes@yahoo.com

        An example of using SorbaMqttWifi Library - Sending simulated data to SORBA and receive back messages. Using Wifi Manager for handling Wifi connection

        Usage and further info:
        https://github.com/reyanvaldes/SorbaMQTT-Wifi

 Libraries or dependencies have to be installed
  WiFi         // Wifi (V1.2.7)                  https://docs.arduino.cc/libraries/wifi/
  WifiManager  // Wifi Web Portal (v2.0.17)      https://github.com/tzapu/WiFiManager
  PubSubClient // for MQTT Messages (V2.8.0)     https://github.com/knolleary/pubsubclient
  ArduinoJson  // For JSON doc handling (V7.3.1) https://arduinojson.org/?utm_source=meta&utm_medium=library.properties
  UUID         // for UUID generator (V0.1.6)    https://github.com/RobTillaart/UUID
  ArduinoQueue // for Queue operations (V1.2.5)  https://github.com/EinarArnason/ArduinoQueue

*/
// Example of how to send simulated data and receive back messages
#include <WiFiClient.h>   // For non secure connection include <WifiClient.h> or if using SSL <WiFiClientSecure.h>
#include <WiFiManager.h>  // Web Portal for Wifi configuration: https://github.com/tzapu/WiFiManager  (v2.0.17)
#include <sorbamqtt_wifi.h>

// Init communication parameters
 char MQTT_SERVER[25]   = "broker.emqx.io"; // MQTT Server: SORBA Broker u other Public Brokers like "broker.hivemq.com";
 char MQTT_USERNAME[20] = "";               // MQTT User name (if needed)
 char MQTT_PASSWORD[20] = "";               // MQTT Password (if needed)
 uint16_t MQTT_PORT     = 1883;             // MQTT Port
 
 #define  SORBA_GROUP    "PV"                 // Group will used in Sorba structure: <Asset>.<Group>
 #define  MQTT_TOPIC_PUB "sorba/data/Asset1"  // Topic for publish <SORBA_MAIN_TOPIC>/<SORBA_ASSET>;
 #define  MQTT_TOPIC_SUB "sorba/data/Asset1Back" // Topic used for subscribing 
 
WiFiClient wifiClient;            // Create simple WifiClient object

SorbaMqttWifi sorba(wifiClient); // Create main SORBA object to allow connection,  send or receive messages using MQTT


String topic;   // Topic used when receiving messages from MQTT Broker
 

struct tData {
  float temp;
  float pres;
  int   c; 
};

tData param; // Reading Sensors Data

// Init basic parameters 
void initData()
{
  param.temp = 0;
  param.pres = 0;
  param.c = 0;
}

// Simulate Reading sensors values
void readSensorsData()
{ 
 param.temp += 0.05;
 param.pres += 0.0612;
 param.c++;

 Serial.println("Simulated values");
}


void setup() {
  // put your setup code here, to run once:
 // Setup Serial speed for monitoring 
 Serial.begin(115200);    // Set baudrate
 Serial.println("SORBA- sending Data Demo"); 

 // This blocks until connected or portal timeout
 // Initialize WiFiManager, Access Point with IP: 192.168.4.1, and Access Point "ESP32_AP", can connnect a Tablet or Cell Phoone to it and 
 // configure the SSID and PWD for Wifi configuration. After both parameters are stored in NVS (flash memory)
  WiFiManager wm;

  if (!wm.autoConnect("ESP32_AP")) {     // Failed to connect, change to Access Point with simple WebPortal for configuration
    Serial.println("Failed to connect");
    ESP.restart();
  }

 Serial.println("Wi-Fi connected by WiFiManager!");
 Serial.print("IP Address: "); Serial.println(WiFi.localIP());

 String ssid = WiFi.SSID();  // Has to capture before disconnect, because disconnect reset both from memory
 String pwd = WiFi.psk();

  // --- Disconnect WiFiManager to take full control ---
  WiFi.disconnect(false, true); // Disconnect, keep credentials in NVS (flash memory)
  delay(1000);

 // connect to Wifi
 sorba.connectWifi(ssid, pwd);  // It will kep trying until get connection to the Wifi, otherwise cannot do anything
 
 // Connect to MQTT Broker with username & password
 // Future feature: including certificate for SSL connection and zlib library compression
 sorba.connect(MQTT_SERVER, MQTT_PORT, MQTT_USERNAME, MQTT_PASSWORD);

 // Show MQTT connection status
 if (sorba.isConnected())
  Serial.println("MQTT connection OK");
 else
  Serial.println("MQTT connection Failed");

 // Subscribe to a topic to receive messages back
 sorba.subscribe(MQTT_TOPIC_SUB);

 // Init sensors data
 initData();
}

void loop() {
   // Read sensors data
   readSensorsData();
 
   // Prepare the message and packing all parameters values
   sorba.msgInit(); // Clear any previous message from memory, this must be called initially to prepare the JSON object correctly
   sorba.msgPack (SORBA_GROUP, "temp",  param.temp);    // Will use default decimal places = 2, can change with setFloatDecimals(#)
   sorba.msgPack (SORBA_GROUP, "press", param.pres, 3); // if want to use custom decimal places different from default, the third parameter indicate total decimals requested
   sorba.msgPack (SORBA_GROUP, "count", param.c);       // Adding integer to the JSON message
   sorba.msgPack (SORBA_GROUP, "text", "example");      // Adding char[] to the JSON message

   // sendMsg will publish message, e.g: {"PV":{"temp":0.2,"press":0.245,"count":4,"text":"example"}} 
   // Inside sendMsg include the checking of Wifi and MQTT connections and reconnect if needed
   sorba.sendMsg(MQTT_TOPIC_PUB); 

  // How to receive a message from SORBA
  // Check if there are messages in queue and parse it automatically. Later can use msgUnpack to extract the parameter value easily
  while (sorba.recvMsg(topic)) { // if there are messages pending, parse it, e.g: {"PV": {"ad": 60.5, "run": 1}}
     float ad =0.0; 
     short run = 0;
     Serial.print("Received Msg Topic: "); Serial.print(topic);
     sorba.msgUnpack (SORBA_GROUP, "ad", ad);       // Transferring from JSON msg already parsed to each parameter values 
     sorba.msgUnpack (SORBA_GROUP, "run", run); 
     Serial.printf("ad: %f",ad);
     Serial.printf(", run: %i\r\n", run);
	 // For simple message, e.g: {"ad": 60.5, "run": 1} can use first argument as empty char[]:
     // sorba.msgUnpack ("", "ad", ad); 
     // sorba.msgUnpack ("", "run", run);
   }
   
 delay(5000); // Pacing for sending. Can use  sorba.setTimer(time_ms) and bool sorba.timerDone() to control of sending data
 
 
}
