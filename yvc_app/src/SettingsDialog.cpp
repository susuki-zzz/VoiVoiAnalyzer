// VoiVoi GUI Application - Settings Dialog Implementation
// License: GPLv3

#include "SettingsDialog.h"

#include <utility>

namespace yvc::app {

namespace {
constexpr int kDialogWidth = 380;
constexpr int kDialogHeight = 300;
} // namespace

void SettingsDialog::showDialog(const AppSettings& currentSettings, juce::Component* parent, OnClose onClose) {
    auto* dialog = new SettingsDialog(currentSettings, std::move(onClose));

    juce::DialogWindow::LaunchOptions options;
    options.content.set(dialog, true);
    options.dialogTitle = "Settings";
    options.dialogBackgroundColour = juce::Colours::black.withAlpha(0.85f);
    options.escapeKeyTriggersCloseButton = true;
    options.useNativeTitleBar = true;
    options.resizable = false;
    options.componentToCentreAround = parent;
    options.launchAsync();
}

SettingsDialog::SettingsDialog(const AppSettings& currentSettings, OnClose onClose)
    : workingCopy_(currentSettings), onClose_(std::move(onClose)) {
    setSize(kDialogWidth, kDialogHeight);

    titleLabel_.setText("Audio & Recording", juce::dontSendNotification);
    titleLabel_.setFont(juce::Font(18.0f, juce::Font::bold));
    addAndMakeVisible(titleLabel_);

    sampleRateLabel_.setText("Sample Rate", juce::dontSendNotification);
    sampleRateLabel_.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(sampleRateLabel_);

    sampleRateBox_.addItem("44.1 kHz", 1);
    sampleRateBox_.addItem("48 kHz", 2);
    sampleRateBox_.addItem("96 kHz", 3);
    sampleRateBox_.setSelectedId(workingCopy_.sampleRate == 44100 ? 1 : workingCopy_.sampleRate == 96000 ? 3 : 2);
    sampleRateBox_.addListener(this);
    addAndMakeVisible(sampleRateBox_);

    bufferSizeLabel_.setText("Buffer Size", juce::dontSendNotification);
    bufferSizeLabel_.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(bufferSizeLabel_);

    bufferSizeBox_.addItem("128", 1);
    bufferSizeBox_.addItem("256", 2);
    bufferSizeBox_.addItem("512", 3);
    bufferSizeBox_.addItem("1024", 4);
    if (workingCopy_.bufferSize == 128)
        bufferSizeBox_.setSelectedId(1);
    else if (workingCopy_.bufferSize == 256)
        bufferSizeBox_.setSelectedId(2);
    else if (workingCopy_.bufferSize == 1024)
        bufferSizeBox_.setSelectedId(4);
    else
        bufferSizeBox_.setSelectedId(3);
    bufferSizeBox_.addListener(this);
    addAndMakeVisible(bufferSizeBox_);

    maxRecordingLabel_.setText("Max Recording", juce::dontSendNotification);
    maxRecordingLabel_.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(maxRecordingLabel_);

    maxRecordingSlider_.setRange(60.0, 3600.0, 30.0);
    maxRecordingSlider_.setValue(workingCopy_.maxRecordingTimeSeconds);
    maxRecordingSlider_.setSliderStyle(juce::Slider::LinearHorizontal);
    maxRecordingSlider_.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
    addAndMakeVisible(maxRecordingSlider_);
    maxRecordingSlider_.setTextValueSuffix(" s");

    autoSaveToggle_.setButtonText("Auto save sessions");
    autoSaveToggle_.setToggleState(workingCopy_.autoSave, juce::dontSendNotification);
    addAndMakeVisible(autoSaveToggle_);

    preprocToggle_.setButtonText("Enable preprocessor");
    preprocToggle_.setToggleState(workingCopy_.enablePreprocessing, juce::dontSendNotification);
    addAndMakeVisible(preprocToggle_);

    okButton_.addListener(this);
    cancelButton_.addListener(this);
    addAndMakeVisible(okButton_);
    addAndMakeVisible(cancelButton_);
}

SettingsDialog::~SettingsDialog() {
    if (!hasClosed_ && onClose_)
        onClose_(false, workingCopy_);
}

void SettingsDialog::resized() {
    auto bounds = getLocalBounds().reduced(20);
    titleLabel_.setBounds(bounds.removeFromTop(28));

    auto sampleRow = bounds.removeFromTop(40);
    sampleRateLabel_.setBounds(sampleRow.removeFromLeft(150));
    sampleRateBox_.setBounds(sampleRow.removeFromLeft(160));

    auto bufferRow = bounds.removeFromTop(40);
    bufferSizeLabel_.setBounds(bufferRow.removeFromLeft(150));
    bufferSizeBox_.setBounds(bufferRow.removeFromLeft(160));

    auto sliderRow = bounds.removeFromTop(60);
    maxRecordingLabel_.setBounds(sliderRow.removeFromTop(24));
    maxRecordingSlider_.setBounds(sliderRow);

    auto toggles = bounds.removeFromTop(60);
    autoSaveToggle_.setBounds(toggles.removeFromTop(28));
    preprocToggle_.setBounds(toggles.removeFromTop(28));

    auto buttonRow = bounds.removeFromBottom(36);
    okButton_.setBounds(buttonRow.removeFromRight(100));
    buttonRow.removeFromRight(12);
    cancelButton_.setBounds(buttonRow.removeFromRight(100));
}

void SettingsDialog::buttonClicked(juce::Button* button) {
    if (button == &okButton_) {
        applyTo(workingCopy_);
        close(true);
    } else if (button == &cancelButton_) {
        close(false);
    }
}

void SettingsDialog::comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) {
    if (comboBoxThatHasChanged == &sampleRateBox_) {
        if (sampleRateBox_.getSelectedId() == 1)
            workingCopy_.sampleRate = 44100;
        else if (sampleRateBox_.getSelectedId() == 3)
            workingCopy_.sampleRate = 96000;
        else
            workingCopy_.sampleRate = 48000;
    } else if (comboBoxThatHasChanged == &bufferSizeBox_) {
        switch (bufferSizeBox_.getSelectedId()) {
        case 1: workingCopy_.bufferSize = 128; break;
        case 2: workingCopy_.bufferSize = 256; break;
        case 4: workingCopy_.bufferSize = 1024; break;
        default: workingCopy_.bufferSize = 512; break;
        }
    }
}

void SettingsDialog::applyTo(AppSettings& settings) const {
    settings = workingCopy_;
    settings.maxRecordingTimeSeconds = maxRecordingSlider_.getValue();
    settings.autoSave = autoSaveToggle_.getToggleState();
    settings.enablePreprocessing = preprocToggle_.getToggleState();
}

void SettingsDialog::close(bool accepted) {
    if (hasClosed_)
        return;

    if (accepted)
        applyTo(workingCopy_);

    hasClosed_ = true;

    if (onClose_)
        onClose_(accepted, workingCopy_);

    if (auto* window = findParentComponentOfClass<juce::DialogWindow>())
        window->closeButtonPressed();
}

} // namespace yvc::app
