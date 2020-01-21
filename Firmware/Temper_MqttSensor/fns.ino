
void goToSleep() {
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
  unsigned long startupTime = now() - round(millis() * 0.001); // real startup time, substitute millis to get accurate bootup time.
  if (startupTime - json["last_wake"].as<unsigned int>() < json["wake"].as<unsigned int>() * 0.95) {
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

void mqtt_connect() {
  const char* mqtt_usr = json["mqttusr"].as<const char*>();
  const char* mqtt_pass = json["mqttpass"].as<const char*>();
  int i = 0;
  while (!client.connected() && i < 5) { // Try 5 times, then give up and go to sleep.
    Serial.println("Attempting MQTT connection...");
    if (mqtt_usr[0] != '\0' && mqtt_pass[0] != '\0') {
      if (client.connect(String("hugo_" + macLastThreeSegments(mac)).c_str(), mqtt_usr, mqtt_pass)) {
        Serial.println("MQTT connected using credentials.");
        return;
      }
    } else {
      if (client.connect(String("hugo_" + macLastThreeSegments(mac)).c_str())) {
        Serial.println("MQTT connected anonymously.");
        return;
      }
    }
    Serial.print("MQTT connection attempt failed, rc=");
    Serial.println(client.state());
    ++i;
    delay(10);
  }
  goToSleep();
}

bool publishData(String topic, String payload) {
  topic.replace("[id]", macLastThreeSegments(mac));
  if (topic.length() > 0 && payload.length() > 0) {
    return client.publish(topic.c_str(), payload.c_str());
  } else {
    Serial.println("Target is not defined. Set it in config portal.");
  }
}

void publishBatteryLevel() {
  String batTopic = json["batt"].as<String>();
  batTopic.replace("[id]", macLastThreeSegments(mac));
  if (batTopic.length() > 0) {
    delay(20); // lets give the broker little breath time
    client.publish(batTopic.c_str(), String(batteryPercentage()).c_str());
    Serial.print("Battery percentage: ");
    Serial.print(batteryPercentage());
    Serial.println("%");
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
    Serial.println("Creating file and going to sleep. Try again!");
    json["ssid"] = json["pass"] = json["ip"] = json["gw"] = json["sn"] = json["broker"] = json["port"] = json["mqttusr"] = json["mqttpass"]  = json["ttopic"]  = json["htopic"] = json["batt"] = "";
    json["wake"] = WAKE_TIME_DEFAULT;
    saveConfig();
    goToSleep();
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
