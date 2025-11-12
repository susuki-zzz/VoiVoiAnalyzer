// VoiVoi GUI Application - Metrics Visualization Components Implementation
// License: GPLv3

#include "MetricsComponents.h"

#include <cmath>

namespace yvc::app {

F0GaugeComponent::F0GaugeComponent() {
    setName("F0 Gauge");
}

void F0GaugeComponent::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat().reduced(6.0f);
    g.setColour(juce::Colours::darkgrey.darker(0.6f));
    g.fillRoundedRectangle(bounds, 12.0f);

    g.setColour(juce::Colours::white.withAlpha(0.15f));
    g.drawRoundedRectangle(bounds, 12.0f, 2.0f);

    auto centre = bounds.getCentre();
    auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.42f;
    auto arcBounds = juce::Rectangle<float>(centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f);

    const float startAngle = juce::degreesToRadians(135.0f);
    const float endAngle = juce::degreesToRadians(405.0f);

    g.setColour(juce::Colours::darkslateblue.brighter(0.3f));
    g.drawArc(arcBounds, startAngle, endAngle, 8.0f);

    if (valid_) {
        auto norm = juce::jlimit(0.0f, 1.0f, (currentF0_ - 60.0f) / 380.0f);
        auto angle = startAngle + norm * (endAngle - startAngle);
        juce::Path needle;
        needle.addTriangle(centre.x, centre.y,
                           centre.x + std::cos(angle) * radius * 0.85f,
                           centre.y + std::sin(angle) * radius * 0.85f,
                           centre.x + std::cos(angle) * radius * 0.7f,
                           centre.y + std::sin(angle) * radius * 0.7f);
        g.setColour(juce::Colours::lightskyblue);
        g.fillPath(needle);
    }

    auto targetMinAngle = startAngle + juce::jlimit(0.0f, 1.0f, (targetMin_ - 60.0f) / 380.0f) * (endAngle - startAngle);
    auto targetMaxAngle = startAngle + juce::jlimit(0.0f, 1.0f, (targetMax_ - 60.0f) / 380.0f) * (endAngle - startAngle);
    g.setColour(juce::Colours::green.withAlpha(0.4f));
    g.drawArc(arcBounds.reduced(6.0f), targetMinAngle, targetMaxAngle - targetMinAngle, 12.0f);

    g.setColour(valid_ ? juce::Colours::white : juce::Colours::lightgrey);
    g.setFont(juce::Font(20.0f, juce::Font::bold));
    auto f0Text = valid_ ? juce::String(currentF0_, 1) + " Hz" : "--";
    g.drawText(f0Text, bounds.withHeight(32.0f).withY(bounds.getCentreY() - 16.0f), juce::Justification::centred);

    g.setFont(juce::Font(14.0f));
    g.setColour(juce::Colours::lightgrey);
    g.drawText("F0", bounds.removeFromBottom(28.0f), juce::Justification::centredBottom);
}

void F0GaugeComponent::resized() {}

void F0GaugeComponent::update(const yvc::AnalysisResults& results) {
    currentF0_ = results.f0;
    valid_ = results.f0_valid;
    repaint();
}

ScalarMeterComponent::ScalarMeterComponent(Options opts,
                                           std::function<float(const yvc::AnalysisResults&)> getter,
                                           std::function<bool(const yvc::AnalysisResults&)> validityGetter)
    : options_(std::move(opts)), getter_(std::move(getter)), validityGetter_(std::move(validityGetter)) {
    setName(options_.label);
}

void ScalarMeterComponent::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat().reduced(6.0f);
    g.setColour(juce::Colours::darkgrey.darker(0.5f));
    g.fillRoundedRectangle(bounds, 8.0f);

    g.setColour(juce::Colours::white.withAlpha(0.1f));
    g.drawRoundedRectangle(bounds, 8.0f, 1.5f);

    if (!isValid_) {
        g.setColour(juce::Colours::darkred.withAlpha(0.4f));
        g.fillRoundedRectangle(bounds.reduced(4.0f), 6.0f);
    }

    const float range = options_.maximum - options_.minimum;
    float normalised = 0.5f;
    if (range > 0.0f)
        normalised = juce::jlimit(0.0f, 1.0f, (currentValue_ - options_.minimum) / range);

    auto meterBounds = bounds.reduced(12.0f, 22.0f);
    auto filled = meterBounds.withWidth(meterBounds.getWidth() * normalised);

    g.setColour(juce::Colours::cornflowerblue.withAlpha(0.8f));
    g.fillRoundedRectangle(filled, 4.0f);

    if (options_.showBaseline && range > 0.0f) {
        const float zeroPos = juce::jlimit(0.0f, 1.0f, (-options_.minimum) / range);
        g.setColour(juce::Colours::white.withAlpha(0.2f));
        g.drawVerticalLine(static_cast<int>(meterBounds.getX() + meterBounds.getWidth() * zeroPos),
                           meterBounds.getY(), meterBounds.getBottom());
    }

    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(16.0f, juce::Font::bold));
    g.drawText(options_.label, bounds.removeFromTop(24.0f), juce::Justification::centredLeft);

    g.setFont(juce::Font(14.0f));
    juce::String valueText = isValid_ ? juce::String(currentValue_, 1) + " " + options_.unit : "--";
    g.drawText(valueText, bounds.removeFromBottom(24.0f), juce::Justification::centredRight);
}

void ScalarMeterComponent::update(const yvc::AnalysisResults& results) {
    currentValue_ = getter_(results);
    if (validityGetter_)
        isValid_ = validityGetter_(results);
    else
        isValid_ = true;
    repaint();
}

VadMeterComponent::VadMeterComponent() {
    setName("Voice Activity");
}

void VadMeterComponent::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat().reduced(6.0f);
    g.setColour(juce::Colours::darkgrey.darker(0.6f));
    g.fillRoundedRectangle(bounds, 8.0f);

    g.setColour(juce::Colours::white.withAlpha(0.1f));
    g.drawRoundedRectangle(bounds, 8.0f, 1.5f);

    auto indicator = bounds.removeFromTop(bounds.getHeight() * 0.5f).reduced(12.0f);
    g.setColour(voiceActive_ ? juce::Colours::mediumseagreen : juce::Colours::slategrey);
    g.fillRoundedRectangle(indicator, 6.0f);
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(16.0f, juce::Font::bold));
    g.drawText(voiceActive_ ? "Active" : "Idle", indicator, juce::Justification::centred);

    g.setFont(juce::Font(14.0f));
    g.setColour(juce::Colours::lightgrey);
    auto lower = bounds.reduced(12.0f);
    g.drawText("Speech Rate: " + juce::String(speechRate_, 2) + " syl/s", lower.removeFromTop(lower.getHeight() / 2),
               juce::Justification::centredLeft);
    g.drawText("Pause Ratio: " + juce::String(pauseRatio_, 2), lower, juce::Justification::centredLeft);
}

void VadMeterComponent::update(const yvc::AnalysisResults& results) {
    voiceActive_ = results.voice_active;
    speechRate_ = results.speech_rate;
    pauseRatio_ = results.pause_ratio;
    repaint();
}

MetricsDisplayComponent::MetricsDisplayComponent() {
    setOpaque(false);
}

void MetricsDisplayComponent::setDisplayedMetrics(const std::vector<MetricDisplayType>& types) {
    activeTypes_ = types;
    components_.clear();
    for (auto type : activeTypes_) {
        auto component = createComponentFor(type);
        if (component != nullptr) {
            addAndMakeVisible(component.get());
            components_.push_back(std::move(component));
        }
    }
    resized();
    repaint();
}

void MetricsDisplayComponent::updateMetrics(const yvc::AnalysisResults& results) {
    for (auto& component : components_)
        component->update(results);
}

void MetricsDisplayComponent::resized() {
    if (components_.empty())
        return;

    auto bounds = getLocalBounds().reduced(12);
    const int columns = juce::jmax(1, static_cast<int>(std::ceil(std::sqrt(static_cast<double>(components_.size())))));
    const int rows = static_cast<int>(std::ceil(components_.size() / static_cast<double>(columns)));

    int cellWidth = bounds.getWidth() / columns;
    int cellHeight = bounds.getHeight() / rows;

    int index = 0;
    for (auto& component : components_) {
        int row = index / columns;
        int column = index % columns;
        component->setBounds(bounds.getX() + column * cellWidth,
                             bounds.getY() + row * cellHeight,
                             cellWidth,
                             cellHeight);
        ++index;
    }
}

std::unique_ptr<MetricComponent> MetricsDisplayComponent::createComponentFor(MetricDisplayType type) {
    using Options = ScalarMeterComponent::Options;

    switch (type) {
    case MetricDisplayType::F0Gauge:
        return std::make_unique<F0GaugeComponent>();
    case MetricDisplayType::CPP:
        return std::make_unique<ScalarMeterComponent>(
            Options{ "CPP", "dB", -10.0f, 20.0f, 0.0f, false },
            [](const yvc::AnalysisResults& r) { return r.cpp; });
    case MetricDisplayType::HNR:
        return std::make_unique<ScalarMeterComponent>(
            Options{ "HNR", "dB", -20.0f, 40.0f, 0.0f, false },
            [](const yvc::AnalysisResults& r) { return r.hnr; });
    case MetricDisplayType::SpectralTilt:
        return std::make_unique<ScalarMeterComponent>(
            Options{ "Spectral Tilt", "dB/oct", -12.0f, 6.0f, -6.0f, true },
            [](const yvc::AnalysisResults& r) { return r.spectral_tilt; });
    case MetricDisplayType::SpeechRate:
        return std::make_unique<ScalarMeterComponent>(
            Options{ "Speech Rate", "syl/s", 0.0f, 8.0f, 3.0f, false },
            [](const yvc::AnalysisResults& r) { return r.speech_rate; });
    case MetricDisplayType::PauseRatio:
        return std::make_unique<ScalarMeterComponent>(
            Options{ "Pause Ratio", "", 0.0f, 1.0f, 0.2f, false },
            [](const yvc::AnalysisResults& r) { return r.pause_ratio; });
    case MetricDisplayType::VoiceActivity:
        return std::make_unique<VadMeterComponent>();
    case MetricDisplayType::RMS:
        return std::make_unique<ScalarMeterComponent>(
            Options{ "RMS", "dBFS", -90.0f, 6.0f, -18.0f, false },
            [](const yvc::AnalysisResults& r) { return r.rms; });
    case MetricDisplayType::Peak:
        return std::make_unique<ScalarMeterComponent>(
            Options{ "Peak", "dBFS", -90.0f, 6.0f, -12.0f, false },
            [](const yvc::AnalysisResults& r) { return r.peak; });
    case MetricDisplayType::CrestFactor:
        return std::make_unique<ScalarMeterComponent>(
            Options{ "Crest", "ratio", 0.0f, 20.0f, 6.0f, false },
            [](const yvc::AnalysisResults& r) { return r.crest_factor; });
    case MetricDisplayType::SCentroid:
        return std::make_unique<ScalarMeterComponent>(
            Options{ "/s/ Centroid", "Hz", 1000.0f, 10000.0f, 4000.0f, false },
            [](const yvc::AnalysisResults& r) { return r.s_centroid; },
            [](const yvc::AnalysisResults& r) { return r.s_detected; });
    default:
        break;
    }
    return nullptr;
}

} // namespace yvc::app
