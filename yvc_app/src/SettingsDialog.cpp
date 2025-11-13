// VoiVoi GUI Application - Settings Dialog Implementation
// License: GPLv3

#include "SettingsDialog.h"
#include "LocalizationManager.h"
#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_devices/juce_audio_devices.h>

namespace yvc::app {

namespace {
constexpr int kDialogWidth = 500;
constexpr int kDialogHeight = 400;
constexpr int kButtonHeight = 32;
constexpr int kRowHeight = 30;
constexpr int kMargin = 12;
}

void SettingsDialog::showDialog(const AppSettings& currentSettings, juce::Component* parent, juce::AudioDeviceManager& audioDeviceManager, OnClose onClose) {
    auto* dialog = new SettingsDialog(currentSettings, audioDeviceManager, std::move(onClose));
    
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

SettingsDialog::SettingsDialog(const AppSettings& currentSettings, juce::AudioDeviceManager& audioDeviceManager, OnClose onClose)
    : workingCopy_(currentSettings)
    , onClose_(std::move(onClose))
    , audioDeviceManager_(&audioDeviceManager)
    , titleLabel_("", TRANS("settings_title"))
    , okButton_(TRANS("settings_apply"))
    , cancelButton_(TRANS("settings_cancel"))
    , inputDeviceLabel_("", TRANS("settings_input_device") + ":")
    , sampleRateLabel_("", TRANS("settings_sample_rate") + ":")
    , bufferSizeLabel_("", TRANS("settings_buffer_size") + ":")
    , performanceModeLabel_("", TRANS("settings_performance_mode") + ":")
    , maxRecordingLabel_("", TRANS("settings_max_duration") + ":")
    , languageLabel_("", TRANS("settings_language") + ":")
    , heatmapResolutionLabel_("", TRANS("heatmap_resolution") + ":")
    , privacyInfoLabel_("", TRANS("privacy_local_processing")) {
    
    // Create tabs before any layout/resized() calls that rely on them.
    createTabbedInterface();
    updateUILanguage();

    // Device list
    inputDeviceBox_.addListener(this);
    populateAudioDeviceList();

    // Sample rate / buffer sizes
    populateSampleRateAndBufferBoxes();

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
    
    // Now safe to trigger resized() logic.
    setSize(kDialogWidth, kDialogHeight);
}

SettingsDialog::~SettingsDialog() {
    okButton_.removeListener(this);
    cancelButton_.removeListener(this);
    inputDeviceBox_.removeListener(this);
    sampleRateBox_.removeListener(this);
    bufferSizeBox_.removeListener(this);
    performanceModeBox_.removeListener(this);
    languageBox_.removeListener(this);
    heatmapResolutionBox_.removeListener(this);

    // Fallback: if the window was closed via title bar/ESC without pressing OK/Cancel,
    // ensure the callback is still fired with accepted = false to keep app logic consistent.
    if (!hasClosed_ && onClose_) {
        hasClosed_ = true;
        onClose_(false, workingCopy_);
    }
}

void SettingsDialog::createTabbedInterface() {
    tabbedComponent_ = std::make_unique<juce::TabbedComponent>(juce::TabbedButtonBar::TabsAtTop);
    
    // Audio Settings Tab
    auto* audioTab = new juce::Component();
    audioTab->addAndMakeVisible(inputDeviceLabel_);
    audioTab->addAndMakeVisible(inputDeviceBox_);
    audioTab->addAndMakeVisible(sampleRateLabel_);
    audioTab->addAndMakeVisible(sampleRateBox_);
    audioTab->addAndMakeVisible(bufferSizeLabel_);
    audioTab->addAndMakeVisible(bufferSizeBox_);
    audioTab->addAndMakeVisible(performanceModeLabel_);
    audioTab->addAndMakeVisible(performanceModeBox_);
    tabbedComponent_->addTab(TRANS("settings_audio"), juce::Colours::darkgrey, audioTab, true);

    // Build layout rows for Audio tab
    audioTabItems_.clear();
    audioTabItems_.push_back({ &inputDeviceLabel_, &inputDeviceBox_ });
    audioTabItems_.push_back({ &sampleRateLabel_, &sampleRateBox_ });
    audioTabItems_.push_back({ &bufferSizeLabel_, &bufferSizeBox_ });
    audioTabItems_.push_back({ &performanceModeLabel_, &performanceModeBox_ });
    
    // Recording Settings Tab
    auto* recordingTab = new juce::Component();
    recordingTab->addAndMakeVisible(maxRecordingLabel_);
    recordingTab->addAndMakeVisible(maxRecordingSlider_);
    recordingTab->addAndMakeVisible(autoSaveToggle_);
    recordingTab->addAndMakeVisible(preprocToggle_);
    tabbedComponent_->addTab(TRANS("settings_recording"), juce::Colours::darkgrey, recordingTab, true);

    recordingTabItems_.clear();
    recordingTabItems_.push_back({ &maxRecordingLabel_, &maxRecordingSlider_ });
    recordingTabItems_.push_back({ nullptr, &autoSaveToggle_ });
    recordingTabItems_.push_back({ nullptr, &preprocToggle_ });
    
    // Display Settings Tab
    auto* displayTab = new juce::Component();
    displayTab->addAndMakeVisible(languageLabel_);
    displayTab->addAndMakeVisible(languageBox_);
    displayTab->addAndMakeVisible(advancedVisualizationToggle_);
    displayTab->addAndMakeVisible(spectralAnalysisToggle_);
    displayTab->addAndMakeVisible(heatmapResolutionLabel_);
    displayTab->addAndMakeVisible(heatmapResolutionBox_);
    tabbedComponent_->addTab(TRANS("display_settings"), juce::Colours::darkgrey, displayTab, true);

    displayTabItems_.clear();
    displayTabItems_.push_back({ &languageLabel_, &languageBox_ });
    displayTabItems_.push_back({ nullptr, &advancedVisualizationToggle_ });
    displayTabItems_.push_back({ nullptr, &spectralAnalysisToggle_ });
    displayTabItems_.push_back({ &heatmapResolutionLabel_, &heatmapResolutionBox_ });
    
    // Privacy Settings Tab
    auto* privacyTab = new juce::Component();
    privacyTab->addAndMakeVisible(privacyInfoLabel_);
    privacyTab->addAndMakeVisible(ramOnlyToggle_);
    privacyTab->addAndMakeVisible(networkingDisabledToggle_);
    tabbedComponent_->addTab(TRANS("privacy_settings"), juce::Colours::darkgrey, privacyTab, true);

    privacyTabItems_.clear();
    privacyTabItems_.push_back({ nullptr, &privacyInfoLabel_ });
    privacyTabItems_.push_back({ nullptr, &ramOnlyToggle_ });
    privacyTabItems_.push_back({ nullptr, &networkingDisabledToggle_ });
}

void SettingsDialog::paint(juce::Graphics& g) {
    g.fillAll(juce::Colours::darkgrey);
}

void SettingsDialog::resized() {
    if (!tabbedComponent_) return;
    
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
    
    // Layout tab contents
    if (auto* audioTab = tabbedComponent_->getTabContentComponent(0)) {
        layoutTabItems(audioTab, audioTabItems_);
    }
    if (auto* recordingTab = tabbedComponent_->getTabContentComponent(1)) {
        layoutTabItems(recordingTab, recordingTabItems_);
    }
    if (auto* displayTab = tabbedComponent_->getTabContentComponent(2)) {
        layoutTabItems(displayTab, displayTabItems_);
    }
    if (auto* privacyTab = tabbedComponent_->getTabContentComponent(3)) {
        layoutTabItems(privacyTab, privacyTabItems_);
    }
}

void SettingsDialog::layoutTabItems(juce::Component* tab, const std::vector<std::pair<juce::Component*, juce::Component*>>& items) {
    if (!tab) return;
    
    auto bounds = tab->getLocalBounds().reduced(kMargin);
    
    for (const auto& item : items) {
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
        LocalizationManager::getInstance().setLanguage(selectedLanguage);
        updateUILanguage();
    } else if (comboBoxThatHasChanged == &inputDeviceBox_) {
        workingCopy_.inputDeviceName = inputDeviceBox_.getText();
        // デバイスに応じた選択肢更新（簡易: 現状は固定候補のまま）
        // populateSampleRateAndBufferBoxes(); // 必要なら有効化
    }
    // 他は OK 時に反映
}

void SettingsDialog::updateUILanguage() {
    titleLabel_.setText(TRANS("settings_title"), juce::dontSendNotification);
    okButton_.setButtonText(TRANS("settings_apply"));
    cancelButton_.setButtonText(TRANS("settings_cancel"));
    
    // Update labels
    inputDeviceLabel_.setText(TRANS("settings_input_device") + ":", juce::dontSendNotification);
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

void SettingsDialog::populateAudioDeviceList() {
    inputDeviceBox_.clear(juce::dontSendNotification);
    if (!audioDeviceManager_) return;

    // 現在デバイスタイプの入力デバイス一覧
    if (auto* typeObj = audioDeviceManager_->getCurrentDeviceTypeObject()) {
        typeObj->scanForDevices();
        auto names = typeObj->getDeviceNames(true); // input devices
        for (int i = 0; i < names.size(); ++i) {
            inputDeviceBox_.addItem(names[i], i + 1);
        }
    }

    // 現在のデバイス名を選択
    juce::AudioDeviceManager::AudioDeviceSetup setup;
    audioDeviceManager_->getAudioDeviceSetup(setup);
    juce::String want = workingCopy_.inputDeviceName.isNotEmpty() ? workingCopy_.inputDeviceName : setup.inputDeviceName;

    if (want.isNotEmpty()) {
        inputDeviceBox_.setText(want, juce::dontSendNotification);
    } else if (inputDeviceBox_.getNumItems() > 0) {
        inputDeviceBox_.setSelectedItemIndex(0, juce::dontSendNotification);
    }
}

void SettingsDialog::populateSampleRateAndBufferBoxes() {
    // サンプルレート
    sampleRateBox_.clear(juce::dontSendNotification);
    if (audioDeviceManager_ != nullptr) {
        if (auto* dev = audioDeviceManager_->getCurrentAudioDevice()) {
            auto srs = dev->getAvailableSampleRates();
            for (auto sr : srs) {
                sampleRateBox_.addItem(juce::String(sr, 0) + " Hz", static_cast<int>(sr));
            }
        }
    }
    // デバイスから取得できない場合のフォールバック
    if (sampleRateBox_.getNumItems() == 0) {
        sampleRateBox_.addItem("44100 Hz", 44100);
        sampleRateBox_.addItem("48000 Hz", 48000);
        sampleRateBox_.addItem("88200 Hz", 88200);
        sampleRateBox_.addItem("96000 Hz", 96000);
    }
    sampleRateBox_.setSelectedId(workingCopy_.sampleRate, juce::dontSendNotification);
    sampleRateBox_.addListener(this);

    // バッファサイズ
    bufferSizeBox_.clear(juce::dontSendNotification);
    if (audioDeviceManager_ != nullptr) {
        if (auto* dev = audioDeviceManager_->getCurrentAudioDevice()) {
            auto sizes = dev->getAvailableBufferSizes();
            for (auto sz : sizes) {
                bufferSizeBox_.addItem(juce::String(sz), sz);
            }
        }
    }
    if (bufferSizeBox_.getNumItems() == 0) {
        bufferSizeBox_.addItem("128", 128);
        bufferSizeBox_.addItem("256", 256);
        bufferSizeBox_.addItem("512", 512);
        bufferSizeBox_.addItem("1024", 1024);
    }
    bufferSizeBox_.setSelectedId(workingCopy_.bufferSize, juce::dontSendNotification);
    bufferSizeBox_.addListener(this);
}

void SettingsDialog::applyTo(AppSettings& settings) const {
    settings.inputDeviceName = inputDeviceBox_.getText();
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
