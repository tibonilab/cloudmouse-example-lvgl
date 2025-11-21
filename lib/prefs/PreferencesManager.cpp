/**
 * CloudMouse SDK - Preferences Manager Implementation
 *
 * Implementation of persistent storage management using ESP32 NVS (Non-Volatile Storage).
 * Handles device configuration, WiFi credentials with safe access patterns.
 */

#include "PreferencesManager.h"

namespace CloudMouse::Prefs
{
    // ============================================================================
    // SYSTEM INITIALIZATION
    // ============================================================================

    void PreferencesManager::init()
    {
        // Create mutex for thread-safe access
        if (nvsMutex == NULL)
        {
            nvsMutex = xSemaphoreCreateMutex();
            if (nvsMutex == NULL)
            {
                Serial.println("❌ FATAL: Failed to create NVS mutex!");
            }
            else
            {
                Serial.println("✅ NVS mutex created");
            }
        }

        initDeviceSettings();
    }

    void PreferencesManager::begin(bool readOnly)
    {
        if (nvsMutex != NULL)
        {
            xSemaphoreTake(nvsMutex, portMAX_DELAY);
        }

        preferences.begin(space, readOnly);
    }

    void PreferencesManager::end()
    {
        preferences.end();

        if (nvsMutex != NULL)
        {
            xSemaphoreGive(nvsMutex);
        }
    }

    // ============================================================================
    // WIFI CREDENTIAL MANAGEMENT
    // ============================================================================

    bool PreferencesManager::hasWiFiCredentials()
    {
        return !(getWiFiSSID().equals("") || getWiFiPassword().equals(""));
    }

    void PreferencesManager::saveWiFiCredentials(const String &ssid, const String &password)
    {
        begin(false);
        preferences.putString("wifi_ssid", ssid);
        preferences.putString("wifi_password", password);
        end();
    }

    String PreferencesManager::getWiFiSSID()
    {
        begin(true);
        String value = preferences.getString("wifi_ssid", "");
        end();
        return value;
    }

    String PreferencesManager::getWiFiPassword()
    {
        begin(true);
        String value = preferences.getString("wifi_password", "");
        end();
        return value;
    }

    // ============================================================================
    // GENERIC STORAGE INTERFACE
    // ============================================================================

    bool PreferencesManager::save(const char *key, const String &value)
    {
        begin(false);
        preferences.putString(key, value);
        end();
        return true;
    }

    String PreferencesManager::get(const char *key)
    {
        begin(true);
        String value = preferences.getString(key, "");
        end();
        return value;
    }

    // ============================================================================
    // BATCH OPERATIONS
    // ============================================================================

    bool PreferencesManager::beginBatch(bool readOnly)
    {

        // Lock mutex FIRST
        if (nvsMutex != NULL)
        {
            xSemaphoreTake(nvsMutex, portMAX_DELAY);
        }

        // Check if already open (nested call)
        if (batchOpen)
        {
            Serial.println("⚠️ Batch already open (nested call), keeping it open");
            batchDepth++;
            return true;
        }

        // Try up to 3 times with small delay
        for (int attempt = 0; attempt < 3; attempt++)
        {
            batchOpen = preferences.begin(space, readOnly);

            if (batchOpen)
            {
                batchDepth = 1;
                if (attempt > 0)
                {
                    Serial.printf("✅ Batch opened on attempt %d\n", attempt + 1);
                }
                return true;
            }

            Serial.printf("⚠️ Batch open failed (attempt %d/3), retrying...\n", attempt + 1);
            delay(10);
        }

        Serial.printf("❌ Batch open FAILED after 3 attempts! (namespace=%s)\n", space);
        
        // Release mutex on failure
        if (nvsMutex != NULL)
        {
            xSemaphoreGive(nvsMutex);
        }
        
        return false;
    }

    void PreferencesManager::endBatch()
    {
        if (!batchOpen)
        {
            // Not open, but still release mutex if taken
            if (nvsMutex != NULL && batchDepth > 0)
            {
                xSemaphoreGive(nvsMutex);
                batchDepth = 0;
            }
            return;
        }

        // Handle nested calls
        batchDepth--;
        if (batchDepth > 0)
        {
            Serial.printf("⚠️ Nested batch, depth now: %d\n", batchDepth);
            return;
        }

        // Actually close
        preferences.end();
        batchOpen = false;
        batchDepth = 0;

        // Release mutex LAST
        if (nvsMutex != NULL)
        {
            xSemaphoreGive(nvsMutex);
        }
    }

    bool PreferencesManager::putString(const char *key, const String &value)
    {
        if (!batchOpen)
        {
            // Fallback to normal save
            return save(key, value);
        }

        return preferences.putString(key, value.c_str());
    }

    String PreferencesManager::getString(const char *key, const String &defaultValue)
    {
        if (!batchOpen)
        {
            // Fallback to normal get
            return get(key);
        }

        return preferences.getString(key, defaultValue.c_str());
    }

    // ============================================================================
    // RESET OPERATIONS
    // ============================================================================

    void PreferencesManager::clear()
    {
        begin(false);
        preferences.clear(); // Clear all preferences in current namespace
        end();

        // Reinitialize namespace to ensure clean state
        preferences.begin(space, false);
        preferences.clear();
        end();
    }

    void PreferencesManager::clearAll()
    {
        Serial.println("🗑️ Clearing all preferences...");
        begin(false);
        preferences.clear();
        end();
        Serial.println("✅ All preferences cleared!");
    }

    // ============================================================================
    // DEVICE SETTINGS INITIALIZATION
    // ============================================================================

    void PreferencesManager::initDeviceSettings()
    {
        // Initialize brightness setting if not present
        if (get("conf.brightness").equals(""))
        {
            save("conf.brightness", String(DEFAULT_BRIGHTNESS));
        }

        // Initialize LED color setting if not present
        if (get("conf.ledColor").equals(""))
        {
            save("conf.ledColor", DEFAULT_LED_COLOR);
        }

        // Initialize language setting if not present
        if (get("conf.language").equals(""))
        {
            save("conf.language", DEFAULT_LANGUAGE);
        }

        // Initialize theme setting if not present
        if (get("conf.theme").equals(""))
        {
            save("conf.theme", DEFAULT_THEME);
        }
    }

} // namespace CloudMouse::Prefs