/*
  Used Libs

  ----------

  Download MQTT Client from
  https://github.com/knolleary/pubsubclient

  Download ModbusMaster by Doc Walker

  Board: Generic ESP8266 Module
  Flash Mode: DIO
  Cristal Freq:: 26 MHz
  Flash Freq: 40 MHz
  Upload Using: Serial
  CPU Freq: 80 MHz
  Flash Size: 4 MB (FS:none OTA~1019KB)
  UploadSpeed: 115200

  Thanks to Jethro Kairys
  https://github.com/jkairys/growatt-esp8266

  Thanks to Otti
  https://github.com/otti/Growatt_ShineWiFi-S

*/
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiManager.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ArduinoJson.h>

#include <PubSubClient.h>
#include "Growatt.h"

#include "index.h"

#include <stdio.h>
#include <stdlib.h>

#define PROJECT_NAME "GROWATT"                              // Access-Point name, in config-mode / mDNS name / Homepage title

#define LED_GREEN 0  // GPIO0
#define LED_RED   2  // GPIO2
#define LED_BLUE  16 // GPIO16

#define BUTTON    0  // GPIO0

const char* update_path           = "/firmware";
const char* update_username       = "username";
const char* update_password       = "password";

const String mqttNode             = "growatt";              // Your unique name for this device
const String mqttDiscoveryPrefix  = "homeassistant";        // Home Assistant MQTT Discovery, see https://home-assistant.io/docs/mqtt/discovery/

int     sensorNumber              = 1;
String  mqttName                  = "inverter_" + String(sensorNumber);
String  stateTopic                = mqttNode"+/+"mqttName;

char    mqttPayload[256]          = "{\"status\": \"disconnected\" }";

char*   mqtt_server               = "mqtt_server";          // needs to be set by settings page
int     mqtt_port                 = 1883;                   // needs to be set by settings page
char*   mqtt_username             = "mqtt_username";        // needs to be set by settings page
char*   mqtt_password             = "mqtt_password";        // needs to be set by settings page

long    inverter_read = 10000;

WiFiClient              espClient;
PubSubClient            mqttClient(espClient);
Growatt                 Inverter;
ESP8266WebServer        httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;


// -------------------------------------------------------
// Setup WiFi and (re)connect
// -------------------------------------------------------
void WiFi_Setup() {
  digitalWrite(LED_GREEN, 0);
  WiFi.mode(WIFI_STA);
  WiFiManager wifiManager; // Connect to Wi-Fi
  wifiManager.setAPCallback(configModeCallback);
  wifiManager.setTimeout(180);
  if (!wifiManager.autoConnect(PROJECT_NAME)) {
    delay(1000);
    ESP.restart(); //reset and try again
  }
}
//gets called when WiFiManager enters configuration mode
long led01S = 0;
void configModeCallback (WiFiManager *myWiFiManager) {
  // Toggle red LED when no connection
  long now = millis();
  if (now - led01S > 500) {
      digitalWrite(LED_RED, !digitalRead(LED_RED));
  }
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());
}

// -------------------------------------------------------
// Check the MQTT status and reconnect if necessary
// -------------------------------------------------------
void mqttConnect()
{
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    if (WiFi.status() != WL_CONNECTED) {
      break;
    }
    Serial.print("Attempting MQTT connection...");

    // Attempt to connect with last will
    DynamicJsonDocument doc(1024);
    char buffer[256];
    doc["status"] = "disconnected";
    size_t n = serializeJson(doc, buffer);
    sprintf(mqttPayload, buffer);

    if (mqttClient.connect(mqttNode.c_str(), mqtt_username, mqtt_password, stateTopic.c_str(), 1, 1, buffer)) { //

      sendMQTT_Power_DiscoveryMsg();
      sendMQTT_EnergyToday_DiscoveryMsg();
      sendMQTT_EnergyTotal_DiscoveryMsg();
      Serial.println("connected");

    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

// -------------------------------------------------------
// Setup
// -------------------------------------------------------
void setup() {

  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);

  httpServer.on("/", handleRoot);
  httpServer.on("/status", handleSendJSON);
  httpServer.onNotFound(handleNotFound);

  WiFi_Setup();

  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttClient.setBufferSize(1024);

  mqttConnect();

  if (!MDNS.begin(PROJECT_NAME, WiFi.localIP())) {
    Serial.println("Error setting up mDNS responder!");
  }
  Serial.println("mDNS responder started");

  Inverter.begin(Serial);

  httpUpdater.setup(&httpServer, update_path, update_username, update_password);
  httpServer.begin();
}

uint16_t u16PacketCnt = 0;

bool testMQTT = false;

// -------------------------------------------------------
// Main loop
// -------------------------------------------------------
long timer01S = 0;
long timer10S = 0;

void loop() {

  MDNS.update();

  long now = millis();

  //WiFi_Reconnect();

  // check MQTT connection
  if (!mqttClient.connected())
  {
    mqttConnect();
  }

  // MQTT client loop
  if (mqttClient.connected())
  {
    mqttClient.loop();
  }

  httpServer.handleClient();

  // Toggle green LED with 1 Hz (alive)
  // ------------------------------------------------------------
  if (now - timer01S > 500) {
    if (WiFi.status() == WL_CONNECTED) {
      digitalWrite(LED_RED, 0);
      digitalWrite(LED_GREEN, !digitalRead(LED_GREEN));
    } else {
      digitalWrite(LED_GREEN, 0);
    }
    timer01S = now;
  }

  // Read Inverter every X s
  // ------------------------------------------------------------
  if (now - timer10S > inverter_read) {
    if (mqttClient.connected() && (WiFi.status() == WL_CONNECTED)) {
      DynamicJsonDocument doc(1024);
      char buffer[256];
      if (!testMQTT) {
        if (Inverter.UpdateData()) { // get new data from inverter
          u16PacketCnt++;
          switch (Inverter.GetStatus())
          {
            case GwStatusWaiting:
              doc["status"] = "waiting";
              break;
            case GwStatusNormal:
              doc["status"] = "normal";
              break;
            case GwStatusFault:
              doc["status"] = "fault";
              break; 
          }

          doc["dc_voltage"]      = Inverter.GetDcVoltage();
          doc["ac_freq"]         = Inverter.GetAcFrequency();
          doc["ac_voltage"]      = Inverter.GetAcVoltage();
          doc["ac_power"]        = Inverter.GetAcPower();
          doc["energy_today"]    = Inverter.GetEnergyToday();
          doc["energy_total"]    = Inverter.GetEnergyTotal();
          doc["operatingtime"]   = Inverter.GetOperatingTime();
          doc["temperature"]     = Inverter.GetInverterTemperature();
          doc["count"]           = u16PacketCnt;
        }
        size_t n = serializeJson(doc, buffer);
        digitalWrite(LED_BLUE, 0); // clear blue led if everything is ok
      } else if (testMQTT) {
        u16PacketCnt++;
        doc["ac_power"]        = 75;
        doc["energy_today"]    = 10;
        doc["energy_total"]    = 5000;
        doc["count"]           = u16PacketCnt;
        size_t n = serializeJson(doc, buffer);
        digitalWrite(LED_BLUE, 0); // clear blue led if everything is ok
      } else {
        doc["status"] = "disconnected";
        size_t n = serializeJson(doc, buffer);
        digitalWrite(LED_BLUE, 1); // set blue led in case of error
      }
      
   
      if (buffer[0] != '\0') {
        sprintf(mqttPayload, buffer);
        bool published = mqttClient.publish(stateTopic.c_str(), buffer, true);
      }

    }
    timer10S = now;
  }
}
