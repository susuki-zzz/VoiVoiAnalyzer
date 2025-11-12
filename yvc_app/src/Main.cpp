// VoiVoi GUI Application - Main Entry Point
// License: GPLv3

#include "AppConfig.h"

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_devices/juce_audio_devices.h>

#include "MainComponent.h"
#include "LocalizationManager.h"
#include "yvc_core/MetricsBus.h"

namespace {

class VoiVoiApplication : public juce::JUCEApplication {
public:
    VoiVoiApplication() = default;

    const juce::String getApplicationName() override { 
        return JUCE_APPLICATION_NAME_STRING; 
    }
    
    const juce::String getApplicationVersion() override { 
        return JUCE_APPLICATION_VERSION_STRING; 
    }
    
    bool moreThanOneInstanceAllowed() override { 
        return false; // Single instance application
    }

    void initialise(const juce::String& commandLineParameters) override {
        // Initialize localization system
        yvc::app::LocalizationManager::getInstance().loadLanguagePreference();
        
        // Initialize audio device manager
        audioDeviceManager_ = std::make_unique<juce::AudioDeviceManager>();
        
        // Setup default audio configuration
        juce::String audioError = audioDeviceManager_->initialise(
            1,    // numInputChannelsNeeded
            0,    // numOutputChannelsNeeded 
            nullptr, // savedState
            true  // selectDefaultDeviceOnFailure
        );
        
        if (audioError.isNotEmpty()) {
            // Log audio initialization warning but continue
            DBG("Audio device initialization warning: " + audioError);
        }
        
        // Initialize metrics bus
        metricsBus_ = std::make_unique<yvc::MetricsBus>();
        
        // Create main window
        mainWindow_.reset(new MainWindow(getApplicationName(), *metricsBus_, *audioDeviceManager_));
    }

    void shutdown() override {
        mainWindow_ = nullptr;
        metricsBus_.reset();
        audioDeviceManager_.reset();
    }
    
    void suspended() override {
        // Handle application suspension (mobile platforms)
        if (audioDeviceManager_) {
            audioDeviceManager_->closeAudioDevice();
        }
    }
    
    void resumed() override {
        // Handle application resume (mobile platforms)
        if (audioDeviceManager_) {
            audioDeviceManager_->restartLastAudioDevice();
        }
    }
    
    void systemRequestedQuit() override {
        quit();
    }
    
    void anotherInstanceStarted(const juce::String& commandLine) override {
        // Bring existing instance to front
        if (mainWindow_) {
            mainWindow_->toFront(true);
        }
    }

private:
    class MainWindow : public juce::DocumentWindow {
    public:
        MainWindow(juce::String name, yvc::MetricsBus& bus, juce::AudioDeviceManager& audioManager)
            : juce::DocumentWindow(name,
                                   juce::Desktop::getInstance().getDefaultLookAndFeel()
                                       .findColour(juce::ResizableWindow::backgroundColourId),
                                   juce::DocumentWindow::allButtons),
              audioDeviceManager_(audioManager) {
            
            setUsingNativeTitleBar(true);
            setResizable(true, true);
            
            // Set window constraints
            setResizeLimits(yvc::app::kMinWindowWidth, yvc::app::kMinWindowHeight, 
                          2400, 1800);
            
            // Create main component
            setContentOwned(new yvc::app::MainComponent(bus), true);
            
            // Center and show window
            centreWithSize(yvc::app::kDefaultWindowWidth, yvc::app::kDefaultWindowHeight);
            setVisible(true);
            
            // Request focus for audio permissions if needed
            if (juce::RuntimePermissions::isRequired(juce::RuntimePermissions::recordAudio)) {
                juce::RuntimePermissions::request(
                    juce::RuntimePermissions::recordAudio,
                    [this](bool granted) {
                        if (!granted) {
                            juce::AlertWindow::showMessageBoxAsync(
                                juce::AlertWindow::WarningIcon,
                                "Microphone Permission Required",
                                "VoiVoi Analyzer needs microphone access for voice analysis. "
                                "Please grant permission in system settings and restart the application.",
                                "OK"
                            );
                        }
                    }
                );
            }
        }
        
        ~MainWindow() override {
            // Ensure content component is destroyed first
            setContentComponent(nullptr);
        }

        void closeButtonPressed() override {
            juce::JUCEApplication::getInstance()->systemRequestedQuit();
        }
        
        void moved() override {
            // Save window position preference
            auto& properties = getApplicationProperties();
            if (auto* userSettings = properties.getUserSettings()) {
                userSettings->setValue("windowX", getX());
                userSettings->setValue("windowY", getY());
                userSettings->saveIfNeeded();
            }
        }
        
        void resized() override {
            DocumentWindow::resized();
            
            // Save window size preference
            auto& properties = getApplicationProperties();
            if (auto* userSettings = properties.getUserSettings()) {
                userSettings->setValue("windowWidth", getWidth());
                userSettings->setValue("windowHeight", getHeight());
                userSettings->saveIfNeeded();
            }
        }
        
    private:
        juce::AudioDeviceManager& audioDeviceManager_;
        
        juce::ApplicationProperties& getApplicationProperties() {
            static juce::ApplicationProperties appProperties;
            static bool initialized = false;
            
            if (!initialized) {
                juce::PropertiesFile::Options options;
                options.applicationName = JUCE_APPLICATION_NAME_STRING;
                options.filenameSuffix = ".settings";
                options.osxLibrarySubFolder = "Application Support";
                options.folderName = "VoiVoi";
                
                appProperties.setStorageParameters(options);
                initialized = true;
            }
            
            return appProperties;
        }
    };

    std::unique_ptr<MainWindow> mainWindow_;
    std::unique_ptr<yvc::MetricsBus> metricsBus_;
    std::unique_ptr<juce::AudioDeviceManager> audioDeviceManager_;
};

} // namespace

START_JUCE_APPLICATION(VoiVoiApplication)
