#include "sorbamqtt_wifi.h"

// Class for Sending data to SORBA using MQTT using Wifi in boards like ESP32, ESP8266
// SORBOTICS
// Author: Reyan V.
// Date: 5/29/2025
// Version: 1.0.1
// email: reyanvaldes@yahoo.com
// https://github.com/reyanvaldes/SorbaMQTT-Wifi 

// Foot print self and dependecies
// Program Storage: 703 KB
// Dynamic Memory: 40 KB 

// Dependencies
// Libraries or dependencies required for sorbamqtt_wifi
//#include <WiFi.h>          // Wifi (V1.2.7)                  https://docs.arduino.cc/libraries/wifi/
//#include <ESP8266Wifi.h>   // Wifi for ESP8266
//#include <PubSubClient.h>  // for MQTT Messages (V2.8.0)     https://github.com/knolleary/pubsubclient
//#include <ArduinoJson.h>   // For JSON doc handling (V7.3.1) https://arduinojson.org/?utm_source=meta&utm_medium=library.properties
//#include <UUID.h>          // for UUID generator (V0.1.6)    https://github.com/RobTillaart/UUID
//#include <ArduinoQueue.h>  // for Queue operations (V1.2.5)  https://github.com/EinarArnason/ArduinoQueue

// Global variables

// WiFiClient wifiClient; // for Wifi Client

// PubSubClient client(wifiClient); // Simple MQTT client

DynamicJsonDocument _jsDoc (MQTT_JSON_LIMIT); // Working with JSON doc for both sending MQTT messages or subscribing

ArduinoQueue <tSubMsg> subMsgQueue (MQTT_QUEUE_LIMIT); // Queue to receive subscription messages

void defCallback(char* topic, byte* payload, unsigned int length) {  // Calling back for subscription

   Serial.print("Message arrived topic: ");
   Serial.print(topic);
   Serial.print(", Payload: ");

   String msgTopic (topic);
   String msgPayload;
 
   for (int i = 0; i < length; i++) {
    msgPayload += (char)payload[i];
   } // for
   
   Serial.print(msgPayload);
   Serial.print(", Len: "); Serial.println(length);

   // Setup msg to be ready insert in a queue
   tSubMsg msg;
   msg.topic = msgTopic;
   msg.payload = msgPayload;

   // insert msg into the queue to be consumed by the application any time
   subMsgQueue.enqueue(msg);   
  } // callback

//********************************************************************************

SorbaMqttWifi::SorbaMqttWifi (Client& awifiClient) : client(awifiClient)
{// constructor
    setCallback(defCallback); // Set default callback for MQTT subscribing msg
}

//********************************************************************************
// setup parameters and connect to the MQTT Broker 
bool SorbaMqttWifi::connect(char mqtt_Server[], uint16_t mqtt_Port, char userName[], char password[], uint16_t mqtt_qos){
    // transfer to get reconnection again based on this parameters
    strncpy(mqttServer, mqtt_Server, sizeof(mqttServer)); 
    mqttPort =  mqtt_Port;
	mqttQoS  =  mqtt_qos; // MQTT Quality of Service: 0: At Most Once ("Fire and Forget"),1: At Least Once (Acknowledged), 2: Exactly Once (Assured) 
    strncpy(mqttUserName, userName, sizeof(mqttUserName));
    strncpy(mqttPassword, password, sizeof(mqttPassword));

    UUID     uuid;  // create the instance for UUID
    strncpy(mqttClientID, uuid.toCharArray(), sizeof(mqttClientID)); // It is important to copy validating its max limit to avoid overflow
    
    return connect();
}


//********************************************************************************
// Connect to the MQTT broker
bool SorbaMqttWifi::connect() {
    Serial.println("MQTT try to connect"); 
    Serial.print("MQTT Server: "); Serial.print(mqttServer); 
    Serial.print(" MQTT port: "); Serial.println(mqttPort); 
    Serial.print("MQTT Client ID: "); Serial.print(mqttClientID); 
    Serial.print(" MQTT User: "); Serial.println(mqttUserName); 
    client.setServer(mqttServer, mqttPort);
    client.setKeepAlive(mqttKeepAlive);
    client.setSocketTimeout(mqttSocketTimeout);
    client.setCallback(callback);
    
    uint16_t count =0;
    
    while (!isConnected() && (count <retryLimit)) {
  // (mqttClientID, mqttUserName, mqttPassword)
      if (client.connect(mqttClientID, mqttUserName, mqttPassword)) {// This has to be unique otherwise has conflict with other client and could make connection lost
        showState(); 
        startTimer(); // for timer control
        return true;
      }
      else {
       showState();
       Serial.println(" try again in short time");
       // Wait few ms before retrying
       delay(500); 
      }
      
      count += 1;
     }// retry while

     return false;
}

//********************************************************************************
// Return the MQTT state
int SorbaMqttWifi::state() {
    return client.state();
}

//********************************************************************************
// Disconnect from MQTT broker
void SorbaMqttWifi::disconnect() {
    Serial.println("MQTT disconnecting"); 
    client.disconnect();
   };
   
//********************************************************************************
// Is it connected to the Broker?
 bool SorbaMqttWifi::SorbaMqttWifi::isConnected() {
  return client.connected();
}

//********************************************************************************
// Check connection to the MQTT Broker, if there is a problem it will try to reconnect
void SorbaMqttWifi::checkConnection() { // Check connection to the MQTT broker
    if (!isConnected())
     connect();
}


//********************************************************************************
// Reconnect doing disconnection and connection
bool SorbaMqttWifi::reconnect() {
     Serial.println("MQTT reconnecting");
     disconnect(); 
     delay(100);
     return connect();
}
   
//********************************************************************************
// Show MQTT state
void SorbaMqttWifi::showState() {
    Serial.print("MQTT State=");
    Serial.print(state());
    Serial.print(" Text: ");
    switch (client.state()) {
    case MQTT_CONNECTED:
      // Client is connected
      Serial.println("MQTT client connected");
      break;
    case MQTT_CONNECT_BAD_CREDENTIALS:
      Serial.println("MQTT bad user name or password");
      break;
    case MQTT_CONNECT_UNAVAILABLE:
      Serial.println("MQTT server unavailable");
      break;  
    case MQTT_CONNECT_BAD_CLIENT_ID:
      Serial.println("MQTT Bad Client ID");
      break;  
    case MQTT_CONNECT_BAD_PROTOCOL:
      Serial.println("MQTT Bad Protocol");
      break;      
    case MQTT_CONNECTION_TIMEOUT:
      // Connection timed out
      Serial.println("MQTT connection timeout");
      break;
    case MQTT_CONNECTION_LOST:
      // Connection lost
      Serial.println("MQTT connection lost");
      break;
    case MQTT_CONNECT_FAILED:
      // Connection failed
      Serial.println("MQTT connection failed");
      break;
    case MQTT_DISCONNECTED:
      // Client is disconnected
      Serial.println("MQTT client disconnected");
      break;
    case MQTT_CONNECT_UNAUTHORIZED:
       // Client is not authorized for connection
       Serial.println("MQTT client not authorized to connect");
      break;
    default:
      // Unknown state
      Serial.println("Unknown MQTT client state");
      break;
    } // switch
  }

 //********************************************************************************
 // Set the parameters as array of chars and connect to the Wifi
 bool SorbaMqttWifi::connectWifi(char wifi_ssid[], char wifi_pwd[]) {
      WiFi.mode(WIFI_STA); // Acting as Station Only
      strcpy(wifiSSID, wifi_ssid);  // transfer to get reconnection again
      strcpy(wifiPwd, wifi_pwd); 
      return connectWifi();
}

// Set the parameters as String and connect to the Wifi
bool SorbaMqttWifi::connectWifi(String wifi_ssid, String wifi_pwd)
{
   WiFi.mode(WIFI_STA); // Acting as Station Only
   wifi_ssid.toCharArray(wifiSSID, sizeof(wifiSSID));  // transfer to get reconnection again
   wifi_pwd.toCharArray(wifiPwd, sizeof(wifiPwd));
   return connectWifi();
}

//********************************************************************************
// Connect to Wifi with retry loop
bool SorbaMqttWifi::connectWifi() {
    // perform connection
      WiFi.begin(wifiSSID, wifiPwd);
      Serial.print("Connecting to WiFi => "); Serial.print(wifiSSID);
      while (WiFi.status() != WL_CONNECTED) {
       Serial.print('.');
       delay(1000);
      }
      Serial.println("");
      Serial.println("WiFi connected");
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());
      Serial.print("MAC address: ");
      Serial.println(WiFi.macAddress()); 

      return true;
   }

 //********************************************************************************
 // Disconnect from Wifi
   void SorbaMqttWifi::disconnectWifi() {
    WiFi.disconnect(); 
    Serial.println("WiFi disconnecting"); 
   }
   
//********************************************************************************
// Reconnecting to the Wifi
   bool SorbaMqttWifi::reconnectWifi() {
     Serial.println("WiFi reconnecting");
     disconnectWifi(); 
     delay(100);
     return connectWifi();
   }

//********************************************************************************
// Check if it is connected to the Wifi
   bool SorbaMqttWifi::isConnectedWifi() {
    return WiFi.isConnected();
   }

//********************************************************************************
// Verify if it is connected to Wifi, if not it will try to connect with retry loop
   void SorbaMqttWifi::checkConnectionWifi() { // Check connection to the Wifi
    if (!isConnectedWifi())
     connectWifi();
   }
//********************************************************************************
// Scan Wifi Network
   uint16_t SorbaMqttWifi::scanWifiNetwork() {
    // WiFi.scanNetworks will return the number of networks found
    int n = WiFi.scanNetworks();
    Serial.println("Scan Wifi done");
    if (n == 0) {
      Serial.println("no Wifi networks found");
    } else {
    Serial.print(n);
    Serial.println(" Wifi networks found. *-Secure");
    for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      #if defined (ESP8266)
       Serial.println((WiFi.encryptionType(i) == AUTH_OPEN)?" ":"*");
       delay(10);
      #else 
       Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
       delay(10);
     #endif
    }
   }
   
   return n;
  }
 
//********************************************************************************

float SorbaMqttWifi::roundToDec( float in_value, uint16_t decimal_place)
 {
     float multiplier = powf( 10.0f, decimal_place );
      in_value = roundf( in_value * multiplier ) / multiplier;
      return in_value;
}

//********************************************************************************

double SorbaMqttWifi::roundToDec( double in_value, uint16_t decimal_place)
{
     double multiplier = pow( 10.0f, decimal_place );
      in_value = round( in_value * multiplier ) / multiplier;
      return in_value;
}
    
//********************************************************************************
// Send MQTT message, the payload should be a valid JSON
bool SorbaMqttWifi::sendMsg(char topic[]){ // Send the message, need to call first msgInit and msgPack
    
    checkConnectionWifi(); // Check Wifi Connection, if there is a problem, will reconnect
    
    checkConnection(); // Check MQTT connection, if there is a problem, will reconnect
   
    if (isConnected()) { // If it is connected to MQTT Broker, send the message
      
      client.loop(); // take the change and process the callback when subscribing
      
      msgToChar(); // Serializing the JSON, convert JSON msg to char [] 
      
      bool result =  client.publish( topic, mqttMsg, mqttQoS);  // Publish the MQTT message based on the QoS defined: 0: At Most Once ("Fire and Forget"),1: At Least Once (Acknowledged), 2: Exactly Once (Assured)
      if (result)
      {
	   totalPackSent ++;  // Increment total packages sent
       Serial.println("Sent Data to MQTT"); // print serial message if the message was sent to the server
      }
     return result;
    }
    
    return false;
   }

//********************************************************************************
// Send the message with custom QoS , need to call first msgInit and msgPack
bool SorbaMqttWifi::sendMsg(char topic[], uint16_t mqtt_qos)
{
  uint16_t backupQoS = mqttQoS; // Backup existing QoS, so it can be restored after sending
  
  mqttQoS = mqtt_qos; // Set the new QoS for this sending operation: 0: At Most Once ("Fire and Forget"),1: At Least Once (Acknowledged), 2: Exactly Once (Assured) 
  
  bool result = sendMsg(topic); // Publish
  
  mqttQoS = backupQoS; // Restore
  
  return result;
}

//********************************************************************************
// Receive message from Subscribing
bool SorbaMqttWifi::recvMsg(String &topic, String &payload ) { 

    // reset both variables 
    topic.clear();
    payload.clear();
    
    client.loop(); // take the change and process the callback when subscribing

    if (!subMsgQueue.isEmpty())
    {
      tSubMsg msg = subMsgQueue.dequeue();  // extract the msg from queue
      topic = msg.topic;
      payload = msg.payload;
	  
	  totalPackRecv ++; // Increment Total Packages Received

      return true; // indicating there is a message read
    }

    return false; // nothing to read
   }

//********************************************************************************
// Receive message from Subscribing and parse the JSON the output can get from JSON
bool SorbaMqttWifi::recvMsg(String &topic) { 

    // reset both variables 
    topic.clear();
    _jsDoc.clear();
    
    client.loop(); // take the change and process the callback when subscribing

    if (!subMsgQueue.isEmpty())
    {
      tSubMsg msg = subMsgQueue.dequeue();  // extract the msg from queue
      topic = msg.topic;
      DeserializationError error = deserializeJson(_jsDoc, msg.payload);

      if (error) {
        Serial.print("deserializeJson() failed: "); Serial.println(error.c_str());
        topic.clear();
        _jsDoc.clear();
        return false;
      }

      return true; // indicating there is a message read and there is no error
    }

    return false; // nothing to read
   }
   
//********************************************************************************
// Parse the JSON from string, after can extract parameter values using msgUnpack
bool SorbaMqttWifi::parseMsg(String msg) { 
  
   DeserializationError error = deserializeJson(_jsDoc, msg);
   
   if (error) {
	Serial.print("deserializeJson() failed: "); Serial.println(error.c_str());
	return false;
   }
   
   return true;
}

//********************************************************************************

