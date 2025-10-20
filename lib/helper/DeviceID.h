#ifndef DEVICEID_H
#define DEVICEID_H

#include <Arduino.h>
#include <esp_system.h>

class DeviceID {
public:
    // Ottieni l'ID univoco dell'ESP32 (ultimi 4 byte del MAC)
    static String getDeviceID() {
        uint64_t chipid = ESP.getEfuseMac();
        uint32_t low = (uint32_t)chipid;
        
        char id[9];
        snprintf(id, sizeof(id), "%08x", low);
        
        return String(id);
    }
    
    // 👇 UUID VERO basato sul MAC completo!
    static String getDeviceUUID() {
        uint64_t mac = ESP.getEfuseMac();
        uint8_t* macBytes = (uint8_t*)&mac;
        
        // Formato UUID: xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx
        // Usiamo il MAC (6 byte = 48 bit) + chip ID per riempire i 128 bit
        
        uint32_t chipID = (uint32_t)ESP.getEfuseMac();
        uint16_t chipRev = ESP.getChipRevision();
        
        char uuid[37];
        snprintf(uuid, sizeof(uuid),
                 "%02x%02x%02x%02x-%02x%02x-4%01x%02x-%02x%02x-%02x%02x%02x%02x%04x",
                 macBytes[5], macBytes[4], macBytes[3], macBytes[2],  // 8 char
                 macBytes[1], macBytes[0],                             // 4 char
                 (chipID >> 12) & 0x0F, (chipID >> 8) & 0xFF,         // 4 char (versione 4)
                 0x80 | ((chipID >> 4) & 0x3F), chipID & 0xFF,        // 4 char (variant)
                 macBytes[5], macBytes[4], macBytes[3],               // 6 char
                 macBytes[2], macBytes[1], chipRev                    // 6 char
                );
        
        return String(uuid);
    }
    
    // 👇 Versione alternativa - UUID v4 Random-Style usando MAC come seed
    static String getDeviceUUIDv4Style() {
        uint64_t mac = ESP.getEfuseMac();
        
        // Usa il MAC come seed per numeri pseudo-casuali deterministici
        uint32_t part1 = (uint32_t)(mac & 0xFFFFFFFF);
        uint32_t part2 = (uint32_t)((mac >> 32) & 0xFFFF);
        uint32_t part3 = ESP.getChipRevision() * 0x1A2B3C4D;
        uint32_t part4 = part1 ^ part2;
        
        char uuid[37];
        snprintf(uuid, sizeof(uuid),
                 "%08x-%04x-4%03x-%04x-%08x%04x",
                 part1,                           // 8 caratteri
                 (uint16_t)(part2 & 0xFFFF),     // 4 caratteri
                 (uint16_t)(part3 & 0xFFF),      // 3 caratteri (+ "4" per version)
                 (uint16_t)(0x8000 | (part4 & 0x3FFF)),  // 4 caratteri (variant)
                 part1 ^ part3,                   // 8 caratteri
                 (uint16_t)(part2 ^ part4)       // 4 caratteri
                );
        
        return String(uuid);
    }
    
    // Ottieni SSID per Access Point
    static String getAPSSID() {
        return "CloudMouse-" + getDeviceID();
    }
    
    // Password semplice (primi 8 caratteri dell'ID)
    static String getAPPassword() {
        String id = getDeviceID();
        return id.substring(0, 8);
    }
    
    // Password più sicura con mixing
    static String getAPPasswordSecure() {
        uint64_t mac = ESP.getEfuseMac();
        uint8_t* macBytes = (uint8_t*)&mac;
        
        // Crea una password mescolando i byte del MAC
        char pass[11];
        snprintf(pass, sizeof(pass), "%02x%02x%02x%02x%02x",
                 macBytes[0] ^ macBytes[3],
                 macBytes[1] ^ macBytes[4],
                 macBytes[2] ^ macBytes[5],
                 macBytes[3] ^ macBytes[0],
                 macBytes[4] ^ macBytes[1]
                );
        
        return String(pass);
    }
    
    // Ottieni MAC address formattato
    static String getMACAddress() {
        uint64_t mac = ESP.getEfuseMac();
        uint8_t* macBytes = (uint8_t*)&mac;
        
        char macStr[18];
        snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
                 macBytes[0], macBytes[1], macBytes[2],
                 macBytes[3], macBytes[4], macBytes[5]);
        
        return String(macStr);
    }
    
    // Log info del device
    static void printDeviceInfo() {
        Serial.println("\n📱 Device Information:");
        Serial.printf("   Device ID: %s\n", getDeviceID().c_str());
        Serial.printf("   Device UUID: %s\n", getDeviceUUID().c_str());
        Serial.printf("   MAC Address: %s\n", getMACAddress().c_str());
        Serial.printf("   AP SSID: %s\n", getAPSSID().c_str());
        Serial.printf("   AP Password: %s\n", getAPPassword().c_str());
        Serial.printf("   AP Password (Secure): %s\n", getAPPasswordSecure().c_str());
        Serial.printf("   Chip Model: %s\n", ESP.getChipModel());
        Serial.printf("   Chip Revision: %d\n", ESP.getChipRevision());
        Serial.printf("   CPU Frequency: %d MHz\n", ESP.getCpuFreqMHz());
        Serial.println();
    }
};

#endif