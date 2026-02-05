#include "wifi_manager.h"
#include "config.h"
#include "pins.h"
#include <WiFi.h>

namespace WifiManager {

const int MAX_RETRIES = 10;

// LED blinking state
// Use project-wide LED_PIN and LED_ON/LED_OFF from pins.h
static const int WIFI_LED_PIN = LED_PIN;
static unsigned long _lastBlink = 0;
static bool _ledState = false; // true means LED currently ON
static const unsigned long CONNECT_BLINK_MS = 500; // slow (500ms)

static inline void setLed(bool on) {
    digitalWrite(WIFI_LED_PIN, on ? LED_ON : LED_OFF);
}

// Connect to WiFi using Config credentials with retry
bool begin(unsigned long timeoutMs) {
    if (!Config::hasWifiCredentials()) {
        Serial.println("WifiManager: No WiFi credentials available");
        return false;
    }

    String ssid = Config::wifiSsid();
    String pass = Config::wifiPass();
    int attempt = 0;
    bool connected = false;

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    WiFi.persistent(false);

    // prepare LED (start OFF)
    pinMode(WIFI_LED_PIN, OUTPUT);
    _ledState = false;
    setLed(false);
    _lastBlink = millis();

    while (attempt < MAX_RETRIES && !connected) {
        Serial.print("WifiManager: Attempting connection to SSID '");
        Serial.print(ssid);
        Serial.print("' (Attempt ");
        Serial.print(attempt + 1);
        Serial.println(")");

        WiFi.disconnect(true);    // ensure clean state
        delay(500);

        WiFi.begin(ssid.c_str(), pass.c_str());
        WiFi.setTxPower(WIFI_POWER_8_5dBm); // workaround for esp32-c3 super mini wifi instability
        unsigned long start = millis();
        while (millis() - start < timeoutMs) {
            // update connection LED (slow blink)
            if (millis() - _lastBlink >= CONNECT_BLINK_MS) {
                _ledState = !_ledState;
                setLed(_ledState);
                _lastBlink = millis();
            }

            if (WiFi.status() == WL_CONNECTED) {
                Serial.print("WifiManager: Connected! IP: ");
                Serial.println(WiFi.localIP());
                // Turn LED on solid to indicate connected state
                setLed(true);
                connected = true;
                break;
            }
            delay(50);
        }

        if (!connected) {
            Serial.println("WifiManager: Connection attempt failed.");
            attempt++;
            delay(500); // pause before next attempt
        }
    }

    if (!connected) {
        Serial.println("WifiManager: All connection attempts failed.");
        setLed(false);
    }

    return connected;
}

bool isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

} // namespace WifiManager
