// VoiVoi GUI Application - Waveform Component
// License: GPLv3

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <vector>

namespace yvc::app {

/// <summary>
/// Simple waveform painter for a short mono buffer (legacy mini waveform).
/// </summary>
class WaveformComponent : public juce::Component {
public:
    WaveformComponent();
    void paint(juce::Graphics& g) override;

    /// <summary>
    /// Sets the waveform samples and their sample rate.
    /// </summary>
    void setSamples(const std::vector<float>& samples, float sampleRate);

    /// <summary>
    /// Sets vertical range of the waveform display.
    /// </summary>
    void setVerticalRange(float minV, float maxV) { minY_ = minV; maxY_ = maxV; }

private:
    std::vector<float> samples_;
    float sampleRate_ = 48000.0f;
    float minY_ = -1.0f;
    float maxY_ = 1.0f;
};

} // namespace yvc::app
