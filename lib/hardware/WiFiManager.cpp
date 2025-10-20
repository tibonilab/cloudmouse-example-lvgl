// hardware/WiFiManager.cpp
#include "./WiFiManager.h"
#include "../helper/NTPManager.h"

using namespace CloudMouse;

// 🆕 Definizione della variabile statica
WiFiManager* WiFiManager::staticInstance = nullptr;

WiFiManager::WiFiManager() {
    staticInstance = this;
}

void WiFiManager::init() {
    Serial.println("📶 Initializing WiFiManager...");
    
    // Registra il gestore eventi WiFi
    WiFi.onEvent(WiFiEventHandler);
    
    initialized = true;
    
    // Prova prima con le credenziali salvate
    if (connectWithSavedCredentials()) {
        Serial.println("📶 Trying saved WiFi credentials...");
    } else {
        Serial.println("📶 No saved credentials found");
        setState(WiFiState::CREDENTIAL_NOT_FOUND);
    }
    
    Serial.println("✅ WiFiManager initialized");
}

bool WiFiManager::connectWithSavedCredentials() {
    String savedSSID = prefs.getWiFiSSID();
    String savedPassword = prefs.getWiFiPassword();
    
    if (savedSSID.isEmpty() || savedPassword.isEmpty()) {
        return false;
    }
    
    return connect(savedSSID.c_str(), savedPassword.c_str());
}

bool WiFiManager::connect(const char* ssid, const char* password, uint32_t timeout) {
    if (!initialized) return false;
    
    Serial.printf("📶 Connecting to WiFi: %s\n", ssid);
    
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
    
    setState(WiFiState::CONNECTING);
    connectionStartTime = millis();
    connectionTimeout = timeout;
    
    WiFi.begin(ssid, password);
    
    return true;
}

void WiFiManager::setupAP() {
    setState(WiFiState::AP_MODE_INIT);
    Serial.println("📶 Setting up Access Point...");
    WiFi.mode(WIFI_AP);

    String apSSID = GET_AP_SSID();
    String apPassword = GET_AP_PASSWORD();

    WiFi.softAP(apSSID.c_str(), apPassword.c_str());
    
    setState(WiFiState::AP_MODE);
    
    Serial.printf("✅ AP created: %s / %s\n", apSSID.c_str(), apPassword.c_str());
    Serial.printf("📶 AP IP: %s\n", WiFi.softAPIP().toString().c_str());
}

void WiFiManager::stopAP() {
    WiFi.softAPdisconnect(true);
    Serial.println("📶 Access Point stopped");
}

bool WiFiManager::hasConnectedDevices() {
    return WiFi.softAPgetStationNum() > 0;
}

void WiFiManager::startWPS() {
    if (wpsStarted) return;
    
    Serial.println("📶 Starting WPS...");
    
    WiFi.mode(WIFI_STA);
    
    esp_wps_config_t config = WPS_CONFIG_INIT_DEFAULT(WPS_TYPE_PBC);
    esp_wifi_wps_enable(&config);
    esp_wifi_wps_start(0);
    
    wpsStarted = true;
    setState(WiFiState::WPS_LISTENING);
}

void WiFiManager::stopWPS() {
    if (!wpsStarted) return;
    
    Serial.println("📶 Stopping WPS...");
    
    esp_wifi_wps_disable();
    wpsStarted = false;
}

void WiFiManager::update() {
    if (!initialized) return;
    
    // Handle connection timeout
    if (currentState == WiFiState::CONNECTING) {
        handleConnectionTimeout();
    }
}

void WiFiManager::handleConnectionTimeout() {
    uint32_t connectionTime = millis() - connectionStartTime;
    
    if (connectionTime > connectionTimeout) {
        Serial.printf("⏰ WiFi connection timeout after %d ms\n", connectionTime);
        setState(WiFiState::TIMEOUT);
    }
}

void WiFiManager::setState(WiFiState newState) {
    if (currentState != newState) {
        WiFiState oldState = currentState;
        currentState = newState;
        
        Serial.printf("📶 WiFi State: %d → %d\n", (int)oldState, (int)newState);
    }
}

void WiFiManager::saveCredentials(const String& ssid, const String& password) {
    prefs.saveWiFiCredentials(ssid, password);
    Serial.printf("📶 Saved credentials: %s\n", ssid.c_str());
}

// Event handler statico
void WiFiManager::WiFiEventHandler(WiFiEvent_t event, arduino_event_info_t info) {
    if (!staticInstance) return;
    
    switch (event) {
        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            Serial.println("✅ WiFi Connected!");
            Serial.printf("📶 IP: %s\n", WiFi.localIP().toString().c_str());
            staticInstance->saveCredentials(WiFi.SSID(), WiFi.psk());
            staticInstance->setState(WiFiState::CONNECTED);

            delay(1000);  // Breve pausa per stabilizzare la connessione
            CloudMouse::NTPManager::init();
            break;
            
        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            Serial.println("📶 WiFi Disconnected");
            if (staticInstance->currentState == WiFiState::CONNECTING) {
                // Timeout gestito da handleConnectionTimeout
            } else {
                staticInstance->setState(WiFiState::DISCONNECTED);
            }
            break;
            
        case ARDUINO_EVENT_WPS_ER_SUCCESS:
            Serial.println("✅ WPS Success!");
            staticInstance->stopWPS();
            staticInstance->setState(WiFiState::WPS_SUCCESS);
            WiFi.begin(); // Connetti con le credenziali WPS
            break;
            
        case ARDUINO_EVENT_WPS_ER_FAILED:
        case ARDUINO_EVENT_WPS_ER_TIMEOUT:
            Serial.println("❌ WPS Failed/Timeout");
            staticInstance->stopWPS();
            staticInstance->setState(WiFiState::WPS_FAILED);
            break;
            
        default:
            break;
    }
}

String WiFiManager::getLocalIP() const {
    if (isConnected()) {
        return WiFi.localIP().toString();
    } else if (isAPMode()) {
        return WiFi.softAPIP().toString();
    }
    return "";
}

String WiFiManager::getSSID() const {
    if (isConnected()) {
        return WiFi.SSID();
    } else if (isAPMode()) {
        return GET_AP_SSID();
    }
    return "";
}

int WiFiManager::getRSSI() const {
    if (isConnected()) {
        return WiFi.RSSI();
    }
    return 0;
}

uint32_t WiFiManager::getConnectionTime() const {
    if (currentState == WiFiState::CONNECTING) {
        return millis() - connectionStartTime;
    }
    return 0;
}

void WiFiManager::disconnect() {
    Serial.println("📶 Disconnecting WiFi...");
    WiFi.disconnect();
    setState(WiFiState::DISCONNECTED);
}

void WiFiManager::reconnect() {
    Serial.println("🔄 Attempting to reconnect...");
    if (connectWithSavedCredentials()) {
        Serial.println("📶 Reconnecting with saved credentials...");
    } else {
        Serial.println("❌ No saved credentials for reconnection");
        setState(WiFiState::CREDENTIAL_NOT_FOUND);
    }
}