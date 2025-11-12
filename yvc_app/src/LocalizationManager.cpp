// VoiVoi GUI Application - Localization Manager Implementation
// License: GPLv3

#include "LocalizationManager.h"
#include <juce_core/juce_core.h>

namespace yvc::app {

namespace {
    constexpr const char* kLanguagePreferenceKey = "language_preference";
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

juce::String LocalizationManager::getString(const juce::String& key) const {
    auto it = translations_.find(key.toStdString());
    if (it != translations_.end()) {
        return juce::String(it->second);
    }
    
    // Return key if translation not found (for debugging)
    return "[" + key + "]";
}

std::vector<LocalizationManager::Language> LocalizationManager::getAvailableLanguages() const {
    return { Language::English, Language::Japanese };
}

juce::String LocalizationManager::getLanguageName(Language language) const {
    switch (language) {
        case Language::English: return "English";
        case Language::Japanese: return "日本語";
        default: return "Unknown";
    }
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
        // Japanese translations
        translations_ = {
            // Main UI  
            {"app_title", "ヴォイヴォイ解析器"},
            {"settings", "設定"},
            {"preset_description_natural", "日常会話用のバランス取れたメトリクス"},
            {"preset_description_phone", "VoIP・電話通話向けの明瞭性"},
            {"preset_description_resonance", "スペクトラルバランスと共鳴"},
            {"preset_description_diagnostic", "評価用の完全解析"},
            
            // Presets
            {"preset_natural_conversation", "自然な会話"},
            {"preset_phone_training", "電話トレーニング"},
            {"preset_resonance_focus", "共鳴フォーカス"},
            {"preset_diagnostic", "診断"},
            
            // Metrics
            {"metric_f0", "基本周波数"},
            {"metric_cpp", "ケプストラルピーク突出度"},
            {"metric_hnr", "調波雑音比"},
            {"metric_spectral_tilt", "スペクトラル傾斜"},
            {"metric_speech_rate", "発話速度"},
            {"metric_pause_ratio", "ポーズ比率"},
            {"metric_voice_activity", "音声活動"},
            {"metric_rms", "RMS"},
            {"metric_peak", "ピーク"},
            {"metric_crest_factor", "クレスト係数"},
            {"metric_s_centroid", "/s/重心"},
            
            // Units - keep some in English for technical accuracy
            {"unit_hz", "Hz"},
            {"unit_db", "dB"},
            {"unit_dbfs", "dBFS"},
            {"unit_db_octave", "dB/オクターブ"},
            {"unit_syl_per_sec", "音節/秒"},
            {"unit_ratio", "比率"},
            
            // Status
            {"status_active", "アクティブ"},
            {"status_idle", "待機中"},
            {"status_fps", "FPS"},
            {"status_cpu", "CPU"},
            {"status_ram", "RAM"},
            {"status_time_left", "残り時間"},
            
            // Settings dialog
            {"settings_title", "ヴォイヴォイ設定"},
            {"settings_audio", "オーディオ設定"},
            {"settings_sample_rate", "サンプルレート"},
            {"settings_buffer_size", "バッファサイズ"},
            {"settings_performance_mode", "パフォーマンスモード"},
            {"settings_recording", "録音設定"},
            {"settings_max_duration", "最大録音時間"},
            {"settings_auto_save", "停止時に自動保存"},
            {"settings_language", "言語"},
            {"settings_apply", "適用"},
            {"settings_cancel", "キャンセル"},
            {"settings_ok", "OK"},
            
            // Performance modes
            {"mode_light", "軽量 (≤40ms)"},
            {"mode_standard", "標準 (≤60ms)"},
            {"mode_diagnostic", "診断 (≤80ms)"},
            
            // Notifications
            {"fps_limited", "自動的に%d FPSに制限されました"},
            {"recording_started", "録音を開始しました"},
            {"recording_stopped", "録音を停止しました"},
            {"file_saved", "ファイルの保存に成功しました"},
            
            // Heatmaps
            {"heatmap_f0", "F0ヒートマップ"},
            {"heatmap_level", "レベルヒートマップ"},
            {"heatmap_awaiting_data", "データ待機中"},
            
            // Privacy
            {"privacy_local_processing", "100%ローカル処理"},
            {"privacy_no_network", "オーディオのネットワーク送信なし"},
            {"privacy_ram_only", "RAMのみのライブ解析"},
            
            // Additional UI elements
            {"display_settings", "表示"},
            {"privacy_settings", "プライバシー"},
            {"enable_preprocessing", "前処理を有効にする"},
            {"advanced_visualization", "高度な可視化"},
            {"spectral_analysis", "スペクトル解析"},
            {"heatmap_resolution", "ヒートマップ解像度"},
            {"resolution_high", "高"},
            {"resolution_medium", "中"},
            {"resolution_low", "低"},
            {"comparison", "比較"}
        };
    }
}

} // namespace yvc::app
