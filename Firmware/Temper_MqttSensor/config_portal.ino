
void startConfigPortal() {

  clearScreen();
  drawImage(config_screen);

  WiFi.disconnect();
  WiFi.mode(WIFI_AP);
  IPAddress ap_ip(10, 10, 10, 1);
  WiFi.softAPConfig(ap_ip, ap_ip, IPAddress(255, 255, 255, 0));
  String ap_name = AP_NAME + macLastThreeSegments(mac);
  WiFi.softAP(ap_name.c_str());
  IPAddress ip = WiFi.softAPIP();
  Serial.print("IP address: ");
  Serial.println(ip);
  httpUpdateServer.setup(&server);
  server.on("/", handleRoot);
  server.begin();

  delay(3000); // delay to prevent accidental shut down of the config portal

  while (deviceMode == CONFIG_MODE) { // BLOCKING INFINITE LOOP
    if (millis() - configTimer > CONFIG_TIMEOUT) {
      goToSleep();
      return;
    }
    server.handleClient();
  }

}


void toggleConfigMode() {
  if (batteryPercentage() > 100 && buttonWakeUp() && digitalRead(12) == HIGH) {
    deviceMode = CONFIG_MODE;
    configTimer = millis(); // start counter
  }
}

void handleRoot() {
  if (server.args()) {

    if (server.hasArg("ssid")) {
      json["ssid"] = server.arg("ssid");
    }
    if (server.hasArg("pass")) {
      json["pass"] = server.arg("pass");
    }
    if (server.hasArg("ip")) {
      json["ip"] = server.arg("ip");
    }
    if (server.hasArg("gw")) {
      json["gw"] = server.arg("gw");
    }
    if (server.hasArg("sn")) {
      json["sn"] = server.arg("sn");
    }

    if (server.hasArg("broker")) {
      json["broker"] = server.arg("broker");
    }
    if (server.hasArg("port")) {
      json["port"] = server.arg("port");
    }
    if (server.hasArg("mqttusr")) {
      json["mqttusr"] = server.arg("mqttusr");
    }
    if (server.hasArg("mqttpass")) {
      json["mqttpass"] = server.arg("mqttpass");
    }
    if (server.hasArg("htopic")) {
      json["htopic"] = server.arg("htopic");
    }
    if (server.hasArg("ttopic")) {
      json["ttopic"] = server.arg("ttopic");
    }
    if (server.hasArg("batt")) {
      json["batt"] = server.arg("batt");
    }
    if (server.hasArg("wake")) {
      json["wake"] = server.arg("wake");
    }
    saveConfig();
  }

  const char* batteryColor = "#a53e3e"; // default RED
  int batteryPercent = batteryPercentage();
  if (batteryPercent >= 40) batteryColor = "#a57d3e";
  if (batteryPercent >= 60) batteryColor = "#9ea53e";
  if (batteryPercent >= 80) batteryColor = "#7ca53e";
  if (batteryPercent > 100) batteryColor = "#3e7ea5";

  String html = "<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width,initial-scale=1\"><title>Temper Configuration</title><style>html,body{margin:0;padding:0;font-size:16px;background:#444444;}body,*{box-sizing:border-box;font-family:-apple-system,BlinkMacSystemFont,\"Segoe UI\",Roboto,\"Helvetica Neue\",Arial,sans-serif;}a{color:inherit;text-decoration:underline;}.wrapper{padding:30px 0;}.container{margin:auto;padding:40px;max-width:500px;color:#fff;background:#000;box-shadow:0 0 100px rgba(0,0,0,.5);border-radius:50px;}.row{margin-bottom:15px;}h1{margin:0 0 10px 0;font-family:Arial,sans-serif;font-weight:300;font-size:2rem;}h1 + p{margin-bottom:30px;}h2{margin:30px 0 0 0;font-family:Arial,sans-serif;font-weight:300;font-size:1.5rem;}p{font-size:.85rem;margin:0 0 20px 0;color:rgba(255,255,255,.7);}label{display:block;width:100%;margin-bottom:5px;}input[type=\"text\"],input[type=\"password\"],select{display:inline-block;width:100%;height:42px;line-height:38px;padding:0 20px;color:#fff;border:2px solid #666;background:none;border-radius:5px;transition:.15s;box-shadow:none;outline:none;}input[type=\"text\"]:hover,input[type=\"password\"]:hover,select:hover{border-color:#ababab;}input[type=\"text\"]:focus,input[type=\"password\"]:focus,select:focus{border-color:#fff;}option{color:#000;}button{display:block;width:100%;padding:10px 20px;font-size:1rem;font-weight:700;text-transform:uppercase;background:#ff9c29;border:0;border-radius:5px;cursor:pointer;transition:.15s;outline:none;}button:hover{background:#ffba66;}.github{margin-top:15px;text-align:center;}.github a{color:#ff9c29;transition:.15s;}.github a:hover{color:#ffba66;}.bat p{margin:0 0 5px 0;text-align:center;text-transform:uppercase;font-size:.8rem;}.bat >div{position:relative;margin:0 auto 20px;width:300px;height:10px;background:#272727;border-radius:5px;}.bat >div >div{position:absolute;left:0;top:0;bottom:0;border-radius:5px;min-width:10px;}.mac{display:inline-block;margin-top:8px;padding:2px 5px;color:#fff;background:#444;border-radius:3px;}</style><style media=\"all and (max-width:520px)\">.wrapper{padding:20px 0;}.container{padding:25px 15px;border-radius:0;}</style></head><body><div class=\"wrapper\">";
  html += "<div class=\"bat\"><p>Battery level: " + ((batteryPercent > 100) ? "Charging" : (String)batteryPercent + "%") + "</p><div><div style=\"background: " + batteryColor + ";width: " + ((batteryPercent > 100) ? 100 : batteryPercent) + "%\"></div></div></div>";
  html += "<div class=\"container\"> <form method=\"post\" action=\"/\"> <h1>Temper Configuration</h1> <p>Config AP will automatically shut down after 5 minutes. To leave immediately, disconnect USB power and push the button.</p> <h2>Network settings</h2> <p>Select your network settings here.</p> <div class=\"row\"> <label for=\"ssid\">WiFi SSID</label> <input type=\"text\" id=\"ssid\" name=\"ssid\" value=\"";
  html += json["ssid"].as<const char*>();
  html += "\"> </div> <div class=\"row\"> <label for=\"pass\">WIFI Password</label> <input type=\"password\" id=\"pass\" name=\"pass\" value=\"";
  html += json["pass"].as<const char*>();
  html += "\"> </div> <h2>Static IP settings (optional)</h2> <p>Optional settings for static IP, in some cases this might speed up response time. All 3 need to be set and IP should be reserved in router's DHCP settings.";
  html += "<br>MAC address: <span class=\"mac\">";
  html += macToStr(mac);
  html += "</span></p>";
  html += "<div class=\"row\"> <label for=\"ip\">IP Address (optional):</label> <input type=\"text\" id=\"ip\" name=\"ip\" value=\"";
  html += json["ip"].as<const char*>();
  html += "\"> </div> <div class=\"row\"> <label for=\"gw\">Gateway IP (optional):</label> <input type=\"text\" id=\"gw\" name=\"gw\" value=\"";
  html += json["gw"].as<const char*>();
  html += "\"> </div> <div class=\"row\"> <label for=\"sn\">Subnet mask (optional):</label> <input type=\"text\" id=\"sn\" name=\"sn\" value=\"";
  html += json["sn"].as<const char*>();
  html += "\"> </div>";
  html += "<h2>MQTT Configuration</h2>";
  html += "<p>Set your broker address as an IP or name address.<br>For example: \"broker.example.com\" or \"192.168.0.1\"</p>";
  html += "<p>You can also define user and password for more security.<br></p>";
  html += "<div class=\"row\"> <label for=\"broker\">Broker adress</label> <input type=\"text\" id=\"broker\" name=\"broker\" value=\"";
  html += json["broker"].as<const char*>();
  html += "\"> </div>";
  html += "<div class=\"row\"> <label for=\"port\">Broker port</label> <input type=\"text\" id=\"port\" name=\"port\" value=\"";
  html += json["port"].as<const char*>();
  html += "\"> </div>";
  html += "<div class=\"row\"> <label for=\"mqttusr\">User (optional)</label> <input type=\"text\" id=\"mqttusr\" name=\"mqttusr\" value=\"";
  html += json["mqttusr"].as<const char*>();
  html += "\"> </div>";
  html += "<div class=\"row\"> <label for=\"mqttpass\">Password (optional)</label> <input type=\"password\" id=\"mqttpass\" name=\"mqttpass\" value=\"";
  html += json["mqttpass"].as<const char*>();
  html += "\"> </div>";
  html += "<h2>Button settings</h2> <p>Assign MQTT Topic and wake-up interval, example (for Home Assistant):<br>\"homeassistant/sensor/temper_[id]/state\"<br>";
  html += "Use shortcode <strong>[id]</strong> to add 6-character unique identifier. This is especially useful if you have more Temper sensors and want to distinguish between them.<br></p>";
  html += "<div class=\"row\"> <label for=\"ttopic\">Temperature topic</label> <input type=\"text\" id=\"ttopic\" name=\"ttopic\" value=\"";
  html += json["ttopic"].as<const char*>();
  html += "\"></div>";
    html += "<div class=\"row\"> <label for=\"htopic\">Humidity topic</label> <input type=\"text\" id=\"htopic\" name=\"htopic\" value=\"";
  html += json["htopic"].as<const char*>();
  html += "\"></div>";
  html += "<div class=\"row\"><label for=\"wake\">Wake-up interval</label>";
  html += "<select id=\"wake\" name=\"wake\">";

  unsigned int wake = json["wake"].as<unsigned int>();
  html += "<option value=\"60\"";
  if (wake == 60) html += " selected";
  html += ">1 min.</option>";
  html += "<option value=\"600\"";
  if (wake == 600) html += " selected";
  html += ">10 min.</option>";
  html += "<option value=\"1800\"";
  if (wake == 1800) html += " selected";
  html += ">30 min.</option>";
  html += "<option value=\"3600\"";
  if (wake == 3600) html += " selected";
  html += ">60 min.</option>";

  html += "</select></div>";
  html += "<h2>Battery monitoring</h2><p>You can monitor your battery level percentage (rough estimation).<br>";
  html += "For example: homeassistant/sensor/temper_[id]/battery</p>";
  html += "<div class=\"row\"><label for=\"batt\">Battery Topic</label> <input type=\"text\" id=\"batt\" name=\"batt\" value=\"";
  html += json["batt"].as<const char*>();
  html += "\"></div>";
  html += "<div class=\"row\"> <button type=\"submit\">Save and reboot</button> </div> </form> </div>";
  html += "<div class=\"github\"> <p>MQTT firmware ";
  html += FW_VERSION;
  html += ", check out <a href=\"https://github.com/mcer12/Temper-ESP8266\" target=\"_blank\"><strong>Temper</strong> on GitHub</a></p> </div>";
  html += "</div> </body> </html>";
  server.send(200, "text/html", html);

  if (server.args()) {
    delay(1000);
    ESP.reset();
    delay(100);
  }
}
