// hardware/WebServerManager.cpp
#include "./WebServerManager.h"
#include "../prefs/PreferencesManager.h"

WebServerManager* WebServerManager::instance = nullptr;

WebServerManager::WebServerManager(WiFiManager& wifiMgr) 
    : webServer(80), wifiManager(wifiMgr) {
    instance = this;
}

void WebServerManager::init() {
    Serial.println("🌐 Initializing WebServer...");
    
    scanNetworks();
    
    webServer.on("/", handleRoot);
    webServer.on("/config", HTTP_POST, handleConfig);
    webServer.onNotFound(handleNotFound);
    
    webServer.begin();
    Serial.println("✅ WebServer started on port 80");
}

void WebServerManager::update() {
    webServer.handleClient();
}

void WebServerManager::stop() {
    webServer.stop();
    Serial.println("🌐 WebServer stopped");
}

void WebServerManager::scanNetworks() {
    Serial.println("🔍 Scanning WiFi networks...");
    
    int networkCount = WiFi.scanNetworks();
    networkList = "";
    
    for (int i = 0; i < networkCount; i++) {
        networkList += "<option value='" + WiFi.SSID(i) + "'>";
        networkList += WiFi.SSID(i) + " (" + String(WiFi.RSSI(i)) + " dBm)";
        networkList += "</option>";
    }
    
    Serial.printf("✅ Found %d networks\n", networkCount);
}

String WebServerManager::generateConfigPage() {
    return R"rawliteral(
<!DOCTYPE HTML>
<html lang="it">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>CloudMouse - Configura WiFi</title>
    <style>
        * { box-sizing: border-box; margin: 0; padding: 0; }
        body { 
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            display: flex;
            align-items: center;
            justify-content: center;
            padding: 20px;
        }
        .container {
            background: white;
            border-radius: 16px;
            box-shadow: 0 20px 40px rgba(0,0,0,0.1);
            padding: 40px;
            max-width: 400px;
            width: 100%;
        }
        .logo {
            text-align: center;
            margin-bottom: 30px;
        }
        .logo h1 {
            color: #333;
            font-size: 24px;
            font-weight: 600;
        }
        .logo p {
            color: #666;
            font-size: 14px;
            margin-top: 5px;
        }
        .form-group {
            margin-bottom: 20px;
        }
        label {
            display: block;
            margin-bottom: 8px;
            color: #333;
            font-weight: 500;
        }
        select, input[type="password"] {
            width: 100%;
            padding: 12px 16px;
            border: 2px solid #e1e5e9;
            border-radius: 8px;
            font-size: 16px;
            transition: border-color 0.3s;
        }
        select:focus, input[type="password"]:focus {
            outline: none;
            border-color: #667eea;
        }
        .btn-primary {
            width: 100%;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            border: none;
            padding: 14px;
            border-radius: 8px;
            font-size: 16px;
            font-weight: 600;
            cursor: pointer;
            transition: transform 0.2s;
        }
        .btn-primary:hover {
            transform: translateY(-2px);
        }
        .info {
            background: #f8f9fa;
            border-radius: 8px;
            padding: 16px;
            margin-top: 20px;
            font-size: 14px;
            color: #666;
        }
        .qr-hint {
            text-align: center;
            margin-top: 20px;
            font-size: 12px;
            color: #999;
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="logo">
            <h1>🕐 CloudMouse</h1>
            <p>Configurazione WiFi</p>
        </div>
        
        <form action="/config" method="POST">
            <div class="form-group">
                <label for="ssid">Rete WiFi:</label>
                <select name="ssid" id="ssid" required>
                    <option value="">Seleziona una rete...</option>
)rawliteral" + networkList + R"rawliteral(
                </select>
            </div>
            
            <div class="form-group">
                <label for="password">Password:</label>
                <input type="password" name="password" id="password" 
                       placeholder="Inserisci la password WiFi" required>
            </div>
            
            <button type="submit" class="btn-primary">
                🔗 Connetti
            </button>
        </form>
        
        <div class="info">
            <strong>💡 Suggerimento:</strong><br>
            Dopo la connessione, il dispositivo si riavvierà automaticamente 
            e sarà pronto per l'uso.
        </div>
        
        <div class="qr-hint">
            Hai scansionato il QR code dal display? 📱
        </div>
    </div>
</body>
</html>
)rawliteral";
}

// Static handlers
void WebServerManager::handleRoot() {
    if (!instance) return;
    
    instance->webServer.send(200, "text/html", instance->generateConfigPage());
}

void WebServerManager::handleConfig() {
    if (!instance) return;
    
    if (instance->webServer.hasArg("ssid") && instance->webServer.hasArg("password")) {
        String ssid = instance->webServer.arg("ssid");
        String password = instance->webServer.arg("password");
        
        // Risposta immediata all'utente
        String successPage = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Connessione in corso...</title>
    <style>
        body { 
            font-family: -apple-system, BlinkMacSystemFont, sans-serif;
            text-align: center; 
            padding: 50px;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
        }
        .spinner { 
            border: 4px solid rgba(255,255,255,0.3);
            border-top: 4px solid white;
            border-radius: 50%;
            width: 50px;
            height: 50px;
            animation: spin 1s linear infinite;
            margin: 20px auto;
        }
        @keyframes spin { 0% { transform: rotate(0deg); } 100% { transform: rotate(360deg); } }
    </style>
</head>
<body>
    <h2>🔗 Connessione in corso...</h2>
    <div class="spinner"></div>
    <p>Il dispositivo si sta connettendo alla rete <strong>)rawliteral" + ssid + R"rawliteral(</strong></p>
    <p>Questa pagina si chiuderà automaticamente.</p>
    <script>setTimeout(() => window.close(), 5000);</script>
</body>
</html>
)rawliteral";
        
        instance->webServer.send(200, "text/html", successPage);
        
        // Salva le credenziali e prova a connettersi
        instance->wifiManager.saveCredentials(ssid, password);
        
        // Delay per permettere alla risposta di essere inviata
        delay(1000);
        
        // Prova la connessione
        instance->wifiManager.connect(ssid.c_str(), password.c_str());
        
    } else {
        instance->webServer.send(400, "text/plain", "Errore: SSID o password mancanti");
    }
}

void WebServerManager::handleNotFound() {
    if (!instance) return;
    
    instance->webServer.send(404, "text/plain", "Pagina non trovata");
}