#ifndef SORMAMQTT_WIFI_H
#define SORMAMQTT_WIFI_H

// Class for Sending data to SORBA using MQTT using Wifi in boards like ESP32, ESP8266
// SORBOTICS
// Author: Reyan Valdes
// Date: 5/29/2025
// Version: 1.0
// email: reyanvaldes@yahoo.com
// https://github.com/reyanvaldes/SorbaMQTT-Wifi

// Foot print self and dependecies
// Program Storage: 703 KB
// Dynamic Memory: 40 KB 

// Libraries or dependencies required
// Library for Wifi use different header for ESP32 and ESP8266 but the methods are same
#if defined (ESP8266)
 #include <ESP8266Wifi.h>
#else
 #include <WiFi.h>          // Wifi (V1.2.7)                  https://docs.arduino.cc/libraries/wifi/
#endif

//#include <WiFi.h>          // Wifi (V1.2.7)                  https://docs.arduino.cc/libraries/wifi/
#include <PubSubClient.h>  // for MQTT Messages (V2.8.0)     https://github.com/knolleary/pubsubclient
#include <ArduinoJson.h>   // For JSON doc handling (V7.3.1) https://arduinojson.org/?utm_source=meta&utm_medium=library.properties
#include <UUID.h>          // for UUID generator (V0.1.6)    https://github.com/RobTillaart/UUID
#include <ArduinoQueue.h>  // for Queue operations (V1.2.5)  https://github.com/EinarArnason/ArduinoQueue

// Global constant definitions for memory size- will impact Global variables %
#define KB 1024               // Just 1024 for 1 KB
#define MQTT_JSON_LIMIT  2 * KB  // for JSON doc size  
#define WIFI_SSID_LIMIT  30  // Limit for SSID char []
#define WIFI_PWD_LIMIT   25  // limit for Pwd char []
#define MQTT_SERVER_LIMIT 100 // limit for MQTT Server Url []
#define MQTT_USER_LIMIT  25  // Limit for MQTT User name []
#define MQTT_PWD_LIMIT   25  // Limit for MQTT Password []
#define MQTT_CLIENTID_LIMIT 40 // Limit for MQTT Client ID (Unique ID) Must has enough room to store the UUID, otherwise coud affect the copy
#define MQTT_QUEUE_LIMIT 20  // Limit for MQTT Queue messages receiving from callback

typedef void (*callbackMQTT) (char* topic, byte* payload, unsigned int length);

// Global objects created at starting
extern WiFiClient wifiClient; // for Wifi Client

extern PubSubClient client; // Simple MQTT client

// For JSON smaller than 1KB use StaticJsonDocument, for larger than 1 KB use DynamicJsonDocument
extern DynamicJsonDocument _jsDoc; // Working with JSON doc for both sending MQTT messages or subscribing

extern void defCallback(char* topic, byte* payload, unsigned int length);  // Calling back for subscription

// Struct for ArduinoQueue messages (When subscribing)
struct tSubMsg {
  String topic;
  String payload;
};

extern ArduinoQueue<tSubMsg> subMsgQueue; // Queue to receive subscription messages


//SORBA class definition 
class SorbaMqttWifi
{
  public:
  SorbaMqttWifi (); // Constructor
 
  bool connect(char mqtt_Server[], uint16_t mqtt_Port, char userName[]="", char password[]="");   // Set the parameters and connext to MQTT Broker   
   
  bool connect();    // MQTT broker connection 

  int state();  // Return the MQTT broker connection state
  
  void disconnect();  // Disconnect from MQTT broker
   
  bool isConnected();  // Check if it is connected to MQTT broker

  void checkConnection(); // Check connection to the MQTT broker
  
  bool reconnect(); // Reconnect to the MQTT Broker

  void showState();  // Show in Serial the MQTT connection state
   
  void setKeepAlive(uint16_t time) {mqttKeepAlive= time;}   // Set the Keep Alive for MQTT 
   
  void setSocketTimeout(uint16_t time) {mqttSocketTimeout = time;}     // Set the Socket Timeout for MQTT 
   
  void setRetry(uint16_t totalRetry) {retryLimit = totalRetry;}  // Set the MQTT connection retry
   
  void setQoS(uint16_t qos) {mqttQoS = qos;}   // Set the Quality of Service for MQTT 

  // Wifi methods
   bool connectWifi(char wifi_ssid[], char wifi_pwd[]);  // Set the parameters and connect to the Wifi
   
   bool connectWifi();   // Connect to the Wifi
   
   void disconnectWifi(); // Disconnect from the Wifi
   
   bool reconnectWifi();  // Reconnect to the Wifi
 
   bool isConnectedWifi();  // Indicate if it is connected to Wifi

   void checkConnectionWifi(); // Check connection to the Wifi
   
   uint16_t scanWifiNetwork();  // Scan all SSID available from Wifi and show in the Serial port

  
   WiFiClient* getWifiClient() {return &wifiClient;};   // Get the wifi client object

   float roundToDec( float in_value, uint16_t decimal_place=2);

   double roundToDec( double in_value, uint16_t decimal_place=2);

   DynamicJsonDocument jsMsg() { // Return JSON doc used internally for sending or receiving MQTT to allow application work directely with it
    return _jsDoc;
   }
   
   void msgInit() { // Clear JSON message to be ready for set new JSON object
    _jsDoc.clear();
   }

    void msgPack(char group[], char param[], bool value) { // Setup the Msg parameter for bool values
      _jsDoc[group][param] = value;
   }

    void msgPack(char group[], char param[], signed char value) { // Setup the Msg parameter for signed char values
      _jsDoc[group][param] = value;
   }

    void msgPack(char group[], char param[], unsigned char value) { // Setup the Msg parameter for signed char values
      _jsDoc[group][param] = value;
   }
   
   void msgPack(char group[], char param[], int value) { // Setup the Msg parameter for int values
      _jsDoc[group][param] = value;
   }

   void msgPack(char group[], char param[], unsigned int value) { // Setup the Msg parameter for unsigned int values
      _jsDoc[group][param] = value;
   }

   void msgPack(char group[], char param[], signed short value) { // Setup the Msg parameter for short values
      _jsDoc[group][param] = value;
   }

   void msgPack(char group[], char param[], unsigned short value) { // Setup the Msg parameter for short values
      _jsDoc[group][param] = value;
   }

   void msgPack(char group[], char param[], long int value) { // Setup the Msg parameter for long int values
      _jsDoc[group][param] = value;
   }

   void msgPack(char group[], char param[], unsigned long int value) { // Setup the Msg parameter for long int values
      _jsDoc[group][param] = value;
   }

   void msgPack(char group[], char param[], signed long long value) { // Setup the Msg parameter for long long int values
      _jsDoc[group][param] = value;
   }

   void msgPack(char group[], char param[], unsigned long long value) { // Setup the Msg parameter for long long int values
      _jsDoc[group][param] = value;
   }

   void msgPack(char group[], char param[], float value, uint16_t dec=0) { // Setup the Msg parameter for float values with decimal places
    if (dec==0) { // use the default settings for decimals used for floating values
      dec = mqttFloatDecimals;
    }
      _jsDoc[group][param] = roundToDec(value, dec);
   }

   void msgPack(char group[], char param[], double value, uint16_t dec=0) { // Setup the Msg parameter for float values with decimal places
    if (dec==0) { // use the default settings for decimals used for floating values
      dec = mqttFloatDecimals;
    }
      _jsDoc[group][param] = roundToDec(value, dec);
   }

   void msgPack(char group[], char param[], char value[]) { // Setup the Msg parameter for chars
      _jsDoc[group][param] = value;
   }

   void msgPack(char group[], char param[], String value) { // Setup the Msg parameter for string
      _jsDoc[group][param] = value;
   }
   /*  // TO DO: Including array 
   void msgPack(char group[], char param[], float arr[]) { // Setup the Msg parameter for chars
	 uint32_t total = sizeof(arr)/sizeof(arr[0]);
	 JsonArray values = _jsonDoc.to<JsonArray>();
	 for (uint32_t i=0; i<total; i++)
      values.add(arr[i]);
   }
   
   void msgPack(char group[], char param[], double arr[]) { // Setup the Msg parameter for chars
     JsonArray values = _jsonDoc.to<JsonArray>();
	 for (double value :arr)
      values.add(value);
   }
   
   void msgPack(char group[], char param[], int arr[]) { // Setup the Msg parameter for chars
      JsonArray values = _jsonDoc.to<JsonArray>();
	 for (int value :arr)
      values.add(value);
   }
   */
   void msgToChar() {
    serializeJson (_jsDoc, mqttMsg); // convert from JSON doc to char array, make sure the array has enough buffer
   }

   bool sendMsg(char topic[]); // Send the message, need to call first msgInit and msgPack

   bool recvMsg(String &topic, String &payload ); // Receive message from Subscribing

   bool recvMsg(String &topic); // Receive message from Subscribing and parse the JSON
   
   bool parseMsg(String msg); // Parse the JSON from string, after can extract parameter values using msgUnpack

   void msgUnpack(char group[], char param[], bool &value) { // Setup the Msg parameter for bool values
     if (strlen(group) ==0) // check if there is no group
      value = _jsDoc[param];
     else
      value = _jsDoc[group][param];
   }

   void msgUnpack(char group[], char param[], signed char &value) { // Setup the Msg parameter for char values
   if (strlen(group) ==0) // check if there is no group
      value = _jsDoc[param];
     else
      value = _jsDoc[group][param];
   }

   void msgUnpack(char group[], char param[], unsigned char &value) { // Setup the Msg parameter for char values
    if (strlen(group) ==0) // check if there is no group
      value = _jsDoc[param];
     else
      value = _jsDoc[group][param];
   }
   
   void msgUnpack(char group[], char param[], int &value) { // Setup the Msg parameter for int values
   if (strlen(group) ==0) // check if there is no group
      value = _jsDoc[param];
     else
      value = _jsDoc[group][param];
   }

   void msgUnpack(char group[], char param[], unsigned int &value) { // Setup the Msg parameter for int values
    if (strlen(group) ==0) // check if there is no group
      value = _jsDoc[param];
     else
      value = _jsDoc[group][param];
   }

   void msgUnpack(char group[], char param[], signed short &value) { // Setup the Msg parameter for short values
    if (strlen(group) ==0) // check if there is no group
      value = _jsDoc[param];
     else
      value = _jsDoc[group][param];
   }

   void msgUnpack(char group[], char param[], unsigned short &value) { // Setup the Msg parameter for short values
    if (strlen(group) ==0) // check if there is no group
      value = _jsDoc[param];
     else
      value = _jsDoc[group][param];
   }

   void msgUnpack(char group[], char param[], long int &value) { // Setup the Msg parameter for long int values
    if (strlen(group) ==0) // check if there is no group
      value = _jsDoc[param];
     else
      value = _jsDoc[group][param];
   }

   void msgUnpack(char group[], char param[], unsigned long int &value) { // Setup the Msg parameter for long int values
    if (strlen(group) ==0) // check if there is no group
      value = _jsDoc[param];
     else
      value = _jsDoc[group][param];
   }

   void msgUnpack(char group[], char param[], long long &value) { // Setup the Msg parameter for long long values
    if (strlen(group) ==0) // check if there is no group
      value = _jsDoc[param];
     else
      value = _jsDoc[group][param];
   }

    void msgUnpack(char group[], char param[], unsigned long long &value) { // Setup the Msg parameter for long long values
      if (strlen(group) ==0) // check if there is no group
      value = _jsDoc[param];
     else
      value = _jsDoc[group][param];
   }

   void msgUnpack(char group[], char param[], float &value) { // Setup the Msg parameter for float values with decimal places
    if (strlen(group) ==0) // check if there is no group
      value = _jsDoc[param];
     else
      value = _jsDoc[group][param];
   }

   void msgUnpack(char group[], char param[], double &value) { // Setup the Msg parameter for double values
    if (strlen(group) ==0) // check if there is no group
      value = _jsDoc[param];
     else
      value = _jsDoc[group][param];
   }

   void msgUnpack(char group[], char param[], double &value, int dec=0) { // Setup the Msg parameter for double values with decimals
    if (strlen(group) ==0) // check if there is no group
      value = _jsDoc[param];
     else
      value = _jsDoc[group][param];
      
    value = roundToDec(value, dec);
   }

   void msgUnpack(char group[], char param[], char *value) { // Setup the Msg parameter for char values with chars
    if (strlen(group) ==0) // check if there is no group
     strcpy(value, _jsDoc[param].as<const char*>());
    else
     strcpy(value, _jsDoc[group][param].as<const char*>());
      
   }

   void msgUnpack(char group[], char param[], String &value) { // Setup the Msg parameter for String values
    if (strlen(group) ==0) // check if there is no group
      value = _jsDoc[param].as<String>();
     else
      value = _jsDoc[group][param].as<String>();
   }

   void subscribe(char topic[]) { // Subscribe to a topic
    client.subscribe(topic);
   }

   void setCallback(callbackMQTT acallback) {
    callback = acallback;
   }
 
   void startTimer() {
    startTime = millis();
   }

   bool timerDone() {
    unsigned long nowTime = millis();
    if (nowTime - startTime >= timems) {
      startTimer();
      return true;
    }

    return false;
   }

   void setTimer(unsigned long atime) {// Set timer in milliseconds
      timems = atime;
   }

   void setFloatDecimals( uint16_t decimals) {
     mqttFloatDecimals = decimals;
   }

  private:
  // Attributes
   unsigned long startTime; // For checking elapsed time
   unsigned long timems = 5000;  // Time in ms for checking the timer
   
   uint16_t retryLimit =3; // retry for reconnection and sending
   char     mqttClientID[MQTT_CLIENTID_LIMIT];
     
   char     mqttServer[MQTT_SERVER_LIMIT];
   uint16_t mqttPort;
   char     mqttUserName[MQTT_USER_LIMIT]="";
   char     mqttPassword[MQTT_PWD_LIMIT]="";
   uint16_t mqttQoS=0;
   uint16_t mqttKeepAlive =30;
   uint16_t mqttSocketTimeout = 60;
   uint16_t mqttFloatDecimals = 2;

   // Used for callback when subscribing to MQTT messages
   callbackMQTT callback = NULL; 
   
   // Use for serialize the JSON into char*
   char     mqttMsg[MQTT_JSON_LIMIT]; // Limit the MQTT MSG Limit, this is to avoid Strings to minimize the fragmentation of the heap memory if we were using String class

   char wifiSSID[WIFI_SSID_LIMIT];
   char wifiPwd[WIFI_PWD_LIMIT];

   
};  // SorbaMqttWifi Class end definition

#endif
