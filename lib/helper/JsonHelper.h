#ifndef JSON_HELPER_H
#define JSON_HELPER_H

#include <ArduinoJson.h>
#include <esp_heap_caps.h>

class JsonHelper {
public:
  // Funzione per decodificare il payload JSON della risposta HTTP
  static DynamicJsonDocument decodeResponse(const String& payload) {
    // Alloca memoria per il documento JSON (usa PSRAM se disponibile)
    const size_t capacity = JSON_OBJECT_SIZE(20) + payload.length() + 1000; // allocazione dinamica
    
    DynamicJsonDocument doc(capacity);

    // Deserializza il payload JSON
    DeserializationError error = deserializeJson(doc, payload);

    // Controlla se ci sono errori nella deserializzazione
    if (error) {
      Serial.print("❌ Errore deserializzazione JSON: ");
      Serial.println(error.c_str());
      return DynamicJsonDocument(0);
    }

    return doc;
  }

  static void sortJsonArray(JsonArray& arr) {
    int size = arr.size();

    // Bubble Sort: cicla e scambia gli elementi se l'ordine è sbagliato
    for (int i = 0; i < size - 1; i++) {
      for (int j = 0; j < size - i - 1; j++) {
        if (arr[j]["ord"].as<int>() > arr[j + 1]["ord"].as<int>()) {
          // Scambia i due oggetti direttamente senza creare nuovi oggetti nell'array
          JsonObject obj1 = arr[j];
          JsonObject obj2 = arr[j + 1];

          // Swap: nome e ord tra obj1 e obj2
          String tempName = obj1["name"].as<String>();
          String tempUUID = obj1["uuid"].as<String>();
          int tempOrd = obj1["ord"].as<int>();
          bool tempCompleted = obj1["completed"].as<bool>();
          bool tempStarted = obj1["started"].as<bool>();

          obj1["name"] = obj2["name"];
          obj1["ord"] = obj2["ord"];
          obj1["completed"] = obj2["completed"];
          obj1["started"] = obj2["started"];
          obj1["uuid"] = obj2["uuid"];

          obj2["name"] = tempName;
          obj2["ord"] = tempOrd;
          obj2["completed"] = tempCompleted;
          obj2["started"] = tempStarted;
          obj2["uuid"] = tempUUID;
        }
      }
    }
  }
};

#endif