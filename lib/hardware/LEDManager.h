// hardware/LEDManager.h
#pragma once
#include <Adafruit_NeoPixel.h>
#include "../prefs/PreferencesManager.h"
#include "../core/Events.h"

#define NUM_LEDS 12
#define DATA_PIN 15

namespace CloudMouse {

// 🆕 LED Events per comunicazione con Core
enum class LEDEventType {
    SET_LOADING,        // on/off
    FLASH_COLOR,        // r, g, b, brightness, duration
    ACTIVATE,           // encoder movement detected
    SET_COLOR,          // change main color
    SET_BRIGHTNESS      // change brightness
};

struct LEDEvent {
    LEDEventType type;
    uint8_t r, g, b;
    int brightness;
    int duration;
    bool state;
};

class LEDManager {
public:
    LEDManager();
    ~LEDManager() = default;
    
    void init();
    void startAnimationTask();
    void stopAnimationTask();
    
    // 🆕 Event-based interface (thread-safe)
    void setLoadingState(bool on);
    void flashColor(uint8_t r, uint8_t g, uint8_t b, int brightness, int duration);
    void activate();
    void updateLastEncoderMovementTime();
    void setMainColor(const String& colorName = "");
    
    // State queries
    bool isLoading() const { return loading; }
    bool isPulsating() const { return pulsating; }
    int getBrightness() const { return currentBrightness; }

    bool isInitAnimationComplete() const { return initAnimationCompleted; }

    TaskHandle_t getAnimationTaskHandle() const { return animationTaskHandle; }
    void restartAnimationTask();


private:
    // Hardware
    Adafruit_NeoPixel strip;
    
    // Task management
    TaskHandle_t animationTaskHandle = nullptr;
    QueueHandle_t ledEventQueue = nullptr;
    static const int LED_QUEUE_SIZE = 10;
    
    // Animation state
    bool pulsating = true;
    bool loading = false;
    bool flash = false;
    bool fading = false;
    bool inited = false;
    bool initAnimationCompleted = false;
    
    // Timing
    unsigned long previousMillis = 0;
    unsigned long lastEncMovementTime = 0;
    unsigned long lastFlashStarted = 0;
    unsigned long fadeStartMillis = 0;
    
    // Animation parameters  
    int cursorLED = 0;
    int currentBrightness = 250;
    int startBrightness = 0;
    int targetBrightness = 0;
    int fadeDuration = 3000;
    int flashDuration = 0;
    bool clockwise = true;
    
    // Colors
    uint8_t baseRed = 0, baseGreen = 181, baseBlue = 214;  // Default azure
    uint8_t red = 0, green = 0, blue = 0;
    
    // Constants
    static const unsigned long ANIMATION_INTERVAL = 70;
    static const int IDLE_DELAY_SECONDS = 5;
    
    // 🆕 Static task function
    static void animationTaskFunction(void* parameter);
    
    // 🆕 Core animation loop
    void animationLoop();
    void processLEDEvents();
    void updateAnimations();
    void updateInitAnimation();
    void updatePulsatingAnimation();
    void updateLoadingAnimation();
    void updateFlashAnimation();
    void updateFadeAnimation();
    
    // Helper functions
    void resetAllLEDs();
    void setAllLEDs(uint8_t r, uint8_t g, uint8_t b);
    void fadeToBrightness(int brightness, int duration);
    void applyColorFromPreferences(const String& colorName);
    
    // 🆕 Thread-safe event sending
    bool sendLEDEvent(const LEDEvent& event);
};

} // namespace CloudMouse
