#pragma once
#include <Arduino.h>

// WiFi provisioning manager. Call wifiInit() once from setup() after
// settingsLoad(). Credentials are stored in NVS namespace "wifi".
// BLE always stays active — both transports coexist on ESP32-S3.

void wifiInit();                     // load creds from NVS, connect if present
void wifiPoll();                     // called from loop() — drives retry logic
void wifiSetCredentials(const char* ssid, const char* pass);  // store + connect
bool wifiHasCredentials();
bool wifiIsConnected();
const char* wifiStatusStr();         // "connected 192.168.x.x" / "connecting" / "disconnected" / "no creds"
const char* wifiSSID();              // stored SSID (or "")
