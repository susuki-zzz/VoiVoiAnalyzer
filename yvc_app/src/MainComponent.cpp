// VoiVoi GUI Application - Main Component Implementation
// License: GPLv3

#include "MainComponent.h"
#include "LocalizationManager.h"
#include "SettingsDialog.h"

#include <juce_core/juce_core.h>
#include <yvc_core/Logger.h>

namespace yvc::app {

namespace {
constexpr int kDefaultWidth = 1180;
constexpr int kDefaultHeight = 680;
constexpr double kFpsEvaluationWindowMs = 2000.0;
constexpr double kDegradeThreshold = 0.82;
}

MainComponent::MainComponent(yvc::MetricsBus& metricsBus, juce::AudioDeviceManager& audioDeviceManager)
    : metricsBus_(metricsBus)
    , audioDeviceManager_(audioDeviceManager) {
    setOpaque(true);
    
    // Initialize localization
    auto& locManager = LocalizationManager::getInstance();
    locManager.loadLanguagePreference();

    // Get current device setup
    juce::AudioDeviceManager::AudioDeviceSetup setup;
    audioDeviceManager_.getAudioDeviceSetup(setup);
    settings_.inputDeviceName = setup.inputDeviceName;
    settings_.sampleRate = static_cast<int>(setup.sampleRate > 0 ? setup.sampleRate : 48000);
    settings_.bufferSize = setup.bufferSize > 0 ? setup.bufferSize : 512;

    // Create audio bridge with initial config
    yvc::AudioConfig audioConfig;
    audioConfig.sample_rate = static_cast<yvc::SampleRate>(settings_.sampleRate);
    audioConfig.buffer_size = static_cast<uint32_t>(settings_.bufferSize);
    audioConfig.num_channels = 1;
    audioConfig.mode = yvc::PerformanceMode::Mode_Standard;
    
    audioBridge_ = std::make_unique<JuceAudioBridge>(metricsBus_, audioConfig);

    // Timeline controller shared across time-axis components
    timeline_ = std::make_unique<TimelineController>();
    timeline_->setWindowSeconds(10.0);
    timeline_->setFollowLatest(true);

    // UI setup
    auto& presets = presetManager_.getPresets();
    int presetId = 1;
    for (const auto& preset : presets) {
        presetSelector_.addItem(TRANS(preset.nameKey), presetId++);
    }
    presetSelector_.setSelectedId(1);
    presetSelector_.addListener(this);
    addAndMakeVisible(presetSelector_);

    settingsButton_.setButtonText(TRANS("settings"));
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
    waveformDisplay_ = std::make_unique<WaveformComponent>();
    addAndMakeVisible(waveformDisplay_.get());
    // New scrolling waveform (time vs amplitude)
    scrollingWaveform_ = std::make_unique<ScrollingWaveformComponent>();
    addAndMakeVisible(scrollingWaveform_.get());

    // Wire timeline to time-axis components
    heatmapDisplay_.setTimelineController(timeline_.get());
    if (scrollingWaveform_) scrollingWaveform_->setTimelineController(timeline_.get());

    // Add advanced visualization components if enabled
    if (settings_.enableAdvancedVisualization) {
        advancedF0Heatmap_ = std::make_unique<AdvancedHeatmapComponent>(TRANS("heatmap_f0"));
        advancedLevelHeatmap_ = std::make_unique<AdvancedHeatmapComponent>(TRANS("heatmap_level"));
        
        addAndMakeVisible(advancedF0Heatmap_.get());
        addAndMakeVisible(advancedLevelHeatmap_.get());
        
        AdvancedHeatmapComponent::HeatmapConfig f0Config;
        f0Config.minValue = 60.0f;
        f0Config.maxValue = 400.0f;
        f0Config.unit = "Hz";
        f0Config.lowColour = juce::Colours::blue;
        f0Config.highColour = juce::Colours::red;
        advancedF0Heatmap_->setConfig(f0Config);
        
        AdvancedHeatmapComponent::HeatmapConfig levelConfig;
        levelConfig.minValue = -60.0f;
        levelConfig.maxValue = 6.0f;
        levelConfig.unit = "dBFS";
        levelConfig.lowColour = juce::Colours::darkblue;
        levelConfig.highColour = juce::Colours::yellow;
        advancedLevelHeatmap_->setConfig(levelConfig);

        // Hook zoom/scrub to timeline
        advancedF0Heatmap_->onZoomRangeChanged = [this](double s,double e){ if(timeline_) timeline_->setVisibleRange({s,e}); };
        advancedLevelHeatmap_->onZoomRangeChanged = [this](double s,double e){ if(timeline_) timeline_->setVisibleRange({s,e}); };
        advancedF0Heatmap_->onScrubPositionChanged = [this](double t){ if(timeline_) timeline_->setPlayhead(t); };
        advancedLevelHeatmap_->onScrubPositionChanged = [this](double t){ if(timeline_) timeline_->setPlayhead(t); };
    }
    
    if (settings_.showSpectralAnalysis) {
        spectrumAnalyzer_ = std::make_unique<SpectrumAnalyzerComponent>();
        addAndMakeVisible(spectrumAnalyzer_.get());
    }

    applyPreset(presetManager_.getActivePreset());
    refreshMetrics();
    metricsDisplay_.updateMetrics(currentMetrics_);
    heatmapDisplay_.appendSample(currentMetrics_);
    updateStatusBar();
    setSize(kDefaultWidth, kDefaultHeight);

    recordingStartMs_ = static_cast<juce::int64>(juce::Time::getMillisecondCounterHiRes());

    configureTimerForCurrentBudget();
    
    // Start audio processing
    startAudioProcessing();
}

MainComponent::~MainComponent() {
    stopTimer();
    stopAudioProcessing();
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

    if (settings_.enableAdvancedVisualization
        && advancedF0Heatmap_
        && advancedLevelHeatmap_) {
        auto heatmapArea = area.removeFromRight(static_cast<int>(area.getWidth() * 0.4f));
        auto topHeatmap = heatmapArea.removeFromTop(heatmapArea.getHeight() * 0.6f);
        advancedF0Heatmap_->setBounds(topHeatmap);
        auto midWave = heatmapArea.removeFromTop(heatmapArea.getHeight() * 0.4f);
        if(waveformDisplay_) waveformDisplay_->setBounds(midWave.reduced(0,4));
        if(scrollingWaveform_) scrollingWaveform_->setBounds(midWave.reduced(0,4).removeFromBottom(midWave.getHeight()/2));
        advancedLevelHeatmap_->setBounds(heatmapArea);
        
        if (settings_.showSpectralAnalysis && spectrumAnalyzer_) {
            auto spectrumArea = area.removeFromBottom(static_cast<int>(area.getHeight() * 0.3f));
            spectrumAnalyzer_->setBounds(spectrumArea);
        }
    } else {
        auto heatmapArea = area.removeFromRight(static_cast<int>(area.getWidth() * 0.35f));
        auto f0Area = heatmapArea.removeFromTop(heatmapArea.getHeight() * 0.65f);
        heatmapDisplay_.setBounds(f0Area);
        if(waveformDisplay_) waveformDisplay_->setBounds(heatmapArea.reduced(0,4));
        if(scrollingWaveform_) scrollingWaveform_->setBounds(heatmapArea.reduced(0,4).removeFromBottom(heatmapArea.getHeight()/2));
    }
    
    metricsDisplay_.setBounds(area);
}

void MainComponent::timerCallback() {
    refreshMetrics();
    updateStatusBar();
    metricsDisplay_.updateMetrics(currentMetrics_);
    heatmapDisplay_.appendSample(currentMetrics_);

    // Advance timeline to latest timestamp
    if (timeline_) timeline_->advanceToLatest(currentMetrics_.timestamp);
    // Reflect timeline to advanced heatmaps (if present)
    if (settings_.enableAdvancedVisualization && timeline_) {
        auto st = timeline_->getState();
        if (advancedF0Heatmap_) {
            advancedF0Heatmap_->setZoomRange(st.visibleRange.getStart(), st.visibleRange.getEnd());
            advancedF0Heatmap_->setPlayheadPosition(st.playhead);
        }
        if (advancedLevelHeatmap_) {
            advancedLevelHeatmap_->setZoomRange(st.visibleRange.getStart(), st.visibleRange.getEnd());
            advancedLevelHeatmap_->setPlayheadPosition(st.playhead);
        }
    }

    // Update waveform samples with timestamps
    {
        if (audioBridge_) {
            JuceAudioBridge::AudioSlice slice;
            audioBridge_->getRecentMonoSamples(slice, 1024);
            waveformDisplay_->setSamples(slice.samples, (float)audioBridge_->getSampleRate());
            if(scrollingWaveform_ && !slice.samples.empty())
                scrollingWaveform_->pushSamples(slice.samples.data(), slice.samples.size(), (float)audioBridge_->getSampleRate(), slice.endTimestamp);
        }
    }

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
        SettingsDialog::showDialog(settings_, this, audioDeviceManager_, [this](bool accepted, const AppSettings& updated) {
            if (accepted) {
                auto oldLanguage = settings_.language;
                settings_ = updated;
                
                if (oldLanguage != settings_.language) {
                    LocalizationManager::getInstance().setLanguage(settings_.language);
                    updateUILanguage();
                }

                applyAudioSettings();
                recordingStartMs_ = static_cast<juce::int64>(juce::Time::getMillisecondCounterHiRes());
                updateVisualizationLayout();
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
    presetDescription_.setText(TRANS(preset.descriptionKey), juce::dontSendNotification);
}

void MainComponent::updateStatusBar() {
    auto now = juce::Time::getMillisecondCounterHiRes();
    if (accumulatedFrames_ > 0 && accumulatedFrameTimeMs_ > 0.0) {
        currentFps_ = 1000.0 * static_cast<double>(accumulatedFrames_) / accumulatedFrameTimeMs_;
    }

    double cpuUsage = 5.0; // Placeholder
    int memoryUsage = 128;  // Placeholder MB usage
    
    try {
        auto memSizeMB = static_cast<int>(juce::SystemStats::getMemorySizeInMegabytes() * 0.1);
        if (memSizeMB > 0 && memSizeMB < 8192) {
            memoryUsage = memSizeMB;
        }
    } catch (...) {}

    double elapsedSeconds = (now - recordingStartMs_) / 1000.0;
    double remaining = juce::jmax(0.0, settings_.maxRecordingTimeSeconds - elapsedSeconds);
    int minutes = static_cast<int>(remaining) / 60;
    int seconds = static_cast<int>(remaining) % 60;

    juce::String fpsStr = TRANS("status_fps") + " " + juce::String(currentFps_, 1);
    juce::String cpuStr = TRANS("status_cpu") + " " + juce::String(cpuUsage, 0) + "%";
    juce::String ramStr = TRANS("status_ram") + " " + juce::String(memoryUsage) + " MB";
    juce::String timeStr = TRANS("status_time_left") + " " + 
                          juce::String::formatted("%02d:%02d", minutes, seconds);

    juce::String text = fpsStr + " | " + cpuStr + " | " + ramStr + " | " + timeStr;
    statusBar_.setText(text, juce::dontSendNotification);
}

void MainComponent::updateUILanguage() {
    settingsButton_.setButtonText(TRANS("settings"));
    
    presetSelector_.clear();
    auto& presets = presetManager_.getPresets();
    int presetId = 1;
    for (const auto& preset : presets) {
        presetSelector_.addItem(TRANS(preset.nameKey), presetId++);
    }
    presetSelector_.setSelectedId(1);
    
    const auto& currentPreset = presetManager_.getActivePreset();
    presetDescription_.setText(TRANS(currentPreset.descriptionKey), juce::dontSendNotification);
    
    repaint();
}

void MainComponent::updateVisualizationLayout() {
    if (advancedF0Heatmap_) {
        removeChildComponent(advancedF0Heatmap_.get());
        advancedF0Heatmap_.reset();
    }
    if (advancedLevelHeatmap_) {
        removeChildComponent(advancedLevelHeatmap_.get());
        advancedLevelHeatmap_.reset();
    }
    if (spectrumAnalyzer_) {
        removeChildComponent(spectrumAnalyzer_.get());
        spectrumAnalyzer_.reset();
    }
    // keep waveform components
    
    if (settings_.enableAdvancedVisualization) {
        advancedF0Heatmap_ = std::make_unique<AdvancedHeatmapComponent>(TRANS("heatmap_f0"));
        advancedLevelHeatmap_ = std::make_unique<AdvancedHeatmapComponent>(TRANS("heatmap_level"));
        
        addAndMakeVisible(advancedF0Heatmap_.get());
        addAndMakeVisible(advancedLevelHeatmap_.get());
        
        AdvancedHeatmapComponent::HeatmapConfig f0Config;
        f0Config.minValue = 60.0f;
        f0Config.maxValue = 400.0f;
        f0Config.unit = "Hz";
        advancedF0Heatmap_->setConfig(f0Config);
        
        AdvancedHeatmapComponent::HeatmapConfig levelConfig;
        levelConfig.minValue = -60.0f;
        levelConfig.maxValue = 6.0f;
        levelConfig.unit = "dBFS";
        advancedLevelHeatmap_->setConfig(levelConfig);

        // Hook zoom/scrub to timeline
        advancedF0Heatmap_->onZoomRangeChanged = [this](double s,double e){ if(timeline_) timeline_->setVisibleRange({s,e}); };
        advancedLevelHeatmap_->onZoomRangeChanged = [this](double s,double e){ if(timeline_) timeline_->setVisibleRange({s,e}); };
        advancedF0Heatmap_->onScrubPositionChanged = [this](double t){ if(timeline_) timeline_->setPlayhead(t); };
        advancedLevelHeatmap_->onScrubPositionChanged = [this](double t){ if(timeline_) timeline_->setPlayhead(t); };
    }
    
    if (settings_.showSpectralAnalysis) {
        spectrumAnalyzer_ = std::make_unique<SpectrumAnalyzerComponent>();
        addAndMakeVisible(spectrumAnalyzer_.get());
    }
    
    // Apply heatmap scale mode
    switch (settings_.heatmapScaleMode) {
        case 2: heatmapDisplay_.setScaleMode(HeatmapComponent::ScaleMode::LogHz); break;
        case 3: heatmapDisplay_.setScaleMode(HeatmapComponent::ScaleMode::MidiNote); break;
        default: heatmapDisplay_.setScaleMode(HeatmapComponent::ScaleMode::LinearHz); break;
    }

    // Wire timeline again (in case of re-create)
    heatmapDisplay_.setTimelineController(timeline_.get());
    if (scrollingWaveform_) scrollingWaveform_->setTimelineController(timeline_.get());
    
    resized();
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
            juce::String notice = TRANS("fps_limited").replace("%d", juce::String(frameBudgets_[frameBudgetIndex_]));
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
    fpsIndicator_.setText(TRANS("status_fps") + " Target: " + juce::String(currentBudget), juce::dontSendNotification);
    if (frameBudgetIndex_ == 0) {
        degradationNotice_.setVisible(false);
        degradationNoticeExpiryMs_ = 0;
    }
}

void MainComponent::applyAudioSettings() {
    // Stop audio processing
    stopAudioProcessing();

    juce::AudioDeviceManager::AudioDeviceSetup setup;
    audioDeviceManager_.getAudioDeviceSetup(setup);

    if (settings_.inputDeviceName.isNotEmpty())
        setup.inputDeviceName = settings_.inputDeviceName;
    if (settings_.sampleRate > 0)
        setup.sampleRate = static_cast<double>(settings_.sampleRate);
    if (settings_.bufferSize > 0)
        setup.bufferSize = settings_.bufferSize;

    juce::String error = audioDeviceManager_.setAudioDeviceSetup(setup, true);
    if (error.isNotEmpty()) {
        LOG_WARNF("Audio device setup warning: %s", error.toRawUTF8());
    }

    // Update bridge config
    yvc::AudioConfig audioConfig;
    audioConfig.sample_rate = static_cast<yvc::SampleRate>(settings_.sampleRate);
    audioConfig.buffer_size = static_cast<uint32_t>(settings_.bufferSize);
    audioConfig.num_channels = 1;
    audioConfig.mode = yvc::PerformanceMode::Mode_Standard;
    
    audioBridge_->updateConfig(audioConfig);

    // Restart audio processing
    startAudioProcessing();
}

void MainComponent::startAudioProcessing() {
    if (audioBridge_) {
        audioDeviceManager_.addAudioCallback(audioBridge_.get());
        LOG_INFO("Audio processing started");
    }
}

void MainComponent::stopAudioProcessing() {
    if (audioBridge_) {
        audioDeviceManager_.removeAudioCallback(audioBridge_.get());
        LOG_INFO("Audio processing stopped");
    }
}

} // namespace yvc::app
