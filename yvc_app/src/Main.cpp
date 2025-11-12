// VoiVoi GUI Application - Main Entry Point
// License: GPLv3

#include "AppConfig.h"

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <string>

#ifdef Logger
#undef Logger
#endif
#ifdef DEBUG
#undef DEBUG
#endif
#ifdef TRACE
#undef TRACE
#endif

#include "MainComponent.h"
#include "LocalizationManager.h"
#include "yvc_core/MetricsBus.h"
#include "yvc_core/Logger.h"

namespace { // anonymous

class VoiVoiApplication : public juce::JUCEApplication {
public:
    VoiVoiApplication() = default;

    const juce::String getApplicationName() override { return JUCE_APPLICATION_NAME_STRING; }
    const juce::String getApplicationVersion() override { return JUCE_APPLICATION_VERSION_STRING; }
    bool moreThanOneInstanceAllowed() override { return false; }

    void initialise(const juce::String& commandLineParameters) override {
        auto& logger = ::yvc::Logger::getInstance();
        ::yvc::LoggerConfig logConfig;
#ifndef NDEBUG
        logConfig.minLevel = ::yvc::LogLevel::DEBUG;
        logConfig.enableFile = true;
        logConfig.enableConsole = true;
        logConfig.logFilePath = "voivoi_app_debug.log";
        logConfig.maxFileSize = 5 * 1024 * 1024;
        logConfig.includeTimestamp = true;
        logConfig.includeThreadId = true;
        logConfig.includeSourceLocation = true;
#else
        logConfig.minLevel = ::yvc::LogLevel::WARN;
        logConfig.enableFile = false;
        logConfig.enableConsole = false;
#endif
        logger.configure(logConfig);
        logger.log(::yvc::LogLevel::INFO, __FILE__, __LINE__, __func__, "VoiVoi Analyzer application starting");
        {
            std::string msg = std::string("Command line: ") + commandLineParameters.toRawUTF8();
            logger.log(::yvc::LogLevel::DEBUG, __FILE__, __LINE__, __func__, msg);
        }

        ::yvc::app::LocalizationManager::getInstance().loadLanguagePreference();
        logger.log(::yvc::LogLevel::DEBUG, __FILE__, __LINE__, __func__, "Localization system initialized");

        audioDeviceManager_ = std::make_unique<juce::AudioDeviceManager>();
        juce::String audioError = audioDeviceManager_->initialise(1, 0, nullptr, true);
        if (audioError.isNotEmpty()) {
            std::string msg = std::string("Audio device initialization warning: ") + audioError.toRawUTF8();
            logger.log(::yvc::LogLevel::WARN, __FILE__, __LINE__, __func__, msg);
        } else {
            logger.log(::yvc::LogLevel::INFO, __FILE__, __LINE__, __func__, "Audio device initialized successfully");
        }

        metricsBus_ = std::make_unique<::yvc::MetricsBus>();
        logger.log(::yvc::LogLevel::DEBUG, __FILE__, __LINE__, __func__, "Metrics bus created");

        mainWindow_.reset(new MainWindow(getApplicationName(), *metricsBus_, *audioDeviceManager_));
        logger.log(::yvc::LogLevel::INFO, __FILE__, __LINE__, __func__, "Main window created and shown");
    }

    void shutdown() override {
        auto& logger = ::yvc::Logger::getInstance();
        logger.log(::yvc::LogLevel::INFO, __FILE__, __LINE__, __func__, "Application shutting down");
        mainWindow_ = nullptr;
        metricsBus_.reset();
        audioDeviceManager_.reset();
        logger.log(::yvc::LogLevel::INFO, __FILE__, __LINE__, __func__, "Application shutdown complete");
        ::yvc::Logger::getInstance().shutdown();
    }

    void suspended() override { if (audioDeviceManager_) audioDeviceManager_->closeAudioDevice(); }
    void resumed() override { if (audioDeviceManager_) audioDeviceManager_->restartLastAudioDevice(); }
    void systemRequestedQuit() override { quit(); }
    void anotherInstanceStarted(const juce::String&) override { if (mainWindow_) mainWindow_->toFront(true); }

private:
    class MainWindow : public juce::DocumentWindow {
    public:
        MainWindow(juce::String name, ::yvc::MetricsBus& bus, juce::AudioDeviceManager& audioManager)
            : juce::DocumentWindow(name,
                  juce::Desktop::getInstance().getDefaultLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId),
                  juce::DocumentWindow::allButtons),
              audioDeviceManager_(audioManager) {
            setUsingNativeTitleBar(true);
            setResizable(true, true);
            setResizeLimits(::yvc::app::kMinWindowWidth, ::yvc::app::kMinWindowHeight, 2400, 1800);
            setContentOwned(new ::yvc::app::MainComponent(bus), true);
            centreWithSize(::yvc::app::kDefaultWindowWidth, ::yvc::app::kDefaultWindowHeight);
            setVisible(true);
            if (juce::RuntimePermissions::isRequired(juce::RuntimePermissions::recordAudio)) {
                juce::RuntimePermissions::request(juce::RuntimePermissions::recordAudio, [this](bool granted) {
                    if (!granted) {
                        juce::AlertWindow::showMessageBoxAsync(
                            juce::AlertWindow::WarningIcon,
                            "Microphone Permission Required",
                            "VoiVoi Analyzer needs microphone access for voice analysis. Please grant permission in system settings and restart the application.",
                            "OK");
                    }
                });
            }
        }
        ~MainWindow() override { setContentComponent(nullptr); }
        void closeButtonPressed() override { juce::JUCEApplication::getInstance()->systemRequestedQuit(); }
        void moved() override {
            auto& appProps = getApplicationProperties();
            if (auto* s = appProps.getUserSettings()) { s->setValue("windowX", getX()); s->setValue("windowY", getY()); s->saveIfNeeded(); }
        }
        void resized() override {
            DocumentWindow::resized();
            auto& appProps = getApplicationProperties();
            if (auto* s = appProps.getUserSettings()) { s->setValue("windowWidth", getWidth()); s->setValue("windowHeight", getHeight()); s->saveIfNeeded(); }
        }
    private:
        juce::AudioDeviceManager& audioDeviceManager_;
        juce::ApplicationProperties& getApplicationProperties() {
            static juce::ApplicationProperties props; static bool init = false; if (!init) { juce::PropertiesFile::Options o; o.applicationName = JUCE_APPLICATION_NAME_STRING; o.filenameSuffix = ".settings"; o.osxLibrarySubFolder = "Application Support"; o.folderName = "VoiVoi"; props.setStorageParameters(o); init = true; } return props; }
    };

    std::unique_ptr<MainWindow> mainWindow_;
    std::unique_ptr<::yvc::MetricsBus> metricsBus_;
    std::unique_ptr<juce::AudioDeviceManager> audioDeviceManager_;
};

} // namespace

START_JUCE_APPLICATION(VoiVoiApplication)
