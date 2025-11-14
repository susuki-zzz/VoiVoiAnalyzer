// VoiVoi GUI Application - Waveform Component
// License: GPLv3

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <vector>

namespace yvc::app {

class WaveformComponent : public juce::Component {
public:
    WaveformComponent();
    void paint(juce::Graphics& g) override;

    void setSamples(const std::vector<float>& samples, float sampleRate);
    void setVerticalRange(float minV, float maxV) { minY_ = minV; maxY_ = maxV; }

private:
    std::vector<float> samples_;
    float sampleRate_ = 48000.0f;
    float minY_ = -1.0f;
    float maxY_ = 1.0f;
};

} // namespace yvc::app
