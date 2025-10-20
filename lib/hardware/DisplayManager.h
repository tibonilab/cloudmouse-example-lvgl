// hardware/DisplayManager.h - CloudMouse Boilerplate
#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include "LGFX_ILI9488.h"
#include "../core/Events.h"
#include "../helper/QRCodeManager.h"
#include "../config/DeviceConfig.h"
#include <Arduino.h>

class DisplayManager {
public:
  DisplayManager();
  ~DisplayManager();

  void init();
  void update();
  
  // Event processing (called from UI Task)
  void processEvent(const CloudMouse::Event& event);

  // Display queries
  bool isReady() const { return initialized; }
  int getWidth() const { return 480; }
  int getHeight() const { return 320; }

private:
  // Screen states
  enum class Screen {
    HELLO_WORLD,          // Default screen with encoder info
    WIFI_CONNECTING,      // Connecting animation with spinner
    WIFI_AP_MODE,         // QR code to connect to AP WiFi
    WIFI_AP_CONNECTED     // QR code for setup URL (client connected)
  };

  LGFX_ILI9488 display;
  LGFX_Sprite* sprite = nullptr;
  QRCodeManager qrCodeManager;

  bool initialized = false;

  // Current screen
  Screen currentScreen = Screen::HELLO_WORLD;
  bool needsRedraw = true;

  // Current state
  int lastEncoderValue = 0;
  unsigned long lastClickTime = 0;
  unsigned long lastLongPressTime = 0;
  
  // Colors
  static const uint16_t COLOR_BG = 0x0000;       // Black
  static const uint16_t COLOR_TEXT = 0xFFFF;     // White
  static const uint16_t COLOR_ACCENT = 0x07FF;   // Cyan
  static const uint16_t COLOR_SUCCESS = 0x07E0;  // Green
  static const uint16_t COLOR_WARNING = 0xFD20;  // Orange
  
  unsigned long lastWiFiConnectingRender = 0;
  const unsigned long WIFI_CONNECTING_RENDER_INTERVAL = 100;  // 100ms = 10 FPS


  bool inAPMode = false;

  // Rendering methods
  void renderHelloWorld();
  void renderAPMode();
  void renderAPConnected();
  void renderWiFiConnecting();
  void pushSprite();
  
  // UI Elements
  void drawHeader(const String& title);
  void drawCenteredText(const String& text, int y, uint16_t color = COLOR_TEXT);
  void drawInstructions();
};

#endif