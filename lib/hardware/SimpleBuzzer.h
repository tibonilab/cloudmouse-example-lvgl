#ifndef SIMPLE_BUZZER_H
#define SIMPLE_BUZZER_H

#define BUZZER_PIN 14

class SimpleBuzzer {
  public:
    static void init() {
      pinMode(BUZZER_PIN, OUTPUT);
    }

    static void buzz() {
      SimpleBuzzer::buzzWithPWM(740, 75, 20);
      SimpleBuzzer::buzzWithPWM(120, 75, 20);
      SimpleBuzzer::buzzWithPWM(270, 75, 20);
    }

    static void error() {
      SimpleBuzzer::buzzWithPWM(230, 75, 20);
      SimpleBuzzer::buzzWithPWM(120, 75, 20);
      SimpleBuzzer::buzzWithPWM(230, 75, 20);
      SimpleBuzzer::buzzWithPWM(120, 75, 20);
      SimpleBuzzer::buzzWithPWM(230, 75, 20);
      SimpleBuzzer::buzzWithPWM(120, 75, 20);
    }

    static void buzzWithPWM(int frequency, int duration, int dutyCycle) {
      int period = 1000000 / frequency;  // Periodo in microsecondi
      int pulse = (period * dutyCycle) / 100;  // Durata del ciclo on

      unsigned long startTime = millis();
      while (millis() - startTime < duration) {
        digitalWrite(BUZZER_PIN, HIGH);
        delayMicroseconds(pulse);   // Acceso per la parte on del ciclo
        digitalWrite(BUZZER_PIN, LOW);
        delayMicroseconds(period - pulse);  // Spento per il resto del ciclo
      }
    }
};

#endif
