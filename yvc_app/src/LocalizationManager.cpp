// VoiVoi GUI Application - Localization Manager Implementation
// License: GPLv3

#include "LocalizationManager.h"

// Force JUCE usage when compiled with JUCE
#include <juce_core/juce_core.h>
#include <fstream>
#include <filesystem>

namespace yvc::app {

namespace {
    constexpr const char* kLanguagePreferenceKey = "language_preference";
    constexpr const char* kConfigFileName = "voivoi_settings.txt";
}

LocalizationManager& LocalizationManager::getInstance() {
    static LocalizationManager instance;
    return instance;
}

LocalizationManager::LocalizationManager() {
    initializeTranslations();
    loadLanguagePreference();
}

void LocalizationManager::setLanguage(Language language) {
    if (language != currentLanguage_) {
        currentLanguage_ = language;
        loadLanguageData(language);
        saveLanguagePreference();
    }
}

// Always implement JUCE version when building with JUCE
juce::String LocalizationManager::getString(const juce::String& key) const {
    auto it = translations_.find(key.toStdString());
    if (it != translations_.end()) {
        return juce::String(it->second);
    }
    
    // Return key if translation not found (for debugging)
    return "[" + key + "]";
}

juce::String LocalizationManager::getLanguageName(Language language) const {
    switch (language) {
        case Language::English: 
            return "English";
        case Language::Japanese: 
            return "Japanese";
        default: 
            return "Unknown";
    }
}

std::vector<LocalizationManager::Language> LocalizationManager::getAvailableLanguages() const {
    return { Language::English, Language::Japanese };
}

void LocalizationManager::saveLanguagePreference() {
    juce::PropertiesFile::Options options;
    options.applicationName = "VoiVoiAnalyzer";
    options.filenameSuffix = ".settings";
    options.osxLibrarySubFolder = "Application Support";
    
    auto settings = std::make_unique<juce::PropertiesFile>(options);
    settings->setValue(kLanguagePreferenceKey, static_cast<int>(currentLanguage_));
    settings->saveIfNeeded();
}

void LocalizationManager::loadLanguagePreference() {
    juce::PropertiesFile::Options options;
    options.applicationName = "VoiVoiAnalyzer";
    options.filenameSuffix = ".settings";
    options.osxLibrarySubFolder = "Application Support";
    
    auto settings = std::make_unique<juce::PropertiesFile>(options);
    int languageInt = settings->getIntValue(kLanguagePreferenceKey, static_cast<int>(Language::English));
    
    if (languageInt >= 0 && languageInt < static_cast<int>(Language::Japanese) + 1) {
        currentLanguage_ = static_cast<Language>(languageInt);
    }
    
    loadLanguageData(currentLanguage_);
}

void LocalizationManager::initializeTranslations() {
    loadLanguageData(Language::English);
}

void LocalizationManager::loadLanguageData(Language language) {
    translations_.clear();
    
    if (language == Language::English) {
        // English translations (base language)
        translations_ = {
            // Main UI
            {"app_title", "VoiVoi Analyzer"},
            {"settings", "Settings"},
            {"preset_description_natural", "Balanced metrics for everyday speaking"},
            {"preset_description_phone", "Clarity for VoIP/phone calls"},
            {"preset_description_resonance", "Spectral balance and resonance"},
            {"preset_description_diagnostic", "Complete analysis for assessment"},
            
            // Presets
            {"preset_natural_conversation", "Natural Conversation"},
            {"preset_phone_training", "Phone Training"},
            {"preset_resonance_focus", "Resonance Focus"},
            {"preset_diagnostic", "Diagnostic"},
            
            // Metrics
            {"metric_f0", "F0"},
            {"metric_cpp", "CPP"},
            {"metric_hnr", "HNR"},
            {"metric_spectral_tilt", "Spectral Tilt"},
            {"metric_speech_rate", "Speech Rate"},
            {"metric_pause_ratio", "Pause Ratio"},
            {"metric_voice_activity", "Voice Activity"},
            {"metric_rms", "RMS"},
            {"metric_peak", "Peak"},
            {"metric_crest_factor", "Crest Factor"},
            {"metric_s_centroid", "/s/ Centroid"},
            
            // Units
            {"unit_hz", "Hz"},
            {"unit_db", "dB"},
            {"unit_dbfs", "dBFS"},
            {"unit_db_octave", "dB/oct"},
            {"unit_syl_per_sec", "syl/s"},
            {"unit_ratio", "ratio"},
            
            // Status
            {"status_active", "Active"},
            {"status_idle", "Idle"},
            {"status_fps", "FPS"},
            {"status_cpu", "CPU"},
            {"status_ram", "RAM"},
            {"status_time_left", "Time Left"},
            
            // Settings dialog
            {"settings_title", "VoiVoi Settings"},
            {"settings_audio", "Audio Settings"},
            {"settings_sample_rate", "Sample Rate"},
            {"settings_buffer_size", "Buffer Size"},
            {"settings_performance_mode", "Performance Mode"},
            {"settings_recording", "Recording Settings"},
            {"settings_max_duration", "Maximum Recording Duration"},
            {"settings_auto_save", "Auto Save on Stop"},
            {"settings_language", "Language"},
            {"settings_apply", "Apply"},
            {"settings_cancel", "Cancel"},
            {"settings_ok", "OK"},
            
            // Performance modes
            {"mode_light", "Light (≤40ms)"},
            {"mode_standard", "Standard (≤60ms)"},
            {"mode_diagnostic", "Diagnostic (≤80ms)"},
            
            // Notifications
            {"fps_limited", "Auto limited to %d FPS"},
            {"recording_started", "Recording started"},
            {"recording_stopped", "Recording stopped"},
            {"file_saved", "File saved successfully"},
            
            // Heatmaps
            {"heatmap_f0", "F0 Heatmap"},
            {"heatmap_level", "Level Heatmap"},
            {"heatmap_awaiting_data", "Awaiting data"},
            
            // Privacy
            {"privacy_local_processing", "100% Local Processing"},
            {"privacy_no_network", "No audio transmission over network"},
            {"privacy_ram_only", "RAM-only live analysis"},
            
            // Additional UI elements
            {"display_settings", "Display"},
            {"privacy_settings", "Privacy"},
            {"enable_preprocessing", "Enable Preprocessing"},
            {"advanced_visualization", "Advanced Visualization"},
            {"spectral_analysis", "Spectral Analysis"},
            {"heatmap_resolution", "Heatmap Resolution"},
            {"resolution_high", "High"},
            {"resolution_medium", "Medium"},
            {"resolution_low", "Low"},
            {"comparison", "Comparison"}
        };
    } else if (language == Language::Japanese) {
        // Japanese translations (ASCII-safe for stub version)
        translations_ = {
            // Main UI  
            {"app_title", "VoiVoi Analyzer"},
            {"settings", "Settei"},
            {"preset_description_natural", "Nichijou kaiwa you no barasu toreta metrics"},
            {"preset_description_phone", "VoIP denwa tsuwako no meiryo sei"},
            {"preset_description_resonance", "Spectral balance to kyomei"},
            {"preset_description_diagnostic", "Hyoka you no kanzen kaiseki"},
            
            // Core metrics (keep technical terms in English for consistency)
            {"metric_f0", "Kihon Shuhasu (F0)"},
            {"metric_cpp", "CPP"},
            {"metric_hnr", "Chouha Zatsuon Hi (HNR)"},
            {"metric_spectral_tilt", "Spectral Keisha"},
            {"metric_speech_rate", "Hatsuwa Sokudo"},
            {"metric_pause_ratio", "Pause Hiritsu"},
            {"metric_voice_activity", "Onsei Katsudo"},
            
            // Status
            {"status_active", "Akutibu"},
            {"status_idle", "Taiki chu"},
            {"status_fps", "FPS"},
            {"status_cpu", "CPU"},
            {"status_ram", "RAM"},
            {"status_time_left", "Nokori Jikan"},
            
            // Settings
            {"settings_title", "VoiVoi Settei"},
            {"settings_audio", "Audio Settei"},
            {"settings_language", "Gengo"},
            {"settings_apply", "Tekiyo"},
            {"settings_cancel", "Cancel"},
            {"settings_ok", "OK"},
            
            // Heatmaps
            {"heatmap_awaiting_data", "Data taiki chu"},
            {"spectral_analysis", "Spectral Kaiseki"},
            {"comparison", "Hikaku"},
            
            // All other keys fall back to English
        };
        
        // Add fallbacks for missing keys
        if (translations_.find("display_settings") == translations_.end()) {
            translations_["display_settings"] = "Hyoji Settei";
        }
        if (translations_.find("privacy_settings") == translations_.end()) {
            translations_["privacy_settings"] = "Privacy Settei";
        }
        if (translations_.find("advanced_visualization") == translations_.end()) {
            translations_["advanced_visualization"] = "Kodo Visualization";
        }
    }
}

} // namespace yvc::app
