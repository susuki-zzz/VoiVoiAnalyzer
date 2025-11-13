// VoiVoi GUI Application - Settings Component Implementation
// License: GPLv3

#include "SettingsComponent.h"
#include <yvc_core/PerformanceMode.h>

// Undefine Windows macros that conflict with JUCE
#ifdef Normal
#undef Normal
#endif

namespace yvc::app {

SettingsComponent::SettingsComponent() {
    // Initialize audio backend
    audioBackend_ = yvc::audio::createPlatformBackend();
    
    // Device selection
    deviceLabel_.setText("Input Device:", juce::dontSendNotification);
    deviceLabel_.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(deviceLabel_);

    deviceComboBox_.setTextWhenNothingSelected("Select audio input device...");
    deviceComboBox_.onChange = [this] { 
        auto selectedId = deviceComboBox_.getSelectedId();
        if (selectedId > 0 && selectedId <= static_cast<int>(availableDevices_.size())) {
            const auto& device = availableDevices_[selectedId - 1];
            currentSettings_.inputDeviceId = device.id;
            currentSettings_.inputDeviceName = device.name;
            
            deviceInfoLabel_.setText(
                juce::String("Channels: ") + juce::String(device.channels) +
                juce::String(", Default SR: ") + juce::String(device.sampleRate) + " Hz",
                juce::dontSendNotification
            );
            
            updateSampleRateOptions();
        }
    };
    addAndMakeVisible(deviceComboBox_);

    deviceInfoLabel_.setJustificationType(juce::Justification::centredLeft);
    deviceInfoLabel_.setFont(juce::Font(12.0f));
    deviceInfoLabel_.setColour(juce::Label::textColourId, juce::Colours::grey);
    addAndMakeVisible(deviceInfoLabel_);

    // Sample rate selection
    sampleRateLabel_.setText("Sample Rate:", juce::dontSendNotification);
    sampleRateLabel_.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(sampleRateLabel_);

    sampleRateComboBox_.addItem("Auto (48 kHz preferred)", 1);
    sampleRateComboBox_.addItem("44100 Hz", 2);
    sampleRateComboBox_.addItem("48000 Hz", 3);
    sampleRateComboBox_.addItem("88200 Hz", 4);
    sampleRateComboBox_.addItem("96000 Hz", 5);
    sampleRateComboBox_.setSelectedId(1);
    sampleRateComboBox_.onChange = [this] {
        int id = sampleRateComboBox_.getSelectedId();
        switch (id) {
            case 1: currentSettings_.sampleRate = 48000; break;
            case 2: currentSettings_.sampleRate = 44100; break;
            case 3: currentSettings_.sampleRate = 48000; break;
            case 4: currentSettings_.sampleRate = 88200; break;
            case 5: currentSettings_.sampleRate = 96000; break;
        }
        sampleRateInfoLabel_.setText(
            id == 1 ? "Will negotiate with device (prefers 48 kHz)" : "Fixed sample rate",
            juce::dontSendNotification
        );
    };
    addAndMakeVisible(sampleRateComboBox_);

    sampleRateInfoLabel_.setJustificationType(juce::Justification::centredLeft);
    sampleRateInfoLabel_.setFont(juce::Font(12.0f));
    sampleRateInfoLabel_.setColour(juce::Label::textColourId, juce::Colours::grey);
    addAndMakeVisible(sampleRateInfoLabel_);

    // Buffer size (read-only, OS default)
    bufferSizeLabel_.setText("Buffer Size:", juce::dontSendNotification);
    bufferSizeLabel_.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(bufferSizeLabel_);

    bufferSizeValueLabel_.setText("512 samples (OS default)", juce::dontSendNotification);
    bufferSizeValueLabel_.setJustificationType(juce::Justification::centredLeft);
    bufferSizeValueLabel_.setFont(juce::Font(14.0f, juce::Font::FontStyleFlags::bold));
    addAndMakeVisible(bufferSizeValueLabel_);

    bufferSizeInfoLabel_.setText("Buffer size is fixed to OS default for optimal latency", juce::dontSendNotification);
    bufferSizeInfoLabel_.setJustificationType(juce::Justification::centredLeft);
    bufferSizeInfoLabel_.setFont(juce::Font(12.0f));
    bufferSizeInfoLabel_.setColour(juce::Label::textColourId, juce::Colours::grey);
    addAndMakeVisible(bufferSizeInfoLabel_);

    // Performance mode
    performanceModeLabel_.setText("Performance Mode:", juce::dontSendNotification);
    performanceModeLabel_.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(performanceModeLabel_);

    performanceModeComboBox_.addItem("Light (≤40ms latency)", static_cast<int>(::yvc::PerformanceMode::Mode_Light) + 1);
    performanceModeComboBox_.addItem("Normal (≤60ms latency)", static_cast<int>(::yvc::PerformanceMode::Mode_Standard) + 1);
    performanceModeComboBox_.addItem("Diagnostic (≤80ms latency)", static_cast<int>(::yvc::PerformanceMode::Mode_Diagnostic) + 1);
    performanceModeComboBox_.setSelectedId(static_cast<int>(::yvc::PerformanceMode::Mode_Standard) + 1);
    performanceModeComboBox_.onChange = [this] {
        int id = performanceModeComboBox_.getSelectedId() - 1;
        currentSettings_.performanceMode = static_cast<::yvc::PerformanceMode>(id);
        
        juce::String desc;
        switch (currentSettings_.performanceMode) {
            case ::yvc::PerformanceMode::Mode_Light:
                desc = "FFT: 1024, Hop: 512\nMinimal latency for real-time practice\nEssential metrics only";
                break;
            case ::yvc::PerformanceMode::Mode_Standard:
                desc = "FFT: 2048, Hop: 512\nBalanced response and detail\nAll metrics enabled";
                break;
            case ::yvc::PerformanceMode::Mode_Diagnostic:
                desc = "FFT: 4096, Hop: 1024\nDetailed analysis for evaluation\nFull spectrum with harmonics";
                break;
        }
        performanceModeDescEditor_.setText(desc);
    };
    addAndMakeVisible(performanceModeComboBox_);

    performanceModeDescEditor_.setMultiLine(true);
    performanceModeDescEditor_.setReadOnly(true);
    performanceModeDescEditor_.setFont(juce::Font(12.0f));
    performanceModeDescEditor_.setColour(juce::TextEditor::backgroundColourId, juce::Colours::transparentBlack);
    performanceModeDescEditor_.setColour(juce::TextEditor::outlineColourId, juce::Colours::grey);
    performanceModeDescEditor_.setText("FFT: 2048, Hop: 512\nBalanced response and detail\nAll metrics enabled");
    addAndMakeVisible(performanceModeDescEditor_);

    // Live max duration
    liveMaxLabel_.setText("Live Recording Limit:", juce::dontSendNotification);
    liveMaxLabel_.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(liveMaxLabel_);

    liveMaxSlider_.setRange(5, 120, 5);
    liveMaxSlider_.setValue(60);
    liveMaxSlider_.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    liveMaxSlider_.onValueChange = [this] {
        currentSettings_.liveMaxMinutes = static_cast<int>(liveMaxSlider_.getValue());
        liveMaxValueLabel_.setText(juce::String(currentSettings_.liveMaxMinutes) + " minutes", juce::dontSendNotification);
    };
    addAndMakeVisible(liveMaxSlider_);

    liveMaxValueLabel_.setText("60 minutes", juce::dontSendNotification);
    liveMaxValueLabel_.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(liveMaxValueLabel_);

    // Auto-save options
    autoSaveLabel_.setText("Auto-Save:", juce::dontSendNotification);
    autoSaveLabel_.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(autoSaveLabel_);

    autoSaveToggle_.setButtonText("Enable auto-save");
    autoSaveToggle_.onClick = [this] {
        currentSettings_.autoSaveEnabled = autoSaveToggle_.getToggleState();
        autoSaveOnStopToggle_.setEnabled(currentSettings_.autoSaveEnabled);
        autoSaveOnLimitToggle_.setEnabled(currentSettings_.autoSaveEnabled);
    };
    addAndMakeVisible(autoSaveToggle_);

    autoSaveOnStopToggle_.setButtonText("Save on stop");
    autoSaveOnStopToggle_.onClick = [this] {
        currentSettings_.autoSaveOnStop = autoSaveOnStopToggle_.getToggleState();
    };
    addAndMakeVisible(autoSaveOnStopToggle_);

    autoSaveOnLimitToggle_.setButtonText("Save on limit reached");
    autoSaveOnLimitToggle_.onClick = [this] {
        currentSettings_.autoSaveOnLimit = autoSaveOnLimitToggle_.getToggleState();
    };
    addAndMakeVisible(autoSaveOnLimitToggle_);

    // Buttons
    applyButton_.setButtonText("Apply");
    applyButton_.onClick = [this] { applySettings(); };
    addAndMakeVisible(applyButton_);

    cancelButton_.setButtonText("Cancel");
    cancelButton_.onClick = [this] {
        currentSettings_ = originalSettings_;
        if (auto* window = findParentComponentOfClass<juce::DocumentWindow>()) {
            window->closeButtonPressed();
        }
    };
    addAndMakeVisible(cancelButton_);

    defaultsButton_.setButtonText("Restore Defaults");
    defaultsButton_.onClick = [this] {
        Settings defaults;
        defaults.sampleRate = 48000;
        defaults.bufferSize = 512;
        defaults.performanceMode = ::yvc::PerformanceMode::Mode_Standard;
        defaults.liveMaxMinutes = 60;
        defaults.autoSaveEnabled = true;
        defaults.autoSaveOnStop = true;
        defaults.autoSaveOnLimit = true;
        setSettings(defaults);
    };
    addAndMakeVisible(defaultsButton_);

    // Initialize device list
    updateDeviceList();

    // Set default values
    currentSettings_.sampleRate = 48000;
    currentSettings_.bufferSize = 512;
    currentSettings_.performanceMode = ::yvc::PerformanceMode::Mode_Standard;
    currentSettings_.liveMaxMinutes = 60;
    currentSettings_.autoSaveEnabled = true;
    currentSettings_.autoSaveOnStop = true;
    currentSettings_.autoSaveOnLimit = true;
    originalSettings_ = currentSettings_;

    setSize(600, 650);
}

SettingsComponent::~SettingsComponent() = default;

void SettingsComponent::paint(juce::Graphics& g) {
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    
    auto bounds = getLocalBounds();
    g.setColour(juce::Colours::grey);
    g.drawRect(bounds.reduced(10), 1);
    
    // Draw section separators
    g.drawLine(20.0f, 180.0f, static_cast<float>(getWidth() - 20), 180.0f);
    g.drawLine(20.0f, 280.0f, static_cast<float>(getWidth() - 20), 280.0f);
    g.drawLine(20.0f, 450.0f, static_cast<float>(getWidth() - 20), 450.0f);
}

void SettingsComponent::resized() {
    auto bounds = getLocalBounds().reduced(20);
    
    // Audio Device Section
    bounds.removeFromTop(10);
    deviceLabel_.setBounds(bounds.removeFromTop(24));
    deviceComboBox_.setBounds(bounds.removeFromTop(24).reduced(0, 2));
    deviceInfoLabel_.setBounds(bounds.removeFromTop(20));
    
    bounds.removeFromTop(10);
    sampleRateLabel_.setBounds(bounds.removeFromTop(24));
    sampleRateComboBox_.setBounds(bounds.removeFromTop(24).reduced(0, 2));
    sampleRateInfoLabel_.setBounds(bounds.removeFromTop(20));
    
    bounds.removeFromTop(10);
    bufferSizeLabel_.setBounds(bounds.removeFromTop(24));
    bufferSizeValueLabel_.setBounds(bounds.removeFromTop(24));
    bufferSizeInfoLabel_.setBounds(bounds.removeFromTop(20));
    
    // Performance Mode Section
    bounds.removeFromTop(20);
    performanceModeLabel_.setBounds(bounds.removeFromTop(24));
    performanceModeComboBox_.setBounds(bounds.removeFromTop(24).reduced(0, 2));
    performanceModeDescEditor_.setBounds(bounds.removeFromTop(80).reduced(0, 2));
    
    // Live Recording Section
    bounds.removeFromTop(20);
    auto liveRow = bounds.removeFromTop(24);
    liveMaxLabel_.setBounds(liveRow.removeFromLeft(200));
    liveMaxValueLabel_.setBounds(liveRow.removeFromRight(100));
    liveMaxSlider_.setBounds(liveRow.reduced(10, 0));
    
    // Auto-save Section
    bounds.removeFromTop(30);
    autoSaveLabel_.setBounds(bounds.removeFromTop(24));
    autoSaveToggle_.setBounds(bounds.removeFromTop(24).reduced(20, 0));
    autoSaveOnStopToggle_.setBounds(bounds.removeFromTop(24).reduced(40, 0));
    autoSaveOnLimitToggle_.setBounds(bounds.removeFromTop(24).reduced(40, 0));
    
    // Buttons
    bounds.removeFromTop(20);
    auto buttonRow = bounds.removeFromTop(30);
    defaultsButton_.setBounds(buttonRow.removeFromLeft(150));
    buttonRow.removeFromLeft(10);
    applyButton_.setBounds(buttonRow.removeFromRight(100));
    buttonRow.removeFromRight(10);
    cancelButton_.setBounds(buttonRow.removeFromRight(100));
}

void SettingsComponent::updateDeviceList() {
    deviceComboBox_.clear();
    availableDevices_.clear();
    
    if (!audioBackend_) {
        deviceComboBox_.addItem("No audio backend available", 1);
        deviceComboBox_.setEnabled(false);
        return;
    }
    
    availableDevices_ = audioBackend_->enumerateInputDevices();
    
    if (availableDevices_.empty()) {
        deviceComboBox_.addItem("No audio input devices found", 1);
        deviceComboBox_.setEnabled(false);
        return;
    }
    
    int defaultIndex = 1;
    for (size_t i = 0; i < availableDevices_.size(); ++i) {
        const auto& device = availableDevices_[i];
        juce::String itemText = juce::String(device.name);
        if (device.isDefault) {
            itemText += " (Default)";
            defaultIndex = static_cast<int>(i + 1);
        }
        deviceComboBox_.addItem(itemText, static_cast<int>(i + 1));
    }
    
    deviceComboBox_.setSelectedId(defaultIndex);
    deviceComboBox_.setEnabled(true);
}

void SettingsComponent::updateSampleRateOptions() {
    // Update info based on selected device
    sampleRateInfoLabel_.setText(
        sampleRateComboBox_.getSelectedId() == 1 
            ? "Will negotiate with device (prefers 48 kHz)" 
            : "Fixed sample rate",
        juce::dontSendNotification
    );
}

void SettingsComponent::applySettings() {
    originalSettings_ = currentSettings_;
    
    if (onSettingsChanged) {
        onSettingsChanged(currentSettings_);
    }
    
    if (auto* window = findParentComponentOfClass<juce::DocumentWindow>()) {
        window->closeButtonPressed();
    }
}

SettingsComponent::Settings SettingsComponent::getSettings() const {
    return currentSettings_;
}

void SettingsComponent::setSettings(const Settings& settings) {
    currentSettings_ = settings;
    
    // Update UI to reflect settings
    sampleRateComboBox_.setSelectedId(1);  // Auto by default
    
    performanceModeComboBox_.setSelectedId(static_cast<int>(settings.performanceMode) + 1);
    
    liveMaxSlider_.setValue(settings.liveMaxMinutes);
    liveMaxValueLabel_.setText(juce::String(settings.liveMaxMinutes) + " minutes", juce::dontSendNotification);
    
    autoSaveToggle_.setToggleState(settings.autoSaveEnabled, juce::dontSendNotification);
    autoSaveOnStopToggle_.setToggleState(settings.autoSaveOnStop, juce::dontSendNotification);
    autoSaveOnLimitToggle_.setToggleState(settings.autoSaveOnLimit, juce::dontSendNotification);
    autoSaveOnStopToggle_.setEnabled(settings.autoSaveEnabled);
    autoSaveOnLimitToggle_.setEnabled(settings.autoSaveEnabled);
}

// SettingsWindow implementation
SettingsWindow::SettingsWindow()
    : juce::DocumentWindow("VoiVoi Analyzer - Settings",
                     juce::Desktop::getInstance().getDefaultLookAndFeel()
                         .findColour(juce::ResizableWindow::backgroundColourId),
                     juce::DocumentWindow::closeButton) {
    setUsingNativeTitleBar(true);
    
    auto* comp = new SettingsComponent();
    setContentOwned(comp, true);
    
    centreWithSize(getWidth(), getHeight());
    setResizable(false, false);
    setVisible(true);
}

SettingsWindow::~SettingsWindow() = default;

void SettingsWindow::closeButtonPressed() {
    setVisible(false);
}

SettingsComponent* SettingsWindow::getSettingsComponent() {
    return dynamic_cast<SettingsComponent*>(getContentComponent());
}

} // namespace yvc::app
