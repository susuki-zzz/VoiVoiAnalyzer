// VoiVoi GUI Application - Localization Manager
// License: GPLv3

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <unordered_map>
#include <vector>

// Don't redefine TRANS if JUCE already defined it
#ifndef TRANS
    #define TRANS(x) (yvc::app::LocalizationManager::getInstance().getString(x))
#endif

namespace yvc::app {

class LocalizationManager {
public:
    enum class Language {
        English = 0,
        Japanese = 1
    };

    static LocalizationManager& getInstance();

    void setLanguage(Language language);
    Language getCurrentLanguage() const { return currentLanguage_; }

    juce::String getString(const juce::String& key) const;
    juce::String getLanguageName(Language language) const;

    std::vector<Language> getAvailableLanguages() const;

    void saveLanguagePreference();
    void loadLanguagePreference();

private:
    LocalizationManager();
    ~LocalizationManager() = default;

    // Disable copying
    LocalizationManager(const LocalizationManager&) = delete;
    LocalizationManager& operator=(const LocalizationManager&) = delete;

    void initializeTranslations();
    void loadLanguageData(Language language);

    Language currentLanguage_ = Language::English;
    std::unordered_map<std::string, std::string> translations_;
};

} // namespace yvc::app
