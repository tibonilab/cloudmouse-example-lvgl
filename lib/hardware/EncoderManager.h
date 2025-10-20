#ifndef ENCODERMANAGER_H
#define ENCODERMANAGER_H

#define ENCODER_CLK_PIN 16
#define ENCODER_DT_PIN 18
#define ENCODER_SW_PIN 17

#ifdef PLATFORMIO
  #include "./RotaryEncoderPCNT.h"
#else
  #include <RotaryEncoderPCNT.h>
#endif

#include <Arduino.h>
#include "SimpleBuzzer.h"

class EncoderManager {
public:
  EncoderManager();
  
  // Initialization
  void init();
  void update();

  // Event consumption (IMPORTANT: These consume events!)
  int getMovement();           // Returns movement and resets to 0
  bool getClicked();          // Returns click state and resets
  bool getLongPressed();      // Returns long press state and resets  
  bool getUltraLongPressed(); // Returns ultra long press state and resets
  
  // State queries (non-consuming)
  bool isButtonDown() const;
  int getPressTime() const;
  int getLastPressDuration() const;

private:
  RotaryEncoderPCNT encoder;
  
  // Encoder state
  int lastValue = 0;
  int movement = 0;
  bool movementPending = false;  // 🆕 QUESTA MANCAVA!
  
  // Button state
  bool lastButtonState = HIGH;
  unsigned long pressStartTime = 0;      // 🆕 QUESTA MANCAVA!
  int lastPressDuration = 0;
  
  // Button event flags (🆕 QUESTE MANCAVANO TUTTE!)
  bool clickPending = false;
  bool longPressPending = false;
  bool ultraLongPressPending = false;
  bool longPressBuzzed = false;
  bool ultraLongPressNotified = false;
  
  // Configuration (🆕 QUESTE MANCAVANO!)
  static const int ULTRA_LONG_PRESS_DURATION = 3000;
  static const int LONG_PRESS_DURATION = 1000; 
  static const unsigned long CLICK_TIMEOUT = 500;
  
  // Helper methods
  void processEncoder();
  void processButton();
  unsigned long getCurrentPressTime() const;
};

#endif