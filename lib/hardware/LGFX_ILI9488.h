#ifndef LGFX_ILI9488_H
#define LGFX_ILI9488_H

#include <LovyanGFX.hpp>
#include "../config/DeviceConfig.h"

#define TFT_BL 8
#define TFT_PWR 1

#ifndef HSPI_HOST
#define HSPI_HOST SPI2_HOST
#endif

class LGFX_ILI9488 : public lgfx::LGFX_Device {
  lgfx::Panel_ILI9488 _panel_instance;
  lgfx::Bus_SPI _bus_instance;
  lgfx::Light_PWM _light_instance;

public:
  LGFX_ILI9488(void) {
    // Configurazione bus SPI
    auto bus_cfg = _bus_instance.config();
    bus_cfg.spi_host = HSPI_HOST;
    bus_cfg.spi_mode = 0;
    bus_cfg.freq_write = 40000000;
    bus_cfg.freq_read = 16000000;
    bus_cfg.spi_3wire = false;
    bus_cfg.use_lock = true;
    bus_cfg.dma_channel = 1;
    bus_cfg.pin_sclk = 6;
    bus_cfg.pin_mosi = 7;
    bus_cfg.pin_miso = -1;
    bus_cfg.pin_dc = 5;
    _bus_instance.config(bus_cfg);
    _panel_instance.setBus(&_bus_instance);

    // Configurazione pannello
    auto panel_cfg = _panel_instance.config();
    panel_cfg.pin_cs = 4;
    panel_cfg.pin_rst = 21;
    panel_cfg.pin_busy = -1;
    panel_cfg.memory_width = 480;
    panel_cfg.memory_height = 320;
    panel_cfg.panel_width = 480;
    panel_cfg.panel_height = 320;
    panel_cfg.offset_x = 0;
    panel_cfg.offset_y = 0;
    panel_cfg.offset_rotation = 0;
    panel_cfg.dummy_read_pixel = 8;
    panel_cfg.dummy_read_bits = 1;
    panel_cfg.readable = false;
    panel_cfg.invert = false;
    panel_cfg.rgb_order = false;
    panel_cfg.dlen_16bit = false;
    panel_cfg.bus_shared = true;
    _panel_instance.config(panel_cfg);

    // Configurazione retroilluminazione
    auto light_cfg = _light_instance.config();
    light_cfg.pin_bl = 8;
    light_cfg.invert = false;
    light_cfg.freq = 5000;
    light_cfg.pwm_channel = 7;
    _light_instance.config(light_cfg);
    _panel_instance.setLight(&_light_instance);

    setPanel(&_panel_instance);
  }

  void setMyRotation(uint8_t rot = 0) {
    startWrite();
    uint8_t madctl;
    switch (rot & 3) {
      case 0:           // 0°
        madctl = 0x48;  // MY=0, MX=0, MV=0, BGR=1 (classico portrait)
        break;
      case 1:           // 90°
        madctl = 0x28;  // MY=0, MX=1, MV=1, BGR=1
        break;
      case 2:           // 180°
        madctl = 0x88;  // MY=1, MX=1, MV=0, BGR=1
        break;
      case 3:           // 270°
        madctl = 0xE8;  // MY=1, MX=0, MV=1, BGR=1
        break;
    }
    // invert BGR color order
    _panel_instance.writeCommand(0x21, 1);

    // set orientation command
    _panel_instance.writeCommand(0x36, 1);  // MADCTL
    // _panel_instance.writeData(madctl, 1);

    // orientation landscape with BGR
    _panel_instance.writeData(0x60 | 0x08, 1);
    endWrite();
  }

  void init() {

    if (TFT_PWR >= 0) {
      pinMode(TFT_PWR, OUTPUT);
      if (PCB_VERSION == 4) {
        digitalWrite(TFT_PWR, LOW);
      }

      if (PCB_VERSION == 5) {
        digitalWrite(TFT_PWR, HIGH);
      }
    }

    begin();
    setMyRotation();
    fillScreen(TFT_BLACK);

    setBrightness(100);
  }
};

#endif