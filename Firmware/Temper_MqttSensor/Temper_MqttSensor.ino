/*
  Temper ESP8266
  MQTT firmware

  For more information and help, head over to Temper's github repository:
  https://github.com/mcer12/Temper-ESP8266

  3D printed case:
  ---THINGIVERSE LINK---

  ***

  Copyright (c) 2019 Martin Cerny

  GNU General Public License v3.0

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <FS.h>
#include <ArduinoJson.h>

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>

#include <Ticker.h>
#include <ShiftRegister74HC595.h>
#include <Wire.h>

#define AP_NAME "TEMPER_" // Last 6 MAC address characters will be appended at the end of the OTA name
#define OTA_NAME "TEMPER_" // Last 6 MAC address characters will be appended at the end of the OTA name
#define FW_VERSION "1.0"
#define OTA_BLINK_SPEED 100
#define OTA_TIMEOUT 300000 // 5 minutes
#define CONFIG_BLINK_SPEED 500
#define CONFIG_TIMEOUT 300000 // 5 minutes
#define DISPLAY_DARK_TIME 1 // millis, smaller number = brighter display
#define ADDRESS 0x44 // SHT30 I2C address

// DO NOT CHANGE DEFINES BELOW
#define NORMAL_MODE 0
#define OTA_MODE 1
#define CONFIG_MODE 2
#define CONFIG_MODE_LOCAL 3
#define HASS_REGISTER_MODE 4
#if MQTT_MAX_PACKET_SIZE < 512  // If the max message size is too small, throw an error at compile time. See PubSubClient.cpp line 359
#error "MQTT_MAX_PACKET_SIZE is too small in libraries/PubSubClient/src/PubSubClient.h, increase it to 512"
#endif

uint8_t cols[] = {
  22, // 1
  21, // 2
  20, // 3
  18, // 4
  17, // 5
  15, // 6
  10, // 7
  14, // 8
  13, // 9
  9, // 10
  11, // 11
  12, // 12
  19, // 13
};
uint8_t rows[] = {
  7, //
  6, //
  5, //
  4, //
  3, //
  2, //
  1, //
};

uint8_t content[7][13] = {
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,},
};
uint8_t buffer[7][13] = {
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,},
};
uint8_t ota_screen[7][13] = {
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,},
  {0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 0, 0,},
  {0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0,},
  {0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0,},
  {0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0,},
  {0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0,},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,},
};
uint8_t config_screen[7][13] = {
  {0, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0,},
  {0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0,},
  {0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0,},
  {0, 0, 1, 1, 1, 1, 0, 1, 1, 1, 0, 0, 0,},
  {0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0,},
  {0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0,},
  {0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0,},
};

uint8_t deviceMode = NORMAL_MODE;

bool otaModeStarted = false;
volatile bool ledState = false;
int buttonTreshold = 2000;
unsigned long sleepTime = 3600000000; // 1h by default, configurable in config portal

// TIMERS
unsigned long otaMillis, sleepMillis, ledMillis, configTimer, otaTimer;

int counter;
byte mac[6];
rst_info *rinfo;
unsigned int data[6]; // SHT30 buffer

Ticker ticker;
ESP8266WebServer server(80);
WiFiClient espClient;
PubSubClient client(espClient);
DynamicJsonDocument json(2048); // config buffer
ShiftRegister74HC595 shift(3, 13, 14, 15);

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  Serial.begin(115200);
  Serial.println("");

  pinMode(12, INPUT_PULLUP);
  pinMode(2, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED OFF (inversed)

  if (!SPIFFS.begin()) {
    Serial.println("Failed to mount file system");
  }

  WiFi.macAddress(mac);
  rinfo = ESP.getResetInfoPtr();
  readConfig();

  const char* ssid = json["ssid"].as<const char*>();
  const char* pass = json["pass"].as<const char*>();
  const char* ip = json["ip"].as<const char*>();
  const char* gw = json["gw"].as<const char*>();
  const char* sn = json["sn"].as<const char*>();

  const char* broker = json["broker"].as<const char*>();
  int port = json["port"].as<int>();

  if (ssid[0] != '\0' && pass[0] != '\0') {
    Serial.println("setting up wifi");
    WiFi.mode(WIFI_STA);

    if (ip[0] != '\0' && gw[0] != '\0' && sn[0] != '\0') {
      IPAddress ip_address, gateway_ip, subnet_mask;
      if (!ip_address.fromString(ip) || !gateway_ip.fromString(gw) || !subnet_mask.fromString(sn)) {
        Serial.println("Error setting up static IP, using auto IP instead. Check your configuration.");
      } else {
        WiFi.config(ip_address, gateway_ip, subnet_mask);
      }
    }

    //serializeJson(json, Serial);

    WiFi.begin(ssid, pass);

    for (int i = 0; i < 50; i++) {
      if (WiFi.status() != WL_CONNECTED) {
        if (i > 40) {
          deviceMode = CONFIG_MODE;
          Serial.print("Failed to connect to: ");
          Serial.println(ssid);
          break;
        }
        delay(100);
      } else {
        Serial.println("Wifi connected...");
        Serial.print("SSID: ");
        Serial.println(WiFi.SSID());
        Serial.print("Mac address: ");
        Serial.println(WiFi.macAddress());
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
        break;
      }
    }


    // MQTT SETUP
    if (broker[0] != '\0' && port != 0) {
      client.setServer(broker, port);
    } else {
      deviceMode = CONFIG_MODE;
      Serial.println("Broker address or port is not set, going to config mode.");
    }

  } else {
    deviceMode = CONFIG_MODE;
    Serial.println("No credentials set, going to config mode.");
    startConfigPortal();
    //goToSleep();
  }

  Wire.begin();
  shift.setAllLow(); // set all pins LOW

  ticker.attach_ms(DISPLAY_DARK_TIME, refreshScreen); // medium brightness

  setupOTA();
}

// the loop function runs over and over again forever
void loop() {

  toggleOTAMode();

  toggleConfigMode();

  if (deviceMode == OTA_MODE) {
    Serial.println("WAITING FOR OTA UPDATE...");
    startOTA();
    Serial.println("RETURNING TO NORMAL MODE...");
    return;
  }

  if (deviceMode == CONFIG_MODE) {
    Serial.println("STARTING CONFIG ACCESS POINT, PRESS ANY BUTTON TO EXIT...");
    startConfigPortal();
    Serial.println("RETURNING TO NORMAL MODE...");
    return;
  }

  mqtt_connect();

  if (deviceMode == HASS_REGISTER_MODE) {
    Serial.println("STARTING HOME ASSISTANT DISCOVERY...");
    doHassRegister();
    Serial.println("RETURNING TO NORMAL MODE...");
    return;
  }

  if (deviceMode == NORMAL_MODE) {
    getSensorData(ADDRESS, data);

    int cTemp = (((((data[0] * 256) + data[1]) * 175 * 10) / 65535) - 450);
    int humidity = ((((data[3] * 256) + data[4]) * 100 * 10) / 65535);
    float cTempFloat = 10 * (((((data[0] * 256) + data[1]) * 175) / 65535) - 45);
    float humidityFloat = 10 * ((((data[3] * 256) + data[4]) * 100) / 65535);
    Serial.println(cTempFloat);
    Serial.println(humidityFloat);

    if ((*rinfo).reason != REASON_DEEP_SLEEP_AWAKE) {

      drawNumberL((cTemp / 100) % 10, 0, 0);
      drawNumberL((cTemp / 10) % 10, 5, 0);
      drawPixel(10, 0);
      drawPixel(11, 0);
      drawPixel(12, 0);
      drawPixel(10, 1);
      drawPixel(12, 1);
      drawPixel(10, 2);
      drawPixel(11, 2);
      drawPixel(12, 2);
      //drawNumberS((cTemp / 100) % 10, 0, 1);
      //drawNumberS((cTemp / 10) % 10, 4, 1);
      //drawPixel(8, 5);
      //drawNumberS(cTemp % 10, 10, 1);
      delay(5000);
    }
    else {
      clearScreen();
      drawPixel(0, 0);
      drawPixel(12, 0);
            delay(1000);
    }
    goToSleep();
  }
}
