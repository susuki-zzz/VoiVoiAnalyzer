// VoiVoi GUI Application - Enhanced Visualization Components
// License: GPLv3

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "yvc_core/Types.h"
#include <deque>
#include <vector>
#include <functional>

namespace yvc::app {

namespace test {
class AdvancedHeatmapComponentTestPeer;
class ComparativeMetricsComponentTestPeer;
class SpectrumAnalyzerComponentTestPeer;
} // namespace test

/**
 * @brief Advanced heatmap with zoom, scrubbing, and export capabilities
 * 
 * Provides enhanced visualization features according to roadmap:
 * - Zoom controls for time range selection
 * - Time-range scrubbing with playback position indicator
 * - Resolution toggle (high/low detail modes)
 * - Export to image with annotations
 */
class AdvancedHeatmapComponent : public juce::Component {
public:
    explicit AdvancedHeatmapComponent(const juce::String& title);
    
    // Configuration
    struct HeatmapConfig {
        float minValue = 0.0f;
        float maxValue = 100.0f;
        juce::String unit = "";
        juce::Colour lowColour = juce::Colours::blue;
        juce::Colour highColour = juce::Colours::red;
        bool showGrid = true;
        bool showTimestamps = true;
        int maxSamples = 300;
    };
    
    void setConfig(const HeatmapConfig& config);
    void appendSample(float value, double timestamp);
    void clear();
    
    // Zoom and scrubbing controls
    void setZoomRange(double startTime, double endTime);
    void setPlayheadPosition(double timestamp);
    void setResolutionMode(bool highResolution);
    
    // Export functionality
    juce::Image createExportImage(int width, int height, bool includeAnnotations = true);
    bool exportToPNG(const juce::File& file, int width = 1920, int height = 1080);
    
    // Callbacks
    std::function<void(double, double)> onZoomRangeChanged;
    std::function<void(double)> onScrubPositionChanged;
    
    // Component overrides
    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override;
    
private:
    struct Sample {
        float value = 0.0f;
        double timestamp = 0.0;
        bool valid = true;
    };
    
    juce::String title_;
    HeatmapConfig config_;
    std::deque<Sample> samples_;
    
    // Zoom and view state
    double zoomStart_ = 0.0;
    double zoomEnd_ = 60.0;  // Default 60 second view
    double playheadPosition_ = -1.0;
    bool highResolution_ = true;
    bool isZooming_ = false;
    juce::Point<int> zoomStartPoint_;
    
    // UI elements
    juce::Rectangle<int> heatmapArea_;
    juce::Rectangle<int> timeAxis_;
    juce::Rectangle<int> valueAxis_;
    juce::Rectangle<int> zoomControls_;
    
    void drawHeatmapData(juce::Graphics& g, juce::Rectangle<int> area);
    void drawTimeAxis(juce::Graphics& g, juce::Rectangle<int> area);
    void drawValueAxis(juce::Graphics& g, juce::Rectangle<int> area);
    void drawPlayhead(juce::Graphics& g, juce::Rectangle<int> area);
    void drawZoomOverlay(juce::Graphics& g);

    juce::Colour getValueColour(float value) const;
    double timestampToX(double timestamp, juce::Rectangle<int> area) const;
    double xToTimestamp(int x, juce::Rectangle<int> area) const;

    friend class test::AdvancedHeatmapComponentTestPeer;
};

/**
 * @brief Comparative session overlay component
 * 
 * Allows overlaying metrics from different sessions for comparison.
 * Supports multiple sessions with different colors and transparency.
 */
class ComparativeMetricsComponent : public juce::Component {
public:
    ComparativeMetricsComponent();
    
    struct SessionData {
        juce::String name;
        juce::Colour colour;
        std::vector<yvc::AnalysisResults> data;
        float alpha = 1.0f;
        bool visible = true;
    };
    
    // Session management
    void addSession(const juce::String& name, const std::vector<yvc::AnalysisResults>& data, 
                   juce::Colour colour = juce::Colours::white);
    void removeSession(const juce::String& name);
    void setSessionVisibility(const juce::String& name, bool visible);
    void setSessionAlpha(const juce::String& name, float alpha);
    void clearAllSessions();
    
    // Display configuration
    void setDisplayedMetric(const juce::String& metricName);
    void setTimeRange(double startTime, double endTime);
    
    // Component overrides
    void paint(juce::Graphics& g) override;
    void resized() override;
    
private:
    std::vector<SessionData> sessions_;
    juce::String currentMetric_ = "f0";
    double timeStart_ = 0.0;
    double timeEnd_ = 60.0;
    
    void drawSession(juce::Graphics& g, const SessionData& session, juce::Rectangle<int> area);
    float getMetricValue(const yvc::AnalysisResults& result, const juce::String& metric);
    juce::String getMetricUnit(const juce::String& metric);
    std::pair<float, float> getMetricRange(const juce::String& metric);

    friend class test::ComparativeMetricsComponentTestPeer;
};

/**
 * @brief Advanced spectrum analyzer with FFT visualization
 * 
 * Real-time FFT display with:
 * - Configurable frequency range
 * - Peak hold functionality
 * - Harmonic overlay for pitch detection
 * - Spectral tilt visualization
 */
class SpectrumAnalyzerComponent : public juce::Component, public juce::Timer {
public:
    SpectrumAnalyzerComponent();
    
    // Configuration
    void setFFTSize(int fftSize);
    void setSampleRate(float sampleRate);
    void setFrequencyRange(float minFreq, float maxFreq);
    void setPeakHoldEnabled(bool enabled);
    void setHarmonicsOverlayEnabled(bool enabled);
    
    // Data input
    void updateSpectrum(const float* magnitudeSpectrum, int spectrumSize);
    void updateF0(float f0, bool valid);
    
    // Component overrides
    void paint(juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;
    
private:
    int fftSize_ = 2048;
    float sampleRate_ = 48000.0f;
    float minFreq_ = 20.0f;
    float maxFreq_ = 20000.0f;
    bool peakHoldEnabled_ = true;
    bool harmonicsOverlayEnabled_ = true;
    
    std::vector<float> currentSpectrum_;
    std::vector<float> peakSpectrum_;
    float currentF0_ = 0.0f;
    bool f0Valid_ = false;
    
    float peakDecayRate_ = 0.95f;
    
    void drawFrequencyAxis(juce::Graphics& g, juce::Rectangle<int> area);
    void drawMagnitudeAxis(juce::Graphics& g, juce::Rectangle<int> area);
    void drawSpectrum(juce::Graphics& g, juce::Rectangle<int> area);
    void drawPeakHold(juce::Graphics& g, juce::Rectangle<int> area);
    void drawHarmonics(juce::Graphics& g, juce::Rectangle<int> area);
    void drawSpectralTilt(juce::Graphics& g, juce::Rectangle<int> area);
    
    float frequencyToX(float freq, juce::Rectangle<int> area) const;
    float magnitudeToY(float mag, juce::Rectangle<int> area) const;
    int frequencyToBin(float freq) const;

    friend class test::SpectrumAnalyzerComponentTestPeer;
};

/**
 * @brief Mini waveform display with VU-style meters
 */
class MiniWaveformComponent : public juce::Component {
public:
    MiniWaveformComponent();
    
    void updateLevel(float rms, float peak);
    void appendSample(float sample);
    void setDisplayLength(double seconds);
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
private:
    std::deque<float> waveformData_;
    float currentRMS_ = 0.0f;
    float currentPeak_ = 0.0f;
    double displayLength_ = 2.0;  // 2 seconds
    int maxSamples_ = 1000;
    
    void drawWaveform(juce::Graphics& g, juce::Rectangle<int> area);
    void drawVUMeters(juce::Graphics& g, juce::Rectangle<int> area);
};

} // namespace yvc::app
