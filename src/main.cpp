#include <Arduino.h>
#include "config.h"
#include "miner.h"
#include "wifi_manager.h"
#include "secrets.h"

// Include central pin definitions
#include "pins.h"

void setup() {
  // Initialize serial communication
  Serial.begin(115200);

  // ESP32-C3 specific: Give extra time for USB Serial/JTAG to initialize
  delay(3000);

  // Force serial output to start
  Serial.println();
  Serial.println("=== ESP32-C3 Starch Miner Starting ===");
  Serial.println("Serial communication established!");
  Serial.flush(); // Ensure output is sent

  // Initialize configuration
  Serial.println("Initializing configuration...");
  Config::begin();
  Serial.print("Miner ID: ");
  Serial.println(Config::minerId());
  Serial.flush();

  // Try to connect to WiFi
  if (!WifiManager::begin()) {
    Serial.println("WiFi connection failed.");
  } else {
    Serial.println("WiFi connected successfully");
  }

  // Initialize miner module
  Serial.println("Initializing miner...");
  Miner::begin();

  // Set LED pin as output
  pinMode(LED_PIN, OUTPUT);

  // Turn off LED initially using shared polarity
  digitalWrite(LED_PIN, LED_OFF);

  Serial.println("Setup complete!");
  Serial.println("===================");
  Serial.flush();

  // Turn LED on (use shared polarity)
  digitalWrite(LED_PIN, LED_ON);
}

void loop() {
  static unsigned long lastReconnectAttempt = 0;
  static unsigned long lastMinerUpdate = 0;
  const unsigned long RECONNECT_INTERVAL =
      30000; // 30 seconds between reconnect attempts
  const unsigned long MINER_UPDATE_INTERVAL =
      15000; // 15 seconds for blockchain hash and block submission

  unsigned long currentTime = millis();

  // Only attempt reconnection if we have WiFi credentials
  if (!WifiManager::isConnected() && Config::hasWifiCredentials() &&
      currentTime - lastReconnectAttempt >= RECONNECT_INTERVAL) {
    Serial.println("WiFi disconnected - attempting reconnection...");
    WifiManager::begin(10000); // Try to reconnect with 10s timeout
    lastReconnectAttempt = currentTime;
  }

  // Fetch blockchain hash and submit block every 15 seconds if WiFi is
  // connected
  if (WifiManager::isConnected() &&
      currentTime - lastMinerUpdate >= MINER_UPDATE_INTERVAL) {
    Miner::fetchLastHash();
    lastMinerUpdate = currentTime;
  }

  // Small delay to prevent excessive CPU usage
  delay(100);
}
