/*
        Author: Reyan Valdes
        email: reyanvaldes@yahoo.com

        An example of using SorbaMqttWifi Library - Sending simulated data to SORBA and receive back messages. Using Wifi Manager for handling Wifi connection

        Usage and further info:
        https://github.com/reyanvaldes/SorbaMQTT-Wifi

 Libraries or dependencies have to be installed
  WiFi         // Wifi (V1.2.7)                   https://docs.arduino.cc/libraries/wifi/
  WifiManager  // Wifi Web Portal (v2.0.17)       https://github.com/tzapu/WiFiManager
  PubSubClient // for MQTT Messages (V2.8.0)      https://github.com/knolleary/pubsubclient
  ArduinoJson  // For JSON doc handling (V7.3.1)  https://arduinojson.org/?utm_source=meta&utm_medium=library.properties
  UUID         // for UUID generator (V0.1.6)     https://github.com/RobTillaart/UUID
  ArduinoQueue // for Queue operations (V1.2.5)   https://github.com/EinarArnason/ArduinoQueue
  TFT_eSPI     // For TFT/OLED Display (v2.5.43)  https://github.com/Bodmer/TFT_eSPI
               **Need to modify Documents\Arduino\libraries\TFT_eSPI\User_Setup_Select.h in the TFT_eSPI library according to the model of OLED Display using:
                  //#include <User_Setup.h>     <= comment this line
                  #include <User_Setups/Setup25_TTGO_T_Display.h>    // Setup file for ESP32 and TTGO T-Display ST7789V SPI bus TFT  <= uncomment the OLED Display board version, in this example one compatible with TTGO     
*/
// Example of how to send simulated data and receive back messages
#include <WiFiClient.h>     // For non secure connection include <WifiClient.h> or if using SSL <WiFiClientSecure.h>
#include <WiFiManager.h>    // Web Portal for Wifi configuration: https://github.com/tzapu/WiFiManager  (v2.0.17)
#include <sorbamqtt_wifi.h> // MQTT & Wifi operation for Sorba send and receive easy operations 
#include <TFT_eSPI.h>       // Graphics display (TFT / OLED) and font library for ST7735 driver chip
#include <SPI.h>            // For display communication

// Init communication parameters
 char MQTT_SERVER[25]   = "broker.emqx.io"; // MQTT Server: SORBA Broker u other Public Brokers like "broker.hivemq.com";
 char MQTT_USERNAME[20] = "";               // MQTT User name (if needed)
 char MQTT_PASSWORD[20] = "";               // MQTT Password (if needed)
 uint16_t MQTT_PORT     = 1883;             // MQTT Port
 uint16_t MQTT_QoS      = 0;                // MQTT Quality of Service: 0: At Most Once ("Fire and Forget"),1: At Least Once (Acknowledged), 2: Exactly Once (Assured)

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

TFT_eSPI tft = TFT_eSPI();  // Invoke Display library, pins defined in User_Setup.h

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

void drawCheckMark(int x, int y, int size, uint16_t color) {
  // Draw two lines to form a check mark
  // First short downward stroke
  tft.drawLine(x, y + (size / 2), x + (size / 3), y + size, color);
  // Second longer upward stroke
  tft.drawLine(x + (size / 3), y + size, x + size, y, color);
}

void drawCheckMarkAt(int size, uint16_t color) {
  drawCheckMark(tft.getCursorX(), tft.getCursorY(), size, color);
}

// Show display info
void TFTdisplay(bool waitingAP, bool wifiStatus=false,bool mqttStatus=false, int32_t totalPack=0)
{
 
  tft.setRotation(1); // Set horizontal
  // Fill screen with color so we can see the effect of printing with and without 
  // a background colour defined otherwise it will overlapping with previous printing 
  tft.fillScreen(TFT_BLACK);

  //tft.drawRect(0, 0, tft.width(), tft.height(), TFT_PURPLE);
  
  // Set "cursor" at top left corner of display (0,0) and select font 2
  // (cursor will move to next line automatically during printing with 'tft.println'
  //  or stay on the line is there is room for the text with tft.print)
  tft.setCursor(0, 0);
  // Set the font colour to be white with a black background, set text size multiplier to 1
  tft.setTextColor(TFT_BLUE,TFT_BLACK);  tft.setTextSize(2);
  // We can now plot text on screen using the "print" class
  tft.print("SORBA Demo");

  if (waitingAP) {
    tft.setCursor(0, 20);
    tft.setTextColor(TFT_YELLOW); 
    tft.setTextFont(1);
    tft.println("Waiting for AP");
  }


 // Set the font colour to be yellow with no background
 // can call also setCursor(x,y, fontsize) same as setTExtFont(fontsize)
  tft.setCursor(0, 40);
  tft.setTextColor(TFT_YELLOW); 
  tft.setTextFont(1);
  tft.print("Wifi:");
  if (wifiStatus) {
    drawCheckMarkAt(14, TFT_GREEN); // Draw check mark
  }
  else {
    tft.setTextColor(TFT_RED);
    tft.println("X"); 
  }

  tft.setCursor(0, 65);
  tft.setTextColor(TFT_ORANGE); tft.setTextFont(1);
  tft.print("MQTT:");
  if (mqttStatus) {
    drawCheckMarkAt(14, TFT_GREEN); // Draw check mark
  }
  else {
    tft.setTextColor(TFT_RED);
    tft.println("X"); 
  }

  tft.setCursor(0, 90);
  tft.setTextColor(TFT_ORANGE); tft.setTextFont(1);
  tft.print("Packs:");
  tft.println(totalPack);

}


void setup() {
  // put your setup code here, to run once:
 // Setup Serial speed for monitoring 
 Serial.begin(115200);    // Set baudrate
 Serial.println("SORBA- sending Data Demo"); 

 // Initially will assuming it is waiting for Access Point
 tft.init(); // Init display driver
 TFTdisplay(true); 

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

 // Update display with Wifi connection status
 TFTdisplay(false, sorba.isConnectedWifi());
 
 // Connect to MQTT Broker with username & password
 // Future feature: including certificate for SSL connection and zlib library compression
 // MQTT_QoS: 0: At Most Once ("Fire and Forget"),1: At Least Once (Acknowledged), 2: Exactly Once (Assuredt
 // Regarding QoS, there are 3 ways to setup:
 // a) Can set it when calling connect, the QoS by default (0), this will be used always when calling sendMsg(topic) while publishing MQTT messages 
 // b) Set after connect the QoS by setQoS(mqtt_qos)
 // c) Calling sendMsg(topic, mqtt_qos) with QoS parameter instead of sendMsg(topic) for custom QoS
 
 sorba.connect(MQTT_SERVER, MQTT_PORT, MQTT_USERNAME, MQTT_PASSWORD, MQTT_QoS);

 // Show MQTT connection status
 if (sorba.isConnected())
  Serial.println("MQTT connection OK");
 else
  Serial.println("MQTT connection Failed");

 // Update display with MQTT connection status
 TFTdisplay(false, sorba.isConnected());

 // Subscribe to a topic to receive messages back
 sorba.subscribe(MQTT_TOPIC_SUB);

 // Setup timer used for sending and receiving frequency, e.g: 3 seconds = 3000 msec
 sorba.setTimer(3000);

 // Init sensors data
 initData();
}


void loop() {

   if (sorba.timerDone()) { // to control of sending data based on elapsed time defined in sorba.setTimer(3000);
 
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
   // Can get the total packages sent using .GetTotalPackSent() which is uint32_t 
   sorba.sendMsg(MQTT_TOPIC_PUB); 

   // Update Display
   TFTdisplay(false,sorba.isConnectedWifi(), sorba.isConnected(), sorba.GetTotalPackSent());

  // How to receive a message from SORBA
  // Check if there are messages in queue and parse it automatically. Later can use msgUnpack to extract the parameter value easily
  // Can get the total packages received using .GetTotalPackRecv() which is uint32_t 
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
    }// while end
  } // Sorba Timer is Done
}
