#include "app.h"

bool running = false;

void startAP() {
  IPAddress apIP(192, 168, 40, 1);

  WiFi.mode(WIFI_AP_STA);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));

  String apName = "ESP" + String(ESP.getChipId());
  WiFi.softAP(apName.c_str());
  IPAddress actualIP = WiFi.softAPIP();
  running = true;

  Serial.print("AP: ");
  Serial.println(apName);
  Serial.print("IP: ");
	Serial.println(actualIP);
}

void stopAP() {
  running = false;
  WiFi.disconnect();
}
