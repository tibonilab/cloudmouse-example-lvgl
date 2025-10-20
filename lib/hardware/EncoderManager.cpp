#include "./EncoderManager.h"

EncoderManager::EncoderManager() 
  : encoder(ENCODER_CLK_PIN, ENCODER_DT_PIN) {
  // Constructor minimal - real init in init()
}

void EncoderManager::init() {
  Serial.println("🎮 Initializing EncoderManager...");
  
  pinMode(ENCODER_SW_PIN, INPUT_PULLUP);
  
  // Initialize encoder
  encoder.init();
  lastValue = encoder.position() / 4;
  
  // Initialize button state
  lastButtonState = digitalRead(ENCODER_SW_PIN);
  
  Serial.printf("✅ EncoderManager initialized (pins: CLK=%d, DT=%d, SW=%d)\n", 
                ENCODER_CLK_PIN, ENCODER_DT_PIN, ENCODER_SW_PIN);
}

void EncoderManager::update() {
  processEncoder();
  processButton();
}

void EncoderManager::processEncoder() {
  int newValue = encoder.position() / 4;

  if (newValue != lastValue) {
    movement += newValue - lastValue;  // 🆕 Accumulate movement
    lastValue = newValue;
    movementPending = true;
    
    // Optional: Log for debugging
    // Serial.printf("🔄 Encoder: %d (total pending: %d)\n", newValue - lastValue, movement);
  }
}

void EncoderManager::processButton() {
  bool currentButtonState = digitalRead(ENCODER_SW_PIN);
  unsigned long currentTime = millis();
  
  // Button press detected (HIGH → LOW transition)
  if (currentButtonState != lastButtonState && currentButtonState == LOW) {
    pressStartTime = currentTime;
    longPressBuzzed = false;  // Reset buzz flag for new press
  }
  
  // Button release detected (LOW → HIGH transition)  
  if (currentButtonState != lastButtonState && currentButtonState == HIGH) {
    unsigned long pressDuration = currentTime - pressStartTime;
    lastPressDuration = pressDuration;
    
    Serial.printf("👆 Button released after %lu ms\n", pressDuration);
    
    // Determine event type based on duration
    if (pressDuration >= ULTRA_LONG_PRESS_DURATION) {
      if (!ultraLongPressNotified) {
        ultraLongPressPending = true;
        ultraLongPressNotified = true;
        Serial.println("👆🔒🔒 Ultra long press event!");
      }
    } else if (pressDuration >= LONG_PRESS_DURATION) {
      longPressPending = true;
      Serial.println("👆🔒 Long press event!");
    } else if (pressDuration < CLICK_TIMEOUT) {
      clickPending = true;
      Serial.println("👆 Click event!");
    }
  }
  
  // Handle ongoing press feedback
  if (currentButtonState == LOW) {
    unsigned long pressTime = getCurrentPressTime();
    
    // Long press buzz (one time only)
    if (pressTime >= LONG_PRESS_DURATION && !longPressBuzzed) {
      // SimpleBuzzer::buzz();
      // longPressBuzzed = true;
      // Serial.println("🔊 Long press buzz");
    }
    
    // Ultra long press notification (one time only)
    if (pressTime >= ULTRA_LONG_PRESS_DURATION && !ultraLongPressNotified) {
      ultraLongPressPending = true;
      ultraLongPressNotified = true;
      Serial.println("👆🔒🔒 Ultra long press triggered!");
    }
  } else {
    // Button released - reset ultra long press flag
    ultraLongPressNotified = false;
  }
  
  lastButtonState = currentButtonState;
}

// 🆕 EVENT CONSUMPTION METHODS (One-shot, auto-reset)
int EncoderManager::getMovement() {
  if (movementPending) {
    int result = movement;
    movement = 0;
    movementPending = false;
    return result;
  }
  return 0;
}

bool EncoderManager::getClicked() {
  if (clickPending) {
    clickPending = false;
    return true;
  }
  return false;
}

bool EncoderManager::getLongPressed() {
  if (longPressPending) {
    longPressPending = false; 
    return true;
  }
  return false;
}

bool EncoderManager::getUltraLongPressed() {
  if (ultraLongPressPending) {
    ultraLongPressPending = false;
    return true; 
  }
  return false;
}

// 🆕 STATE QUERY METHODS (Non-consuming)
bool EncoderManager::isButtonDown() const {
  return digitalRead(ENCODER_SW_PIN) == LOW;
}

int EncoderManager::getPressTime() const {
  return getCurrentPressTime();
}

int EncoderManager::getLastPressDuration() const {
  return lastPressDuration;
}

unsigned long EncoderManager::getCurrentPressTime() const {
  if (digitalRead(ENCODER_SW_PIN) == LOW) {
    return millis() - pressStartTime;
  }
  return 0;
}
