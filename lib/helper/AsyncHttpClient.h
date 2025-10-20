#ifndef ASYNCHTTPCLIENT_H
#define ASYNCHTTPCLIENT_H

#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <functional>

class Header {
public:
  String key;
  String value;
  Header(String k, String v)
    : key(k), value(v) {}
};

class AsyncHttpClient {
public:
  AsyncHttpClient()
    : loading(false), useInsecure(false) {}

  ~AsyncHttpClient() {
    // Cleanup più semplice senza task
    if (loading) {
      Serial.println("⚠️ AsyncHttpClient destroyed while loading");
    }
  }

  bool isLoading() {
    return loading;
  }

  void init(String method, String endpoint) {
    requestMethod = method;
    requestEndpoint = endpoint;
    // Pulisci headers precedenti
    headers.clear();
  }

  void setInsecure(bool insecure = true) {
    useInsecure = insecure;
  }

  void send(String payload = "") {
    if (loading) {
      Serial.println("Richiesta già in corso...");
      return;
    }

    // Rate limiting come prima
    static unsigned long lastRequest = 0;
    unsigned long now = millis();
    if (now - lastRequest < 1000) {
      unsigned long waitTime = 1000 - (now - lastRequest);
      Serial.printf("⏱️ Rate limiting: aspetto %lu ms\n", waitTime);
      delay(waitTime);
    }
    lastRequest = millis();

    // Diagnostica memoria
    Serial.printf("🧠 Memoria prima HTTP request:\n");
    Serial.printf("   Free heap: %d bytes\n", ESP.getFreeHeap());
    Serial.printf("   Min free: %d bytes\n", ESP.getMinFreeHeap());
    Serial.printf("   Largest block: %d bytes\n", ESP.getMaxAllocHeap());

    // Controllo preventivo memoria
    if (ESP.getFreeHeap() < 30000) {  // 30KB minimo (ridotto senza task)
      Serial.println("❌ Memoria insufficiente per HTTP request!");
      if (errorCallback) {
        errorCallback(-999);
      }
      return;
    }

    loading = true;
    requestPayload = payload;

    // 🔥 CHIAMATA DIRETTA SENZA TASK
    performRequest();
  }

  void onError(std::function<void(int)> callback) {
    this->errorCallback = callback;
  }

  void onResponse(std::function<void(String)> callback) {
    this->responseCallback = callback;
  }

  // 🆕 CALLBACK PER LOADING STATE
  void onLoadingChange(std::function<void(bool)> callback) {
    this->loadingCallback = callback;
  }

  void addHeader(String key, String value) {
    headers.push_back(Header(key, value));
  }

private:
  volatile bool loading;
  bool useInsecure;

  std::function<void(String)> responseCallback;
  std::function<void(int)> errorCallback;
  std::function<void(bool)> loadingCallback;  // 🆕 Callback per loading state
  std::vector<Header> headers;

  String requestMethod;
  String requestEndpoint;
  String requestPayload;

  // 🔥 OGGETTO STATICO INVECE CHE PUNTATORE - meno problemi di gestione memoria
  static WiFiClientSecure& getSecureClient() {
    static WiFiClientSecure instance;
    return instance;
  }
  
  const char* root_ca =
    "-----BEGIN CERTIFICATE-----\n"
    "MIICCTCCAY6gAwIBAgINAgPlwGjvYxqccpBQUjAKBggqhkjOPQQDAzBHMQswCQYD\n"
    "VQQGEwJVUzEiMCAGA1UEChMZR29vZ2xlIFRydXN0IFNlcnZpY2VzIExMWzEUMBIG\n"
    "A1UEAxMLR1RTIFJvb3QgUjQwHhcNMTYwNjIyMDAwMDAwWhcNMzYwNjIyMDAwMDAw\n"
    "WjBHMQswCQYDVQQGEwJVUzEiMCAGA1UEChMZR29vZ2xlIFRydXN0IFNlcnZpY2Vz\n"
    "IExMQzEUMBIGA1UEAxMLR1RTIFJvb3QgUjQwdjAQBgcqhkjOPQIBBgUrgQQAIgNi\n"
    "AATzdHOnaItgrkO4NcWBMHtLSZ37wWHO5t5GvWvVYRg1rkDdc/eJkTBa6zzuhXyi\n"
    "QHY7qca4R9gq55KRanPpsXI5nymfopjTX15YhmUPoYRlBtHci8nHc8iMai/lxKvR\n"
    "HYqjQjBAMA4GA1UdDwEB/wQEAwIBhjAPBgNVHRMBAf8EBTADAQH/MB0GA1UdDgQW\n"
    "BBSATNbrdP9JNqPV2Py1PsVq8JQdjDAKBggqhkjOPQQDAwNpADBmAjEA6ED/g94D\n"
    "9J+uHXqnLrmvT/aDHQ4thQEd0dlq7A/Cr8deVl5c1RxYIigL9zC2L7F8AjEA8GE8\n"
    "p/SgguMh1YQdc4acLa/KNJvxn7kjNuK8YAOdgLOaVsjh4rsUecrNIdSUtUlD\n"
    "-----END CERTIFICATE-----\n";

  void performRequest() {
    HTTPClient https;
    
    // 🔥 NOTIFICA LOADING START 
    notifyLoadingStart();
    
    // 🔥 FORZA IL CORE A ELABORARE SUBITO L'EVENTO
    // Se hai una funzione nel Core tipo processEvents(), chiamala qui
    // Oppure usa yield() multipli
    for(int i = 0; i < 10; i++) {
        yield();
        delay(5);  // Totale 50ms per elaborare evento
    }
    
    Serial.printf("🌐 Memoria libera PRIMA richiesta: %d bytes\n", ESP.getFreeHeap());

    bool isHttps = requestEndpoint.startsWith("https://");

    if (isHttps) {
      // 🔥 USA OGGETTO STATICO - no new/delete, meno memory leak
      WiFiClientSecure& client = getSecureClient();
      
      if (useInsecure) {
        client.setInsecure();
      } else {
        client.setCACert(root_ca);
      }

      client.setTimeout(15000);

      if (!https.begin(client, requestEndpoint)) {
        Serial.println("❌ Errore inizializzazione HTTPS");
        notifyError(-1);
        return;
      }
    } else {
      https.begin(requestEndpoint);
    }

    https.setReuse(false);
    https.setTimeout(15000);

    Serial.printf("📡 Invio richiesta a: %s\n", requestEndpoint.c_str());

    // Aggiungi headers
    for (const Header& header : headers) {
      https.addHeader(header.key, header.value);
    }

    int httpCode = -1;

    // Esegui richiesta in base al metodo
    if (requestMethod == "GET") {
      httpCode = https.GET();
    } else if (requestMethod == "POST") {
      httpCode = https.POST(requestPayload);
    } else if (requestMethod == "PUT") {
      httpCode = https.PUT(requestPayload);
    }

    Serial.printf("📊 Response code: %d\n", httpCode);

    if (httpCode > 0) {
      size_t responseSize = https.getSize();
      Serial.printf("📦 Response size: %d bytes\n", responseSize);

      // Controllo memoria più conservativo
      if (responseSize > 0 && ESP.getFreeHeap() < (responseSize * 2)) {
        Serial.println("❌ Memoria insufficiente per response!");
        cleanup(https, isHttps);
        notifyError(-2);
        return;
      }

      String payload = https.getString();
      
      // Cleanup PRIMA della callback per liberare memoria
      cleanup(https, isHttps);
      
      // 🔥 DA TEMPO AL MAIN LOOP DI ELABORARE GLI EVENTI
      yield();
      delay(10);  // Piccolo delay per permettere al Core di processare API_LOADING
      
      // Notifica successo
      notifyResponse(payload);

    } else {
      Serial.printf("❌ Errore HTTP: %s\n", https.errorToString(httpCode).c_str());
      cleanup(https, isHttps);
      
      // 🔥 DA TEMPO AL MAIN LOOP 
      yield();
      delay(10);
      
      notifyError(httpCode);
    }

    Serial.printf("🧠 Memoria libera DOPO richiesta: %d bytes\n", ESP.getFreeHeap());
  }

  // 🔥 CLEANUP CENTRALIZZATO E SICURO
  void cleanup(HTTPClient& https, bool isHttps) {
    https.end();
    
    if (isHttps) {
      WiFiClientSecure& client = getSecureClient();
      client.flush();
      client.stop();
      
      // Verifica disconnessione
      int retries = 3;  // Ridotto da 5 a 3
      while (client.connected() && retries-- > 0) {
        delay(50);  // Ridotto da 100 a 50ms
        Serial.println("⏳ Aspettando disconnessione...");
      }
      
      if (client.connected()) {
        Serial.println("⚠️ Client ancora connesso dopo cleanup");
      }
    }
    
    loading = false;  // 🔥 IMPORTANTE: reset loading SEMPRE
    
    // 🔥 NOTIFICA FINE RICHIESTA
    notifyLoadingEnd();
  }

  // 🔥 NOTIFICHE SEMPLIFICATE (no thread-safety needed)
  void notifyResponse(const String& payload) {
    if (responseCallback) {
      responseCallback(payload);
    }
  }

  void notifyError(int code) {
    if (errorCallback) {
      errorCallback(code);
    }
  }

  // 🆕 NOTIFICHE LOADING STATE
  void notifyLoadingStart() {
    if (loadingCallback) {
      loadingCallback(true);  // Loading iniziato
    }
  }

  void notifyLoadingEnd() {
    if (loadingCallback) {
      loadingCallback(false);  // Loading finito
    }
  }
};

#endif
// #ifndef ASYNCHTTPCLIENT_H
// #define ASYNCHTTPCLIENT_H

// #include <HTTPClient.h>
// #include <WiFiClientSecure.h>
// #include <ArduinoJson.h>
// #include <FreeRTOS.h>
// #include <functional>

// class Header {
// public:
//   String key;
//   String value;
//   Header(String k, String v)
//     : key(k), value(v) {}
// };

// class AsyncHttpClient {
// public:
//   AsyncHttpClient()
//     : loading(false), useInsecure(false) {}

//   ~AsyncHttpClient() {
//     // Aspetta che eventuali task finiscano
//     while (loading) {
//       delay(10);
//     }
//   }

//   bool isLoading() {
//     return loading;
//   }

//   void init(String method, String endpoint) {
//     requestMethod = method;
//     requestEndpoint = endpoint;
//   }

//   void setInsecure(bool insecure = true) {
//     useInsecure = insecure;
//   }

//   void send(String payload = "") {
//     if (loading) {
//       Serial.println("Richiesta già in corso...");
//       return;
//     }

//     // 🆕 Delay minimo tra richieste per evitare saturazione
//     static unsigned long lastRequest = 0;
//     unsigned long now = millis();
//     if (now - lastRequest < 1000) {  // Min 1 secondo tra richieste
//       unsigned long waitTime = 1000 - (now - lastRequest);
//       Serial.printf("⏱️ Rate limiting: aspetto %lu ms\n", waitTime);
//       delay(waitTime);
//     }
//     lastRequest = millis();

//     // 🆕 DIAGNOSTICA MEMORIA
//     Serial.printf(" Memoria prima HTTP task:\n");
//     Serial.printf("   Free heap: %d bytes\n", ESP.getFreeHeap());
//     Serial.printf("   Min free: %d bytes\n", ESP.getMinFreeHeap());
//     Serial.printf("   Largest block: %d bytes\n", ESP.getMaxAllocHeap());
//     Serial.printf("   Tasks count: %d\n", uxTaskGetNumberOfTasks());

//     // Controllo preventivo
//     if (ESP.getFreeHeap() < 40000) {  // 40KB minimo
//       Serial.println("Memoria insufficiente per task HTTP!");
//       if (errorCallback) {
//         errorCallback(-999);  // Codice custom per memoria
//       }
//       return;
//     }

//     loading = true;
//     requestPayload = payload;

//     // Stack ridotto + priority bassa
//     BaseType_t result = xTaskCreate([](void* param) {
//       Serial.println("🧵 Task HTTP INIZIATO");
//       AsyncHttpClient* client = static_cast<AsyncHttpClient*>(param);
//       client->performRequest();
//       Serial.println("🧵 Task HTTP FINITO");
//       vTaskDelete(NULL);  // Rimuovo delay che non viene mai eseguito
//     },
//                                     "HTTP", 12288, this, 1, NULL);  // Stack ridotto per compensare i font

//     if (result != pdPASS) {
//       Serial.println("Errore creazione task HTTP!");
//       Serial.printf("   Result code: %d\n", result);
//       loading = false;

//       if (errorCallback) {
//         errorCallback(-998);  // Codice custom per task creation
//       }
//     } else {
//       Serial.println("Task HTTP creato con successo");
//     }
//   }

//   void onError(std::function<void(int)> callback) {
//     this->errorCallback = callback;
//   }

//   void onResponse(std::function<void(String)> callback) {
//     this->responseCallback = callback;
//   }

//   void addHeader(String key, String value) {
//     headers.push_back(Header(key, value));
//   }

// private:
//   volatile bool loading;
//   bool useInsecure;
//   TaskHandle_t httpTaskHandle = NULL;

//   std::function<void(String)> responseCallback;
//   std::function<void(int)> errorCallback;
//   std::vector<Header> headers;

//   String requestMethod;
//   String requestEndpoint;
//   String requestPayload;

//   const char* root_ca =
//     "-----BEGIN CERTIFICATE-----\n"
//     "MIICCTCCAY6gAwIBAgINAgPlwGjvYxqccpBQUjAKBggqhkjOPQQDAzBHMQswCQYD\n"
//     "VQQGEwJVUzEiMCAGA1UEChMZR29vZ2xlIFRydXN0IFNlcnZpY2VzIExMQzEUMBIG\n"
//     "A1UEAxMLR1RTIFJvb3QgUjQwHhcNMTYwNjIyMDAwMDAwWhcNMzYwNjIyMDAwMDAw\n"
//     "WjBHMQswCQYDVQQGEwJVUzEiMCAGA1UEChMZR29vZ2xlIFRydXN0IFNlcnZpY2Vz\n"
//     "IExMQzEUMBIGA1UEAxMLR1RTIFJvb3QgUjQwdjAQBgcqhkjOPQIBBgUrgQQAIgNi\n"
//     "AATzdHOnaItgrkO4NcWBMHtLSZ37wWHO5t5GvWvVYRg1rkDdc/eJkTBa6zzuhXyi\n"
//     "QHY7qca4R9gq55KRanPpsXI5nymfopjTX15YhmUPoYRlBtHci8nHc8iMai/lxKvR\n"
//     "HYqjQjBAMA4GA1UdDwEB/wQEAwIBhjAPBgNVHRMBAf8EBTADAQH/MB0GA1UdDgQW\n"
//     "BBSATNbrdP9JNqPV2Py1PsVq8JQdjDAKBggqhkjOPQQDAwNpADBmAjEA6ED/g94D\n"
//     "9J+uHXqnLrmvT/aDHQ4thQEd0dlq7A/Cr8deVl5c1RxYIigL9zC2L7F8AjEA8GE8\n"
//     "p/SgguMh1YQdc4acLa/KNJvxn7kjNuK8YAOdgLOaVsjh4rsUecrNIdSUtUlD\n"
//     "-----END CERTIFICATE-----\n";

//   void performRequest() {
//     HTTPClient https;
//     WiFiClientSecure* client = nullptr;

//     Serial.print("Memoria libera PRIMA richiesta: ");
//     Serial.println(ESP.getFreeHeap());

//     bool isHttps = requestEndpoint.startsWith("https://");

//     if (isHttps) {
//       client = new WiFiClientSecure();

//       if (useInsecure) {
//         client->setInsecure();
//       } else {
//         client->setCACert(root_ca);
//       }

//       client->setTimeout(15000);

//       if (!https.begin(*client, requestEndpoint)) {
//         Serial.println("Errore inizializzazione HTTPS");
//         delete client;
//         notifyError(-1);
//         return;
//       }
//     } else {
//       https.begin(requestEndpoint);
//     }

//     https.setReuse(false);

//     Serial.print("Invio richiesta a: ");
//     Serial.println(requestEndpoint);

//     https.setTimeout(15000);

//     for (const Header& header : headers) {
//       https.addHeader(header.key, header.value);
//     }

//     int httpCode = -1;

//     if (requestMethod == "GET") {
//       httpCode = https.GET();
//     } else if (requestMethod == "POST") {
//       httpCode = https.POST(requestPayload);
//     } else if (requestMethod == "PUT") {
//       httpCode = https.PUT(requestPayload);
//     }

//     Serial.print("Response code: ");
//     Serial.println(httpCode);

//     if (httpCode > 0) {
//       size_t responseSize = https.getSize();

//       Serial.print("Response size: ");
//       Serial.println(responseSize);

//       // Controllo memoria più conservativo
//       if (responseSize > 0 && ESP.getFreeHeap() < (responseSize * 2)) {
//         Serial.println("Memoria insufficiente!");
//         https.end();
//         if (client) {  // 🆕 Controllo se client esiste (solo per HTTPS)
//           client->stop();
//           delay(50);
//           Serial.printf("Connessione chiusa? %s\n", client->connected() ? "NO" : "SÌ");
//           delete client;
//         }
//         notifyError(-2);
//         return;
//       }

//       String payload = https.getString();

//       // Notifica nel main thread in modo sicuro
//       notifyResponse(payload);

//     } else {
//       Serial.printf("Errore HTTP: %s\n", https.errorToString(httpCode).c_str());
//       notifyError(httpCode);
//     }

//     // CLEANUP SICURO
//     https.end();
    
//     if (client) {  // 🆕 Cleanup solo se client esiste (HTTPS)
//       client->flush();
//       client->stop();

//       // Verifica che sia davvero disconnesso
//       int retries = 5;
//       while (client->connected() && retries-- > 0) {
//         delay(100);
//         Serial.println("Aspettando disconnessione...");
//       }

//       delete client;

//       // Se connection refused, NON fare delay nel task
//       if (httpCode == HTTPC_ERROR_CONNECTION_REFUSED) {
//         Serial.println("Connection refused - no delay in task");
//         loading = false;  // 🆕 RESET LOADING 
//         return;  // 🆕 ESCI SUBITO, senza delay
//       }
//     }

//     Serial.print("Memoria libera DOPO richiesta: ");
//     Serial.println(ESP.getFreeHeap());

//     loading = false;  // 🆕 IMPORTANTE: resetta loading PRIMA di tutto
//   }

//   // Metodi per notificare in modo thread-safe
//   void notifyResponse(const String& payload) {
//     if (responseCallback) {
//       // Copia la stringa per evitare problemi di concorrenza
//       String safeCopy = payload;
//       responseCallback(safeCopy);
//     }
//   }

//   void notifyError(int code) {
//     if (errorCallback) {
//       errorCallback(code);
//     }
    
//     // 🆕 Delay nel main thread per connection refused
//     if (code == HTTPC_ERROR_CONNECTION_REFUSED) {
//       Serial.println("Scheduling retry delay in main thread...");
//       // Qui potresti schedulare un timer o semplicemente aspettare
//       // che l'utente riprovi invece di fare delay automatico
//     }
//   }
// };

// #endif