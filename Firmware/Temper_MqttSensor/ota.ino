void setupOTA() {
  String ota_name = OTA_NAME + macLastThreeSegments(mac);
  ArduinoOTA.setHostname(ota_name.c_str());
  ArduinoOTA.begin();

  ArduinoOTA.onStart([]() {
    Serial.println("OTA UPLOAD STARTED...");
    clearScreen();
  });

  ArduinoOTA.onError([](ota_error_t error) {
    (void)error;
    ESP.restart();
  });

  ArduinoOTA.onEnd([]() {
    Serial.println("OTA UPLOAD DONE...");
  });
}

void startOTA() {
  clearScreen();
  drawImage(ota_screen);
  while (millis() - otaTimer < OTA_TIMEOUT) {
    ArduinoOTA.handle();
    delay(50);
  }
  //stopBlinking();
  goToSleep();
}

void toggleOTAMode() {
  if (digitalRead(12) == LOW) {
    deviceMode = OTA_MODE;
    otaTimer = millis(); // start counter
    return;
  }
}
