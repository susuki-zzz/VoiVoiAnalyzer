// VoiVoi GUI Application - Localization Manager
// License: GPLv3

#pragma once

#ifdef JUCE_CORE_H_INCLUDED
    #include <juce_gui_basics/juce_gui_basics.h>
    #define USE_JUCE 1
    #define TRANS(x) (yvc::app::LocalizationManager::getInstance().getString(x))
#else
    #define USE_JUCE 0
    #include <string>
    #define TRANS(x) (yvc::app::LocalizationManager::getInstance().getString(x))
#endif

#include <unordered_map>
#include <vector>

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

#if USE_JUCE
    juce::String getString(const juce::String& key) const;
    juce::String getLanguageName(Language language) const;
#else
    std::string getString(const std::string& key) const;
    std::string getLanguageName(Language language) const;
#endif

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
