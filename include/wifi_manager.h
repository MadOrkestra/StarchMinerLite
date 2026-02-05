#pragma once
#include <WiFi.h>

namespace WifiManager {

bool begin(unsigned long timeoutMs = 10000);  // default 10s timeout
bool isConnected();

} // namespace WifiManager
