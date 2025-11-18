// VoiVoi GUI Application - Preset Manager
// License: GPLv3

#pragma once

#include <juce_core/juce_core.h>

#include "MetricsComponents.h"

namespace yvc::app {

    /// <summary>
    /// UI preset describing which metrics to display and their order.
    /// </summary>
    struct Preset {
        juce::String nameKey;        // Localization key for preset name
        juce::String descriptionKey; // Localization key for description
        std::vector<MetricDisplayType> metrics;
    };

    /// <summary>
    /// Manages available presets and the currently active preset.
    /// </summary>
    class PresetManager {
    public:
        PresetManager();

        /// <summary>
        /// Returns all available presets.
        /// </summary>
        const std::vector<Preset>& getPresets() const noexcept { return presets_; }

        /// <summary>
        /// Returns the currently active preset.
        /// </summary>
        const Preset& getActivePreset() const noexcept { return presets_[activePresetIndex_]; }

        /// <summary>
        /// Returns index of the currently active preset.
        /// </summary>
        int getActivePresetIndex() const noexcept { return static_cast<int>(activePresetIndex_); }

        /// <summary>
        /// Sets active preset by index and returns it.
        /// </summary>
        const Preset& setActivePreset(size_t index);

    private:
        std::vector<Preset> presets_;
        size_t activePresetIndex_ = 0;
    };

} // namespace yvc::app
