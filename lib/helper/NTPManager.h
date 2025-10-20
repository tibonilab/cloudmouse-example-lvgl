// helper/NTPManager.h
#pragma once
#include <WiFi.h>
#include <time.h>
#include <Arduino.h>

namespace CloudMouse {

class NTPManager {
public:
  static void init();
  static bool isTimeSet();
  static String getCurrentDateTime();
  static String getCurrentDate();
  static String getCurrentTime();
  static void printCurrentTime();

  static String getCurrentDateTimeUTC();
  static String getCurrentDateUTC();
  static String getCurrentTimeUTC();
  static void printCurrentTimeUTC();
  static time_t getEpochTime();

private:
  static bool timeInitialized;
  static const char* ntpServer1;
  static const char* ntpServer2;
  static const char* ntpServer3;
  static const long gmtOffset_sec;
  static const int daylightOffset_sec;
};

}  // namespace CloudMouse
