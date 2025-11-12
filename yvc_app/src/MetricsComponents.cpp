// VoiVoi GUI Application - Metrics Visualization Components Implementation
// License: GPLv3

#include "MetricsComponents.h"

#ifdef JUCE_GRAPHICS_H_INCLUDED
    #include <juce_graphics/juce_graphics.h>
    #define USE_JUCE 1
#else
    #define USE_JUCE 0
    // Stub implementations for non-JUCE builds
#endif

namespace yvc::app {

#if USE_JUCE

F0GaugeComponent::F0GaugeComponent() {
    setSize(120, 120);
}

void F0GaugeComponent::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();
    auto center = bounds.getCentre();
    auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.35f;
    auto arcBounds = juce::Rectangle<float>(center.x - radius, center.y - radius, radius * 2.0f, radius * 2.0f);

    // Background arc
    g.setColour(juce::Colours::darkgrey);
    auto startAngle = 2.25f * juce::MathConstants<float>::pi;
    auto endAngle = 0.75f * juce::MathConstants<float>::pi;
    
    // Use fillPath for arc drawing compatibility
    juce::Path arcPath;
    arcPath.addCentredArc(center.x, center.y, radius, radius, 0.0f, startAngle, endAngle);
    g.strokePath(arcPath, juce::PathStrokeType(8.0f));

    if (valid_) {
        // Value indicator
        float normalizedValue = juce::jlimit(0.0f, 1.0f, (currentF0_ - 100.0f) / 300.0f);
        float valueAngle = startAngle + normalizedValue * (endAngle - startAngle);
        
        g.setColour(juce::Colours::lightgreen);
        juce::Path valuePath;
        valuePath.addCentredArc(center.x, center.y, radius, radius, 0.0f, startAngle, valueAngle);
        g.strokePath(valuePath, juce::PathStrokeType(8.0f));

        // Target range overlay
        float targetMinNorm = juce::jlimit(0.0f, 1.0f, (targetMin_ - 100.0f) / 300.0f);
        float targetMaxNorm = juce::jlimit(0.0f, 1.0f, (targetMax_ - 100.0f) / 300.0f);
        float targetMinAngle = startAngle + targetMinNorm * (endAngle - startAngle);
        float targetMaxAngle = startAngle + targetMaxNorm * (endAngle - startAngle);

        g.setColour(juce::Colours::yellow.withAlpha(0.3f));
        juce::Path targetPath;
        targetPath.addCentredArc(center.x, center.y, radius - 6.0f, radius - 6.0f, 0.0f, 
                                targetMinAngle, targetMaxAngle);
        g.strokePath(targetPath, juce::PathStrokeType(12.0f));
        
        // Center text
        g.setColour(juce::Colours::white);
        g.setFont(16.0f);
        auto textArea = bounds.withSizeKeepingCentre(60.0f, 20.0f);
        g.drawText(juce::String(currentF0_, 1) + " Hz", textArea, juce::Justification::centred);
    } else {
        g.setColour(juce::Colours::red.withAlpha(0.5f));
        auto textArea = bounds.withSizeKeepingCentre(80.0f, 20.0f);
        g.drawText("No F0", textArea, juce::Justification::centred);
    }
}

void F0GaugeComponent::resized() {
    // Nothing to layout
}

void F0GaugeComponent::update(const yvc::AnalysisResults& results) {
    currentF0_ = results.f0;
    valid_ = results.f0_valid;
    repaint();
}

ScalarMeterComponent::ScalarMeterComponent(Options opts, 
                                           std::function<float(const yvc::AnalysisResults&)> getter,
                                           std::function<bool(const yvc::AnalysisResults&)> validityGetter)
    : options_(std::move(opts)), getter_(std::move(getter)), validityGetter_(std::move(validityGetter)) {
    setSize(80, 120);
}

void ScalarMeterComponent::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();
    
    // Background
    g.setColour(juce::Colours::darkgrey.withAlpha(0.3f));
    g.fillRect(bounds);
    
    // Label
    g.setColour(juce::Colours::white);
    g.setFont(12.0f);
    auto labelArea = bounds.removeFromTop(20.0f);
    g.drawText(options_.label, labelArea, juce::Justification::centred);
    
    // Meter area
    auto meterArea = bounds.reduced(8.0f, 4.0f);
    
    g.setColour(juce::Colours::black);
    g.fillRect(meterArea);
    
    if (isValid_) {
        // Value bar
        float normalizedValue = (currentValue_ - options_.minimum) / (options_.maximum - options_.minimum);
        normalizedValue = juce::jlimit(0.0f, 1.0f, normalizedValue);
        
        auto valueHeight = meterArea.getHeight() * normalizedValue;
        auto valueRect = meterArea.removeFromBottom(valueHeight);
        
        // Color based on value
        juce::Colour valueColour;
        if (normalizedValue < 0.3f) {
            valueColour = juce::Colours::blue;
        } else if (normalizedValue < 0.7f) {
            valueColour = juce::Colours::green;
        } else {
            valueColour = juce::Colours::orange;
        }
        
        g.setColour(valueColour);
        g.fillRect(valueRect);
        
        // Baseline indicator
        if (options_.showBaseline) {
            float baselineNorm = (options_.defaultValue - options_.minimum) / (options_.maximum - options_.minimum);
            if (baselineNorm >= 0.0f && baselineNorm <= 1.0f) {
                float baselineY = meterArea.getBottom() - baselineNorm * meterArea.getHeight();
                g.setColour(juce::Colours::yellow);
                g.fillRect(meterArea.getX(), baselineY - 1.0f, meterArea.getWidth(), 2.0f);
            }
        }
        
        // Value text
        g.setColour(juce::Colours::white);
        g.setFont(10.0f);
        auto textArea = meterArea.removeFromBottom(15.0f);
        juce::String valueStr = juce::String(currentValue_, 1) + " " + options_.unit;
        g.drawText(valueStr, textArea, juce::Justification::centred);
    } else {
        g.setColour(juce::Colours::red.withAlpha(0.5f));
        g.drawText("N/A", meterArea, juce::Justification::centred);
    }
}

void ScalarMeterComponent::update(const yvc::AnalysisResults& results) {
    currentValue_ = getter_(results);
    isValid_ = validityGetter_ ? validityGetter_(results) : true;
    repaint();
}

VadMeterComponent::VadMeterComponent() {
    setSize(100, 80);
}

void VadMeterComponent::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();
    
    // Voice activity indicator
    auto vadArea = bounds.removeFromTop(30.0f);
    g.setColour(voiceActive_ ? juce::Colours::green : juce::Colours::darkgrey);
    g.fillEllipse(vadArea.reduced(10.0f));
    
    // Speech rate and pause info
    g.setColour(juce::Colours::white);
    g.setFont(12.0f);
    
    auto rateArea = bounds.removeFromTop(20.0f);
    g.drawText(juce::String(speechRate_, 1) + " syl/s", rateArea, juce::Justification::centred);
    
    auto pauseArea = bounds.removeFromTop(20.0f);
    g.drawText("Pause: " + juce::String((int)(pauseRatio_ * 100)) + "%", pauseArea, juce::Justification::centred);
}

void VadMeterComponent::update(const yvc::AnalysisResults& results) {
    voiceActive_ = results.voice_active;
    speechRate_ = results.speech_rate;
    pauseRatio_ = results.pause_ratio;
    repaint();
}

MetricsDisplayComponent::MetricsDisplayComponent() {
    setSize(400, 300);
}

void MetricsDisplayComponent::setDisplayedMetrics(const std::vector<MetricDisplayType>& types) {
    activeTypes_ = types;
    components_.clear();
    
    for (const auto& type : activeTypes_) {
        components_.push_back(createComponentFor(type));
        if (components_.back()) {
            addAndMakeVisible(components_.back().get());
        }
    }
    
    resized();
}

void MetricsDisplayComponent::updateMetrics(const yvc::AnalysisResults& results) {
    for (auto& component : components_) {
        if (component) {
            component->update(results);
        }
    }
}

void MetricsDisplayComponent::resized() {
    auto area = getLocalBounds();
    int itemsPerRow = 4;
    int rows = (static_cast<int>(components_.size()) + itemsPerRow - 1) / itemsPerRow;
    
    if (rows > 0) {
        int itemHeight = area.getHeight() / rows;
        int itemWidth = area.getWidth() / itemsPerRow;
        
        for (size_t i = 0; i < components_.size(); ++i) {
            if (components_[i]) {
                int row = static_cast<int>(i) / itemsPerRow;
                int col = static_cast<int>(i) % itemsPerRow;
                
                auto itemBounds = juce::Rectangle<int>(
                    col * itemWidth, 
                    row * itemHeight,
                    itemWidth, 
                    itemHeight
                ).reduced(4);
                
                components_[i]->setBounds(itemBounds);
            }
        }
    }
}

std::unique_ptr<MetricComponent> MetricsDisplayComponent::createComponentFor(MetricDisplayType type) {
    switch (type) {
        case MetricDisplayType::F0Gauge:
            return std::make_unique<F0GaugeComponent>();
            
        case MetricDisplayType::CPP: {
            ScalarMeterComponent::Options opts;
            opts.label = "CPP";
            opts.unit = "dB";
            opts.minimum = 0.0f;
            opts.maximum = 30.0f;
            opts.defaultValue = 10.0f;
            return std::make_unique<ScalarMeterComponent>(
                opts, 
                [](const yvc::AnalysisResults& r) { return r.cpp; }
                // No validity check for now - assume always valid
            );
        }
        
        case MetricDisplayType::HNR: {
            ScalarMeterComponent::Options opts;
            opts.label = "HNR";
            opts.unit = "dB";
            opts.minimum = 0.0f;
            opts.maximum = 30.0f;
            opts.defaultValue = 15.0f;
            return std::make_unique<ScalarMeterComponent>(
                opts,
                [](const yvc::AnalysisResults& r) { return r.hnr; }
                // No validity check for now - assume always valid
            );
        }
        
        case MetricDisplayType::VoiceActivity:
            return std::make_unique<VadMeterComponent>();
            
        default:
            return nullptr;
    }
}

HeatmapComponent::HeatmapComponent() {
    setSize(200, 300);
}

void HeatmapComponent::appendSample(const yvc::AnalysisResults& results) {
    if (results.f0_valid) {
        f0History_.push_back(results.f0);
    } else {
        f0History_.push_back(0.0f);
    }
    
    rmsHistory_.push_back(results.rms);
    
    while (static_cast<int>(f0History_.size()) > maxSamples_) {
        f0History_.pop_front();
    }
    
    while (static_cast<int>(rmsHistory_.size()) > maxSamples_) {
        rmsHistory_.pop_front();
    }
    
    repaint();
}

void HeatmapComponent::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();
    
    // Split into two heatmaps
    auto f0Area = bounds.removeFromTop(bounds.getHeight() * 0.5f);
    auto rmsArea = bounds;
    
    drawHeatmap(g, f0Area, f0History_, 80.0f, 400.0f, "F0", "Hz");
    drawHeatmap(g, rmsArea, rmsHistory_, -60.0f, 0.0f, "RMS", "dBFS");
}

void HeatmapComponent::resized() {
    // Update max samples based on width
    setMaxSamples(getWidth() / 2);
}

void HeatmapComponent::drawHeatmap(juce::Graphics& g, juce::Rectangle<float> area, 
                                   const std::deque<float>& samples, float minValue, float maxValue, 
                                   const juce::String& label, const juce::String& unit) {
    // Background
    g.setColour(juce::Colours::black);
    g.fillRect(area);
    
    // Border
    g.setColour(juce::Colours::darkgrey);
    g.drawRect(area, 1.0f);
    
    // Label
    g.setColour(juce::Colours::white);
    g.setFont(12.0f);
    auto labelArea = area.removeFromTop(20.0f);
    g.drawText(label + " (" + unit + ")", labelArea.reduced(4.0f), juce::Justification::centredLeft);
    
    // Draw samples
    if (!samples.empty() && area.getWidth() > 0 && area.getHeight() > 0) {
        float sampleWidth = area.getWidth() / static_cast<float>(samples.size());
        
        for (size_t i = 0; i < samples.size(); ++i) {
            float normalizedValue = (samples[i] - minValue) / (maxValue - minValue);
            normalizedValue = juce::jlimit(0.0f, 1.0f, normalizedValue);
            
            // Color mapping
            juce::Colour sampleColour;
            if (normalizedValue < 0.33f) {
                sampleColour = juce::Colours::blue.interpolatedWith(juce::Colours::green, normalizedValue * 3.0f);
            } else if (normalizedValue < 0.66f) {
                sampleColour = juce::Colours::green.interpolatedWith(juce::Colours::yellow, (normalizedValue - 0.33f) * 3.0f);
            } else {
                sampleColour = juce::Colours::yellow.interpolatedWith(juce::Colours::red, (normalizedValue - 0.66f) * 3.0f);
            }
            
            g.setColour(sampleColour);
            
            auto sampleRect = juce::Rectangle<float>(
                area.getX() + i * sampleWidth, 
                area.getY(), 
                sampleWidth, 
                area.getHeight()
            );
            
            g.fillRect(sampleRect);
        }
    }
}

#else

// Stub implementations for non-JUCE builds
F0GaugeComponent::F0GaugeComponent() {}
void F0GaugeComponent::paint(juce::Graphics&) {}
void F0GaugeComponent::resized() {}
void F0GaugeComponent::update(const yvc::AnalysisResults&) {}

ScalarMeterComponent::ScalarMeterComponent(Options, std::function<float(const yvc::AnalysisResults&)>, std::function<bool(const yvc::AnalysisResults&)>) {}
void ScalarMeterComponent::paint(juce::Graphics&) {}
void ScalarMeterComponent::update(const yvc::AnalysisResults&) {}

VadMeterComponent::VadMeterComponent() {}
void VadMeterComponent::paint(juce::Graphics&) {}
void VadMeterComponent::update(const yvc::AnalysisResults&) {}

MetricsDisplayComponent::MetricsDisplayComponent() {}
void MetricsDisplayComponent::setDisplayedMetrics(const std::vector<MetricDisplayType>&) {}
void MetricsDisplayComponent::updateMetrics(const yvc::AnalysisResults&) {}
void MetricsDisplayComponent::resized() {}
std::unique_ptr<MetricComponent> MetricsDisplayComponent::createComponentFor(MetricDisplayType) { return nullptr; }

HeatmapComponent::HeatmapComponent() {}
void HeatmapComponent::appendSample(const yvc::AnalysisResults&) {}
void HeatmapComponent::paint(juce::Graphics&) {}
void HeatmapComponent::resized() {}
void HeatmapComponent::drawHeatmap(juce::Graphics&, juce::Rectangle<float>, const std::deque<float>&, float, float, const juce::String&, const juce::String&) {}

#endif

} // namespace yvc::app
