
void goToSleep() {
  ticker.detach();
  shift.setAllLow(); // set all pins LOW
  unsigned int seconds = json["wake"].as<unsigned int>();
  Serial.println("SLEEP: Save timestamp before sleep");
  json["last_wake"] = now(); // remember when the device was put to sleep to know if it was woken up by timer or button
  saveConfig();
  Serial.print("SLEEP: Going to sleep for ");
  Serial.print(seconds);
  Serial.println(" seconds.");
  yield();
  delay(10);
  ESP.deepSleep(seconds * 1000000); // convert seconds to us
  delay(100);
  yield();
}

bool buttonWakeUp() {
  unsigned int startupTime = now() - round(millis() / 1000); // real startup time, substitute millis to get accurate bootup time.
  /*
    Serial.print("Now: ");
    Serial.println(now());
    Serial.print("Compensated startup timestamp: ");
    Serial.println(startupTime);
    Serial.print("last_wake: ");
    Serial.println(json["last_wake"].as<unsigned int>());
    Serial.print("Compensated startup timestamp - last_wake: ");
    Serial.println(startupTime - json["last_wake"].as<unsigned int>());
    Serial.print("wake timeout: ");
    Serial.println(json["wake"].as<unsigned int>());
    Serial.print("wake timeout compensated: ");
    Serial.println(json["wake"].as<unsigned int>() * CRYSTAL_COMPENSATION_MULTIPLIER);
  */
  if (startupTime - json["last_wake"].as<unsigned int>() < json["wake"].as<unsigned int>() * CRYSTAL_COMPENSATION_MULTIPLIER) {
    Serial.println("Button wake-up");
    return true;
  }
  Serial.println("Timer wake-up");
  return false;
}

String macToStr(const uint8_t* mac) {
  String result;
  for (int i = 0; i < 6; ++i) {
    if (mac[i] < 0x10) result += "0";
    result += String(mac[i], HEX);
    if (i < 5)
      result += ':';
  }
  result.toUpperCase();
  return result;
}

String macLastThreeSegments(const uint8_t* mac) {
  String result;
  for (int i = 3; i < 6; ++i) {
    if (mac[i] < 0x10) result += "0";
    result += String(mac[i], HEX);
  }
  result.toUpperCase();
  return result;
}

void wifi_connect() {

  const char* ssid = json["ssid"].as<const char*>();
  const char* pass = json["pass"].as<const char*>();
  const char* ip = json["ip"].as<const char*>();
  const char* gw = json["gw"].as<const char*>();
  const char* sn = json["sn"].as<const char*>();

  const char* broker = json["broker"].as<const char*>();
  int port = json["port"].as<int>();

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
}

void mqtt_connect() {
  if(deviceMode == CONFIG_MODE) return;
  const char* mqtt_usr = json["mqttusr"].as<const char*>();
  const char* mqtt_pass = json["mqttpass"].as<const char*>();
  int i = 0;
  while (!client.connected() && i < 5) { // Try 5 times, then give up and go to sleep.
    Serial.println("Attempting MQTT connection...");
    if (mqtt_usr[0] != '\0' && mqtt_pass[0] != '\0') {
      if (client.connect(String("temper_" + macLastThreeSegments(mac)).c_str(), mqtt_usr, mqtt_pass)) {
        Serial.println("MQTT connected using credentials.");
        return;
      }
    } else {
      if (client.connect(String("temper_" + macLastThreeSegments(mac)).c_str())) {
        Serial.println("MQTT connected anonymously.");
        return;
      }
    }
    Serial.print("MQTT connection attempt failed, rc=");
    Serial.println(client.state());
    ++i;
    delay(10);
  }
  deviceMode = CONFIG_MODE;
}

bool publishData(String topic, String payload) {
  topic.replace("[id]", macLastThreeSegments(mac));
  if (topic.length() > 0 && payload.length() > 0) {
    return client.publish(topic.c_str(), payload.c_str());
  } else {
    Serial.println("Target is not defined. Set it in config portal.");
  }
}

/* Read analog input for battery measurement */
int ReadAIN()
{
  int Read = analogRead(A0);
  int num = 10;

  for (int i = 0 ; i < num ; i++)
  {
    int newRead = analogRead(A0);
    if (newRead > Read)
    {
      Read = newRead;
    }
    delay(1);
  }
  return (Read);
}

/* Battery percentage estimation, this is not very accurate but close enough */
uint8_t batteryPercentage() {
  int analogValue = ReadAIN();
  if (analogValue > 1000) return 101; // CHARGING
  if (analogValue > 960) return 100;
  if (analogValue > 940) return 90;
  if (analogValue > 931) return 80;
  if (analogValue > 922) return 70;
  if (analogValue > 913) return 60; // 3.8v ... 920
  if (analogValue > 904) return 50;
  if (analogValue > 895) return 40;
  if (analogValue > 886) return 30;
  if (analogValue > 877) return 20; // 3.65v ... 880
  if (analogValue > 868) return 10;
  return 0;
}

bool readConfig() {
  File stateFile = SPIFFS.open("/config.json", "r");
  if (!stateFile) {
    Serial.println("Failed to read config file... first run?");
    Serial.println("Creating new file...");
    //json["ssid"] = json["pass"] = json["ip"] = json["gw"] = json["sn"] = json["broker"] = json["port"] = json["mqttusr"] = json["mqttpass"]  = json["ttopic"]  = json["htopic"] = json["batt"] = "";
    json["wake"] = WAKE_TIME_DEFAULT;
    saveConfig();
    return false;
  }
  DeserializationError error = deserializeJson(json, stateFile.readString());
  stateFile.close();
  return true;
}

bool saveConfig() {
  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
    return false;
  }
  serializeJson(json, configFile);
  //serializeJson(json, Serial);
  configFile.close();
  return true;
}
