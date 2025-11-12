// VoiVoi GUI Application - Main Component Implementation
// License: GPLv3

#include "MainComponent.h"

#include <juce_core/juce_core.h>

namespace yvc::app {

namespace {
constexpr int kDefaultWidth = 1180;
constexpr int kDefaultHeight = 680;
constexpr double kFpsEvaluationWindowMs = 2000.0;
constexpr double kDegradeThreshold = 0.82;
}

MainComponent::MainComponent(yvc::MetricsBus& metricsBus)
    : metricsBus_(metricsBus) {
    setSize(kDefaultWidth, kDefaultHeight);
    setOpaque(true);

    auto& presets = presetManager_.getPresets();
    int presetId = 1;
    for (const auto& preset : presets) {
        presetSelector_.addItem(preset.name, presetId++);
    }
    presetSelector_.setSelectedId(1);
    presetSelector_.addListener(this);
    addAndMakeVisible(presetSelector_);

    settingsButton_.addListener(this);
    addAndMakeVisible(settingsButton_);

    presetDescription_.setJustificationType(juce::Justification::centredLeft);
    presetDescription_.setMinimumHorizontalScale(1.0f);
    presetDescription_.setFont(juce::Font(14.0f));
    presetDescription_.setInterceptsMouseClicks(false, false);
    presetDescription_.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    addAndMakeVisible(presetDescription_);

    statusBar_.setJustificationType(juce::Justification::centredRight);
    statusBar_.setColour(juce::Label::backgroundColourId, juce::Colours::black.withAlpha(0.4f));
    statusBar_.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    addAndMakeVisible(statusBar_);

    fpsIndicator_.setJustificationType(juce::Justification::centred);
    fpsIndicator_.setFont(juce::Font(14.0f, juce::Font::bold));
    fpsIndicator_.setColour(juce::Label::textColourId, juce::Colours::lightskyblue);
    fpsIndicator_.setInterceptsMouseClicks(false, false);
    addAndMakeVisible(fpsIndicator_);

    degradationNotice_.setJustificationType(juce::Justification::centredLeft);
    degradationNotice_.setColour(juce::Label::textColourId, juce::Colours::orange);
    degradationNotice_.setInterceptsMouseClicks(false, false);
    degradationNotice_.setVisible(false);
    addAndMakeVisible(degradationNotice_);

    addAndMakeVisible(metricsDisplay_);
    addAndMakeVisible(heatmapDisplay_);

    applyPreset(presetManager_.getActivePreset());
    refreshMetrics();
    metricsDisplay_.updateMetrics(currentMetrics_);
    heatmapDisplay_.appendSample(currentMetrics_);
    updateStatusBar();
    recordingStartMs_ = static_cast<juce::int64>(juce::Time::getMillisecondCounterHiRes());

    configureTimerForCurrentBudget();
}

MainComponent::~MainComponent() {
    stopTimer();
    settingsButton_.removeListener(this);
    presetSelector_.removeListener(this);
}

void MainComponent::paint(juce::Graphics& g) {
    auto now = static_cast<juce::int64>(juce::Time::getMillisecondCounterHiRes());
    if (lastPaintTimestampMs_ > 0) {
        auto delta = static_cast<double>(now - lastPaintTimestampMs_);
        accumulatedFrameTimeMs_ += delta;
        accumulatedFrames_ += 1;
        evaluateFrameBudget();
    }
    lastPaintTimestampMs_ = now;

    g.fillAll(juce::Colours::black.withAlpha(0.92f));

    g.setColour(juce::Colours::white.withAlpha(0.05f));
    auto bounds = getLocalBounds().toFloat().reduced(12.0f);
    g.drawRoundedRectangle(bounds, 10.0f, 1.0f);
}

void MainComponent::resized() {
    auto area = getLocalBounds().reduced(12);
    auto header = area.removeFromTop(48);
    presetSelector_.setBounds(header.removeFromLeft(220));
    header.removeFromLeft(12);

    settingsButton_.setBounds(header.removeFromRight(120));
    header.removeFromRight(12);

    fpsIndicator_.setBounds(header.removeFromRight(140));
    header.removeFromRight(8);
    presetDescription_.setBounds(header);

    auto statusArea = area.removeFromBottom(28);
    auto degradationArea = statusArea.removeFromLeft(240);
    degradationNotice_.setBounds(degradationArea.reduced(4, 0));
    statusBar_.setBounds(statusArea);

    auto heatmapArea = area.removeFromRight(static_cast<int>(area.getWidth() * 0.35f));
    heatmapDisplay_.setBounds(heatmapArea);
    metricsDisplay_.setBounds(area);
}

void MainComponent::timerCallback() {
    refreshMetrics();
    updateStatusBar();
    metricsDisplay_.updateMetrics(currentMetrics_);
    heatmapDisplay_.appendSample(currentMetrics_);

    auto now = static_cast<juce::int64>(juce::Time::getMillisecondCounterHiRes());
    if (degradationNoticeExpiryMs_ > 0 && now > degradationNoticeExpiryMs_) {
        degradationNotice_.setVisible(false);
        degradationNoticeExpiryMs_ = 0;
    }
    repaint();
}

void MainComponent::comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) {
    if (comboBoxThatHasChanged == &presetSelector_) {
        auto index = static_cast<size_t>(presetSelector_.getSelectedId() - 1);
        const auto& preset = presetManager_.setActivePreset(index);
        applyPreset(preset);
    }
}

void MainComponent::buttonClicked(juce::Button* button) {
    if (button == &settingsButton_) {
        SettingsDialog::showDialog(settings_, this, [this](bool accepted, const AppSettings& updated) {
            if (accepted) {
                settings_ = updated;
                recordingStartMs_ = static_cast<juce::int64>(juce::Time::getMillisecondCounterHiRes());
            }
        });
    }
}

void MainComponent::refreshMetrics() {
    yvc::AnalysisResults latest;
    if (metricsBus_.read(latest)) {
        currentMetrics_ = latest;
    } else {
        currentMetrics_ = metricsBus_.getLatest();
    }
}

void MainComponent::applyPreset(const Preset& preset) {
    metricsDisplay_.setDisplayedMetrics(preset.metrics);
    presetDescription_.setText(preset.description, juce::dontSendNotification);
}

void MainComponent::updateStatusBar() {
    auto now = juce::Time::getMillisecondCounterHiRes();
    if (accumulatedFrames_ > 0 && accumulatedFrameTimeMs_ > 0.0) {
        currentFps_ = 1000.0 * static_cast<double>(accumulatedFrames_) / accumulatedFrameTimeMs_;
    }

    auto cpuUsage = juce::SystemStats::getCpuUsage() * 100.0;
    cpuUsage = juce::jlimit(0.0, 100.0, cpuUsage);
    auto memoryUsage = juce::SystemStats::getMemoryUsageInMegabytes();

    double elapsedSeconds = (now - recordingStartMs_) / 1000.0;
    double remaining = juce::jmax(0.0, settings_.maxRecordingTimeSeconds - elapsedSeconds);
    int minutes = static_cast<int>(remaining) / 60;
    int seconds = static_cast<int>(remaining) % 60;

    juce::String text = juce::String::formatted("FPS %.1f | CPU %.0f%% | RAM %d MB | Time Left %02d:%02d",
                                                currentFps_, cpuUsage, memoryUsage, minutes, seconds);
    statusBar_.setText(text, juce::dontSendNotification);
}

void MainComponent::evaluateFrameBudget() {
    accumulatedFrameTimeMs_ = juce::jmin(accumulatedFrameTimeMs_, 10000.0);
    if (fpsEvaluationStartMs_ == 0)
        fpsEvaluationStartMs_ = lastPaintTimestampMs_;

    auto elapsed = static_cast<double>(lastPaintTimestampMs_ - fpsEvaluationStartMs_);
    if (elapsed >= kFpsEvaluationWindowMs && accumulatedFrames_ > 0) {
        double averageFrameMs = accumulatedFrameTimeMs_ / accumulatedFrames_;
        double targetFrameMs = 1000.0 / static_cast<double>(frameBudgets_[frameBudgetIndex_]);
        if (averageFrameMs > targetFrameMs / kDegradeThreshold && frameBudgetIndex_ + 1 < frameBudgets_.size()) {
            frameBudgetIndex_++;
            configureTimerForCurrentBudget();
            juce::String notice = "Auto limited to " + juce::String(frameBudgets_[frameBudgetIndex_]) + " FPS";
            degradationNotice_.setText(notice, juce::dontSendNotification);
            degradationNotice_.setVisible(true);
            degradationNoticeExpiryMs_ = lastPaintTimestampMs_ + 4000;
        }
        accumulatedFrameTimeMs_ = 0.0;
        accumulatedFrames_ = 0;
        fpsEvaluationStartMs_ = lastPaintTimestampMs_;
    }
}

void MainComponent::configureTimerForCurrentBudget() {
    stopTimer();
    auto currentBudget = frameBudgets_[frameBudgetIndex_];
    startTimerHz(currentBudget);
    fpsIndicator_.setText("FPS Target: " + juce::String(currentBudget), juce::dontSendNotification);
    if (frameBudgetIndex_ == 0) {
        degradationNotice_.setVisible(false);
        degradationNoticeExpiryMs_ = 0;
    }
}

} // namespace yvc::app
