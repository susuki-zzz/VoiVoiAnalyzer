// VoiVoi GUI Application - Settings Dialog
// License: GPLv3

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "LocalizationManager.h"

#include <functional>

namespace yvc::app {

struct AppSettings {
    int sampleRate = 48000;
    int bufferSize = 512;
    double maxRecordingTimeSeconds = 600.0;
    bool autoSave = true;
    bool enablePreprocessing = true;
    LocalizationManager::Language language = LocalizationManager::Language::English;
    bool enableAdvancedVisualization = true;
    bool showSpectralAnalysis = false;
    int heatmapResolution = 1; // 1=high, 2=medium, 3=low
};

class SettingsDialog : public juce::Component,
                       private juce::Button::Listener,
                       private juce::ComboBox::Listener {
public:
    using OnClose = std::function<void(bool, const AppSettings&)>;

    static void showDialog(const AppSettings& currentSettings, juce::Component* parent, OnClose onClose);

    void resized() override;
    void paint(juce::Graphics& g) override;
    ~SettingsDialog() override;

private:
    SettingsDialog(const AppSettings& currentSettings, OnClose onClose);

    void buttonClicked(juce::Button* button) override;
    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override;

    void applyTo(AppSettings& settings) const;
    void close(bool accepted);
    void updateUILanguage();
    void createTabbedInterface();

    AppSettings workingCopy_;
    OnClose onClose_;

    // Main UI
    juce::Label titleLabel_;
    std::unique_ptr<juce::TabbedComponent> tabbedComponent_;
    juce::TextButton okButton_;
    juce::TextButton cancelButton_;
    
    // Audio Settings Tab
    juce::Component audioTab_;
    juce::ComboBox sampleRateBox_;
    juce::ComboBox bufferSizeBox_;
    juce::ComboBox performanceModeBox_;
    juce::Label sampleRateLabel_;
    juce::Label bufferSizeLabel_;
    juce::Label performanceModeLabel_;
    
    // Recording Settings Tab
    juce::Component recordingTab_;
    juce::Label maxRecordingLabel_;
    juce::Slider maxRecordingSlider_;
    juce::ToggleButton autoSaveToggle_;
    juce::ToggleButton preprocToggle_;
    
    // Display Settings Tab
    juce::Component displayTab_;
    juce::ComboBox languageBox_;
    juce::Label languageLabel_;
    juce::ToggleButton advancedVisualizationToggle_;
    juce::ToggleButton spectralAnalysisToggle_;
    juce::ComboBox heatmapResolutionBox_;
    juce::Label heatmapResolutionLabel_;
    
    // Privacy Settings Tab  
    juce::Component privacyTab_;
    juce::Label privacyInfoLabel_;
    juce::ToggleButton ramOnlyToggle_;
    juce::ToggleButton networkingDisabledToggle_;

    bool hasClosed_ = false;
};

} // namespace yvc::app
