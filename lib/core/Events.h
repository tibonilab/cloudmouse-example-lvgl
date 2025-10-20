// core/Events.h - CloudMouse Boilerplate
#pragma once
#include <Arduino.h>

namespace CloudMouse {

enum class EventType {
  // System events
  BOOTING_COMPLETE,
  
  // Encoder events
  ENCODER_ROTATION,
  ENCODER_CLICK,
  ENCODER_LONG_PRESS,
  
  // Display events
  DISPLAY_WAKE_UP,
  DISPLAY_UPDATE,
  DISPLAY_CLEAR,
  
  // WiFi display events
  DISPLAY_WIFI_CONNECTING,
  DISPLAY_WIFI_CONNECTED,
  DISPLAY_WIFI_ERROR,
  DISPLAY_WIFI_AP_MODE,
  DISPLAY_WIFI_SETUP_URL,
  
  // WiFi system events
  WIFI_CONNECTING,
  WIFI_CONNECTED,
  WIFI_DISCONNECTED,
  WIFI_ERROR,
  WIFI_AP_MODE,
};

struct Event {
  EventType type;
  int32_t value;        // Numeric value (rotation count, timing, etc.)
  char stringData[256]; // String data (SSID, IP, messages, etc.)
  
  // Constructors
  Event() : type(EventType::ENCODER_ROTATION), value(0) {
    memset(stringData, 0, sizeof(stringData));
  }
  
  Event(EventType t) : type(t), value(0) {
    memset(stringData, 0, sizeof(stringData));
  }
  
  Event(EventType t, int32_t v) : type(t), value(v) {
    memset(stringData, 0, sizeof(stringData));
  }
  
  // String data setter (safe)
  void setStringData(const String& str) {
    strncpy(stringData, str.c_str(), sizeof(stringData) - 1);
    stringData[sizeof(stringData) - 1] = '\0';
  }
  
  // String data getter
  String getStringData() const { 
    return String(stringData); 
  }
  
  // WiFi helpers
  void setWiFiData(const char* ssid, const char* ip = "", int32_t connectionTime = 0) {
    value = connectionTime;
    snprintf(stringData, sizeof(stringData), "%s|%s", ssid, ip);
  }
  
  String getSSID() const {
    String data(stringData);
    int separatorIndex = data.indexOf('|');
    return separatorIndex >= 0 ? data.substring(0, separatorIndex) : data;
  }
  
  String getIP() const {
    String data(stringData);
    int separatorIndex = data.indexOf('|');
    return separatorIndex >= 0 ? data.substring(separatorIndex + 1) : "";
  }
};

}  // namespace CloudMouse