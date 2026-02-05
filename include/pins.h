// pins.h - central place for board pin definitions
#pragma once

// Built-in LED pin for ESP32-C3-DevKitM-1
// Adjust if using a different board
#define LED_PIN 8
// Define whether the board's LED is active-low (1) or active-high (0)
#define LED_ACTIVE_LOW 1

#if LED_ACTIVE_LOW
#define LED_ON LOW
#define LED_OFF HIGH
#else
#define LED_ON HIGH
#define LED_OFF LOW
#endif
