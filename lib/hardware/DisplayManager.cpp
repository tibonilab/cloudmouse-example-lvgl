// hardware/DisplayManager.cpp - CloudMouse Boilerplate
#include "./DisplayManager.h"
#include "../core/EventBus.h"

using namespace CloudMouse;

DisplayManager::DisplayManager() {
  // Constructor
}

DisplayManager::~DisplayManager() {
  if (sprite) {
    delete sprite;
    sprite = nullptr;
  }
}

void DisplayManager::init() {
  Serial.println("🖥️ Initializing DisplayManager...");

  // Initialize display hardware
  display.init();
  display.setBrightness(200);  // 0-255

  // Create sprite buffer in PSRAM
  sprite = new LGFX_Sprite(&display);
  sprite->setPsram(true);          // Use PSRAM for buffer
  sprite->setColorDepth(16);       // 16-bit color (RGB565)
  sprite->createSprite(480, 320);  // Full screen buffer

  if (sprite) {
    initialized = true;
    Serial.println("✅ Sprite buffer created (480x320, 16-bit)");
    
    // Initialize QR code manager
    qrCodeManager.init(sprite);
    
    // Set default text properties
    sprite->setTextColor(COLOR_TEXT);
    sprite->setTextDatum(MC_DATUM);  // Middle-center
    
  } else {
    Serial.println("❌ Failed to create sprite buffer!");
    return;
  }

  Serial.printf("✅ DisplayManager initialized (resolution: %dx%d)\n", getWidth(), getHeight());
  Serial.printf("   Memory used by sprite: %d bytes\n", 480 * 320 * 2);
  Serial.printf("   Free PSRAM: %d bytes\n", ESP.getFreePsram());
}

void DisplayManager::update() {
  // Process events from main task
  Event event;
  while (EventBus::instance().receiveFromMain(event, 0)) {
    processEvent(event);
  }

  // Keep animation running for connecting screen
  if (needsRedraw && currentScreen == Screen::WIFI_CONNECTING) {
    static unsigned long lastAnimationUpdate = 0;
    
    if (millis() - lastAnimationUpdate >= 100) {
      renderWiFiConnecting();
      lastAnimationUpdate = millis();
    }
  }
}

void DisplayManager::processEvent(const Event& event) {
  switch (event.type) {
    case EventType::DISPLAY_WAKE_UP:
      Serial.println("📺 Display wake up");
      currentScreen = Screen::HELLO_WORLD;
      needsRedraw = true;
      renderHelloWorld();
      break;
      
    case EventType::DISPLAY_WIFI_CONNECTING:
      Serial.println("📡 Display: Showing WiFi connecting screen");
      currentScreen = Screen::WIFI_CONNECTING;
      needsRedraw = true;
      renderWiFiConnecting();
      break;
      
    case EventType::ENCODER_ROTATION:
      Serial.printf("🔄 Display received rotation: %d\n", event.value);
      lastEncoderValue = event.value;
      currentScreen = Screen::HELLO_WORLD;
      needsRedraw = false;  // Stop animations
      lastWiFiConnectingRender = 0;
      renderHelloWorld();
      break;
      
    case EventType::ENCODER_CLICK:
      Serial.println("🖱️ Display received click");
      lastClickTime = millis();
      if (currentScreen == Screen::HELLO_WORLD) {
        renderHelloWorld();
      }
      break;
      
    case EventType::ENCODER_LONG_PRESS:
      Serial.println("⏱️ Display received long press");
      lastLongPressTime = millis();
      if (currentScreen == Screen::HELLO_WORLD) {
        renderHelloWorld();
      }
      break;
      
    case EventType::DISPLAY_WIFI_AP_MODE:
      Serial.println("📱 Showing AP Mode screen - Connect to WiFi");
      currentScreen = Screen::WIFI_AP_MODE;
      needsRedraw = false;  // Stop animations
      renderAPMode();
      break;
      
    case EventType::DISPLAY_WIFI_SETUP_URL:
      Serial.println("🌐 Showing Setup URL screen - Client connected!");
      currentScreen = Screen::WIFI_AP_CONNECTED;
      needsRedraw = false;  // Stop animations
      renderAPConnected();
      break;
      
    case EventType::DISPLAY_CLEAR:
      sprite->fillSprite(COLOR_BG);
      pushSprite();
      break;
      
    default:
      break;
  }
}

void DisplayManager::renderHelloWorld() {
  if (!sprite) return;

  // Clear screen
  sprite->fillSprite(COLOR_BG);
  
  // Draw header
  drawHeader("CloudMouse Boilerplate");
  
  // Main title
  sprite->setTextSize(3);
  drawCenteredText("Hello CloudMouse!", 100, COLOR_ACCENT);
  
  // Encoder status
  sprite->setTextSize(2);
  
  // Check for recent click or long press (within 2 seconds)
  if (millis() - lastClickTime < 2000 && lastClickTime > 0) {
    drawCenteredText("Button Clicked!", 160, COLOR_SUCCESS);
  } else if (millis() - lastLongPressTime < 2000 && lastLongPressTime > 0) {
    drawCenteredText("Long Press!", 160, COLOR_WARNING);
  } else if (lastEncoderValue > 0) {
    drawCenteredText("Rotating RIGHT", 160, COLOR_TEXT);
  } else if (lastEncoderValue < 0) {
    drawCenteredText("Rotating LEFT", 160, COLOR_TEXT);
  } else {
    drawCenteredText("Ready!", 160, COLOR_TEXT);
  }
  
  // Instructions
  drawInstructions();
  
  // Push to display
  pushSprite();
}

void DisplayManager::renderWiFiConnecting() {
  if (!sprite) return;

  // Clear screen
  sprite->fillSprite(COLOR_BG);
  
  // Draw header
  drawHeader("CloudMouse Boilerplate");
  
  // Title
  sprite->setTextSize(3);
  drawCenteredText("Connecting to WiFi", 120, COLOR_ACCENT);
  
  // Animated dots
  static int dotCount = 0;
  static unsigned long lastDotUpdate = 0;
  
  if (millis() - lastDotUpdate > 500) {
    lastDotUpdate = millis();
    dotCount = (dotCount + 1) % 4;
  }
  
  String dots = "";
  for (int i = 0; i < dotCount; i++) {
    dots += ".";
  }
  
  sprite->setTextSize(2);
  drawCenteredText("Please wait" + dots, 170, COLOR_TEXT);
  
  // Spinner animation
  int centerX = 240;
  int centerY = 230;
  int radius = 20;
  int frame = (millis() / 100) % 12;
  
  for (int i = 0; i < 12; i++) {
    float angle = (i * 30) * PI / 180.0;
    int x = centerX + cos(angle) * radius;
    int y = centerY + sin(angle) * radius;
    
    uint16_t color = (i == frame) ? COLOR_ACCENT : 0x4208;  // Bright or dim
    sprite->fillCircle(x, y, 3, color);
  }
  
  // Push to display
  pushSprite();
}

void DisplayManager::renderAPMode() {
  if (!sprite) return;

  // Clear screen
  sprite->fillSprite(COLOR_BG);
  
  // Draw header
  drawHeader("WiFi Setup Required");
  
  // Title
  sprite->setTextSize(2);
  drawCenteredText("Connect to CloudMouse", 60, COLOR_ACCENT);
  
  // AP Info
  sprite->setTextSize(1.5);
  String apSSID = GET_AP_SSID();
  String apPassword = GET_AP_PASSWORD();
  
  sprite->setTextSize(1.5);
  drawCenteredText("WiFi Network:", 90, COLOR_TEXT);
  drawCenteredText(apSSID, 115, COLOR_SUCCESS);
  drawCenteredText(apPassword, 135, COLOR_SUCCESS);

  // QR Code
  int qrX = 154;  // Center horizontally (480 - QR size) / 2
  int qrY = 154;
  int qrPixelSize = 4;
  
  // Generate QR with AP connection string
  String qrData = String("WIFI:T:WPA;S:") + apSSID + ";P:" + apPassword + ";;";
  qrCodeManager.setOffset(qrX, qrY);
  qrCodeManager.setPixelSide(qrPixelSize);
  qrCodeManager.create(qrData.c_str());
  
  // IP address
  sprite->setTextSize(1.5);
  // drawCenteredText("or visit: " + apIP, 290, COLOR_TEXT);
  
  // Push to display
  pushSprite();
}

void DisplayManager::renderAPConnected() {
  if (!sprite) return;

  // Clear screen
  sprite->fillSprite(TFT_DARKGREEN);
  
  // Draw header
  drawHeader("WiFi Configuration");
  
  // Title
  sprite->setTextSize(2);
  drawCenteredText("✅ Connected!", 60, COLOR_SUCCESS);
  
  sprite->setTextSize(1.5);
  drawCenteredText("Scan QR to setup WiFi", 100, COLOR_TEXT);
  
  // QR Code for setup URL
  int qrSize = 172;
  int qrX = (480 - qrSize) / 2;
  int qrY = 130;
  int qrPixelSize = 4;
  
  // Generate QR with setup URL
  qrCodeManager.setOffset(qrX, qrY);
  qrCodeManager.setPixelSide(qrPixelSize);
  qrCodeManager.create(WIFI_CONFIG_SERVICE);
  
  // Show URL
  sprite->setTextSize(1.5);
  drawCenteredText(WIFI_CONFIG_SERVICE, 290, COLOR_TEXT);
  
  // Push to display
  pushSprite();
}

void DisplayManager::drawHeader(const String& title) {
  // Header background
  sprite->fillRect(0, 0, 480, 50, 0x0010);  // Dark blue
  sprite->drawFastHLine(0, 50, 480, 0x001F); // Blue line
  
  // Header text
  sprite->setTextSize(2);
  sprite->setTextColor(COLOR_TEXT);
  sprite->setTextDatum(MC_DATUM);
  sprite->drawString(title, 240, 25);
}

void DisplayManager::drawCenteredText(const String& text, int y, uint16_t color) {
  sprite->setTextColor(color);
  sprite->setTextDatum(MC_DATUM);
  sprite->drawString(text, 240, y);
}

void DisplayManager::drawInstructions() {
  sprite->setTextSize(1);
  sprite->setTextColor(0x7BEF);  // Light gray
  
  int startY = 220;
  int lineHeight = 20;
  
  drawCenteredText("Rotate: Turn encoder left/right", startY, 0x7BEF);
  drawCenteredText("Click: Press encoder button", startY + lineHeight, 0x7BEF);
  drawCenteredText("Long Press: Hold encoder button", startY + lineHeight * 2, 0x7BEF);
}

void DisplayManager::pushSprite() {
  if (sprite) {
    sprite->pushSprite(0, 0);
  }
}