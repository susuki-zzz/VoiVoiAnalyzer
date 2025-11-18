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

    /// <summary>
    /// Manages UI localization and runtime language switching.
    /// Provides string lookup via keys and persists user language preference.
    /// </summary>
    class LocalizationManager {
    public:
        /// <summary>
        /// Supported languages.
        /// </summary>
        enum class Language {
            English = 0,
            Japanese = 1
        };

        /// <summary>
        /// Gets the singleton instance.
        /// </summary>
        static LocalizationManager& getInstance();

        /// <summary>
        /// Sets the current UI language.
        /// </summary>
        void setLanguage(Language language);

        /// <summary>
        /// Gets the current UI language.
        /// </summary>
        Language getCurrentLanguage() const { return currentLanguage_; }

        /// <summary>
        /// Returns a localized string for a given key.
        /// </summary>
        juce::String getString(const juce::String& key) const;

        /// <summary>
        /// Gets human-readable language name.
        /// </summary>
        juce::String getLanguageName(Language language) const;

        /// <summary>
        /// Returns available languages.
        /// </summary>
        std::vector<Language> getAvailableLanguages() const;

        /// <summary>
        /// Saves current language preference to disk.
        /// </summary>
        void saveLanguagePreference();

        /// <summary>
        /// Loads language preference from disk.
        /// </summary>
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
