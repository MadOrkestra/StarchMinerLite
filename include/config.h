// config.h
// Simple config interface. Values are set in main.cpp
#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

namespace Config {
    // Initialize config (sets values from main.cpp)
    void begin();

    // Accessors for config values
    String minerId();
    String minerColor();
    String wifiSsid();
    String wifiPass();
    bool hasWifiCredentials();
}

#endif // CONFIG_H
