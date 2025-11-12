// VoiVoi GUI Application - Preset Manager Implementation
// License: GPLv3

#include "PresetManager.h"

namespace yvc::app {

PresetManager::PresetManager() {
    presets_.push_back(Preset{ "Natural Conversation",
                               "Balanced metrics for everyday speaking",
                               { MetricDisplayType::F0Gauge, MetricDisplayType::CPP, MetricDisplayType::HNR,
                                 MetricDisplayType::SpectralTilt, MetricDisplayType::SpeechRate,
                                 MetricDisplayType::PauseRatio, MetricDisplayType::VoiceActivity } });

    presets_.push_back(Preset{ "Phone Training",
                               "Clarity for VoIP/phone calls",
                               { MetricDisplayType::F0Gauge, MetricDisplayType::SpectralTilt,
                                 MetricDisplayType::SpeechRate, MetricDisplayType::PauseRatio,
                                 MetricDisplayType::VoiceActivity } });

    presets_.push_back(Preset{ "Resonance Focus",
                               "Spectral balance and resonance",
                               { MetricDisplayType::SpectralTilt, MetricDisplayType::SCentroid,
                                 MetricDisplayType::F0Gauge, MetricDisplayType::CPP } });

    presets_.push_back(Preset{ "Diagnostic",
                               "Complete analysis for assessment",
                               { MetricDisplayType::F0Gauge,  MetricDisplayType::CPP,        MetricDisplayType::HNR,
                                 MetricDisplayType::SpectralTilt, MetricDisplayType::SpeechRate,
                                 MetricDisplayType::PauseRatio,   MetricDisplayType::VoiceActivity,
                                 MetricDisplayType::RMS,          MetricDisplayType::Peak,
                                 MetricDisplayType::CrestFactor,  MetricDisplayType::SCentroid } });
}

const Preset& PresetManager::setActivePreset(size_t index) {
    if (index < presets_.size())
        activePresetIndex_ = index;
    return presets_[activePresetIndex_];
}

} // namespace yvc::app
