// VoiVoi GUI Application - Settings Component
// License: GPLv3

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <yvc_core/AudioBackend.h>
#include <yvc_core/Types.h>
#include <memory>
#include <vector>

namespace yvc::app {

    /// <summary>
    /// UI component for editing application audio and visualization settings.
    /// </summary>
    class SettingsComponent : public juce::Component {
    public:
        SettingsComponent();
        ~SettingsComponent() override;

        void paint(juce::Graphics& g) override;
        void resized() override;

        /// <summary>
        /// Aggregate settings snapshot.
        /// </summary>
        struct Settings {
            std::string inputDeviceId;
            std::string inputDeviceName;
            yvc::SampleRate sampleRate;
            size_t bufferSize;
            yvc::PerformanceMode performanceMode;
            int liveMaxMinutes;
            bool autoSaveEnabled;
            bool autoSaveOnStop;
            bool autoSaveOnLimit;
        };

        /// <summary>
        /// Gets current settings.
        /// </summary>
        Settings getSettings() const;

        /// <summary>
        /// Applies provided settings to the UI.
        /// </summary>
        void setSettings(const Settings& settings);

        /// <summary>
        /// Invoked when settings change.
        /// </summary>
        std::function<void(const Settings&)> onSettingsChanged;

    private:
        void updateDeviceList();
        void updateSampleRateOptions();
        void applySettings();

        // Audio backend
        std::unique_ptr<yvc::audio::IAudioBackend> audioBackend_;
        std::vector<yvc::audio::AudioDeviceInfo> availableDevices_;

        // UI Components
        juce::Label deviceLabel_;
        juce::ComboBox deviceComboBox_;
        juce::Label deviceInfoLabel_;

        juce::Label sampleRateLabel_;
        juce::ComboBox sampleRateComboBox_;
        juce::Label sampleRateInfoLabel_;

        juce::Label bufferSizeLabel_;
        juce::Label bufferSizeValueLabel_;
        juce::Label bufferSizeInfoLabel_;

        juce::Label performanceModeLabel_;
        juce::ComboBox performanceModeComboBox_;
        juce::TextEditor performanceModeDescEditor_;

        juce::Label liveMaxLabel_;
        juce::Slider liveMaxSlider_;
        juce::Label liveMaxValueLabel_;

        juce::Label autoSaveLabel_;
        juce::ToggleButton autoSaveToggle_;
        juce::ToggleButton autoSaveOnStopToggle_;
        juce::ToggleButton autoSaveOnLimitToggle_;

        juce::TextButton applyButton_;
        juce::TextButton cancelButton_;
        juce::TextButton defaultsButton_;

        Settings currentSettings_;
        Settings originalSettings_;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsComponent)
    };

    /// <summary>
    /// Window container for the SettingsComponent.
    /// </summary>
    class SettingsWindow : public juce::DocumentWindow {
    public:
        SettingsWindow();
        ~SettingsWindow() override;

        void closeButtonPressed() override;

        /// <summary>
        /// Gets underlying SettingsComponent pointer.
        /// </summary>
        SettingsComponent* getSettingsComponent();

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsWindow)
    };

} // namespace yvc::app
