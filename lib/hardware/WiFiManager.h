// hardware/WiFiManager.h
#pragma once
#include <WiFi.h>
#include <esp_wps.h>
#include <string>
#include "../prefs/PreferencesManager.h"
#include "../helper/NTPManager.h"
#include "../config/DeviceConfig.h"

class WiFiManager {
public:
    enum class WiFiState {
        DISCONNECTED,
        CONNECTING,
        CONNECTED,
        TIMEOUT,
        AP_MODE_INIT,
        AP_MODE,
        WPS_LISTENING,
        WPS_SUCCESS,
        WPS_FAILED,
        ERROR,
        CREDENTIAL_NOT_FOUND,
    };
    
    WiFiManager();
    ~WiFiManager() = default;
    
    void init();
    void update();
    
    // Connection management
    bool connectWithSavedCredentials();
    bool connect(const char* ssid, const char* password, uint32_t timeout = 10000);
    void disconnect();
    void reconnect();
    
    // Access Point mode
    void setupAP();
    void stopAP();
    bool hasConnectedDevices();
    String getAPSSID() const { return AP_SSID; }
    String getAPPassword() const { return AP_PASSWORD; }
    String getAPIP() const { return "192.168.4.1"; }
    
    // WPS support
    void startWPS();
    void stopWPS();
    bool isWPSListening() const { return wpsStarted; }
    
    // Status queries
    WiFiState getState() const { return currentState; }
    bool isConnected() const { return currentState == WiFiState::CONNECTED; }
    bool isConnecting() const { return currentState == WiFiState::CONNECTING; }
    bool isTimedOut() const { return currentState == WiFiState::TIMEOUT; }
    bool isAPMode() const { return currentState == WiFiState::AP_MODE; }
    
    String getLocalIP() const;
    String getSSID() const;
    int getRSSI() const;
    uint32_t getConnectionTime() const;
    wifi_mode_t getMode() const { return WiFi.getMode(); }
    
    // Configuration
    void saveCredentials(const String& ssid, const String& password);

    // Check if there's a client connected to our AP
    bool hasAPClient() const {
        return WiFi.softAPgetStationNum() > 0;
    }
    
private:
    WiFiState currentState = WiFiState::DISCONNECTED;
    PreferencesManager prefs;
    
    uint32_t connectionStartTime = 0;
    uint32_t connectionTimeout = 10000;
    
    bool wpsStarted = false;
    bool initialized = false;
    
    // AP Configuration
    const String AP_SSID = "CloudMouse-b126aaaf";
    const String AP_PASSWORD = "123456789";
    
    // 🆕 Static instance per eventi WiFi
    static WiFiManager* staticInstance;
    
    void setState(WiFiState newState);
    void handleConnectionTimeout();
    static void WiFiEventHandler(WiFiEvent_t event, arduino_event_info_t info);
    void logStatus();
};