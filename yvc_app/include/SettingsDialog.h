// VoiVoi GUI Application - Settings Dialog
// License: GPLv3

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include "LocalizationManager.h"

#include <functional>
#include <vector>

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
    juce::String inputDeviceName; // 追加: 入力デバイス名
};

class SettingsDialog : public juce::Component,
                       private juce::Button::Listener,
                       private juce::ComboBox::Listener {
public:
    using OnClose = std::function<void(bool, const AppSettings&)>;

    static void showDialog(const AppSettings& currentSettings, juce::Component* parent, juce::AudioDeviceManager& audioDeviceManager, OnClose onClose);

    // Test helpers: allow constructing and interacting with the dialog in unit tests
    static std::unique_ptr<SettingsDialog> createForTest(const AppSettings& currentSettings, juce::AudioDeviceManager& audioDeviceManager, OnClose onClose) {
        return std::unique_ptr<SettingsDialog>(new SettingsDialog(currentSettings, audioDeviceManager, std::move(onClose)));
    }

    // Lightweight getters/setters for tests (no production code depends on these)
    int getSelectedSampleRateId() const { return sampleRateBox_.getSelectedId(); }
    int getSelectedBufferSizeId() const { return bufferSizeBox_.getSelectedId(); }
    int getHeatmapResolutionId() const { return heatmapResolutionBox_.getSelectedId(); }
    LocalizationManager::Language getSelectedLanguage() const { return static_cast<LocalizationManager::Language>(juce::jmax(1, languageBox_.getSelectedId()) - 1); }
    bool isAutoSaveEnabled() const { return autoSaveToggle_.getToggleState(); }
    bool isPreprocessingEnabled() const { return preprocToggle_.getToggleState(); }
    bool isAdvancedVisualizationEnabled() const { return advancedVisualizationToggle_.getToggleState(); }
    bool isSpectralAnalysisEnabled() const { return spectralAnalysisToggle_.getToggleState(); }
    double getMaxRecordingTimeForTest() const { return maxRecordingSlider_.getValue(); }

    void setSelectedSampleRateForTest(int sr) { sampleRateBox_.setSelectedId(sr, juce::sendNotification); }
    void setSelectedBufferSizeForTest(int bs) { bufferSizeBox_.setSelectedId(bs, juce::sendNotification); }
    void setHeatmapResolutionForTest(int id) { heatmapResolutionBox_.setSelectedId(id, juce::sendNotification); }
    void setLanguageForTest(LocalizationManager::Language lang) { languageBox_.setSelectedId(static_cast<int>(lang) + 1, juce::sendNotification); }
    void setAutoSaveForTest(bool enabled) { autoSaveToggle_.setToggleState(enabled, juce::sendNotification); }
    void setPreprocessingForTest(bool enabled) { preprocToggle_.setToggleState(enabled, juce::sendNotification); }
    void setAdvancedVisualizationForTest(bool enabled) { advancedVisualizationToggle_.setToggleState(enabled, juce::sendNotification); }
    void setSpectralAnalysisForTest(bool enabled) { spectralAnalysisToggle_.setToggleState(enabled, juce::sendNotification); }
    void setMaxRecordingTimeForTest(double seconds) { maxRecordingSlider_.setValue(seconds, juce::sendNotification); }
    void simulateOkForTest() { close(true); }
    void simulateCancelForTest() { close(false); }

    void resized() override;
    void paint(juce::Graphics& g) override;
    ~SettingsDialog() override;

private:
    SettingsDialog(const AppSettings& currentSettings, juce::AudioDeviceManager& audioDeviceManager, OnClose onClose);

    void buttonClicked(juce::Button* button) override;
    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override;

    void applyTo(AppSettings& settings) const;
    void close(bool accepted);
    void updateUILanguage();
    void createTabbedInterface();
    void layoutTabItems(juce::Component* tab, const std::vector<std::pair<juce::Component*, juce::Component*>>& items);
    void populateAudioDeviceList();
    void populateSampleRateAndBufferBoxes();

    AppSettings workingCopy_;
    OnClose onClose_;
    juce::AudioDeviceManager* audioDeviceManager_ = nullptr;

    // Main UI
    juce::Label titleLabel_;
    std::unique_ptr<juce::TabbedComponent> tabbedComponent_;
    juce::TextButton okButton_;
    juce::TextButton cancelButton_;
    
    // Audio Settings Tab
    juce::Component audioTab_;
    juce::Label inputDeviceLabel_;
    juce::ComboBox inputDeviceBox_;
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

    // Layout data for tabs
    std::vector<std::pair<juce::Component*, juce::Component*>> audioTabItems_;
    std::vector<std::pair<juce::Component*, juce::Component*>> recordingTabItems_;
    std::vector<std::pair<juce::Component*, juce::Component*>> displayTabItems_;
    std::vector<std::pair<juce::Component*, juce::Component*>> privacyTabItems_;

    bool hasClosed_ = false;
};

} // namespace yvc::app
