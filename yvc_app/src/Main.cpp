// VoiVoi GUI Application - Main Entry Point
// License: GPLv3

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>

#include "MainComponent.h"
#include "yvc_core/MetricsBus.h"

namespace {

class VoiVoiApplication : public juce::JUCEApplication {
public:
    VoiVoiApplication() = default;

    const juce::String getApplicationName() override { return "VoiVoi Analyzer"; }
    const juce::String getApplicationVersion() override { return "0.1.0"; }

    void initialise(const juce::String&) override {
        metricsBus_ = std::make_unique<yvc::MetricsBus>();
        mainWindow_.reset(new MainWindow(getApplicationName(), *metricsBus_));
    }

    void shutdown() override {
        mainWindow_ = nullptr;
        metricsBus_.reset();
    }

private:
    class MainWindow : public juce::DocumentWindow {
    public:
        MainWindow(juce::String name, yvc::MetricsBus& bus)
            : juce::DocumentWindow(name,
                                   juce::Desktop::getInstance().getDefaultLookAndFeel()
                                       .findColour(juce::ResizableWindow::backgroundColourId),
                                   juce::DocumentWindow::allButtons) {
            setUsingNativeTitleBar(true);
            setResizable(true, true);
            setContentOwned(new yvc::app::MainComponent(bus), true);
            centreWithSize(1200, 720);
            setVisible(true);
        }

        void closeButtonPressed() override {
            juce::JUCEApplication::getInstance()->systemRequestedQuit();
        }
    };

    std::unique_ptr<MainWindow> mainWindow_;
    std::unique_ptr<yvc::MetricsBus> metricsBus_;
};

} // namespace

START_JUCE_APPLICATION(VoiVoiApplication)
