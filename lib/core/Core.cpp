// core/Core.cpp - CloudMouse Boilerplate
#include "./Core.h"
#include "../hardware/EncoderManager.h"
#include "../hardware/DisplayManager.h"
#include "../hardware/WiFiManager.h"

namespace CloudMouse {

void Core::initialize() {
  Serial.println("🚀 Core initialization starting...");
  
  // Serial output device info
  DeviceID::printDeviceInfo();

  // Initialize event bus
  EventBus::instance().initialize();
  
  // Start in BOOTING state
  setState(SystemState::BOOTING);
  
  Serial.println("🎬 Starting boot sequence - LED animation only");
  Serial.println("✅ Core initialized in BOOTING state");
}

void Core::startUITask() {
  if (uiTaskHandle != nullptr) {
    Serial.println("🎮 UI Task already running");
    return;
  }

  // Create UI task on Core 1
  xTaskCreatePinnedToCore(
    uiTaskFunction,
    "UI_Task",
    8192,          // Stack size
    this,          // Parameter
    3,             // Priority
    &uiTaskHandle,
    1              // Core 1
  );

  if (uiTaskHandle) {
    Serial.println("✅ UI Task running on Core 1");
    
    // Start LED animation task
    if (ledManager) {
      ledManager->startAnimationTask();
    }
  } else {
    setState(SystemState::ERROR);
    Serial.println("❌ Failed to start UI Task!");
  }
}

void Core::start() {
  if (currentState != SystemState::READY) {
    Serial.println("❌ Core not ready to start!");
    return;
  }

  setState(SystemState::RUNNING);
  Serial.println("✅ Core started - System RUNNING");
}

void Core::setState(SystemState state) {
  if (currentState != state) {
    Serial.printf("🔄 State change: %d → %d\n", (int)currentState, (int)state);
    currentState = state;
    stateStartTime = millis();
  }
}

void Core::coordinationLoop() {
  // Handle booting state
  if (currentState == SystemState::BOOTING) {
    handleBootingState();
    // Don't return - continue processing WiFi and other tasks
  }

  // Handle WiFi connection in background
  if (wifi) {
    wifi->update();
    handleWiFiConnection();
  }

  // WiFi AP mode - update web server
  if (wifi && wifi->getState() == WiFiManager::WiFiState::AP_MODE && webServer) {
    webServer->update();
  }

  // Auto-start when ready
  if (currentState == SystemState::READY) {
    start();
  }

  // Process serial commands
  processSerialCommands();

  // Process events from UI task
  processEvents();
  
  coordinationCycles++;

  // Health check every 5 seconds
  if (millis() - lastHealthCheck > 5000) {
    checkHealth();
    lastHealthCheck = millis();
  }
}

void Core::handleBootingState() {
    if (millis() >= 3500) {
        setState(SystemState::INITIALIZING);
        
        #if WIFI_REQUIRED
            Serial.println("📡 WiFi is REQUIRED - showing connecting screen");
            
            if (wifi) {
                EventBus::instance().sendToUI(Event(EventType::DISPLAY_WIFI_CONNECTING));
                
                wifi->init();
            }
        #else
            Serial.println("📡 WiFi NOT required - going to ready state");

            EventBus::instance().sendToUI(Event(EventType::DISPLAY_WAKE_UP));
            setState(SystemState::READY);
        #endif
    }
}

void Core::handleWiFiConnection() {
  static WiFiManager::WiFiState lastWiFiState = WiFiManager::WiFiState::DISCONNECTED;
  WiFiManager::WiFiState currentWiFiState = wifi->getState();

  // State changed
  if (currentWiFiState != lastWiFiState) {
    lastWiFiState = currentWiFiState;
    
    switch (currentWiFiState) {
      case WiFiManager::WiFiState::CONNECTING:
        Serial.println("📡 WiFi: Connecting...");
        setState(SystemState::WIFI_CONNECTING);
        
        // // Show connecting screen
        // EventBus::instance().sendToUI(Event(EventType::DISPLAY_WIFI_CONNECTING));
        break;
        
      case WiFiManager::WiFiState::CONNECTED:
        {
          Serial.println("✅ WiFi: Connected! FROM COORDINATION LOOP");
          String ssid = wifi->getSSID();
          String ip = wifi->getLocalIP();
          Serial.printf("   SSID: %s, IP: %s\n", ssid.c_str(), ip.c_str());
          
          // Flash green LED as feedback
          if (ledManager) {
            ledManager->flashColor(0, 255, 0, 255, 500);
          }
          
          // Exit WiFi connecting screen, show hello world
          Event helloEvent(EventType::ENCODER_ROTATION, 0);
          EventBus::instance().sendToUI(helloEvent);
          
          // WiFi connected - go to READY state
          setState(SystemState::READY);
        }
        break;
        
      case WiFiManager::WiFiState::CREDENTIAL_NOT_FOUND:
      case WiFiManager::WiFiState::TIMEOUT:
      case WiFiManager::WiFiState::ERROR:
        Serial.println("❌ WiFi: No credentials or connection failed - starting AP mode");
        
        // Start AP mode
        if (wifi) {
          wifi->setupAP();
        }
        break;
        
      case WiFiManager::WiFiState::AP_MODE:
        Serial.println("📱 WiFi: AP Mode active");
        setState(SystemState::WIFI_AP_MODE);
        
        if (webServer) {
          webServer->init();
          String apIP = wifi->getAPIP();
          String apSSID = wifi->getSSID();
          
          Serial.printf("   AP SSID: %s\n", apSSID.c_str());
          Serial.printf("   AP IP: %s\n", apIP.c_str());
          
          // Show AP mode on display with SSID and IP - QR for WiFi connection
          Event apEvent(EventType::DISPLAY_WIFI_AP_MODE);
          apEvent.setStringData((apSSID + "|" + apIP).c_str());
          EventBus::instance().sendToUI(apEvent);
          
          // Flash blue LED
          if (ledManager) {
            ledManager->flashColor(0, 100, 255, 255, 1000);
          }
        }
        break;
        
      default:
        break;
    }
  }
  
  // Check if a client connected to our AP
  if (currentWiFiState == WiFiManager::WiFiState::AP_MODE && wifi) {
    static bool clientWasConnected = false;
    bool clientIsConnected = wifi->hasAPClient();
    
    // Client just connected!
    if (clientIsConnected && !clientWasConnected) {
      Serial.println("📱 Client connected to AP - showing setup URL");
      
      String apIP = wifi->getAPIP();
      String setupURL = "http://" + apIP + "/setup";
      
      // Show setup URL with QR code
      Event setupEvent(EventType::DISPLAY_WIFI_SETUP_URL);
      setupEvent.setStringData(setupURL.c_str());
      EventBus::instance().sendToUI(setupEvent);
      
      // Flash green LED
      if (ledManager) {
        ledManager->flashColor(0, 255, 0, 255, 300);
      }
    }
    
    clientWasConnected = clientIsConnected;
  }
}


void Core::processEvents() {
  Event event;
  
  // Process all events from UI task
  while (EventBus::instance().receiveFromUI(event, 0)) {
    eventsProcessed++;
    
    switch (event.type) {
      case EventType::ENCODER_ROTATION:
        handleEncoderRotation(event);
        break;
        
      case EventType::ENCODER_CLICK:
        handleEncoderClick(event);
        break;
        
      case EventType::ENCODER_LONG_PRESS:
        handleEncoderLongPress(event);
        break;
        
      default:
        // Unknown event
        break;
    }
  }
}

void Core::handleEncoderRotation(const Event& event) {
  Serial.printf("🔄 Encoder rotation: %d\n", event.value);
  
  // Notify LED manager
  if (ledManager) {
    ledManager->activate();
  }
  
  // Send rotation event to display
  EventBus::instance().sendToUI(event);
}

void Core::handleEncoderClick(const Event& event) {
  Serial.println("🖱️ Encoder clicked!");
  
  // Flash LED
  if (ledManager) {
    ledManager->flashColor(0, 255, 0, 255, 200);
  }
  
  // Buzz
  SimpleBuzzer::buzz();
  
  // Send click event to display
  EventBus::instance().sendToUI(event);
}

void Core::handleEncoderLongPress(const Event& event) {
  Serial.println("⏱️ Encoder long press!");
  
  // Different color flash
  if (ledManager) {
    ledManager->flashColor(255, 165, 0, 255, 500);
  }
  
  // Different buzz pattern
  SimpleBuzzer::error();
  
  // Send long press event to display
  EventBus::instance().sendToUI(event);
}

void Core::uiTaskFunction(void* param) {
  Core* core = static_cast<Core*>(param);
  core->runUITask();
}

void Core::runUITask() {
  TickType_t lastWake = xTaskGetTickCount();
  
  Serial.println("🎮 UI Task started");
  
  while (true) {
    // Read encoder input
    if (encoder) {
      encoder->update();
      
      // Check for rotation
      int movement = encoder->getMovement();
      if (movement != 0) {
        Event rotationEvent(EventType::ENCODER_ROTATION, movement);
        EventBus::instance().sendToMain(rotationEvent);
        Serial.printf("🔄 Encoder rotation: %d\n", movement);
      }
      
      // Check for click
      if (encoder->getClicked()) {
        Event clickEvent(EventType::ENCODER_CLICK);
        EventBus::instance().sendToMain(clickEvent);
        Serial.println("🖱️ Encoder clicked (from UI task)");
      }
      
      // Check for long press
      if (encoder->getLongPressed()) {
        Event longPressEvent(EventType::ENCODER_LONG_PRESS);
        EventBus::instance().sendToMain(longPressEvent);
        Serial.println("⏱️ Encoder long pressed (from UI task)");
      }
    }

    // Update display
    if (display) {
      display->update();
    }
    
    // 30Hz update rate (33ms)
    vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(33));
  }
}

void Core::checkHealth() {
  uint32_t freeHeap = ESP.getFreeHeap();
  uint32_t minFreeHeap = ESP.getMinFreeHeap();
  
  Serial.printf("🏥 Health: Free=%d, Min=%d, Tasks=%d, Cycles=%d, Events=%d\n",
                freeHeap, minFreeHeap, uxTaskGetNumberOfTasks(), 
                coordinationCycles, eventsProcessed);
  
  // Check UI task stack
  if (uiTaskHandle) {
    UBaseType_t uiStack = uxTaskGetStackHighWaterMark(uiTaskHandle);
    Serial.printf("🎮 UI Task stack remaining: %d bytes\n", uiStack * sizeof(StackType_t));
  }
  
  // Check LED task stack
  if (ledManager && ledManager->getAnimationTaskHandle()) {
    UBaseType_t ledStack = uxTaskGetStackHighWaterMark(ledManager->getAnimationTaskHandle());
    Serial.printf("💡 LED Task stack remaining: %d bytes\n", ledStack * sizeof(StackType_t));
    
    // Restart LED task if stack is critically low
    if (ledStack < 512) {
      Serial.println("⚠️ LED Task stack critically low - restarting task");
      ledManager->restartAnimationTask();
    }
  }
  
  // Log event bus status
  EventBus::instance().logStatus();
  
  // Low memory warning
  if (freeHeap < 50000) {
    Serial.println("⚠️ LOW MEMORY WARNING!");
  }
}

void Core::processSerialCommands() {
  static String commandBuffer = "";
  
  // Read all available serial data
  while (Serial.available() > 0) {
    char c = Serial.read();
    
    if (c == '\n' || c == '\r') {
      // Command complete - process it
      if (commandBuffer.length() > 0) {
        commandBuffer.trim();
        commandBuffer.toLowerCase();
        
        Serial.printf("\n💬 Command received: '%s'\n", commandBuffer.c_str());
        
        if (commandBuffer == "get uuid") {
          String uuid = GET_DEVICE_UUID();
          String deviceId = GET_DEVICE_ID();
          String mac = DeviceID::getMACAddress();
          
          // Output in formato JSON per parsing facile
          Serial.println("\n📱 DEVICE_INFO_START");
          Serial.println("{");
          Serial.printf("  \"uuid\": \"%s\",\n", uuid.c_str());
          Serial.printf("  \"device_id\": \"%s\",\n", deviceId.c_str());
          Serial.printf("  \"mac_address\": \"%s\",\n", mac.c_str());
          Serial.printf("  \"pcb_version\": %d,\n", PCB_VERSION);
          Serial.printf("  \"firmware_version\": \"%s\",\n", FIRMWARE_VERSION);
          Serial.printf("  \"chip_model\": \"%s\",\n", ESP.getChipModel());
          Serial.printf("  \"chip_revision\": %d\n", ESP.getChipRevision());
          Serial.println("}");
          Serial.println("📱 DEVICE_INFO_END\n");
          
        } else if (commandBuffer == "reboot") {
          Serial.println("🔄 Rebooting device...");
          Serial.flush();
          delay(500);
          ESP.restart();
          
        } else if (commandBuffer == "hard reset") {
          Serial.println("🗑️ Performing hard reset - clearing all preferences...");
          prefs.clearAll();
          Serial.println("✅ Preferences cleared!");
          Serial.println("🔄 Rebooting device...");
          Serial.flush();
          delay(500);
          ESP.restart();
          
        } else if (commandBuffer == "help") {
          Serial.println("\n📋 Available commands:");
          Serial.println("  reboot      - Reboot the device");
          Serial.println("  hard reset  - Clear all preferences and reboot");
          Serial.println("  status      - Show system status");
          Serial.println("  help        - Show this help message\n");
          
        } else if (commandBuffer == "status") {
          Serial.println("\n📊 System Status:");
          Serial.printf("  State: %d\n", (int)currentState);
          Serial.printf("  Uptime: %lu seconds\n", millis() / 1000);
          Serial.printf("  Free Heap: %d bytes\n", ESP.getFreeHeap());
          Serial.printf("  Free PSRAM: %d bytes\n", ESP.getFreePsram());
          Serial.printf("  Coordination Cycles: %d\n", coordinationCycles);
          Serial.printf("  Events Processed: %d\n", eventsProcessed);
          if (wifi) {
            Serial.printf("  WiFi State: %d\n", (int)wifi->getState());
            if (wifi->isConnected()) {
              Serial.printf("  WiFi SSID: %s\n", wifi->getSSID().c_str());
              Serial.printf("  WiFi IP: %s\n", wifi->getLocalIP().c_str());
              Serial.printf("  WiFi RSSI: %d dBm\n", wifi->getRSSI());
            }
          }
          Serial.println();
          
        } else {
          Serial.printf("❌ Unknown command: '%s'\n", commandBuffer.c_str());
          Serial.println("   Type 'help' for available commands\n");
        }
        
        commandBuffer = "";
      }
    } else {
      // Add character to buffer
      commandBuffer += c;
    }
  }
}

}  // namespace CloudMouse