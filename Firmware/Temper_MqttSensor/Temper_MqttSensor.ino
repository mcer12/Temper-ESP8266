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
#include <math.h>

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>
//#include <ArduinoOTA.h>
#include <TimeLib.h>
#include <Ticker.h>
#include <ShiftRegister74HC595.h>
#include <Wire.h>

#define AP_NAME "TEMPER_" // Last 6 MAC address characters will be appended at the end of the OTA name
#define OTA_NAME "TEMPER_" // Last 6 MAC address characters will be appended at the end of the OTA name
#define FW_VERSION "1.01"
#define OTA_BLINK_SPEED 100
#define OTA_TIMEOUT 300000 // 5 minutes
#define CONFIG_BLINK_SPEED 500
#define CONFIG_TIMEOUT 300000 // 5 minutes
#define DISPLAY_DARK_TIME 1 // millis, smaller number = brighter display
#define ADDRESS 0x44 // SHT30 I2C address
#define WAKE_TIME_DEFAULT 1800 // possible values: 60 = 1min, 600 = 10min, 1800 = 30min, 3600 = 1h
#define CRYSTAL_COMPENSATION_MULTIPLIER 0.91 // compensate huge inaccuracy in deep sleep timer

// ONLY CHANGE DEFINES BELOW IF YOU KNOW WHAT YOU'RE DOING!
#define NORMAL_MODE 0
#define OTA_MODE 1
#define CONFIG_MODE 2
#define CONFIG_MODE_LOCAL 3
#define HASS_REGISTER_MODE 4
#define UPDATE_SUCCESS 1
#define UPDATE_FAIL 2
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
uint8_t ok[7][13] = {
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,},
  {0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,},
  {0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0,},
  {0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0,},
  {0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,},
};

uint8_t deviceMode = NORMAL_MODE;

bool otaModeStarted = false;
volatile bool ledState = false;
int buttonTreshold = 2000;
uint8_t timeUpdateResult = 0; // 0 = no update, 1 = update success, 2 = update fail,
uint8_t failedAttempts = 0;

// TIMERS
unsigned long otaMillis, sleepMillis, ledMillis, configTimer, otaTimer;

int counter;
byte mac[6];
rst_info *rinfo;
unsigned long sleep_time = 3600000000; // 3600000000 = 1h, 10000000 = 10s
float data[6]; // SHT30 buffer
//static const char ntpServerName[] = "us.pool.ntp.org";
//static const char ntpServerName[] = "time.nist.gov";
//static const char ntpServerName[] = "time-a.timefreq.bldrdoc.gov";
//static const char ntpServerName[] = "time-b.timefreq.bldrdoc.gov";
static const char ntpServerName[] = "pool.ntp.org";

Ticker ticker;
ESP8266WebServer server(80);
WiFiClient espClient;
PubSubClient client(espClient);
DynamicJsonDocument json(2048); // config buffer
ShiftRegister74HC595 shift(3, 13, 14, 15);
WiFiUDP Udp;
ESP8266HTTPUpdateServer httpUpdateServer;
unsigned int localPort = 8888;  // local port to listen for UDP packets

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
    Serial.println("SPIFFS: Failed to mount file system");
  }

  WiFi.macAddress(mac);
  rinfo  = ESP.getResetInfoPtr();
  readConfig();

  Serial.print("RST Reason: ");
  Serial.println((*rinfo).reason);

  const char* ssid = json["ssid"].as<const char*>();
  const char* pass = json["pass"].as<const char*>();
  const char* ip = json["ip"].as<const char*>();
  const char* gw = json["gw"].as<const char*>();
  const char* sn = json["sn"].as<const char*>();

  const char* broker = json["broker"].as<const char*>();
  int port = json["port"].as<int>();

  Wire.begin();

  shift.setAllLow(); // set all pins LOW

  if (ssid != NULL && pass != NULL && ssid[0] != '\0' && pass[0] != '\0') {
    Serial.println("WIFI: Setting up wifi");
    WiFi.mode(WIFI_STA);

    if (ip != NULL && gw != NULL && sn != NULL && ip[0] != '\0' && gw[0] != '\0' && sn[0] != '\0') {
      IPAddress ip_address, gateway_ip, subnet_mask;
      if (!ip_address.fromString(ip) || !gateway_ip.fromString(gw) || !subnet_mask.fromString(sn)) {
        Serial.println("Error setting up static IP, using auto IP instead. Check your configuration.");
      } else {
        WiFi.config(ip_address, gateway_ip, subnet_mask);
      }
    }

    // serializeJson(json, Serial);

    WiFi.begin(ssid, pass);

    for (int i = 0; i < 100; i++) {
      if (WiFi.status() != WL_CONNECTED) {
        if (i > 50) {
          deviceMode = CONFIG_MODE;
          Serial.print("WIFI: Failed to connect to: ");
          Serial.println(ssid);
          break;
        }
        delay(200);
      } else {
        Serial.println("WIFI: Connected...");
        Serial.print("SSID: ");
        Serial.println(WiFi.SSID());
        Serial.print("WIFI: Mac address: ");
        Serial.println(WiFi.macAddress());
        Serial.print("WIFI: IP address: ");
        Serial.println(WiFi.localIP());
        break;
      }
    }

    // MQTT SETUP
    if (broker != NULL && broker[0] != '\0' && port != 0) {
      client.setServer(broker, port);
    } else {
      deviceMode = CONFIG_MODE;
      Serial.println("MQTT: Broker address or port is not set, going to config mode.");
    }

  } else {
    deviceMode = CONFIG_MODE;
    Serial.println("SETTINGS: No credentials set, going to config mode.");
    startConfigPortal();
    //goToSleep();
  }

  setupNTP();
  ticker.attach_ms(DISPLAY_DARK_TIME, refreshScreen); // medium brightness
}

// the loop function runs over and over again forever
void loop() {

  toggleConfigMode();

  if (deviceMode == CONFIG_MODE) {
    Serial.println("CONFIG AP: STARTING CONFIG ACCESS POINT, PRESS ANY BUTTON TO EXIT...");
    startConfigPortal();
    Serial.println("CONFIG AP: Returning to normal mode...");
    return;
  }

  if (deviceMode == NORMAL_MODE) {
    mqtt_connect();

    if (json["reg"].as<unsigned int>() == 1) {
      doHassRegister();
      json["reg"] = 0;
    }

    getSensorData(ADDRESS, data);

    float cTempFloat = ((((data[0] * 256) + data[1]) * 175) / 65535) - 45;
    float fTempFloat = (cTempFloat * 1.8) + 32;
    float humFloat = (((data[3] * 256) + data[4]) * 100) / 65535;
    int cTemp = round(cTempFloat);
    int fTemp = round(fTempFloat);
    int humidity = round(fTempFloat);
    Serial.print("Temperature C°: ");
    Serial.println(cTempFloat);
    Serial.print("Temperature F°: ");
    Serial.println(fTempFloat);
    Serial.print("Humidity: ");
    Serial.println(humFloat);

    int batt_percentage = batteryPercentage();
    if (json["chrg"].as<int>() == 1 && batt_percentage > 100) {
      Serial.println("Measurement publishing is disabled, disconnect charger or change settings in Config portal.");
    } else {
      String ttopic = json["ttopic"].as<String>();
      String htopic = json["htopic"].as<String>();
      String batt = json["batt"].as<String>();
      String tpayload;
      if (json["tscale"].as<unsigned int>() == 1) {
        tpayload = String(fTempFloat, 2);
      } else {
        tpayload = String(cTempFloat, 2);
      }
      String hpayload = String(humFloat, 2);

      publishData(ttopic, tpayload);
      publishData(htopic, hpayload);
      publishData(batt, String(batt_percentage));

    }
    if (!buttonWakeUp() && json["last_wake"] > 0) {
      //clearScreen();
      //drawImage(ok);
      //delay(200);
    }
    else {
      if (json["tscale"].as<unsigned int>() == 1) {
        drawNumberL((fTemp / 10) % 10, 0, 0);
        drawNumberL((fTemp) % 10, 5, 0);
      } else {
        drawNumberL((cTemp / 10) % 10, 0, 0);
        drawNumberL((cTemp) % 10, 5, 0);
      }
      drawDegreesSign();
      //drawNumberS((cTemp / 100) % 10, 0, 1);
      //drawNumberS((cTemp / 10) % 10, 4, 1);
      //drawPixel(8, 5);
      //drawNumberS(cTemp % 10, 10, 1);
      //serializeJson(json, Serial);
      delay(5000);
    }
    client.loop();
    client.disconnect();
    goToSleep();
  }
}
