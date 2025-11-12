// VoiVoi GUI Application - Preset Manager
// License: GPLv3

#pragma once

#include <juce_core/juce_core.h>

#include "MetricsComponents.h"

namespace yvc::app {

struct Preset {
    juce::String nameKey;        // Localization key for preset name
    juce::String descriptionKey; // Localization key for description
    std::vector<MetricDisplayType> metrics;
};

class PresetManager {
public:
    PresetManager();

    const std::vector<Preset>& getPresets() const noexcept { return presets_; }
    const Preset& getActivePreset() const noexcept { return presets_[activePresetIndex_]; }
    int getActivePresetIndex() const noexcept { return static_cast<int>(activePresetIndex_); }

    const Preset& setActivePreset(size_t index);

private:
    std::vector<Preset> presets_;
    size_t activePresetIndex_ = 0;
};

} // namespace yvc::app
