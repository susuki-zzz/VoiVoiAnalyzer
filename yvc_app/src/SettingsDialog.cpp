// VoiVoi GUI Application - Settings Dialog Implementation
// License: GPLv3

#include "SettingsDialog.h"
#include "LocalizationManager.h"
#include <juce_gui_extra/juce_gui_extra.h>

namespace yvc::app {

namespace {
constexpr int kDialogWidth = 500;
constexpr int kDialogHeight = 400;
constexpr int kButtonHeight = 32;
constexpr int kRowHeight = 30;
constexpr int kMargin = 12;
}

void SettingsDialog::showDialog(const AppSettings& currentSettings, juce::Component* parent, OnClose onClose) {
    auto* dialog = new SettingsDialog(currentSettings, std::move(onClose));
    
    juce::DialogWindow::LaunchOptions options;
    options.dialogTitle = TRANS("settings_title");
    options.content.setOwned(dialog);
    options.componentToCentreAround = parent;
    options.escapeKeyTriggersCloseButton = true;
    options.useNativeTitleBar = true;
    options.resizable = false;
    options.dialogBackgroundColour = juce::Colours::darkgrey;
    
    // Use launchAsync instead of runModal for compatibility
    options.launchAsync();
}

SettingsDialog::SettingsDialog(const AppSettings& currentSettings, OnClose onClose)
    : workingCopy_(currentSettings)
    , onClose_(std::move(onClose))
    , titleLabel_("", TRANS("settings_title"))
    , okButton_(TRANS("settings_apply"))
    , cancelButton_(TRANS("settings_cancel"))
    , sampleRateLabel_("", TRANS("settings_sample_rate") + ":")
    , bufferSizeLabel_("", TRANS("settings_buffer_size") + ":")
    , performanceModeLabel_("", TRANS("settings_performance_mode") + ":")
    , maxRecordingLabel_("", TRANS("settings_max_duration") + ":")
    , languageLabel_("", TRANS("settings_language") + ":")
    , heatmapResolutionLabel_("", TRANS("heatmap_resolution") + ":")
    , privacyInfoLabel_("", TRANS("privacy_local_processing")) {
    
    setSize(kDialogWidth, kDialogHeight);
    
    createTabbedInterface();
    updateUILanguage();
    
    // Configure sample rate options
    sampleRateBox_.addItem("44.1 kHz", 44100);
    sampleRateBox_.addItem("48 kHz", 48000);
    sampleRateBox_.addItem("88.2 kHz", 88200);
    sampleRateBox_.addItem("96 kHz", 96000);
    sampleRateBox_.setSelectedId(workingCopy_.sampleRate, juce::dontSendNotification);
    sampleRateBox_.addListener(this);
    
    // Configure buffer size options
    bufferSizeBox_.addItem("128", 128);
    bufferSizeBox_.addItem("256", 256);
    bufferSizeBox_.addItem("512", 512);
    bufferSizeBox_.addItem("1024", 1024);
    bufferSizeBox_.setSelectedId(workingCopy_.bufferSize, juce::dontSendNotification);
    bufferSizeBox_.addListener(this);
    
    // Configure performance mode options
    performanceModeBox_.addItem(TRANS("mode_light"), 1);
    performanceModeBox_.addItem(TRANS("mode_standard"), 2);
    performanceModeBox_.addItem(TRANS("mode_diagnostic"), 3);
    performanceModeBox_.setSelectedId(2, juce::dontSendNotification); // Default to Standard
    performanceModeBox_.addListener(this);
    
    // Configure max recording duration
    maxRecordingSlider_.setRange(60.0, 3600.0, 60.0);
    maxRecordingSlider_.setValue(workingCopy_.maxRecordingTimeSeconds, juce::dontSendNotification);
    maxRecordingSlider_.setSliderStyle(juce::Slider::LinearHorizontal);
    maxRecordingSlider_.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
    maxRecordingSlider_.setNumDecimalPlacesToDisplay(0);
    
    // Configure language selection
    auto& locManager = LocalizationManager::getInstance();
    auto languages = locManager.getAvailableLanguages();
    for (size_t i = 0; i < languages.size(); ++i) {
        languageBox_.addItem(locManager.getLanguageName(languages[i]), static_cast<int>(languages[i]) + 1);
    }
    languageBox_.setSelectedId(static_cast<int>(workingCopy_.language) + 1, juce::dontSendNotification);
    languageBox_.addListener(this);
    
    // Configure heatmap resolution
    heatmapResolutionBox_.addItem(TRANS("resolution_high"), 1);
    heatmapResolutionBox_.addItem(TRANS("resolution_medium"), 2);
    heatmapResolutionBox_.addItem(TRANS("resolution_low"), 3);
    heatmapResolutionBox_.setSelectedId(workingCopy_.heatmapResolution, juce::dontSendNotification);
    heatmapResolutionBox_.addListener(this);
    
    // Configure toggles
    autoSaveToggle_.setButtonText(TRANS("settings_auto_save"));
    autoSaveToggle_.setToggleState(workingCopy_.autoSave, juce::dontSendNotification);
    
    preprocToggle_.setButtonText(TRANS("enable_preprocessing"));
    preprocToggle_.setToggleState(workingCopy_.enablePreprocessing, juce::dontSendNotification);
    
    advancedVisualizationToggle_.setButtonText(TRANS("advanced_visualization"));
    advancedVisualizationToggle_.setToggleState(workingCopy_.enableAdvancedVisualization, juce::dontSendNotification);
    
    spectralAnalysisToggle_.setButtonText(TRANS("spectral_analysis"));
    spectralAnalysisToggle_.setToggleState(workingCopy_.showSpectralAnalysis, juce::dontSendNotification);
    
    ramOnlyToggle_.setButtonText(TRANS("privacy_ram_only"));
    ramOnlyToggle_.setToggleState(true, juce::dontSendNotification);
    ramOnlyToggle_.setEnabled(false); // Always enabled for privacy
    
    networkingDisabledToggle_.setButtonText(TRANS("privacy_no_network"));
    networkingDisabledToggle_.setToggleState(true, juce::dontSendNotification);
    networkingDisabledToggle_.setEnabled(false); // Always enabled for privacy
    
    // Buttons
    okButton_.addListener(this);
    cancelButton_.addListener(this);
    
    addAndMakeVisible(titleLabel_);
    addAndMakeVisible(tabbedComponent_.get());
    addAndMakeVisible(okButton_);
    addAndMakeVisible(cancelButton_);
}

SettingsDialog::~SettingsDialog() {
    okButton_.removeListener(this);
    cancelButton_.removeListener(this);
    sampleRateBox_.removeListener(this);
    bufferSizeBox_.removeListener(this);
    performanceModeBox_.removeListener(this);
    languageBox_.removeListener(this);
    heatmapResolutionBox_.removeListener(this);
}

void SettingsDialog::createTabbedInterface() {
    tabbedComponent_ = std::make_unique<juce::TabbedComponent>(juce::TabbedButtonBar::TabsAtTop);
    
    // Audio Settings Tab
    auto* audioTab = new juce::Component();
    audioTab->addAndMakeVisible(sampleRateLabel_);
    audioTab->addAndMakeVisible(sampleRateBox_);
    audioTab->addAndMakeVisible(bufferSizeLabel_);
    audioTab->addAndMakeVisible(bufferSizeBox_);
    audioTab->addAndMakeVisible(performanceModeLabel_);
    audioTab->addAndMakeVisible(performanceModeBox_);
    
    tabbedComponent_->addTab(TRANS("settings_audio"), juce::Colours::darkgrey, audioTab, true);
    
    // Recording Settings Tab
    auto* recordingTab = new juce::Component();
    recordingTab->addAndMakeVisible(maxRecordingLabel_);
    recordingTab->addAndMakeVisible(maxRecordingSlider_);
    recordingTab->addAndMakeVisible(autoSaveToggle_);
    recordingTab->addAndMakeVisible(preprocToggle_);
    
    tabbedComponent_->addTab(TRANS("settings_recording"), juce::Colours::darkgrey, recordingTab, true);
    
    // Display Settings Tab
    auto* displayTab = new juce::Component();
    displayTab->addAndMakeVisible(languageLabel_);
    displayTab->addAndMakeVisible(languageBox_);
    displayTab->addAndMakeVisible(advancedVisualizationToggle_);
    displayTab->addAndMakeVisible(spectralAnalysisToggle_);
    displayTab->addAndMakeVisible(heatmapResolutionLabel_);
    displayTab->addAndMakeVisible(heatmapResolutionBox_);
    
    tabbedComponent_->addTab(TRANS("display_settings"), juce::Colours::darkgrey, displayTab, true);
    
    // Privacy Settings Tab
    auto* privacyTab = new juce::Component();
    privacyTab->addAndMakeVisible(privacyInfoLabel_);
    privacyTab->addAndMakeVisible(ramOnlyToggle_);
    privacyTab->addAndMakeVisible(networkingDisabledToggle_);
    
    tabbedComponent_->addTab(TRANS("privacy_settings"), juce::Colours::darkgrey, privacyTab, true);
    
    // Layout components in tabs
    auto layoutTab = [](juce::Component* tab, std::vector<std::pair<juce::Component*, juce::Component*>> items) {
        // Create a custom component that handles resizing
        class TabLayoutComponent : public juce::Component {
        public:
            TabLayoutComponent(std::vector<std::pair<juce::Component*, juce::Component*>> items) : items_(std::move(items)) {}
            
            void resized() override {
                auto bounds = getLocalBounds().reduced(kMargin);
                for (auto& item : items_) {
                    auto row = bounds.removeFromTop(kRowHeight);
                    if (item.first) {
                        item.first->setBounds(row.removeFromLeft(150));
                        row.removeFromLeft(10);
                    }
                    if (item.second) {
                        item.second->setBounds(row);
                    }
                    bounds.removeFromTop(5); // spacing
                }
            }
            
        private:
            std::vector<std::pair<juce::Component*, juce::Component*>> items_;
        };
        
        // Replace the tab content with our layout component
        auto* layoutComp = new TabLayoutComponent(std::move(items));
        
        // Transfer child components to layout component
        for (int i = tab->getNumChildComponents() - 1; i >= 0; --i) {
            auto* child = tab->getChildComponent(i);
            tab->removeChildComponent(child);
            layoutComp->addAndMakeVisible(child);
        }
        
        tab->addAndMakeVisible(layoutComp);
    };
    
    layoutTab(audioTab, {
        {&sampleRateLabel_, &sampleRateBox_},
        {&bufferSizeLabel_, &bufferSizeBox_},
        {&performanceModeLabel_, &performanceModeBox_}
    });
    
    layoutTab(recordingTab, {
        {&maxRecordingLabel_, &maxRecordingSlider_},
        {nullptr, &autoSaveToggle_},
        {nullptr, &preprocToggle_}
    });
    
    layoutTab(displayTab, {
        {&languageLabel_, &languageBox_},
        {nullptr, &advancedVisualizationToggle_},
        {nullptr, &spectralAnalysisToggle_},
        {&heatmapResolutionLabel_, &heatmapResolutionBox_}
    });
    
    layoutTab(privacyTab, {
        {nullptr, &privacyInfoLabel_},
        {nullptr, &ramOnlyToggle_},
        {nullptr, &networkingDisabledToggle_}
    });
}

void SettingsDialog::paint(juce::Graphics& g) {
    g.fillAll(juce::Colours::darkgrey);
}

void SettingsDialog::resized() {
    auto bounds = getLocalBounds().reduced(kMargin);
    
    // Title
    titleLabel_.setBounds(bounds.removeFromTop(30));
    bounds.removeFromTop(10);
    
    // Buttons
    auto buttonArea = bounds.removeFromBottom(kButtonHeight);
    buttonArea.removeFromTop(10);
    cancelButton_.setBounds(buttonArea.removeFromRight(80));
    buttonArea.removeFromRight(10);
    okButton_.setBounds(buttonArea.removeFromRight(80));
    bounds.removeFromBottom(10);
    
    // Tabbed component takes remaining space
    tabbedComponent_->setBounds(bounds);
}

void SettingsDialog::buttonClicked(juce::Button* button) {
    if (button == &okButton_) {
        close(true);
    } else if (button == &cancelButton_) {
        close(false);
    }
}

void SettingsDialog::comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) {
    if (comboBoxThatHasChanged == &languageBox_) {
        auto selectedLanguage = static_cast<LocalizationManager::Language>(languageBox_.getSelectedId() - 1);
        workingCopy_.language = selectedLanguage;
        
        // Apply language change immediately for preview
        LocalizationManager::getInstance().setLanguage(selectedLanguage);
        updateUILanguage();
    }
    // Other combo box changes are handled when dialog is accepted
}

void SettingsDialog::updateUILanguage() {
    titleLabel_.setText(TRANS("settings_title"), juce::dontSendNotification);
    okButton_.setButtonText(TRANS("settings_apply"));
    cancelButton_.setButtonText(TRANS("settings_cancel"));
    
    // Update labels
    sampleRateLabel_.setText(TRANS("settings_sample_rate") + ":", juce::dontSendNotification);
    bufferSizeLabel_.setText(TRANS("settings_buffer_size") + ":", juce::dontSendNotification);
    performanceModeLabel_.setText(TRANS("settings_performance_mode") + ":", juce::dontSendNotification);
    maxRecordingLabel_.setText(TRANS("settings_max_duration") + ":", juce::dontSendNotification);
    languageLabel_.setText(TRANS("settings_language") + ":", juce::dontSendNotification);
    
    // Update toggle buttons
    autoSaveToggle_.setButtonText(TRANS("settings_auto_save"));
    preprocToggle_.setButtonText(TRANS("enable_preprocessing"));
    advancedVisualizationToggle_.setButtonText(TRANS("advanced_visualization"));
    spectralAnalysisToggle_.setButtonText(TRANS("spectral_analysis"));
    ramOnlyToggle_.setButtonText(TRANS("privacy_ram_only"));
    networkingDisabledToggle_.setButtonText(TRANS("privacy_no_network"));
    
    // Update tab titles
    if (tabbedComponent_) {
        tabbedComponent_->setTabName(0, TRANS("settings_audio"));
        tabbedComponent_->setTabName(1, TRANS("settings_recording"));
        tabbedComponent_->setTabName(2, TRANS("display_settings"));
        tabbedComponent_->setTabName(3, TRANS("privacy_settings"));
    }
}

void SettingsDialog::applyTo(AppSettings& settings) const {
    settings.sampleRate = sampleRateBox_.getSelectedId();
    settings.bufferSize = bufferSizeBox_.getSelectedId();
    settings.maxRecordingTimeSeconds = maxRecordingSlider_.getValue();
    settings.autoSave = autoSaveToggle_.getToggleState();
    settings.enablePreprocessing = preprocToggle_.getToggleState();
    settings.language = static_cast<LocalizationManager::Language>(languageBox_.getSelectedId() - 1);
    settings.enableAdvancedVisualization = advancedVisualizationToggle_.getToggleState();
    settings.showSpectralAnalysis = spectralAnalysisToggle_.getToggleState();
    settings.heatmapResolution = heatmapResolutionBox_.getSelectedId();
}

void SettingsDialog::close(bool accepted) {
    if (hasClosed_) {
        return;
    }
    hasClosed_ = true;
    
    AppSettings resultSettings = workingCopy_;
    if (accepted) {
        applyTo(resultSettings);
    }
    
    if (onClose_) {
        onClose_(accepted, resultSettings);
    }
    
    if (auto* window = findParentComponentOfClass<juce::DialogWindow>()) {
        window->exitModalState(accepted ? 1 : 0);
    }
}

} // namespace yvc::app
