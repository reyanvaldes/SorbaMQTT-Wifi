# SorbaMQTT-Wifi

An MQTT library with Wi-Fi support for microcontrollers (ESP32x, ESP8266), enabling publishing and subscribing of data, 
in a format compatible with SORBA AI platform (https://sorba.ai). This is compatible with both the SORBA Edge and Cloud platforms. 
Written as a C++ class for wrapping Wifi, MQTT and JSON operations.

## Dependencies

 Install the following libraries first:
 
  **WiFi**         // Wifi (V1.2.7)                  https://docs.arduino.cc/libraries/wifi/
    
  **WifiManager**  // Wifi Web Portal (v2.0.17)      https://github.com/tzapu/WiFiManager
  
  **PubSubClient** // for MQTT Messages (V2.8.0)     https://github.com/knolleary/pubsubclient
  
  **ArduinoJson**  // For JSON doc handling (V7.3.1) https://arduinojson.org/?utm_source=meta&utm_medium=library.properties
  
  **UUID**         // for UUID generator (V0.1.6)    https://github.com/RobTillaart/UUID
  
  **ArduinoQueue** // for Queue operations (V1.2.5)  https://github.com/EinarArnason/ArduinoQueue

## How to install

 Download the source code from latest release, file name SorbaMQTT-Wifi-<version>.zip.
 
 Arduino IDE -> Sketch -> Include Library -> Add .ZIP library.
 
 Select SorbaMqtt-Wifi.zip library and click open to install the library and examples.
 
 Can find some starting examples at File -> Examples -> SorbaMqttWifi:
 
   - send_data:  How to send MQTT messages compatible with SORBA format.
   
   - send_data_sim: How to send MQTT messages compatible with SORBA format using simulated data.

   - send_data_timer_ctrl: Controlling time for sending MQTT messages compatible with SORBA without using a delay.   
   
   - send_recv_data: How to send and receive MQTT messages compatible with SORBA.
   
   - send_recv_raw_data: How to send and receive MQTT messages compatible with SORBA, receiving in a raw format (string).
   
   - send_recv_data_wifi_web: How to send and receive MQTT messages compatible with SORBA, including Web Portal por initial Wifi Configuration as AP.
   
   - send_recv_data_tls: How to send and receive MQTT messages compatible with SORBA using TLS with Server certificate. Includes commented code for handling Client certificates.
   
   - send_recv_data_tls_wifi_web: How to send and receive MQTT messages compatible with SORBA using TLS with Server certificate. It includes Web Portal por initial Wifi Configuration as AP.
   
 
  
## How to use

Include the header file on your code:

```C++
#include <WiFiClient.h>   // For non secure connection include <WifiClient.h> or if using SSL <WiFiClientSecure.h>
#include "sorbamqtt_wifi.h"
```

## Constructor

Create SORBA instance Object for sending or receiving MQTT messages using Wifi

```C++
WiFiClient wifiClient;           // Create simple WifiClient object
SorbaMqttWifi sorba(wifiClient); // Create main SORBA object to allow connection,  send or receive messages using MQTT, it also handle retrying for Wifi reconnection
```

Create a set of global communication parameters that will be used when open Wifi and MQTT connections

```C++
// Init communication parameters
 char WIFI_SSID[30]     = "SSID";           // Your Wifi SSID
 char WIFI_PWD[25]      = "PASSWORD";       // Your Password 
 char MQTT_SERVER[25]   = "broker.emqx.io"; // MQTT Server: SORBA Broker u other Public Brokers like "broker.hivemq.com"
 char MQTT_USERNAME[25] = "";               // MQTT User name (if needed)
 char MQTT_PASSWORD[25] = "";               // MQTT Password (if needed)
 uint16_t MQTT_PORT     = 1883;             // MQTT Port
 
 #define  SORBA_GROUP    "PV"                 // Group will used in Sorba structure: <Asset>.<Group>
 #define  MQTT_TOPIC_PUB "sorba/data/Asset1"  // Topic for publish <SORBA_MAIN_TOPIC>/<SORBA_ASSET>
 #define  MQTT_TOPIC_SUB "sorba/data/Asset1Back" // Topic used for subscribing 
```

Connect to Wifi and MQTT Broker at starting (setup)

```C++
void setup() {

 // Setup Serial speed for monitoring 
 Serial.begin(115200); 
 Serial.println("Sending data example"); 

 // connect to Wifi
 int16_t totalSSIDs = sorba.scanWifiNetwork();  // Scan all SSID available from Wifi and show in the Serial port
 
 sorba.connectWifi(WIFI_SSID, WIFI_PWD); // It will kep trying until get connection to the Wifi, otherwise cannot do anything
 
 // Show Wifi connection status
 if (sorba.isConnectedWifi())
  Serial.print("Wifi connection OK");
 else
  Serial.print("Wifi connection Failed");
 
 // Connect to MQTT Broker with username & password
 sorba.connect(MQTT_SERVER, MQTT_PORT, MQTT_USERNAME, MQTT_PASSWORD);  // Has a retry of 3 times for the connection to the MQTT broker
 
 // Show MQTT connection status
 if (sorba.isConnected())
  Serial.println("MQTT connection OK");
 else
  Serial.println("MQTT connection Failed");

 // Subscribe to a topic to receive messages back
 sorba.subscribe(MQTT_TOPIC_SUB);
}
```

To send a message with Sorba format to the MQTT Broker, preparing the messages and send it

```C++
   // Prepare the message and packing all parameters values
   sorba.msgInit(); // Clear any previous message from memory, this must be called initially to prepare the JSON object correctly
   sorba.msgPack (SORBA_GROUP, "temp",  12.5);       // Will use default decimal places = 2, can change with setFloatDecimals(#)
   sorba.msgPack (SORBA_GROUP, "press", 44.8342, 3); // if want to use custom decimal places different from default, the third parameter indicate total decimals requested
   sorba.msgPack (SORBA_GROUP, "count", 4);          // Adding integer to the JSON message
   sorba.msgPack (SORBA_GROUP, "text", "example");   // Adding char[] to the JSON message

   // sendMsg will publish message, e.g: {"PV":{"temp":12.5,"press":44.834,"count":4,"text":"example"}} 
   // Inside sendMsg include the checking of Wifi and MQTT connections and reconnect if needed
   sorba.sendMsg(MQTT_TOPIC_PUB); 
   
   delay(5000); // Time pacing for sending, also can use bool sorba.timerDone() see example 'send_data_timer_ctrl'

```C++
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

```

To receive a raw message with Sorba format from the MQTT Broker

```C++
// Check if there are messages in queue and parse it automatically. Later can use msgUnpack to extract the parameter value easily
 while (sorba.recvMsg(topic, payload)) { // if there are messages pending, remove it, e.g: {"PV": {"ad": 60.5, "run": 1}}
    Serial.print("Received Msg Topic: "); Serial.print(topic);
	Serial.print(", Payload: "); Serial.println(payload);
	 
	 // if want to do parsing and unpack, need to call .parseMsg(String), after can unpack
	 if (sorba.parseMsg(payload)) {                  // Try to parse from String to JSON, after can do unpack
	  float ad =0.0; 
      short run = 0;
      sorba.msgUnpack (SORBA_GROUP, "ad", ad);       // Transferring from JSON msg already parsed to each parameter values 
      sorba.msgUnpack (SORBA_GROUP, "run", run); 
      Serial.printf("ad: %f",ad);
      Serial.printf(", run: %i\r\n", run);
	  // For simple message, e.g: {"ad": 60.5, "run": 1} can use first argument as empty char[]:
      // sorba.msgUnpack ("", "ad", ad); 
      // sorba.msgUnpack ("", "run", run);
	 }
   }
```

## Thread safety

This library is **not** thread safe. Mutexes are needed for multi-threading.
