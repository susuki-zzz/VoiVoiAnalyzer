// VoiVoi GUI Application - Metrics Visualization Components
// License: GPLv3

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include <functional>
#include <memory>
#include <vector>
#include <deque>
#include <algorithm>
#include <cmath>

#include "yvc_core/Types.h"

namespace yvc::app {

enum class MetricDisplayType {
    F0Gauge,
    CPP,
    HNR,
    SpectralTilt,
    SpeechRate,
    PauseRatio,
    VoiceActivity,
    RMS,
    Peak,
    CrestFactor,
    SCentroid
};

class MetricComponent : public juce::Component {
public:
    MetricComponent() = default;
    ~MetricComponent() override = default;

    virtual void update(const yvc::AnalysisResults& results) = 0;
};

class F0GaugeComponent : public MetricComponent {
public:
    F0GaugeComponent();

    void paint(juce::Graphics& g) override;
    void resized() override;
    void update(const yvc::AnalysisResults& results) override;

private:
    float currentF0_ = 0.0f;
    bool valid_ = false;
    float targetMin_ = 170.0f;
    float targetMax_ = 230.0f;
    // Formants
    float f1_ = 0.0f;
    float f2_ = 0.0f;
    float f3_ = 0.0f;
    float f4_ = 0.0f;
    bool formantsValid_ = false;
};

class ScalarMeterComponent : public MetricComponent {
public:
    struct Options {
        juce::String label;
        juce::String unit;
        float minimum = -40.0f;
        float maximum = 40.0f;
        float defaultValue = 0.0f;
        bool showBaseline = true;
    };

    ScalarMeterComponent(Options opts, std::function<float(const yvc::AnalysisResults&)> getter,
                         std::function<bool(const yvc::AnalysisResults&)> validityGetter = {});

    void paint(juce::Graphics& g) override;
    void update(const yvc::AnalysisResults& results) override;

private:
    Options options_;
    std::function<float(const yvc::AnalysisResults&)> getter_;
    std::function<bool(const yvc::AnalysisResults&)> validityGetter_;
    float currentValue_ = 0.0f;
    bool isValid_ = true;
};

class VadMeterComponent : public MetricComponent {
public:
    VadMeterComponent();

    void paint(juce::Graphics& g) override;
    void update(const yvc::AnalysisResults& results) override;

private:
    bool voiceActive_ = false;
    float speechRate_ = 0.0f;
    float pauseRatio_ = 0.0f;
};

class MetricsDisplayComponent : public juce::Component {
public:
    MetricsDisplayComponent();

    void setDisplayedMetrics(const std::vector<MetricDisplayType>& types);
    void updateMetrics(const yvc::AnalysisResults& results);
    void resized() override;

private:
    std::vector<MetricDisplayType> activeTypes_;
    std::vector<std::unique_ptr<MetricComponent>> components_;

    std::unique_ptr<MetricComponent> createComponentFor(MetricDisplayType type);
};

class HeatmapComponent : public juce::Component {
public:
    enum class ScaleMode { LinearHz, LogHz, MidiNote };

    HeatmapComponent();

    void appendSample(const yvc::AnalysisResults& results);
    void paint(juce::Graphics& g) override;
    void resized() override;

    void setMaxSamples(int samples) { maxSamples_ = juce::jmax(4, samples); }
    void setScaleMode(ScaleMode mode) { scaleMode_ = mode; repaint(); }

private:
    void drawHeatmap(juce::Graphics& g, juce::Rectangle<float> area, const std::deque<float>& samples,
                     float minValue, float maxValue, const juce::String& label, const juce::String& unit);
    float mapFrequencyToY(float freq, float minFreq, float maxFreq, float height) const;
    juce::String formatAxisLabel(float freq) const;

    std::deque<float> f0History_;
    std::deque<float> f0ConfHistory_;
    std::deque<float> rmsHistory_;
    std::deque<float> f1History_;
    std::deque<float> f2History_;
    std::deque<float> f3History_;
    std::deque<float> f4History_;
    int maxSamples_ = 180;
    ScaleMode scaleMode_ = ScaleMode::LinearHz;
};

} // namespace yvc::app
