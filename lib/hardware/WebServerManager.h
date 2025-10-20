// hardware/WebServerManager.h
#pragma once
#include <WebServer.h>
#include <WiFi.h>
#include "WiFiManager.h"

class WebServerManager {
public:
    WebServerManager(WiFiManager& wifiMgr);
    ~WebServerManager() = default;
    
    void init();
    void update();
    void stop();
    
private:
    WebServer webServer;
    WiFiManager& wifiManager;
    String networkList;
    
    void scanNetworks();
    String generateConfigPage();
    
    // Static handlers
    static void handleRoot();
    static void handleConfig();
    static void handleNotFound();
    
    static WebServerManager* instance;
};