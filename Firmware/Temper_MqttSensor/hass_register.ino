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

  Serial.println("Attemting to send discovery data...");
  StaticJsonDocument<512> payload;
  size_t payloadSize;
  String configTopic = "homeassistant/sensor/temper_" + macLastThreeSegments(mac) + "/config";
  String stateTopic = "homeassistant/sensor/temper_" + macLastThreeSegments(mac) + "/state";

  payload["unique_id"] = "temper_" + macLastThreeSegments(mac);
  payload["name"] = "Temper " + macLastThreeSegments(mac);
  payload["stat_t"] = stateTopic;
  payload["ic"] = "mdi:coolant-temperature";
  payload["exp_aft"] = "1";
  JsonObject device = payload.createNestedObject("device");
  device["ids"] = "temper" + macLastThreeSegments(mac);
  device["name"] = "Temper - temperature & humidity sensor";
  device["mf"] = "https://github.com/mcer12/Temper-ESP8266";
  device["mdl"] = "Temper-ESP8266";
  device["sw"] = FW_VERSION;
  sendConfig(payload, configTopic);

  client.loop();

  Serial.println("Done. Attemting to send battery discovery data...");

  // And battery...
  configTopic = "homeassistant/sensor/hugo_" + macLastThreeSegments(mac) + "/battery/config";
  stateTopic = "homeassistant/sensor/hugo_" + macLastThreeSegments(mac) + "/battery";

  payload["uniq_id"] = "temper_" + macLastThreeSegments(mac) + "_battery";
  payload["name"] = "Temper " + macLastThreeSegments(mac) + " - Battery";
  payload["stat_t"] = stateTopic;
  payload["ic"] = "mdi:battery-outline";
  payload["dev_cla"] = "battery";
  payload["unit_of_meas"] = "%";
  //payload["val_tpl"] = "{% if value > 100 %}999{% else %}{{value}}{% endif %}";
  device = payload.createNestedObject("device");
  device["ids"] = "temper" + macLastThreeSegments(mac);
  device["name"] = "Temper - temperature & humidity sensor";
  device["mf"] = "https://github.com/mcer12/Temper-ESP8266";
  device["mdl"] = "Temper-ESP8266";
  device["sw"] = FW_VERSION;
  sendConfig(payload, configTopic);

  client.loop();
  client.disconnect();

  Serial.println("Temper should now be discovered by Home Assistant. Use following topic to update values:");
  Serial.println(stateTopic);

  delay(100);

  goToSleep();
}
