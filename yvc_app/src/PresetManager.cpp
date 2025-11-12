// VoiVoi GUI Application - Preset Manager Implementation
// License: GPLv3

#include "PresetManager.h"

namespace yvc::app {

PresetManager::PresetManager() {
    presets_.push_back(Preset{ 
        "preset_natural_conversation",  // nameKey for localization
        "preset_description_natural",   // descriptionKey for localization
        { MetricDisplayType::F0Gauge, MetricDisplayType::CPP, MetricDisplayType::HNR,
          MetricDisplayType::SpectralTilt, MetricDisplayType::SpeechRate,
          MetricDisplayType::PauseRatio, MetricDisplayType::VoiceActivity } 
    });

    presets_.push_back(Preset{ 
        "preset_phone_training",
        "preset_description_phone",
        { MetricDisplayType::F0Gauge, MetricDisplayType::SpectralTilt,
          MetricDisplayType::SpeechRate, MetricDisplayType::PauseRatio,
          MetricDisplayType::VoiceActivity } 
    });

    presets_.push_back(Preset{ 
        "preset_resonance_focus",
        "preset_description_resonance",
        { MetricDisplayType::SpectralTilt, MetricDisplayType::SCentroid,
          MetricDisplayType::F0Gauge, MetricDisplayType::CPP } 
    });

    presets_.push_back(Preset{ 
        "preset_diagnostic",
        "preset_description_diagnostic",
        { MetricDisplayType::F0Gauge,  MetricDisplayType::CPP,        MetricDisplayType::HNR,
          MetricDisplayType::SpectralTilt, MetricDisplayType::SpeechRate,
          MetricDisplayType::PauseRatio,   MetricDisplayType::VoiceActivity,
          MetricDisplayType::RMS,          MetricDisplayType::Peak,
          MetricDisplayType::CrestFactor,  MetricDisplayType::SCentroid } 
    });
}

const Preset& PresetManager::setActivePreset(size_t index) {
    if (index < presets_.size())
        activePresetIndex_ = index;
    return presets_[activePresetIndex_];
}

} // namespace yvc::app
