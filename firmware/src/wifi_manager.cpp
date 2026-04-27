#include "wifi_manager.h"
#include <WiFi.h>
#include <Preferences.h>

static Preferences _wifiPrefs;
static char _ssid[64] = "";
static char _pass[64] = "";
static bool _hasCreds = false;
static bool _connected = false;
static char _ip[20] = "";
static uint32_t _lastAttemptMs = 0;
static const uint32_t RETRY_INTERVAL_MS = 15000;

static void _wifiLoadCreds() {
  _wifiPrefs.begin("wifi", true);
  _wifiPrefs.getString("ssid", _ssid, sizeof(_ssid));
  _wifiPrefs.getString("pass", _pass, sizeof(_pass));
  _wifiPrefs.end();
  _hasCreds = _ssid[0] != 0;
}

static void _wifiConnect() {
  if (!_hasCreds) return;
  Serial.printf("[wifi] connecting to %s\n", _ssid);
  WiFi.begin(_ssid, _pass);
  WiFi.setAutoReconnect(true);
  _lastAttemptMs = millis();
}

static void _onWifiEvent(WiFiEvent_t event) {
  switch (event) {
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      _connected = true;
      strncpy(_ip, WiFi.localIP().toString().c_str(), sizeof(_ip) - 1);
      _ip[sizeof(_ip) - 1] = 0;
      Serial.printf("[wifi] connected ip=%s\n", _ip);
      break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      _connected = false;
      _ip[0] = 0;
      Serial.println("[wifi] disconnected");
      break;
    default:
      break;
  }
}

void wifiInit() {
  WiFi.mode(WIFI_STA);
  WiFi.onEvent(_onWifiEvent);
  _wifiLoadCreds();
  if (_hasCreds) _wifiConnect();
}

void wifiPoll() {
  // Retry if we have creds but aren't connected and enough time has passed
  if (_hasCreds && !_connected && WiFi.status() != WL_CONNECTED) {
    if (millis() - _lastAttemptMs >= RETRY_INTERVAL_MS) {
      Serial.println("[wifi] retrying...");
      WiFi.disconnect();
      _wifiConnect();
    }
  }
}

void wifiSetCredentials(const char* ssid, const char* pass) {
  strncpy(_ssid, ssid, sizeof(_ssid) - 1); _ssid[sizeof(_ssid) - 1] = 0;
  strncpy(_pass, pass, sizeof(_pass) - 1); _pass[sizeof(_pass) - 1] = 0;
  _hasCreds = _ssid[0] != 0;
  _wifiPrefs.begin("wifi", false);
  _wifiPrefs.putString("ssid", _ssid);
  _wifiPrefs.putString("pass", _pass);
  _wifiPrefs.end();
  Serial.printf("[wifi] creds saved ssid=%s\n", _ssid);
  if (_hasCreds) {
    WiFi.disconnect();
    _wifiConnect();
  }
}

bool wifiHasCredentials() { return _hasCreds; }
bool wifiIsConnected() { return _connected; }

const char* wifiStatusStr() {
  if (!_hasCreds) return "no creds";
  if (_connected) return _ip;
  if (WiFi.status() == WL_CONNECTED) return "connected";
  return "connecting";
}

const char* wifiSSID() { return _ssid; }
