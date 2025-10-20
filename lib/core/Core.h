// core/Core.h - CloudMouse Boilerplate
#ifndef CORE_H
#define CORE_H

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "EventBus.h"
#include "Events.h"
#include "../prefs/PreferencesManager.h"
#include "../hardware/LEDManager.h"
#include "../hardware/SimpleBuzzer.h"
#include "../hardware/WebServerManager.h"

// Forward declarations
class EncoderManager;
class DisplayManager;
class WiFiManager;

namespace CloudMouse {

enum class SystemState {
  BOOTING,
  INITIALIZING,
  
  // WiFi states
  WIFI_CONNECTING,
  WIFI_CONNECTED,
  WIFI_AP_MODE,
  WIFI_ERROR,
  
  // Running states
  READY,
  RUNNING,
  
  // Error state
  ERROR
};

class Core {
public:
  // Singleton pattern
  static Core& instance() {
    static Core core;
    return core;
  }

  // System lifecycle
  void initialize();
  void start();
  void startUITask();
  
  // Main coordination loop (runs on Core 0)
  void coordinationLoop();

  // Component registration
  void setEncoder(EncoderManager* encoder) { this->encoder = encoder; }
  void setDisplay(DisplayManager* display) { this->display = display; }
  void setWiFi(WiFiManager* wifi) { this->wifi = wifi; }
  void setWebServer(WebServerManager* webServer) { this->webServer = webServer; }
  void setLEDManager(LEDManager* ledManager) { this->ledManager = ledManager; }

  // State management
  SystemState getState() const { return currentState; }
  void setState(SystemState state);

private:
  Core() = default;
  ~Core() = default;
  Core(const Core&) = delete;
  Core& operator=(const Core&) = delete;

  // System state
  SystemState currentState = SystemState::BOOTING;
  uint32_t stateStartTime = 0;
  
  // WiFi config
  bool wifiRequired = true;

  // Components
  EncoderManager* encoder = nullptr;
  DisplayManager* display = nullptr;
  WiFiManager* wifi = nullptr;
  WebServerManager* webServer = nullptr;
  LEDManager* ledManager = nullptr;

  // Preferences
  PreferencesManager prefs;

  // Task handles
  TaskHandle_t uiTaskHandle = nullptr;

  // Statistics
  uint32_t coordinationCycles = 0;
  uint32_t eventsProcessed = 0;
  uint32_t lastHealthCheck = 0;

  // Task functions
  static void uiTaskFunction(void* param);
  void runUITask();

  // State handlers
  void handleBootingState();
  void handleWiFiConnection();

  // Event processing
  void processEvents();
  void processSerialCommands();
  void handleEncoderRotation(const Event& event);
  void handleEncoderClick(const Event& event);
  void handleEncoderLongPress(const Event& event);

  // System monitoring
  void checkHealth();
};

}  // namespace CloudMouse

#endif