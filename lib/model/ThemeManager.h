#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <string>
#include <map>
#include <stdint.h>
#include <Arduino.h>

class ThemeManager {
public:
    // Costruttore
    ThemeManager();

    // Imposta il tema e lo stile
    void setTheme(const String& theme); // "light" o "dark"
    void setStyle(const String& style); // Es. "red", "blue"
    void setRunningTaskMode(const String& runningTaskMode); // Es. "timer", "busy"

    // Restituisce i colori in funzione del tema e dello stile
    uint16_t getBackgroundColor() const;
    uint16_t getHighlightedColor() const;
    uint16_t getTextColor() const;
    uint16_t getColorForStyle() const;
    uint16_t get(const String& key) const;

    // Restituisce le configurazioni del tema
    String getRunningTaskMode() const;

private:
    String currentTheme = "light"; // Tema corrente ("light" o "dark")
    String currentStyle = "azure"; // Stile corrente (es. "red")
    String currentRunningTaskMode = "timer"; // Running task mode as timer ("timer" o "busy")

    // Tabelle dei colori per temi e stili
    static const std::map<String, std::map<String, uint32_t>> themeColors;
    static const std::map<String, uint32_t> styleColorsHex;

    uint32_t getColorHex(const String& key, const uint32_t fallbackColor = 0x000000) const {
        if (themeColors.count(currentTheme) && 
            themeColors.at(currentTheme).count(key)) {
            return themeColors.at(currentTheme).at(key);
        }
        return fallbackColor; // black
    }

    static uint16_t color565(uint32_t hexColor);
};

#endif // THEMEMANAGER_H
