#ifndef PREFERENCESMANAGER_H
#define PREFERENCESMANAGER_H

#include <Preferences.h>

class PreferencesManager {
public:
  void init() {
    initDeviceSettings();
  }

  void begin(bool readOnly) {
    preferences.begin(space, readOnly);
  }

  bool hasWiFiCredentials() {
    return ! (getWiFiSSID().equals("") || getWiFiPassword().equals(""));
  }

  void saveWiFiCredentials(const String& ssid, const String& password) {
    begin(false);
    preferences.putString("wifi_ssid", ssid);
    preferences.putString("wifi_password", password);
    end();
  }

  String getWiFiSSID() {
    begin(true);
    String value = preferences.getString("wifi_ssid", "");
    end();
    return value;
  }

  String getWiFiPassword() {
    begin(true);
    String value = preferences.getString("wifi_password", "");
    end();
    return value;
  }

  void saveAPIKey(const String& apiKey) {

    Serial.println(apiKey);

    begin(false);
    preferences.putString("api_key", apiKey);
    end();
  }

  String getAPIKey() {
    begin(true);
    String value = preferences.getString("api_key", "");
    end();
    return value;
  }

  void saveUserConfig(const String& userID, const String& userName) {
    begin(false);
    preferences.putString("userID", userID);
    preferences.putString("userName", userName);
    end();
  }

  String getUserName() {
    begin(true);
    String value = preferences.getString("userName", "");
    end();
    return value;
  }

  String getUserID() {
    begin(true);
    String value = preferences.getString("userID", "");
    end();
    return value;
  }

  void save(const char* key, const String& value) {
    begin(false);
    preferences.putString(key, value);
    end();
  }

  String get(const char* key) {
    begin(true);
    String value = preferences.getString(key, "");
    end();
    return value;
  }

  void clear() {
    begin(false);
    preferences.clear();  // Elimina tutte le preferenze salvate
    end();
    preferences.begin("my-app", false);
    preferences.clear();
    end();
  }

  void clearAll() {
    Serial.println("🗑️ Clearing all preferences...");
    begin(false);
    preferences.clear();
    end();
    Serial.println("✅ All preferences cleared!");
  }

  void end() {
    preferences.end();  // Rilascia risorse
  }

private:
  Preferences preferences;

  const char* space = "my-app";

  const int DEFAULT_BRIGHTNESS = 80;
  const String DEFAULT_LED_COLOR = "azure";
  const String DEFAULT_LANGUAGE = "it";
  const String DEFAULT_THEME = "light";

  void initDeviceSettings() {
    if (get("conf.brightness").equals("")) {
      save("conf.brightness", String(DEFAULT_BRIGHTNESS));
    }

    if (get("conf.ledColor").equals("")) {
      save("conf.ledColor", DEFAULT_LED_COLOR);
    }

    if (get("conf.language").equals("")) {
      save("conf.language", DEFAULT_LANGUAGE);
    }

    if (get("conf.theme").equals("")) {
      save("conf.theme", DEFAULT_THEME);
    }
  }
};

#endif
