# CloudMouse LVGL Example

<<<<<<< HEAD
🎨 **Example implementation of LVGL graphics library on top of CloudMouse SDK**
=======
**Event-driven dual-core firmware for [CloudMouse](https://cloudmouse.co) hardware**
>>>>>>> upstream/main

This project demonstrates how to integrate [LVGL (Light and Versatile Graphics Library)](https://lvgl.io/) with the [CloudMouse SDK](https://github.com/yourusername/cloudmouse-sdk) to create rich, interactive user interfaces on the ESP32-based CloudMouse device.

## 📋 Overview

This example extends the CloudMouse SDK boilerplate by replacing the basic display rendering with LVGL's powerful widget system. It showcases:

- Multi-screen navigation (Hello World, WiFi Setup, AP Mode, QR Code displays)
- Encoder integration with LVGL input system
- Event-driven architecture between hardware and UI
- Proper LVGL v9 implementation with ESP32 dual-core support

## 🔧 Compatibility

**Arduino IDE** - out-of-the-box.
**Platformio** - with source code switching (see below!).

### Important: Source Code Switching

<<<<<<< HEAD
> 💡 The project maintains a single codebase that works with both Arduino IDE and PlatformIO. The `src/main.cpp` file is kept in sync but needs to be toggled:
=======
### Event System
Communication between cores happens via FreeRTOS queues:
- `uiToMainQueue` - Events from UI task to Main task
- `mainToUIQueue` - Events from Main task to UI task

This ensures thread-safe operation without race conditions.

## 📦 Required Libraries

### For Arduino IDE
Install these via Arduino Library Manager:

**Core Libraries**
- **LovyanGFX** v1.2.7+ - Display driver for ILI9488
- **Adafruit NeoPixel** v1.12.0+ - WS2812B LED control
- **RotaryEncoderPCNT** v1.1.0+ - Hardware encoder support (vickash/RotaryEncoderPCNT)

**Networking & Communication**
- **ArduinoJson** v7.0.0+ - JSON parsing
- **AsyncTCP** v3.3.2+ - Async TCP for ESP32
- **ESPAsyncWebServer** v3.6.0+ - Async web server
- **ESP32 BLE Keyboard** v0.3.2+ - BLE Keyboard abstraction for Arduino (included in this repo see below!)

**Utilities**
- **QRCode** v0.0.1 - QR code generation for WiFi setup (ricmoo/QRCode)

#### 💡 BLE Keyboard Support

The SDK includes a modified version of ESP32-BLE-Keyboard library 
compatible with ESP32 core v3.x. This is temporary until the 
upstream PR is merged.

Original library: [https://github.com/T-vK/ESP32-BLE-Keyboard](https://github.com/T-vK/ESP32-BLE-Keyboard)

### For PlatformIO
All dependencies are automatically managed via `platformio.ini` - no manual installation needed! 🎉

## 🚀 Quick Start

### Arduino IDE (Recommended for Development)

#### Prerequisites
- Arduino IDE 2.x or newer
- ESP32 board support installed (see [Getting Started Guide](https://cloudmouse.co/docs/getting-started))
- CloudMouse device

#### Installation

1. **Clone the repository**
```bash
git clone https://github.com/cloudmouse-co/cloudmouse-sdk.git
cd cloudmouse-sdk
```

2. **Install required libraries**
   - Open Arduino IDE
   - Go to Sketch → Include Library → Manage Libraries
   - Install each library listed above

3. **Configure Arduino IDE**
   - Board: `ESP32S3 Dev Module`
   - USB CDC On Boot: `Enabled`
   - Flash Size: `16MB`
   - PSRAM: `OPI PSRAM`
   - Partition Scheme: `Default 4MB with spiffs`

4. **Open and Upload**
   - Open `cloudmouse-sdk.ino` in Arduino IDE
   - Connect CloudMouse via USB-C
   - Click Upload
   - If upload fails, hold BOOT button while connecting USB cable, then clicking Upload

5. **Test**
   - Display shows "Hello CloudMouse!"
   - LED ring pulses gently
   - Rotate encoder to see direction detection
   - Press encoder button to see click detection and LED flash

---

### PlatformIO (Optional - For Advanced Users)

This project fully supports PlatformIO for users who prefer command-line workflows, CI/CD integration, or advanced debugging features.

#### Why PlatformIO?
- ✅ Automatic dependency management
- ✅ Built-in debugging support
- ✅ CI/CD integration ready
- ✅ Multi-environment builds
- ✅ Professional development workflow

#### Setup

**Important: Source Code Switching**

The project maintains a single codebase that works with both Arduino IDE and PlatformIO. The `src/main.cpp` file is kept in sync but needs to be toggled:
>>>>>>> upstream/main

**To use PlatformIO:**
1. Open `src/main.cpp` in your editor
2. **Uncomment the entire file**
3. Save and build with PlatformIO

**To switch back to Arduino IDE:**
1. Open `src/main.cpp` in your editor
2. **Re-comment the entire file**
3. Save and build with Arduino IDE

> 💡 **Pro tip**: Most editors support block comment toggling with `Ctrl+/` (Windows/Linux) or `Cmd+/` (Mac). Select all (`Ctrl+A`) then toggle comments!


## 📚 Prerequisites

Before using this example, make sure you have:

1. **CloudMouse SDK** - Base SDK with hardware abstraction layer
   - 📖 [CloudMouse SDK Repository](https://github.com/tibonilab/cloudmouse-boilerplate)
   
2. **LVGL Library** - Graphics library (v9.x recommended)
   - 📖 [LVGL Official Documentation](https://docs.lvgl.io/)
   - Install via Arduino Library Manager: `LVGL`

3. **Required Hardware**
   - CloudMouse device

## 🚀 Setup Instructions

### 1. Install Dependencies

Install the following libraries via Arduino Library Manager:
- `LVGL` (v9.x)

### 2. Configure LVGL

**IMPORTANT:** You need to add the `lv_conf.h` configuration file to your Arduino libraries folder, as [mentioned here](https://docs.lvgl.io/8/get-started/platforms/arduino.html#configure-lvgl).

**Location:**
```
~/Arduino/libraries/lv_conf.h
```

> ℹ️ A reference configuration is available in this repository at `lib/config/lv_conf.h` for guidance.

Or you can copy and paste the following config snippet.

```cpp
#ifndef LV_CONF_H
#define LV_CONF_H

#define LV_COLOR_DEPTH 16
#define LV_COLOR_16_SWAP 1
#define LV_MEM_SIZE (48U * 1024U)

#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_16 1
#define LV_FONT_MONTSERRAT_18 1
#define LV_FONT_MONTSERRAT_20 1
#define LV_FONT_MONTSERRAT_24 1
#define LV_FONT_MONTSERRAT_28 1
#define LV_FONT_MONTSERRAT_32 1
#define LV_FONT_MONTSERRAT_36 1
#define LV_FONT_MONTSERRAT_48 1

#define LV_FONT_DEFAULT &lv_font_montserrat_14
#define LV_TXT_ENC LV_TXT_ENC_UTF8

#define LV_TICK_CUSTOM 1
#define LV_TICK_CUSTOM_INCLUDE "Arduino.h"
#define LV_TICK_CUSTOM_SYS_TIME_EXPR (millis())

#define LV_USE_LOG 1
#define LV_LOG_LEVEL LV_LOG_LEVEL_WARN
#define LV_LOG_PRINTF 1

#define LV_USE_ASSERT_NULL 1
#define LV_USE_ASSERT_MALLOC 1
#define LV_USE_ASSERT_STYLE 0
#define LV_USE_ASSERT_MEM_INTEGRITY 0
#define LV_USE_ASSERT_OBJ 0

#define LV_USE_LABEL 1
#define LV_USE_BUTTON 1
#define LV_USE_IMAGE 1
#define LV_USE_ARC 1
#define LV_USE_BAR 1
#define LV_USE_FLEX 1
#define LV_USE_GRID 1
#define LV_USE_PSRAM 1

#define LV_USE_SPINNER 1
#define LV_USE_QRCODE 1
#define LV_USE_LABEL 1

#endif
```


### 3. Clone and Open

```bash
git clone https://github.com/tibonilab/cloudmouse-lvgl-example
```

Open the `.ino` file in Arduino IDE.

### 4. Upload

Select your ESP32 board and upload the sketch.

## 🏗️ Architecture

This example follows the CloudMouse SDK architecture:

```
┌─────────────────┐
│   Arduino IDE   │
│    (.ino)       │
└────────┬────────┘
         │
    ┌────▼─────┐
    │   Core   │ ◄─── Event Bus
    └────┬─────┘
         │
    ┌────▼──────────────────┐
    │  DisplayManager       │
    │  (LVGL Integration)   │
    └───────────────────────┘
```

**Key Components:**

- **DisplayManager.cpp** - Main LVGL integration with CloudMouse hardware
- **LVGL Ticker** - Global timer for LVGL tick updates (5ms interval)
- **Event System** - Bridges hardware events (encoder, WiFi) to UI

## 📖 Key Files

| File | Description |
|------|-------------|
| [lib/hardware/DisplayManager.cpp](https://github.com/tibonilab/cloudmouse-example-lvgl/blob/main/lib/hardware/DisplayManager.cpp) | LVGL implementation and screen management |
| [lib/hardware/DisplayManager.h](https://github.com/tibonilab/cloudmouse-example-lvgl/blob/main/lib/hardware/DisplayManager.h) | DisplayManager interface with LVGL ticker setup |

## 🎯 Features Demonstrated

### Screens
- **Hello World** - Welcome screen with encoder interaction
- **WiFi Connecting** - Connection status with spinner animation
- **AP Mode** - Access Point setup with QR code
- **AP Connected** - Configuration portal access with QR code

### LVGL Integration
- Custom display driver for ILI9488 via LovyanGFX
- Encoder input device for navigation
- PSRAM buffer allocation for optimal performance
- Proper v9 API usage with dual buffers

### Event Handling
- Encoder rotation events
- Encoder click/long-press events
- WiFi state transitions
- Screen navigation logic

## 🔗 Useful Links

- [LVGL Official Website](https://lvgl.io/)
- [LVGL Documentation](https://docs.lvgl.io/)
- [LVGL Examples](https://docs.lvgl.io/master/examples.html)
- [CloudMouse SDK](https://github.com/tibonilab/cloudmouse-boilerplate)


## 📝 License

This example follows the same license as the CloudMouse SDK.

## 🤝 Contributing

Contributions are welcome! Feel free to open issues or submit pull requests to improve this example.

---

<<<<<<< HEAD
Made with ❤️ for the CloudMouse community
=======
## 🔧 Advanced: ESP-IDF Native

While not officially supported, CloudMouse hardware is compatible with ESP-IDF native development. 
The hardware is fully open, and the community is welcome to create ESP-IDF ports.

If you develop an ESP-IDF version, consider contributing it back! 🚀

---

## 📁 Project Structure

```
cloudmouse-sdk/
├── cloudmouse-sdk.ino   # Main sketch (Arduino IDE)
├── platformio.ini                          # PlatformIO configuration
├── src/
│   └── main.cpp                            # PlatformIO entry (toggle comments)
│
├── lib/                                    # Core libraries
│   ├── core/                               # Event system
│   │   ├── Core.h / Core.cpp               # Main coordinator (Core 0)
│   │   ├── EventBus.h / EventBus.cpp       # FreeRTOS queues
│   │   └── Events.h                        # Event definitions
│   │
│   ├── hardware/                           # Hardware abstraction
│   │   ├── DisplayManager.h/cpp            # ILI9488 display
│   │   ├── LEDManager.h/cpp                # WS2812B LEDs
│   │   ├── EncoderManager.h/cpp            # Rotary encoder
│   │   ├── RotaryEncoderPCNT.h             # Cross-platform PCNT wrapper
│   │   ├── SimpleBuzzer.h                  # Piezo buzzer
│   │   └── LGFX_ILI9488.h                  # Display config
│   │
│   ├── network/
|   |   ├── BluetoothManager.h/cpp          # Bluetooth management
│   │   ├── WiFiManager.h/cpp               # WiFi management
│   │   ├── WebServerManager.h/cpp          # Web server
│   │
│   ├── utils/                              # Utilities
│   │   ├── NTPManager.h/cpp                # Time sync
│   │   ├── QRCodeManager.h/cpp             # QR codes
│   │   ├── JsonHelper.h                    # JSON utils
│   │   ├── AsyncHttpClient.h               # HTTP client
│   │   └── DeviceID.h                      # Device ID
│   │
│   ├── prefs/                              # Storage
│   │   └── PreferencesManager.h            # Preferences
│   │
│   └── config/                             # Configuration
│       └── DeviceConfig.h                  # Device settings
│
├── assets/                                 # Fonts & graphics
│   ├── OpenSans*.h                         # Fonts
│   ├── LeagueSpartan_50.h
│   └── logo.h                              # Logo
│
└── README.md                               # This file
```

## 💡 What the SDK Does

When you upload the firmware, CloudMouse will:

1. **Boot sequence** - Initialize hardware, start dual-core tasks, LED animation
2. **WiFi setup** (if needed) - Create AP, show QR code, provide web config
3. **Normal operation** - Display UI, handle encoder/buttons, LED animations
4. **Time sync** - Automatic NTP synchronization

## 🔧 Customization

### Adding Your Own Logic

#### 1. Add Events
Edit `lib/core/Events.h`:
```cpp
enum class EventType {
  MY_CUSTOM_EVENT,
  // ...
};
```

#### 2. Handle Events
Edit `lib/core/Core.cpp`:
```cpp
void Core::processEvents() {
  Event event;
  while (EventBus::instance().receiveFromUI(event, 0)) {
    switch (event.type) {
      case EventType::MY_CUSTOM_EVENT:
        // Your code here
        break;
    }
  }
}
```

#### 3. Update Display
Edit `lib/hardware/DisplayManager.cpp`:
```cpp
void DisplayManager::renderMyScreen() {
  sprite.fillSprite(TFT_BLACK);
  sprite.drawString("Hello!", 240, 160);
  pushSprite();
}
```

#### 4. Trigger Events
From anywhere:
```cpp
Event event(EventType::MY_CUSTOM_EVENT);
event.value = 42;
EventBus::instance().sendToMain(event);
```

## 🎯 Next Steps

1. **Examples** - [cloudmouse.co/docs/example-code](https://cloudmouse.co/docs/example-code)
2. **Documentation** - [cloudmouse.co/docs](https://cloudmouse.co/docs)
2. **SDK Reference** - [cloudmouse.co/sdk](https://cloudmouse.co/sdk)
3. **Community** - [Discord](https://discord.gg/n4Mh6jxH34)
4. **Build!** - Fork and create something awesome! 🚀

## 🐛 Troubleshooting

### Upload fails
- Hold BOOT button while plugging-in, then upload
- Try different USB cable
- Check port selection

### Display blank
- Verify PSRAM setting (Arduino: "OPI PSRAM", PlatformIO: `board_build.arduino.memory_type = qio_opi`)
- Check library versions
- Re-upload firmware

### Encoder not responding
- Verify pins: CLK=16, DT=18, SW=17
- Check wiring
- Hardware PCNT is auto-configured for both platforms

### Platform switching issues
- **Arduino IDE → PlatformIO**: Uncomment `src/main.cpp`
- **PlatformIO → Arduino IDE**: Re-comment `src/main.cpp`
- Always save after toggling!

### PlatformIO build errors
- Ensure `src/main.cpp` is uncommented
- Run `pio lib install`
- Try `pio run --target clean`

### Arduino IDE compilation errors  
- Ensure `src/main.cpp` is commented out
- Install all required libraries
- Check board configuration

For more help: [cloudmouse.co/docs/troubleshooting](https://cloudmouse.co/docs/troubleshooting)

## 📄 License

MIT License - Free to use, modify, and distribute. See [LICENSE](LICENSE) for details.

## 🌟 Community

- **Discord**: [discord.gg/cloudmouse](https://discord.gg/n4Mh6jxH34)
- **GitHub**: [github.com/cloudmouse-co](https://github.com/cloudmouse-co)
- **Website**: [cloudmouse.co](https://cloudmouse.co)
- **Docs**: [cloudmouse.co/docs](https://cloudmouse.co/docs)
<!-- - **Forum**: [forum.cloudmouse.co](https://forum.cloudmouse.co) -->

## 💖 Credits

Created by the CloudMouse Team.

Special thanks to contributors and the maker community.

**Technical Achievement**: This cloudmouse-sdk features a custom cross-platform `RotaryEncoderPCNT` wrapper that seamlessly bridges ESP-IDF 4.4 and 5.x APIs, enabling true multi-platform compatibility between Arduino IDE and PlatformIO.

---

**Ready to build?** Fork this repo and start creating! 🚀

*Built with ❤️ for makers, by makers.*
>>>>>>> upstream/main
