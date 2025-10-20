// helper/NTPManager.cpp
#include "./NTPManager.h"

namespace CloudMouse {

// Static member initialization
bool NTPManager::timeInitialized = false;
const char* NTPManager::ntpServer1 = "pool.ntp.org";
const char* NTPManager::ntpServer2 = "time.nist.gov";
const char* NTPManager::ntpServer3 = "time.google.com";
const long NTPManager::gmtOffset_sec = 3600;      // GMT+1 for Italy
const int NTPManager::daylightOffset_sec = 3600;  // DST +1 hour

void NTPManager::init() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("⏰ WiFi not connected - cannot initialize NTP");
    return;
  }

  Serial.println("⏰ Initializing NTP...");

  // Configure time with multiple NTP servers for reliability
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2, ntpServer3);

  // Wait for time to be set (max 10 seconds)
  int timeout = 0;
  while (!isTimeSet() && timeout < 100) {
    delay(100);
    timeout++;
  }

  if (isTimeSet()) {
    timeInitialized = true;
    Serial.println("✅ NTP synchronized successfully");
    printCurrentTime();
  } else {
    Serial.println("❌ NTP synchronization failed");
  }
}

bool NTPManager::isTimeSet() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return false;
  }
  // Check if year is reasonable (2024+)
  return (timeinfo.tm_year + 1900) >= 2024;
}

String NTPManager::getCurrentDateTime() {
  if (!timeInitialized || !isTimeSet()) {
    Serial.println("⚠️ NTP not initialized or time not set");
    return "1970-01-01 00:00:00";  // Unix epoch as fallback
  }

  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("❌ Failed to obtain time");
    return "1970-01-01 00:00:00";
  }

  char buffer[20];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(buffer);
}

String NTPManager::getCurrentDate() {
  if (!timeInitialized || !isTimeSet()) {
    Serial.println("⚠️ NTP not initialized or time not set");
    return "1970-01-01";
  }

  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("❌ Failed to obtain time");
    return "1970-01-01";
  }

  char buffer[11];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d", &timeinfo);
  return String(buffer);
}

String NTPManager::getCurrentTime() {
  if (!timeInitialized || !isTimeSet()) {
    return "00:00:00";
  }

  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "00:00:00";
  }

  char buffer[9];
  strftime(buffer, sizeof(buffer), "%H:%M:%S", &timeinfo);
  return String(buffer);
}

void NTPManager::printCurrentTime() {
  if (!isTimeSet()) {
    Serial.println("⏰ Time not available");
    return;
  }

  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    Serial.printf("⏰ Current time: %s\n", getCurrentDateTime().c_str());
    Serial.printf("⏰ Current date: %s\n", getCurrentDate().c_str());
  }
}

String NTPManager::getCurrentDateTimeUTC() {
  if (!timeInitialized || !isTimeSet()) {
    Serial.println("⚠️ NTP not initialized or time not set");
    return "1970-01-01T00:00:00Z";
  }

  // 🎯 Usa time() per UTC timestamp
  time_t now = time(nullptr);
  struct tm* utcTime = gmtime(&now);  // 🎯 gmtime = UTC!

  if (!utcTime) {
    Serial.println("❌ Failed to get UTC time");
    return "1970-01-01T00:00:00Z";
  }

  char buffer[25];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", utcTime);
  return String(buffer);
}

String NTPManager::getCurrentDateUTC() {
  if (!timeInitialized || !isTimeSet()) {
    return "1970-01-01";
  }

  time_t now = time(nullptr);
  struct tm* utcTime = gmtime(&now);

  if (!utcTime) {
    return "1970-01-01";
  }

  char buffer[12];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d", utcTime);
  return String(buffer);
}

String NTPManager::getCurrentTimeUTC() {
  if (!timeInitialized || !isTimeSet()) {
    return "00:00:00";
  }

  time_t now = time(nullptr);
  struct tm* utcTime = gmtime(&now);

  if (!utcTime) {
    return "00:00:00";
  }

  char buffer[10];
  strftime(buffer, sizeof(buffer), "%H:%M:%S", utcTime);
  return String(buffer);
}

void NTPManager::printCurrentTimeUTC() {
  if (!isTimeSet()) {
    Serial.println("⏰ Time not available");
    return;
  }

  time_t now = time(nullptr);
  struct tm* utcTime = gmtime(&now);
  struct tm localTimeInfo;
  getLocalTime(&localTimeInfo);

  if (utcTime) {
    Serial.printf("🌍 UTC time: %s\n", getCurrentDateTimeUTC().c_str());
    Serial.printf("🇮🇹 Local time: %s\n", getCurrentDateTime().c_str());

    // 🎯 Mostra differenza
    int offsetHours = (localTimeInfo.tm_hour - utcTime->tm_hour + 24) % 24;
    if (offsetHours > 12) offsetHours -= 24;  // Handle day boundary
    Serial.printf("⏰ Timezone offset: UTC%+d\n", offsetHours);
  }
}

time_t NTPManager::getEpochTime() {
  if (!timeInitialized || !isTimeSet()) {
    Serial.println("⚠️ NTP not initialized or time not set");
    return 0;
  }
  return time(nullptr);  // ⏳ Epoch time in secondi
}

}  // namespace CloudMouse
