// #ifdef PLATFORMIO
//   #include "../lib/core/Core.h"
//   #include "../lib/hardware/EncoderManager.h"
//   #include "../lib/hardware/DisplayManager.h"
//   #include "../lib/hardware/SimpleBuzzer.h"
//   #include "../lib/hardware/WiFiManager.h"
//   #include "../lib/hardware/WebServerManager.h"
//   #include "../lib/hardware/LEDManager.h"
// #else
//   #include "lib/core/Core.h"
//   #include "lib/hardware/EncoderManager.h"
//   #include "lib/hardware/DisplayManager.h"
//   #include "lib/hardware/SimpleBuzzer.h"
//   #include "lib/hardware/WiFiManager.h"
//   #include "lib/hardware/WebServerManager.h"
//   #include "lib/hardware/LEDManager.h"
// #endif

// using namespace CloudMouse;

// EncoderManager encoder;
// DisplayManager display;
// WiFiManager wifi;
// WebServerManager webServer(wifi);
// LEDManager ledManager;

// void setup() {
//     Serial.begin(115200);
//     delay(1000);
//     Serial.println();
//     Serial.println("🚀 CloudMouse Boilerplate v1.0");
//     Serial.println("   Hello World Edition! 👋");
    
//     // Initialize hardware
//     SimpleBuzzer::init();
//     encoder.init();
//     display.init();
    
//     ledManager.init();

//     // Register components with Core
//     Core::instance().setEncoder(&encoder);
//     Core::instance().setDisplay(&display);
//     Core::instance().setWiFi(&wifi);
//     Core::instance().setWebServer(&webServer);
//     Core::instance().setLEDManager(&ledManager);

//     // Start UI Task (Core 1)
//     Core::instance().startUITask();

//     // Initialize Core system (Core 0)
//     Core::instance().initialize();
    
//     Serial.println("✅ System ready!");
// }

// void loop() {
//     // Main coordination loop runs on Core 0 (20Hz)
//     Core::instance().coordinationLoop();
//     delay(50);
// }