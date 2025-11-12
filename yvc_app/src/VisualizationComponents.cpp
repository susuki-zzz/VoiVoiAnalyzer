// VoiVoi GUI Application - Enhanced Visualization Components Implementation
// License: GPLv3

#include "VisualizationComponents.h"
#include "LocalizationManager.h"
#include <cmath>
#include <algorithm>

namespace yvc::app {

// AdvancedHeatmapComponent Implementation

AdvancedHeatmapComponent::AdvancedHeatmapComponent(const juce::String& title)
    : title_(title) {
    setConfig(HeatmapConfig{});
}

void AdvancedHeatmapComponent::setConfig(const HeatmapConfig& config) {
    config_ = config;
    repaint();
}

void AdvancedHeatmapComponent::appendSample(float value, double timestamp) {
    Sample sample;
    sample.value = value;
    sample.timestamp = timestamp;
    sample.valid = std::isfinite(value);
    
    samples_.push_back(sample);
    
    // Limit sample history
    while (static_cast<int>(samples_.size()) > config_.maxSamples) {
        samples_.pop_front();
    }
    
    // Auto-adjust zoom to follow live data
    if (!samples_.empty()) {
        double latestTime = samples_.back().timestamp;
        double timeSpan = zoomEnd_ - zoomStart_;
        
        if (latestTime > zoomEnd_) {
            zoomStart_ = latestTime - timeSpan;
            zoomEnd_ = latestTime;
            
            if (onZoomRangeChanged) {
                onZoomRangeChanged(zoomStart_, zoomEnd_);
            }
        }
    }
    
    repaint();
}

void AdvancedHeatmapComponent::clear() {
    samples_.clear();
    repaint();
}

void AdvancedHeatmapComponent::setZoomRange(double startTime, double endTime) {
    zoomStart_ = startTime;
    zoomEnd_ = endTime;
    repaint();
}

void AdvancedHeatmapComponent::setPlayheadPosition(double timestamp) {
    playheadPosition_ = timestamp;
    repaint();
}

void AdvancedHeatmapComponent::setResolutionMode(bool highResolution) {
    highResolution_ = highResolution;
    repaint();
}

juce::Image AdvancedHeatmapComponent::createExportImage(int width, int height, bool includeAnnotations) {
    juce::Image image(juce::Image::RGB, width, height, true);
    juce::Graphics g(image);
    
    // Set up export-specific layout
    auto bounds = juce::Rectangle<int>(0, 0, width, height);
    auto titleHeight = includeAnnotations ? 40 : 0;
    auto axisWidth = includeAnnotations ? 80 : 0;
    auto axisHeight = includeAnnotations ? 30 : 0;
    
    auto heatmapArea = bounds.reduced(axisWidth, titleHeight + axisHeight);
    
    // Draw background
    g.fillAll(juce::Colours::white);
    
    if (includeAnnotations) {
        // Draw title
        g.setColour(juce::Colours::black);
        g.setFont(juce::Font(24.0f, juce::Font::bold));
        g.drawText(title_, bounds.removeFromTop(titleHeight), juce::Justification::centred);
        
        // Draw axes
        drawTimeAxis(g, juce::Rectangle<int>(axisWidth, height - axisHeight, width - axisWidth, axisHeight));
        drawValueAxis(g, juce::Rectangle<int>(0, titleHeight, axisWidth, height - titleHeight - axisHeight));
    }
    
    // Draw heatmap
    drawHeatmapData(g, heatmapArea);
    
    return image;
}

bool AdvancedHeatmapComponent::exportToPNG(const juce::File& file, int width, int height) {
    auto image = createExportImage(width, height, true);
    
    juce::FileOutputStream stream(file);
    if (stream.openedOk()) {
        juce::PNGImageFormat pngFormat;
        return pngFormat.writeImageToStream(image, stream);
    }
    
    return false;
}

void AdvancedHeatmapComponent::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds();
    
    // Background
    g.fillAll(juce::Colours::black.withAlpha(0.4f));
    
    // Layout areas
    auto titleArea = bounds.removeFromTop(24);
    auto timeAxisArea = bounds.removeFromBottom(20);
    auto valueAxisArea = bounds.removeFromLeft(60);
    heatmapArea_ = bounds.reduced(2);
    timeAxis_ = timeAxisArea;
    valueAxis_ = valueAxisArea;
    
    // Draw title
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(16.0f, juce::Font::bold));
    g.drawText(title_, titleArea, juce::Justification::centred);
    
    if (samples_.empty()) {
        g.setColour(juce::Colours::lightgrey);
        g.setFont(juce::Font(14.0f));
        g.drawText(TRANS("heatmap_awaiting_data"), heatmapArea_, juce::Justification::centred);
        return;
    }
    
    // Draw main components
    drawHeatmapData(g, heatmapArea_);
    drawTimeAxis(g, timeAxis_);
    drawValueAxis(g, valueAxis_);
    drawPlayhead(g, heatmapArea_);
    
    if (isZooming_) {
        drawZoomOverlay(g);
    }
}

void AdvancedHeatmapComponent::resized() {
    repaint();
}

void AdvancedHeatmapComponent::mouseDown(const juce::MouseEvent& e) {
    if (e.mods.isLeftButtonDown() && heatmapArea_.contains(e.getPosition())) {
        if (e.mods.isShiftDown()) {
            // Start zoom selection
            isZooming_ = true;
            zoomStartPoint_ = e.getPosition();
        } else {
            // Scrub to position
            double timestamp = xToTimestamp(e.x, heatmapArea_);
            if (onScrubPositionChanged) {
                onScrubPositionChanged(timestamp);
            }
        }
    }
}

void AdvancedHeatmapComponent::mouseDrag(const juce::MouseEvent& e) {
    if (isZooming_ && e.mods.isShiftDown()) {
        repaint();
    } else if (e.mods.isLeftButtonDown() && heatmapArea_.contains(e.getPosition())) {
        // Continue scrubbing
        double timestamp = xToTimestamp(e.x, heatmapArea_);
        if (onScrubPositionChanged) {
            onScrubPositionChanged(timestamp);
        }
    }
}

void AdvancedHeatmapComponent::mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) {
    if (heatmapArea_.contains(e.getPosition())) {
        // Zoom with mouse wheel
        double zoomFactor = 1.0 + (wheel.deltaY * 0.1);
        double timeSpan = zoomEnd_ - zoomStart_;
        double newTimeSpan = timeSpan / zoomFactor;
        
        // Center zoom around mouse position
        double mouseTime = xToTimestamp(e.x, heatmapArea_);
        double leftRatio = (mouseTime - zoomStart_) / timeSpan;
        
        zoomStart_ = mouseTime - newTimeSpan * leftRatio;
        zoomEnd_ = mouseTime + newTimeSpan * (1.0 - leftRatio);
        
        if (onZoomRangeChanged) {
            onZoomRangeChanged(zoomStart_, zoomEnd_);
        }
        
        repaint();
    }
}

void AdvancedHeatmapComponent::drawHeatmapData(juce::Graphics& g, juce::Rectangle<int> area) {
    if (samples_.empty()) return;
    
    // Filter samples within zoom range
    std::vector<Sample> visibleSamples;
    for (const auto& sample : samples_) {
        if (sample.timestamp >= zoomStart_ && sample.timestamp <= zoomEnd_) {
            visibleSamples.push_back(sample);
        }
    }
    
    if (visibleSamples.empty()) return;
    
    // Draw heatmap columns
    int columnWidth = highResolution_ ? 1 : juce::jmax(1, area.getWidth() / static_cast<int>(visibleSamples.size()));
    
    for (size_t i = 0; i < visibleSamples.size(); ++i) {
        const auto& sample = visibleSamples[i];
        
        int x = static_cast<int>(timestampToX(sample.timestamp, area));
        auto columnArea = juce::Rectangle<int>(x, area.getY(), columnWidth, area.getHeight());
        
        if (sample.valid) {
            g.setColour(getValueColour(sample.value));
        } else {
            g.setColour(juce::Colours::darkgrey.darker(0.5f));
        }
        
        g.fillRect(columnArea);
    }
    
    // Draw grid if enabled
    if (config_.showGrid) {
        g.setColour(juce::Colours::white.withAlpha(0.1f));
        
        // Horizontal grid lines
        for (int i = 1; i < 4; ++i) {
            int y = area.getY() + (area.getHeight() * i / 4);
            g.drawHorizontalLine(y, static_cast<float>(area.getX()), static_cast<float>(area.getRight()));
        }
        
        // Vertical grid lines
        for (int i = 1; i < 4; ++i) {
            int x = area.getX() + (area.getWidth() * i / 4);
            g.drawVerticalLine(x, static_cast<float>(area.getY()), static_cast<float>(area.getBottom()));
        }
    }
    
    // Border
    g.setColour(juce::Colours::white.withAlpha(0.3f));
    g.drawRect(area, 1);
}

void AdvancedHeatmapComponent::drawTimeAxis(juce::Graphics& g, juce::Rectangle<int> area) {
    if (!config_.showTimestamps) return;
    
    g.setColour(juce::Colours::lightgrey);
    g.setFont(juce::Font(10.0f));
    
    double timeSpan = zoomEnd_ - zoomStart_;
    int numTicks = juce::jmin(10, area.getWidth() / 50);
    
    for (int i = 0; i <= numTicks; ++i) {
        double time = zoomStart_ + (timeSpan * i / numTicks);
        int x = static_cast<int>(timestampToX(time, juce::Rectangle<int>(area.getX(), 0, area.getWidth(), 1)));
        
        juce::String timeStr = juce::String(time, 1) + "s";
        g.drawText(timeStr, x - 20, area.getY(), 40, area.getHeight(), juce::Justification::centred);
        
        // Tick mark
        g.drawVerticalLine(x, static_cast<float>(area.getY() - 2), static_cast<float>(area.getY()));
    }
}

void AdvancedHeatmapComponent::drawValueAxis(juce::Graphics& g, juce::Rectangle<int> area) {
    g.setColour(juce::Colours::lightgrey);
    g.setFont(juce::Font(10.0f));
    
    int numTicks = 5;
    for (int i = 0; i <= numTicks; ++i) {
        float value = config_.maxValue - ((config_.maxValue - config_.minValue) * i / numTicks);
        int y = area.getY() + (area.getHeight() * i / numTicks);
        
        juce::String valueStr = juce::String(value, 1);
        if (!config_.unit.isEmpty()) {
            valueStr += " " + config_.unit;
        }
        
        g.drawText(valueStr, area.getX(), y - 6, area.getWidth() - 2, 12, juce::Justification::centredRight);
        
        // Tick mark
        g.drawHorizontalLine(y, static_cast<float>(area.getRight()), static_cast<float>(area.getRight() + 2));
    }
}

void AdvancedHeatmapComponent::drawPlayhead(juce::Graphics& g, juce::Rectangle<int> area) {
    if (playheadPosition_ < 0.0) return;
    
    if (playheadPosition_ >= zoomStart_ && playheadPosition_ <= zoomEnd_) {
        int x = static_cast<int>(timestampToX(playheadPosition_, area));
        g.setColour(juce::Colours::yellow);
        g.drawVerticalLine(x, static_cast<float>(area.getY()), static_cast<float>(area.getBottom()));
        
        // Playhead indicator
        juce::Path triangle;
        triangle.addTriangle(x - 5, area.getY() - 5, x + 5, area.getY() - 5, x, area.getY());
        g.fillPath(triangle);
    }
}

void AdvancedHeatmapComponent::drawZoomOverlay(juce::Graphics& g) {
    if (isZooming_) {
        auto currentMouse = getMouseXYRelative();
        auto zoomRect = juce::Rectangle<int>::leftTopRightBottom(
            juce::jmin(zoomStartPoint_.x, currentMouse.x),
            juce::jmin(zoomStartPoint_.y, currentMouse.y),
            juce::jmax(zoomStartPoint_.x, currentMouse.x),
            juce::jmax(zoomStartPoint_.y, currentMouse.y)
        );
        
        g.setColour(juce::Colours::yellow.withAlpha(0.3f));
        g.fillRect(zoomRect);
        g.setColour(juce::Colours::yellow);
        g.drawRect(zoomRect, 1);
    }
}

juce::Colour AdvancedHeatmapComponent::getValueColour(float value) const {
    if (!std::isfinite(value)) {
        return juce::Colours::transparentBlack;
    }
    
    float normalised = juce::jlimit(0.0f, 1.0f, (value - config_.minValue) / (config_.maxValue - config_.minValue));
    return config_.lowColour.interpolatedWith(config_.highColour, normalised);
}

double AdvancedHeatmapComponent::timestampToX(double timestamp, juce::Rectangle<int> area) const {
    if (zoomEnd_ <= zoomStart_) return area.getX();
    
    double ratio = (timestamp - zoomStart_) / (zoomEnd_ - zoomStart_);
    return area.getX() + (area.getWidth() * ratio);
}

double AdvancedHeatmapComponent::xToTimestamp(int x, juce::Rectangle<int> area) const {
    if (area.getWidth() <= 0) return zoomStart_;
    
    double ratio = static_cast<double>(x - area.getX()) / area.getWidth();
    return zoomStart_ + (zoomEnd_ - zoomStart_) * ratio;
}

// ComparativeMetricsComponent Implementation

ComparativeMetricsComponent::ComparativeMetricsComponent() {
}

void ComparativeMetricsComponent::addSession(const juce::String& name, const std::vector<yvc::AnalysisResults>& data, 
                                           juce::Colour colour) {
    SessionData session;
    session.name = name;
    session.data = data;
    session.colour = colour;
    sessions_.push_back(session);
    repaint();
}

void ComparativeMetricsComponent::removeSession(const juce::String& name) {
    sessions_.erase(
        std::remove_if(sessions_.begin(), sessions_.end(),
                      [&name](const SessionData& s) { return s.name == name; }),
        sessions_.end());
    repaint();
}

void ComparativeMetricsComponent::setSessionVisibility(const juce::String& name, bool visible) {
    for (auto& session : sessions_) {
        if (session.name == name) {
            session.visible = visible;
            break;
        }
    }
    repaint();
}

void ComparativeMetricsComponent::setSessionAlpha(const juce::String& name, float alpha) {
    for (auto& session : sessions_) {
        if (session.name == name) {
            session.alpha = juce::jlimit(0.0f, 1.0f, alpha);
            break;
        }
    }
    repaint();
}

void ComparativeMetricsComponent::clearAllSessions() {
    sessions_.clear();
    repaint();
}

void ComparativeMetricsComponent::setDisplayedMetric(const juce::String& metricName) {
    currentMetric_ = metricName;
    repaint();
}

void ComparativeMetricsComponent::setTimeRange(double startTime, double endTime) {
    timeStart_ = startTime;
    timeEnd_ = endTime;
    repaint();
}

void ComparativeMetricsComponent::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds();
    
    // Background
    g.fillAll(juce::Colours::black.withAlpha(0.8f));
    g.setColour(juce::Colours::white.withAlpha(0.1f));
    g.drawRect(bounds, 1);
    
    // Title
    auto titleArea = bounds.removeFromTop(30);
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(16.0f, juce::Font::bold));
    g.drawText(currentMetric_ + " " + TRANS("comparison"), titleArea, juce::Justification::centred);
    
    if (sessions_.empty()) {
        g.setColour(juce::Colours::lightgrey);
        g.drawText("No sessions loaded", bounds, juce::Justification::centred);
        return;
    }
    
    // Draw each session
    auto chartArea = bounds.reduced(20, 10);
    for (const auto& session : sessions_) {
        if (session.visible) {
            drawSession(g, session, chartArea);
        }
    }
    
    // Legend
    auto legendArea = bounds.removeFromBottom(60).reduced(10);
    int legendX = legendArea.getX();
    for (const auto& session : sessions_) {
        if (session.visible) {
            g.setColour(session.colour.withAlpha(session.alpha));
            g.fillRect(legendX, legendArea.getY(), 15, 15);
            g.setColour(juce::Colours::white);
            g.setFont(juce::Font(12.0f));
            g.drawText(session.name, legendX + 20, legendArea.getY(), 100, 15, juce::Justification::centredLeft);
            legendX += 120;
        }
    }
}

void ComparativeMetricsComponent::resized() {
    repaint();
}

void ComparativeMetricsComponent::drawSession(juce::Graphics& g, const SessionData& session, juce::Rectangle<int> area) {
    if (session.data.empty()) return;
    
    auto range = getMetricRange(currentMetric_);
    float minValue = range.first;
    float maxValue = range.second;
    
    g.setColour(session.colour.withAlpha(session.alpha * 0.8f));
    
    juce::Path path;
    bool firstPoint = true;
    
    for (const auto& result : session.data) {
        if (result.timestamp >= timeStart_ && result.timestamp <= timeEnd_) {
            float value = getMetricValue(result, currentMetric_);
            if (std::isfinite(value)) {
                int x = static_cast<int>(area.getX() + 
                    ((result.timestamp - timeStart_) / (timeEnd_ - timeStart_)) * area.getWidth());
                int y = static_cast<int>(area.getBottom() - 
                    ((value - minValue) / (maxValue - minValue)) * area.getHeight());
                
                if (firstPoint) {
                    path.startNewSubPath(x, y);
                    firstPoint = false;
                } else {
                    path.lineTo(x, y);
                }
            }
        }
    }
    
    juce::PathStrokeType stroke(2.0f);
    g.strokePath(path, stroke);
}

float ComparativeMetricsComponent::getMetricValue(const yvc::AnalysisResults& result, const juce::String& metric) {
    if (metric == "f0") return result.f0;
    else if (metric == "rms") return result.rms;
    else if (metric == "peak") return result.peak;
    else if (metric == "cpp") return result.cpp;
    else if (metric == "hnr") return result.hnr;
    else if (metric == "spectral_tilt") return result.spectral_tilt;
    else if (metric == "speech_rate") return result.speech_rate;
    else if (metric == "pause_ratio") return result.pause_ratio;
    return 0.0f;
}

juce::String ComparativeMetricsComponent::getMetricUnit(const juce::String& metric) {
    if (metric == "f0") return "Hz";
    else if (metric == "rms" || metric == "peak") return "dBFS";
    else if (metric == "cpp" || metric == "hnr") return "dB";
    else if (metric == "spectral_tilt") return "dB/oct";
    else if (metric == "speech_rate") return "syl/s";
    return "";
}

std::pair<float, float> ComparativeMetricsComponent::getMetricRange(const juce::String& metric) {
    if (metric == "f0") return {50.0f, 500.0f};
    else if (metric == "rms") return {-60.0f, 0.0f};
    else if (metric == "peak") return {-40.0f, 0.0f};
    else if (metric == "cpp") return {-10.0f, 30.0f};
    else if (metric == "hnr") return {-20.0f, 40.0f};
    else if (metric == "spectral_tilt") return {-15.0f, 5.0f};
    else if (metric == "speech_rate") return {0.0f, 8.0f};
    else if (metric == "pause_ratio") return {0.0f, 1.0f};
    return {0.0f, 100.0f};
}

// Additional components would continue here...
// (SpectrumAnalyzerComponent and MiniWaveformComponent implementations)

} // namespace yvc::app
