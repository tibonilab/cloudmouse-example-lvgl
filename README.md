# CloudMouse Boilerplate

**Event-driven dual-core firmware for CloudMouse hardware**

A complete, ready-to-use starting point for building CloudMouse applications. Fork this repo, customize it, and create your own interactive projects with display, encoder, LEDs, WiFi, and more.

## ✨ Features

- **Dual-core architecture** - UI rendering on Core 1, networking and logic on Core 0
- **Event-driven design** - Thread-safe communication via FreeRTOS queues
- **Complete hardware abstraction** - Easy-to-use classes for all CloudMouse components
- **WiFi setup flow** - Captive portal with QR code for easy configuration
- **NTP time sync** - Automatic time synchronization
- **Preferences storage** - Persistent configuration management
- **LED animations** - Smooth pulsing, loading states, and color control
- **Encoder handling** - Rotation, click, long-press detection
- **Display rendering** - High-performance graphics with LovyanGFX

## 🏗️ Architecture

CloudMouse Boilerplate uses a dual-core architecture to maximize performance:

### Core 0 (Main/Networking)
- WiFi management and connection handling
- HTTP/WebSocket communication
- Business logic and state machine
- Event coordination
- NTP time synchronization

### Core 1 (UI)
- Display rendering (LovyanGFX)
- Encoder input reading
- User interface updates
- Real-time graphics

### Event System
Communication between cores happens via FreeRTOS queues:
- `uiToMainQueue` - Events from UI task to Main task
- `mainToUIQueue` - Events from Main task to UI task

This ensures thread-safe operation without race conditions.

## 📦 Required Libraries

Install these **exact versions** via Arduino Library Manager:

### Core Libraries
- **LovyanGFX** v1.2.7 - Display driver for ILI9488
- **Adafruit NeoPixel** v1.15.1 - WS2812B LED control
- **RotaryEncoderPCNT** v1.1.0 - Hardware encoder support

### Networking & Communication
- **ArduinoJson** v7.4.2 - JSON parsing
- **WebSockets** v2.7.0 - WebSocket communication
- **NTPClient** v3.2.1 - Time synchronization

### Utilities
- **QRCode** v0.0.1 - QR code generation for WiFi setup

⚠️ **Important:** Use these exact versions to avoid compatibility issues.

## 🚀 Quick Start

### Prerequisites
- Arduino IDE 2.x or newer
- ESP32 board support installed (see [Getting Started Guide](https://docs.cloudmouse.co/getting-started))
- CloudMouse device

### Installation

1. **Clone the repository**
```bash
git clone https://github.com/cloudmouse-co/boilerplate.git
cd boilerplate
```

2. **Install required libraries**
   - Open Arduino IDE
   - Go to Sketch → Include Library → Manage Libraries
   - Install each library listed above with the exact version

3. **Configure Arduino IDE**
   - Board: `ESP32S3 Dev Module`
   - USB CDC On Boot: `Enabled`
   - Flash Size: `16MB`
   - PSRAM: `OPI PSRAM`
   - Partition Scheme: `Default 4MB with spiffs`

4. **Upload**
   - Connect CloudMouse via USB-C
   - Click Upload in Arduino IDE
   - If upload fails, hold BOOT button while clicking Upload

5. **Test**
   - Display shows "Hello CloudMouse!"
   - LED ring pulses gently
   - Rotate encoder to see direction detection
   - Press encoder button to see click detection and LED flash

## 📁 Project Structure

```
cloudmouse-boilerplate/
├── boilerplate.ino              # Main Arduino sketch
│
├── core/                         # Event system core
│   ├── Core.h / Core.cpp        # Main coordinator (Core 0)
│   ├── EventBus.h / EventBus.cpp # FreeRTOS queue management
│   └── Events.h                  # Event type definitions
│
├── hardware/                     # Hardware abstraction layer
│   ├── DisplayManager.h/cpp      # ILI9488 display control
│   ├── LEDManager.h/cpp          # WS2812B LED animations
│   ├── EncoderManager.h/cpp      # Rotary encoder handling
│   ├── SimpleBuzzer.h            # Piezo buzzer control
│   ├── WiFiManager.h/cpp         # WiFi AP/STA modes
│   ├── WebServerManager.h/cpp    # Web server for config
│   ├── WebSocketManager.h/cpp    # WebSocket communication
│   └── LGFX_ILI9488.h           # LovyanGFX display configuration
│
├── helper/                       # Utility classes
│   ├── NTPManager.h/cpp          # Time synchronization
│   ├── QRCodeManager.h/cpp       # QR code generation
│   └── JsonHelper.h              # JSON utilities
│
├── prefs/                        # Persistent storage
│   └── PreferencesManager.h      # ESP32 Preferences wrapper
│
├── config/                       # Configuration
│   └── DeviceConfig.h            # Device-specific settings
│
├── assets/                       # Fonts and graphics
│   ├── fonts/                    # TrueType font headers
│   └── logo.h                    # CloudMouse logo
│
└── README.md                     # This file
```

## 💡 What the Boilerplate Does

When you upload the boilerplate firmware, CloudMouse will:

1. **Boot sequence**
   - Initialize all hardware components
   - Start dual-core tasks
   - LED animation runs during initialization

2. **WiFi setup** (if not configured)
   - Creates WiFi Access Point
   - Shows QR code on display for easy connection
   - Provides web interface for WiFi configuration
   - Supports WPS connection

3. **Normal operation**
   - Display shows "Hello CloudMouse!" with instructions
   - LED ring pulses gently (idle animation)
   - Encoder rotation: Display shows direction + LED activates
   - Encoder click: Buzzer beeps + LED flashes + Display updates
   - Long press: Alternative action (customize as needed)

4. **Time synchronization**
   - Automatically syncs with NTP server
   - Updates display with current time

## 🔧 Customization

### Adding Your Own Logic

The boilerplate is designed to be forked and customized. Here's how:

#### 1. Add Your Events
Edit `core/Events.h` and add your event types:

```cpp
enum class EventType {
  // ... existing events ...
  MY_CUSTOM_EVENT,
  MY_OTHER_EVENT
};
```

#### 2. Handle Events in Core
Edit `Core.cpp` in the `processEvents()` function:

```cpp
void Core::processEvents() {
  Event event;
  while (EventBus::instance().receiveFromUI(event, 0)) {
    switch (event.type) {
      case EventType::MY_CUSTOM_EVENT:
        // Handle your event
        break;
      // ... other cases ...
    }
  }
}
```

#### 3. Update Display
Edit `DisplayManager.cpp` to customize UI:

```cpp
void DisplayManager::renderMyScreen() {
  sprite.fillSprite(TFT_BLACK);
  sprite.setTextColor(TFT_WHITE);
  sprite.drawString("My Custom Screen", 240, 160);
  pushSprite();
}
```

#### 4. Trigger Events
From anywhere in your code:

```cpp
Event event(EventType::MY_CUSTOM_EVENT);
event.value = 42;
EventBus::instance().sendToMain(event);
```

### Changing LED Colors

Edit LED color in preferences or modify `LEDManager::setMainColor()`:

```cpp
// Available colors: azure, green, red, orange, yellow, blue, violet, purple
ledManager.setMainColor("green");
```

### Customizing WiFi Behavior

Edit `WiFiManager.cpp` to change:
- AP credentials
- Connection timeout
- Retry logic
- WPS behavior

## 🎯 Next Steps

Once you're comfortable with the boilerplate:

1. **Check the examples** - Browse [CloudMouse Examples](https://github.com/cloudmouse-co/examples) for more complex projects
2. **Read the docs** - Visit [docs.cloudmouse.co](https://docs.cloudmouse.co) for detailed hardware and API documentation
3. **Join the community** - Get help and share your projects on [Discord](https://discord.gg/cloudmouse)
4. **Build something awesome** - Fork this repo and create your own CloudMouse application!

## 📚 Documentation

- **Hardware Specs**: [docs.cloudmouse.co/hardware-specs](https://docs.cloudmouse.co/hardware-specs)
- **Pinout Reference**: [docs.cloudmouse.co/pinout](https://docs.cloudmouse.co/pinout)
- **API Documentation**: [docs.cloudmouse.co/api](https://docs.cloudmouse.co/api)
- **Troubleshooting**: [docs.cloudmouse.co/troubleshooting](https://docs.cloudmouse.co/troubleshooting)

## 🤝 Contributing

Found a bug? Want to improve the boilerplate? Contributions are welcome!

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## 🐛 Troubleshooting

### Upload fails
- Hold BOOT button while clicking Upload
- Try different USB cable (must support data transfer)
- Check port selection in Arduino IDE

### Display stays blank
- Verify PSRAM is set to "OPI PSRAM" in Arduino IDE
- Check that all libraries are installed with correct versions
- Try uploading again

### WiFi won't connect
- Device creates AP named "CloudMouse-XXXX" for configuration
- Connect to AP and scan QR code or visit 192.168.4.1
- Check WiFi credentials are correct
- Make sure network is 2.4GHz (ESP32 doesn't support 5GHz)

For more solutions, check the [Troubleshooting Guide](https://docs.cloudmouse.co/troubleshooting).

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

You are free to:
- Use this code in personal and commercial projects
- Modify and distribute the code
- Create closed-source products based on this code

Just give credit and include the MIT license notice.

## 🌟 Community

- **Discord**: [discord.gg/cloudmouse](https://discord.gg/cloudmouse)
- **Forum**: [forum.cloudmouse.co](https://forum.cloudmouse.co)
- **GitHub**: [github.com/cloudmouse-co](https://github.com/cloudmouse-co)
- **Website**: [cloudmouse.co](https://cloudmouse.co)
- **Docs**: [docs.cloudmouse.co](https://docs.cloudmouse.co)

## 💖 Credits

Created and maintained by the CloudMouse Team.

Special thanks to all contributors and the maker community for feedback and support.

---

**Ready to build?** Fork this repo and start creating! 🚀