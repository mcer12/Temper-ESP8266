
void sendConfig(StaticJsonDocument<512>& payload, String configTopic) {
  char output[512];
  serializeJson(payload, output);
  Serial.println(output);
  if (client.publish(configTopic.c_str(), output, true)) {
    Serial.print("Discovery data sent.");
  } else {
    Serial.print("Failed to send discovery data, error = ");
    Serial.println(client.state());
  }
  payload.clear();
}

void doHassRegister() {
  Serial.println("Starting HASS MQTT registration...");
  Serial.println("Attemting to send TEMPERATURE discovery data...");
  StaticJsonDocument<512> payload;
  size_t payloadSize;
  String configTopic = "homeassistant/sensor/temper_" + macLastThreeSegments(mac) + "/temp/config";
  String stateTopic = "homeassistant/sensor/temper_" + macLastThreeSegments(mac) + "/temp/state";

  payload["unique_id"] = "temper_" + macLastThreeSegments(mac) + "_temperature";
  payload["name"] = "Temper " + macLastThreeSegments(mac) + " - Temperature";
  payload["stat_t"] = stateTopic;
  //  payload["ic"] = "mdi:coolant-temperature";
  //  payload["exp_aft"] = "1";
  payload["dev_cla"] = "temperature";
  payload["unit_of_meas"] = "°C";
  JsonObject device = payload.createNestedObject("device");
  device["ids"] = "temper" + macLastThreeSegments(mac);
  device["name"] = "Temper - temperature & humidity sensor";
  device["mf"] = "https://github.com/mcer12/Temper-ESP8266";
  device["mdl"] = "Temper-ESP8266";
  device["sw"] = FW_VERSION;
  sendConfig(payload, configTopic);

  client.loop();

  Serial.println("Done. Make sure to use following topic for TEMPERATURE:");
  Serial.println(stateTopic);
  Serial.println("Attemting to send HUMIDITY discovery data...");

  configTopic = "homeassistant/sensor/temper_" + macLastThreeSegments(mac) + "/hum/config";
  stateTopic = "homeassistant/sensor/temper_" + macLastThreeSegments(mac) + "/hum/state";

  payload["uniq_id"] = "temper_" + macLastThreeSegments(mac) + "_humidity";
  payload["name"] = "Temper " + macLastThreeSegments(mac) + " - Humidity";
  payload["stat_t"] = stateTopic;
  //  payload["ic"] = "mdi:battery-outline";
  payload["dev_cla"] = "humidity";
  payload["unit_of_meas"] = "%";
  //payload["val_tpl"] = "{% if value > 100 %}999{% else %}{{value}}{% endif %}";
  device = payload.createNestedObject("device");
  device["ids"] = "temper" + macLastThreeSegments(mac);
  sendConfig(payload, configTopic);

  client.loop();

  Serial.println("Done. Make sure to use following topic for HUMIDITY:");
  Serial.println(stateTopic);

  Serial.println("Attemting to send BATTERY discovery data...");

  // And battery...
  configTopic = "homeassistant/sensor/temper_" + macLastThreeSegments(mac) + "/battery/config";
  stateTopic = "homeassistant/sensor/temper_" + macLastThreeSegments(mac) + "/battery/state";

  payload["uniq_id"] = "temper_" + macLastThreeSegments(mac) + "_battery";
  payload["name"] = "Temper " + macLastThreeSegments(mac) + " - Battery";
  payload["stat_t"] = stateTopic;
  payload["ic"] = "mdi:battery-outline";
  payload["dev_cla"] = "battery";
  payload["unit_of_meas"] = "%";
  //payload["val_tpl"] = "{% if value > 100 %}999{% else %}{{value}}{% endif %}";
  device = payload.createNestedObject("device");
  device["ids"] = "temper" + macLastThreeSegments(mac);
  sendConfig(payload, configTopic);

  client.loop();
  client.disconnect();

  Serial.println("Done. Make sure to use following topic for BATTERY:");
  Serial.println(stateTopic);

  Serial.println("Registration completed, check for new device in Home Assistant MQTT integration.");

}
