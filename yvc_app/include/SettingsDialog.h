// VoiVoi GUI Application - Settings Dialog
// License: GPLv3

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include <functional>

namespace yvc::app {

struct AppSettings {
    int sampleRate = 48000;
    int bufferSize = 512;
    double maxRecordingTimeSeconds = 600.0;
    bool autoSave = true;
    bool enablePreprocessing = true;
};

class SettingsDialog : public juce::Component,
                       private juce::Button::Listener,
                       private juce::ComboBox::Listener {
public:
    using OnClose = std::function<void(bool, const AppSettings&)>;

    static void showDialog(const AppSettings& currentSettings, juce::Component* parent, OnClose onClose);

    void resized() override;
    ~SettingsDialog() override;

private:
    SettingsDialog(const AppSettings& currentSettings, OnClose onClose);

    void buttonClicked(juce::Button* button) override;
    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override;

    void applyTo(AppSettings& settings) const;
    void close(bool accepted);

    AppSettings workingCopy_;
    OnClose onClose_;

    juce::Label titleLabel_;
    juce::ComboBox sampleRateBox_;
    juce::ComboBox bufferSizeBox_;
    juce::Label sampleRateLabel_;
    juce::Label bufferSizeLabel_;
    juce::Label maxRecordingLabel_;
    juce::Slider maxRecordingSlider_;
    juce::ToggleButton autoSaveToggle_;
    juce::ToggleButton preprocToggle_;
    juce::TextButton okButton_{ "Apply" };
    juce::TextButton cancelButton_{ "Cancel" };

    bool hasClosed_ = false;
};

} // namespace yvc::app
