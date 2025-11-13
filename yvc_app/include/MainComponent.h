// VoiVoi GUI Application - Main Component
// License: GPLv3

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_devices/juce_audio_devices.h>

#include <array>

#include "MetricsComponents.h"
#include "PresetManager.h"
#include "LocalizationManager.h"
#include "VisualizationComponents.h"
#include "SettingsDialog.h"
#include "yvc_core/MetricsBus.h"

namespace yvc::app {

// Forward declarations
struct AppSettings;
class SettingsDialog;

class MainComponent : public juce::Component,
                      private juce::Timer,
                      private juce::ComboBox::Listener,
                      private juce::Button::Listener {
public:
    explicit MainComponent(yvc::MetricsBus& metricsBus, juce::AudioDeviceManager& audioDeviceManager);
    ~MainComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void timerCallback() override;
    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override;
    void buttonClicked(juce::Button* button) override;

    void refreshMetrics();
    void applyPreset(const Preset& preset);
    void updateStatusBar();
    void updateUILanguage();
    void updateVisualizationLayout();
    void evaluateFrameBudget();
    void configureTimerForCurrentBudget();
    void applyAudioSettings(); // 追加: デバイス/レート/バッファ適用

    yvc::MetricsBus& metricsBus_;
    juce::AudioDeviceManager& audioDeviceManager_;
    yvc::AnalysisResults currentMetrics_;
    MetricsDisplayComponent metricsDisplay_;
    HeatmapComponent heatmapDisplay_;
    PresetManager presetManager_;
    AppSettings settings_;

    // Enhanced visualization components
    std::unique_ptr<AdvancedHeatmapComponent> advancedF0Heatmap_;
    std::unique_ptr<AdvancedHeatmapComponent> advancedLevelHeatmap_;
    std::unique_ptr<SpectrumAnalyzerComponent> spectrumAnalyzer_;

    juce::ComboBox presetSelector_;
    juce::TextButton settingsButton_{ "Settings" };
    juce::Label presetDescription_;
    juce::Label statusBar_;
    juce::Label fpsIndicator_;
    juce::Label degradationNotice_;

    juce::int64 lastPaintTimestampMs_ = 0;
    double accumulatedFrameTimeMs_ = 0.0;
    int accumulatedFrames_ = 0;
    double currentFps_ = 60.0;

    const std::array<int, 3> frameBudgets_{ { 60, 45, 30 } };
    size_t frameBudgetIndex_ = 0;

    juce::int64 fpsEvaluationStartMs_ = 0;
    juce::int64 recordingStartMs_ = 0;
    juce::int64 degradationNoticeExpiryMs_ = 0;
};

} // namespace yvc::app
