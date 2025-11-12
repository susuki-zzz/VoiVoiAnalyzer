// VoiVoi GUI Application - Localization Manager
// License: GPLv3

#pragma once

#include <juce_core/juce_core.h>
#include <unordered_map>
#include <string>

namespace yvc::app {

/**
 * @brief Manages multi-language support for the VoiVoi application
 * 
 * Provides runtime language switching with Japanese and English translations.
 * All UI strings are externalized through this system to support localization.
 */
class LocalizationManager {
public:
    enum class Language {
        English,
        Japanese
    };

    static LocalizationManager& getInstance();

    // Set the current language
    void setLanguage(Language language);
    Language getCurrentLanguage() const { return currentLanguage_; }

    // Get localized string by key
    juce::String getString(const juce::String& key) const;

    // Get available languages
    std::vector<Language> getAvailableLanguages() const;
    juce::String getLanguageName(Language language) const;

    // Save/load language preference
    void saveLanguagePreference();
    void loadLanguagePreference();

private:
    LocalizationManager();
    void initializeTranslations();
    void loadLanguageData(Language language);

    Language currentLanguage_ = Language::English;
    std::unordered_map<std::string, std::string> translations_;
};

// Convenience macro for getting localized strings
#define TRANS(key) LocalizationManager::getInstance().getString(key)

} // namespace yvc::app
