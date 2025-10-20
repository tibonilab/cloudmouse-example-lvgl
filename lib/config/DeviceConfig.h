#ifndef DEVICE_CONFIG_H
#define DEVICE_CONFIG_H

#include "../helper/DeviceID.h"

// PCB Version
#define PCB_VERSION 4
// #define PCB_VERSION 5

#define FIRMWARE_VERSION "3.0.0-alpha"

// WiFi Configuration
#define WIFI_REQUIRED true

// Device ID
#define GET_DEVICE_ID() DeviceID::getDeviceID()
#define GET_DEVICE_UUID() DeviceID::getDeviceUUID()

// AP config
#define GET_AP_SSID() DeviceID::getAPSSID()
#define GET_AP_PASSWORD() DeviceID::getAPPasswordSecure()
#define WIFI_CONFIG_SERVICE "http://192.168.4.1/"

#endif