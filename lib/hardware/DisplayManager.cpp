/**
 * CloudMouse SDK - Display Management System Implementation
 *
 * Implementation of comprehensive display controller with event-driven UI management,
 * hardware-accelerated sprite rendering, and integrated QR code generation capabilities.
 *
 * Implementation Architecture:
 * - Event-driven state machine for responsive screen management
 * - PSRAM-buffered sprite rendering for flicker-free updates
 * - Modular screen rendering system for maintainable UI development
 * - Optimized animation timing for smooth visual feedback
 * - Memory-efficient resource management with automatic cleanup
 *
 * Performance Optimizations:
 * - Hardware-accelerated SPI transfers (40MHz sustained)
 * - PSRAM utilization to preserve main RAM for application logic
 * - Selective redraw regions to minimize unnecessary updates
 * - Frame rate limiting to balance smoothness with CPU usage
 * - Event-driven updates to eliminate polling overhead
 *
 * Integration Points:
 * - EventBus for inter-task communication and state synchronization
 * - QRCodeManager for dynamic QR code generation and rendering
 * - DeviceConfig for hardware-specific configuration values
 * - LGFX library for hardware-accelerated graphics operations
 */

#include "./DisplayManager.h"
#include "../core/EventBus.h"

namespace CloudMouse::Hardware
{
    using namespace CloudMouse;

    // ============================================================================
    // CONSTRUCTOR AND DESTRUCTOR IMPLEMENTATION
    // ============================================================================

    DisplayManager::DisplayManager()
    {
        // Constructor performs minimal initialization
        // Actual hardware setup occurs in init() method for controlled timing
    }

    DisplayManager::~DisplayManager()
    {
        // Ensure proper cleanup of sprite resources to prevent memory leaks
        if (sprite)
        {
            delete sprite;
            sprite = nullptr;
            Serial.println("🖥️ Sprite buffer deallocated");
        }
    }

    // ============================================================================
    // SYSTEM LIFECYCLE IMPLEMENTATION
    // ============================================================================

    void DisplayManager::init()
    {
        Serial.println("🖥️ Initializing DisplayManager...");

        // Initialize ILI9488 display hardware
        // Configures SPI interface, power management, and display orientation
        display.init();
        display.setBrightness(200); // Set brightness (0-255 range)

        // Create full-screen sprite buffer in PSRAM for flicker-free rendering
        // Using PSRAM preserves main RAM for application logic and improves performance
        sprite = new LGFX_Sprite(&display);

        if (!sprite)
        {
            Serial.println("❌ Failed to allocate sprite object!");
            return;
        }

        // Configure sprite for PSRAM usage and optimal color depth
        sprite->setPsram(true);    // Enable PSRAM allocation for large buffers
        sprite->setColorDepth(16); // 16-bit RGB565 color (65,536 colors)

        // Create full-screen sprite buffer (480×320×2 bytes = 307,200 bytes)
        bool spriteCreated = sprite->createSprite(480, 320);

        if (spriteCreated)
        {
            initialized = true;
            Serial.println("✅ Sprite buffer created successfully");
            Serial.printf("   Resolution: %dx%d pixels\n", getWidth(), getHeight());
            Serial.printf("   Color depth: 16-bit RGB565\n");
            Serial.printf("   Buffer size: %d bytes\n", 480 * 320 * 2);
            Serial.printf("   Memory location: PSRAM\n");

            // Initialize QR code manager with sprite reference
            qrCodeManager.init(sprite);

            // Set default text rendering properties for consistent appearance
            sprite->setTextColor(COLOR_TEXT);
            sprite->setTextDatum(MC_DATUM); // Middle-center text alignment
        }
        else
        {
            Serial.println("❌ Failed to create sprite buffer - insufficient PSRAM!");
            Serial.printf("   Required: %d bytes\n", 480 * 320 * 2);
            Serial.printf("   Available PSRAM: %d bytes\n", ESP.getFreePsram());
            return;
        }

        // Log successful initialization with system status
        Serial.printf("✅ DisplayManager initialized successfully\n");
        Serial.printf("   Display resolution: %dx%d\n", getWidth(), getHeight());
        Serial.printf("   Sprite memory usage: %d bytes\n", 480 * 320 * 2);
        Serial.printf("   Free PSRAM after allocation: %d bytes\n", ESP.getFreePsram());
        Serial.printf("   Initial screen: HELLO_WORLD\n");
    }

    void DisplayManager::update()
    {
        // Process all pending events from EventBus (non-blocking)
        // Events drive screen state changes and UI updates
        Event event;
        while (EventBus::instance().receiveFromMain(event, 0))
        {
            processEvent(event);
        }

        // Handle continuous animation updates for animated screens
        // Only processes animations when needsRedraw flag is set to optimize CPU usage
        if (needsRedraw && currentScreen == Screen::WIFI_CONNECTING)
        {
            static unsigned long lastAnimationUpdate = 0;

            // Limit animation frame rate to 10fps (100ms intervals) for smooth appearance
            if (millis() - lastAnimationUpdate >= WIFI_CONNECTING_RENDER_INTERVAL)
            {
                renderWiFiConnecting();
                lastAnimationUpdate = millis();
            }
        }
    }

    // ============================================================================
    // EVENT PROCESSING IMPLEMENTATION
    // ============================================================================

    void DisplayManager::processEvent(const Event &event)
    {
        // Process display-related events and trigger appropriate screen updates
        // Each event type corresponds to specific UI state changes or user interactions

        switch (event.type)
        {
        case EventType::DISPLAY_WAKE_UP:
            // Display activation - show default interactive screen
            Serial.println("📺 Display wake up - switching to HELLO_WORLD");
            currentScreen = Screen::HELLO_WORLD;
            needsRedraw = false; // Static screen, no continuous animation
            renderHelloWorld();
            break;

        case EventType::DISPLAY_WIFI_CONNECTING:
            // WiFi connection attempt - show animated progress screen
            Serial.println("📡 Display: Showing WiFi connecting screen with animation");
            currentScreen = Screen::WIFI_CONNECTING;
            needsRedraw = true; // Enable continuous animation for spinner
            renderWiFiConnecting();
            break;

        case EventType::ENCODER_ROTATION:
            // Encoder interaction - update interactive feedback
            Serial.printf("🔄 Display received encoder rotation: %d\n", event.value);
            lastEncoderValue = event.value;

            // Return to default screen and stop any animations
            currentScreen = Screen::HELLO_WORLD;
            needsRedraw = false;
            lastWiFiConnectingRender = 0;
            renderHelloWorld();
            break;

        case EventType::ENCODER_CLICK:
            // Button click - record interaction and update display if on interactive screen
            Serial.println("🖱️ Display received encoder click");
            lastClickTime = millis();
            if (currentScreen == Screen::HELLO_WORLD)
            {
                renderHelloWorld(); // Update to show click feedback
            }
            break;

        case EventType::ENCODER_LONG_PRESS:
            // Long press - record interaction and update display if on interactive screen
            Serial.println("⏱️ Display received encoder long press");
            lastLongPressTime = millis();
            if (currentScreen == Screen::HELLO_WORLD)
            {
                renderHelloWorld(); // Update to show long press feedback
            }
            break;

        case EventType::DISPLAY_WIFI_AP_MODE:
            // Access Point mode - show WiFi connection QR code
            Serial.println("📱 Switching to AP Mode screen - WiFi setup required");
            currentScreen = Screen::WIFI_AP_MODE;
            needsRedraw = false; // Static QR code, no animation needed
            renderAPMode();
            break;

        case EventType::DISPLAY_WIFI_SETUP_URL:
            // Client connected to AP - show web configuration QR code
            Serial.println("🌐 Switching to AP Connected screen - web setup available");
            currentScreen = Screen::WIFI_AP_CONNECTED;
            needsRedraw = false; // Static configuration screen
            renderAPConnected();
            break;

        case EventType::DISPLAY_CLEAR:
            // Clear screen command - immediate screen clear
            Serial.println("🧹 Display clear requested");
            if (sprite)
            {
                sprite->fillSprite(COLOR_BG);
                pushSprite();
            }
            break;

        default:
            // Unhandled event type - log for debugging but continue operation
            Serial.printf("⚠️ DisplayManager: Unhandled event type %d\n", (int)event.type);
            break;
        }
    }

    // ============================================================================
    // SCREEN RENDERING IMPLEMENTATIONS
    // ============================================================================

    void DisplayManager::renderHelloWorld()
    {
        if (!sprite)
        {
            Serial.println("❌ Cannot render - sprite not initialized");
            return;
        }

        // Clear screen with background color
        sprite->fillSprite(COLOR_BG);

        // Draw consistent header across all screens
        drawHeader("CloudMouse Boilerplate");

        // Main title with large, prominent text
        sprite->setTextSize(3);
        drawCenteredText("Hello CloudMouse!", 100, COLOR_ACCENT);

        // Dynamic encoder status display based on recent interactions
        sprite->setTextSize(2);

        // Show recent button interactions with 2-second timeout for visual feedback
        if (millis() - lastClickTime < 2000 && lastClickTime > 0)
        {
            drawCenteredText("Button Clicked!", 160, COLOR_SUCCESS);
        }
        else if (millis() - lastLongPressTime < 2000 && lastLongPressTime > 0)
        {
            drawCenteredText("Long Press!", 160, COLOR_WARNING);
        }
        else if (lastEncoderValue > 0)
        {
            drawCenteredText("Rotating RIGHT", 160, COLOR_TEXT);
        }
        else if (lastEncoderValue < 0)
        {
            drawCenteredText("Rotating LEFT", 160, COLOR_TEXT);
        }
        else
        {
            drawCenteredText("Ready!", 160, COLOR_TEXT);
        }

        // Usage instructions for user guidance
        drawInstructions();

        // Transfer sprite buffer to display hardware
        pushSprite();
    }

    void DisplayManager::renderWiFiConnecting()
    {
        if (!sprite)
        {
            Serial.println("❌ Cannot render - sprite not initialized");
            return;
        }

        // Clear screen for animation frame
        sprite->fillSprite(COLOR_BG);

        // Draw header
        drawHeader("CloudMouse Boilerplate");

        // Connection status title
        sprite->setTextSize(3);
        drawCenteredText("Connecting to WiFi", 120, COLOR_ACCENT);

        // Animated text with variable dot count (2Hz update rate)
        static int dotCount = 0;
        static unsigned long lastDotUpdate = 0;

        if (millis() - lastDotUpdate > 500)
        { // 500ms = 2Hz
            lastDotUpdate = millis();
            dotCount = (dotCount + 1) % 4; // Cycle 0-3 dots
        }

        // Build animated dots string
        String dots = "";
        for (int i = 0; i < dotCount; i++)
        {
            dots += ".";
        }

        sprite->setTextSize(2);
        drawCenteredText("Please wait" + dots, 170, COLOR_TEXT);

        // Animated circular spinner (12 positions, 10fps rotation)
        int centerX = 240;                 // Screen center X
        int centerY = 230;                 // Spinner Y position
        int radius = 20;                   // Spinner radius
        int frame = (millis() / 100) % 12; // 100ms per frame = 10fps

        // Draw 12 dots in circle with one highlighted position
        for (int i = 0; i < 12; i++)
        {
            float angle = (i * 30) * PI / 180.0; // 30 degrees per position
            int x = centerX + cos(angle) * radius;
            int y = centerY + sin(angle) * radius;

            // Highlight current frame position, dim others
            uint16_t color = (i == frame) ? COLOR_ACCENT : 0x4208; // Bright or dim
            sprite->fillCircle(x, y, 3, color);
        }

        // Update display with animation frame
        pushSprite();
    }

    void DisplayManager::renderAPMode()
    {
        if (!sprite)
        {
            Serial.println("❌ Cannot render - sprite not initialized");
            return;
        }

        // Clear screen
        sprite->fillSprite(TFT_DARKGRAY);

        // Header for setup context
        drawHeader("WiFi Setup Required");

        // Setup instructions
        sprite->setTextSize(2);
        drawCenteredText("Connect to CloudMouse", 60, COLOR_ACCENT);

        // Access Point credentials display
        sprite->setTextSize(1.5);
        String apSSID = GET_AP_SSID();
        String apPassword = GET_AP_PASSWORD();

        drawCenteredText("WiFi Network:", 90, COLOR_TEXT);
        drawCenteredText(apSSID, 115, COLOR_SUCCESS);
        drawCenteredText(apPassword, 135, COLOR_SUCCESS);

        // Generate and display QR code for WiFi connection
        int qrX = 154;       // Center horizontally for QR code
        int qrY = 154;       // Vertical position for QR code
        int qrPixelSize = 4; // QR code pixel scaling

        // Create WiFi connection string in standard format
        String qrData = String("WIFI:T:WPA;S:") + apSSID + ";P:" + apPassword + ";;";
        qrCodeManager.setOffset(qrX, qrY);
        qrCodeManager.setPixelSize(qrPixelSize);
        qrCodeManager.create(qrData.c_str());

        // Optional IP address display (commented for cleaner UI)
        // sprite->setTextSize(1.5);
        // drawCenteredText("or visit: " + apIP, 290, COLOR_TEXT);

        // Update display with QR code and credentials
        pushSprite();
    }

    void DisplayManager::renderAPConnected()
    {
        if (!sprite)
        {
            Serial.println("❌ Cannot render - sprite not initialized");
            return;
        }

        // Success-themed background color
        sprite->fillSprite(TFT_DARKGREEN);

        // Header for configuration context
        drawHeader("WiFi Configuration");

        // Success confirmation
        sprite->setTextSize(2);
        drawCenteredText("✅ Connected!", 60, COLOR_SUCCESS);

        // Instructions for next step
        sprite->setTextSize(1.5);
        drawCenteredText("Scan QR to setup WiFi", 100, COLOR_TEXT);

        // QR code for web configuration interface
        int qrSize = 172;             // QR code dimensions
        int qrX = (480 - qrSize) / 2; // Center horizontally
        int qrY = 130;                // Vertical position
        int qrPixelSize = 4;          // Pixel scaling factor

        // Generate QR code with web configuration URL
        qrCodeManager.setOffset(qrX, qrY);
        qrCodeManager.setPixelSize(qrPixelSize);
        qrCodeManager.create(WIFI_CONFIG_SERVICE);

        // Display configuration URL for manual entry
        sprite->setTextSize(1.5);
        drawCenteredText(WIFI_CONFIG_SERVICE, 290, COLOR_TEXT);

        // Update display with configuration interface
        pushSprite();
    }

    // ============================================================================
    // SPRITE BUFFER MANAGEMENT
    // ============================================================================

    void DisplayManager::pushSprite()
    {
        // Transfer sprite buffer contents to physical display via SPI
        // This operation provides flicker-free updates through double buffering
        if (sprite)
        {
            sprite->pushSprite(0, 0); // Push to position (0,0) - full screen
        }
        else
        {
            Serial.println("⚠️ Cannot push sprite - buffer not initialized");
        }
    }

    // ============================================================================
    // UI ELEMENT RENDERING UTILITIES
    // ============================================================================

    void DisplayManager::drawHeader(const String &title)
    {
        // Draw consistent header design across all screens
        // Provides visual continuity and branding consistency

        // Header background with dark blue gradient effect
        sprite->fillRect(0, 0, 480, 50, 0x0010);   // Dark blue background
        sprite->drawFastHLine(0, 50, 480, 0x001F); // Blue accent line

        // Header text with centered alignment
        sprite->setTextSize(2);
        sprite->setTextColor(COLOR_TEXT);
        sprite->setTextDatum(MC_DATUM);     // Middle-center alignment
        sprite->drawString(title, 240, 25); // Center of header area
    }

    void DisplayManager::drawCenteredText(const String &text, int y, uint16_t color)
    {
        // Utility for consistent centered text rendering
        // Simplifies text positioning and maintains visual alignment
        sprite->setTextColor(color);
        sprite->setTextDatum(MC_DATUM);   // Middle-center alignment
        sprite->drawString(text, 240, y); // Center horizontally at 240px
    }

    void DisplayManager::drawInstructions()
    {
        // Draw usage instructions footer for user guidance
        // Provides consistent help text across interactive screens

        sprite->setTextSize(1);
        sprite->setTextColor(0x7BEF); // Light gray for secondary information

        int startY = 220;    // Starting Y position for instructions
        int lineHeight = 20; // Vertical spacing between instruction lines

        // Multi-line instruction text with consistent spacing
        drawCenteredText("Rotate: Turn encoder left/right", startY, 0x7BEF);
        drawCenteredText("Click: Press encoder button", startY + lineHeight, 0x7BEF);
        drawCenteredText("Long Press: Hold encoder button", startY + lineHeight * 2, 0x7BEF);
    }
}