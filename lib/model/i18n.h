#ifndef I18N_H
#define I18N_H

#include <map>
#include <string>
#include <Arduino.h>

class i18n {
public:
    struct Language {
        const std::map<String, String> translations;
    };

    // Dichiarazioni delle lingue
    static const Language language_it;
    static const Language language_en;

    // Metodo per impostare la lingua
    static void setLanguage(const String& langKey);

    // Metodo per ottenere la traduzione corrente
    static const String& translate(const String& key);

    // Metodo per costruire chiavi dinamiche e tradurle
    static String translateDynamic(const String& prefix, const String& dynamicKey);

private:
    static const Language* currentLanguage;
};

#endif // I18N_H
