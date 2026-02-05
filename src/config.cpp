#include "config.h"

// Forward declarations - values are defined in secrets.h (included in main.cpp)
extern String CONFIG_MINER_ID;
extern String CONFIG_MINER_COLOR;
extern String CONFIG_WIFI_SSID;
extern String CONFIG_WIFI_PASSWORD;

namespace Config {

void begin() {
  Serial.println("Config: Configuration initialized");
  Serial.print("Config: MINER_ID = ");
  Serial.println(CONFIG_MINER_ID);
  Serial.print("Config: MINER_COLOR = ");
  Serial.println(CONFIG_MINER_COLOR);
  Serial.print("Config: WIFI_SSID = ");
  Serial.println(CONFIG_WIFI_SSID);
}

String minerId() { return CONFIG_MINER_ID; }

String minerColor() { return CONFIG_MINER_COLOR; }

String wifiSsid() { return CONFIG_WIFI_SSID; }

String wifiPass() { return CONFIG_WIFI_PASSWORD; }

bool hasWifiCredentials() {
  return CONFIG_WIFI_SSID.length() > 0 && CONFIG_WIFI_PASSWORD.length() > 0;
}

} // namespace Config
