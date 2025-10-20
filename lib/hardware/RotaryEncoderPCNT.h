#ifndef ROTARY_ENCODER_PCNT_H
#define ROTARY_ENCODER_PCNT_H

#include <Arduino.h>
#include "esp_err.h"

// Detect which ESP-IDF version we're using
#ifdef PLATFORMIO
  // PlatformIO uses ESP-IDF 4.4 - old API
  #include "driver/pcnt.h"
  #define USE_OLD_PCNT_API
#else
  // Arduino IDE uses ESP-IDF 5.x - new API
  #include "driver/pulse_cnt.h"
  #define USE_NEW_PCNT_API
#endif

#define START_POS_DEFAULT 0
#define GLITCH_NS_DEFAULT 1000

class RotaryEncoderPCNT {
public:
  RotaryEncoderPCNT(int a, int b, int start_pos, uint16_t glitch_ns) {
    glitch_time = glitch_ns;
    offset = start_pos;
    pin_a = a;
    pin_b = b;
  }

  RotaryEncoderPCNT(int a, int b, int start_pos) {
    offset = start_pos;
    pin_a = a;
    pin_b = b;
  }

  RotaryEncoderPCNT(int a, int b) {
    pin_a = a;
    pin_b = b;
  }

  RotaryEncoderPCNT() {}

  ~RotaryEncoderPCNT() {
    deinit();
  }
  
  void init() {
#ifdef USE_OLD_PCNT_API
    // ============ ESP-IDF 4.4 API (PlatformIO) ============
    
    Serial.printf("🔧 Initializing PCNT on pins A=%d, B=%d\n", pin_a, pin_b);
    
    pinMode(pin_a, INPUT_PULLUP);
    pinMode(pin_b, INPUT_PULLUP);

    // Configure PCNT unit for channel 0 (pin A)
    pcnt_config_t pcnt_config_a;
    pcnt_config_a.pulse_gpio_num = pin_a;
    pcnt_config_a.ctrl_gpio_num = pin_b;
    pcnt_config_a.lctrl_mode = PCNT_MODE_KEEP;
    pcnt_config_a.hctrl_mode = PCNT_MODE_REVERSE;
    pcnt_config_a.pos_mode = PCNT_COUNT_INC;
    pcnt_config_a.neg_mode = PCNT_COUNT_DEC;
    pcnt_config_a.counter_h_lim = 32767;
    pcnt_config_a.counter_l_lim = -32768;
    pcnt_config_a.unit = PCNT_UNIT_0;
    pcnt_config_a.channel = PCNT_CHANNEL_0;
    
    esp_err_t err = pcnt_unit_config(&pcnt_config_a);
    if (err != ESP_OK) {
      Serial.printf("❌ PCNT channel 0 config failed: %d\n", err);
      return;
    }
    
    // Configure PCNT unit for channel 1 (pin B)
    pcnt_config_t pcnt_config_b;
    pcnt_config_b.pulse_gpio_num = pin_b;
    pcnt_config_b.ctrl_gpio_num = pin_a;
    pcnt_config_b.lctrl_mode = PCNT_MODE_KEEP;
    pcnt_config_b.hctrl_mode = PCNT_MODE_REVERSE;
    pcnt_config_b.pos_mode = PCNT_COUNT_DEC;
    pcnt_config_b.neg_mode = PCNT_COUNT_INC;
    pcnt_config_b.counter_h_lim = 32767;
    pcnt_config_b.counter_l_lim = -32768;
    pcnt_config_b.unit = PCNT_UNIT_0;
    pcnt_config_b.channel = PCNT_CHANNEL_1;
    
    err = pcnt_unit_config(&pcnt_config_b);
    if (err != ESP_OK) {
      Serial.printf("❌ PCNT channel 1 config failed: %d\n", err);
      return;
    }
    
    // Set glitch filter (if specified)
    if (glitch_time > 0) {
      err = pcnt_set_filter_value(PCNT_UNIT_0, glitch_time);
      if (err == ESP_OK) {
        pcnt_filter_enable(PCNT_UNIT_0);
        Serial.printf("✅ Glitch filter enabled: %dns\n", glitch_time);
      }
    }
    
    // Initialize counter
    pcnt_counter_pause(PCNT_UNIT_0);
    pcnt_counter_clear(PCNT_UNIT_0);
    pcnt_counter_resume(PCNT_UNIT_0);
    
    Serial.println("✅ RotaryEncoder initialized (IDF 4.4 API)");
    
#elif defined(USE_NEW_PCNT_API)
    // ============ ESP-IDF 5.x API (Arduino IDE) ============
    
    pcnt_unit_config_t unit_config = {
      .low_limit = low_limit,
      .high_limit = high_limit,
    };
    pcnt_new_unit(&unit_config, &unit);
    
    pcnt_glitch_filter_config_t filter_config = {
      .max_glitch_ns = glitch_time,
    };
    pcnt_unit_set_glitch_filter(unit, &filter_config);
    
    pcnt_chan_config_t chan_a_config = {
      .edge_gpio_num = pin_a,
      .level_gpio_num = pin_b,
    };
    pcnt_new_channel(unit, &chan_a_config, &chan_a);
    
    pcnt_chan_config_t chan_b_config = {
      .edge_gpio_num = pin_b,
      .level_gpio_num = pin_a,
    };
    pcnt_new_channel(unit, &chan_b_config, &chan_b);
    
    pcnt_channel_set_edge_action(chan_a, PCNT_CHANNEL_EDGE_ACTION_DECREASE, PCNT_CHANNEL_EDGE_ACTION_INCREASE);
    pcnt_channel_set_level_action(chan_a, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE);
    
    pcnt_channel_set_edge_action(chan_b, PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_DECREASE);
    pcnt_channel_set_level_action(chan_b, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE);
    
    pcnt_unit_enable(unit);
    pcnt_unit_clear_count(unit);
    pcnt_unit_start(unit);
    
    Serial.println("✅ RotaryEncoder initialized (IDF 5.x API)");
#endif
  }
  
  void deinit() {
#ifdef USE_OLD_PCNT_API
    pcnt_counter_pause(PCNT_UNIT_0);
    pcnt_counter_clear(PCNT_UNIT_0);
#elif defined(USE_NEW_PCNT_API)
    if (unit) {
      pcnt_unit_stop(unit);
      pcnt_unit_disable(unit);
      pcnt_del_channel(chan_a);
      pcnt_del_channel(chan_b);
      pcnt_del_unit(unit);
      unit = nullptr;
    }
#endif
  }
  
  int position() {
    int value = 0;
    
#ifdef USE_OLD_PCNT_API
    int16_t count = 0;
    esp_err_t err = pcnt_get_counter_value(PCNT_UNIT_0, &count);
    if (err == ESP_OK) {
      value = count + offset;
    } else {
      // Se c'è errore, ritorna l'ultimo valore valido
      static int last_valid = 0;
      value = last_valid;
    }
#elif defined(USE_NEW_PCNT_API)
    int temp_count = 0;
    if (unit) {
      pcnt_unit_get_count(unit, &temp_count);
      value = temp_count + offset;
    }
#endif
    
    return value;
  }
  
  void setPosition(int pos) {
    offset = pos;
#ifdef USE_OLD_PCNT_API
    pcnt_counter_clear(PCNT_UNIT_0);
#elif defined(USE_NEW_PCNT_API)
    pcnt_unit_clear_count(unit);
#endif
  }
  
  void zero() {
    setPosition(START_POS_DEFAULT);
  }
  
  uint8_t pin_a = 255;
  uint8_t pin_b = 255;
  uint16_t glitch_time = GLITCH_NS_DEFAULT;

private:
#ifdef USE_NEW_PCNT_API
  pcnt_unit_handle_t unit = nullptr;
  pcnt_channel_handle_t chan_a = nullptr;
  pcnt_channel_handle_t chan_b = nullptr;
#endif
  
  int16_t low_limit = INT16_MIN;
  int16_t high_limit = INT16_MAX;
  int count = 0;
  int offset = START_POS_DEFAULT;
};

#endif