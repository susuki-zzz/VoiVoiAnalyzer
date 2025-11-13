#include <juce_gui_basics/juce_gui_basics.h>

#include "VisualizationComponents.h"
#include "yvc_core/Types.h"

#include <algorithm>
#include <cmath>
#include <memory>
#include <vector>

namespace yvc::app::test {
namespace {

class ScopedJuceRuntime {
public:
    ScopedJuceRuntime() : init_(std::make_unique<juce::ScopedJuceInitialiser_GUI>()) {}
    ~ScopedJuceRuntime() = default;

private:
    std::unique_ptr<juce::ScopedJuceInitialiser_GUI> init_;
};

class AdvancedHeatmapComponentTestPeer : public AdvancedHeatmapComponent {
public:
    using AdvancedHeatmapComponent::AdvancedHeatmapComponent;
    using AdvancedHeatmapComponent::appendSample;
    using AdvancedHeatmapComponent::setConfig;
    using AdvancedHeatmapComponent::setZoomRange;

    [[nodiscard]] size_t getSampleCount() const { return samples_.size(); }
    [[nodiscard]] double getZoomStart() const { return zoomStart_; }
    [[nodiscard]] double getZoomEnd() const { return zoomEnd_; }
    [[nodiscard]] bool isHighResolutionEnabled() const { return highResolution_; }
};

class ComparativeMetricsComponentTestPeer : public ComparativeMetricsComponent {
public:
    using ComparativeMetricsComponent::ComparativeMetricsComponent;
    using ComparativeMetricsComponent::addSession;
    using ComparativeMetricsComponent::clearAllSessions;
    using ComparativeMetricsComponent::removeSession;
    using ComparativeMetricsComponent::setSessionAlpha;
    using ComparativeMetricsComponent::setSessionVisibility;
    using ComparativeMetricsComponent::setTimeRange;

    [[nodiscard]] const std::vector<SessionData>& getSessions() const { return sessions_; }
    [[nodiscard]] const juce::String& getCurrentMetric() const { return currentMetric_; }
    [[nodiscard]] double getTimeStart() const { return timeStart_; }
    [[nodiscard]] double getTimeEnd() const { return timeEnd_; }
    [[nodiscard]] std::pair<float, float> getRangeFor(const juce::String& metric) { return getMetricRange(metric); }
    [[nodiscard]] juce::String getUnitFor(const juce::String& metric) { return getMetricUnit(metric); }
};

class SpectrumAnalyzerComponentTestPeer : public SpectrumAnalyzerComponent {
public:
    using SpectrumAnalyzerComponent::SpectrumAnalyzerComponent;
    using SpectrumAnalyzerComponent::setFFTSize;
    using SpectrumAnalyzerComponent::setHarmonicsOverlayEnabled;
    using SpectrumAnalyzerComponent::setPeakHoldEnabled;
    using SpectrumAnalyzerComponent::setSampleRate;
    using SpectrumAnalyzerComponent::updateF0;
    using SpectrumAnalyzerComponent::updateSpectrum;

    [[nodiscard]] const std::vector<float>& getCurrentSpectrum() const { return currentSpectrum_; }
    [[nodiscard]] const std::vector<float>& getPeakSpectrum() const { return peakSpectrum_; }
    [[nodiscard]] float getCurrentF0() const { return currentF0_; }
    [[nodiscard]] bool isF0Valid() const { return f0Valid_; }
    [[nodiscard]] int getFFTSize() const { return fftSize_; }
    [[nodiscard]] bool isHarmonicsOverlayEnabled() const { return harmonicsOverlayEnabled_; }
    [[nodiscard]] float mapFrequencyToX(float freq, juce::Rectangle<int> area) { return frequencyToX(freq, area); }
    [[nodiscard]] int mapFrequencyToBin(float freq) { return frequencyToBin(freq); }
    [[nodiscard]] float mapMagnitudeToY(float magnitude, juce::Rectangle<int> area) { return magnitudeToY(magnitude, area); }
};

class HeatmapScenarioTest : public juce::UnitTest {
public:
    HeatmapScenarioTest() : juce::UnitTest("AdvancedHeatmapComponent Scenarios", "Visualization") {}

    void runTest() override {
        ScopedJuceRuntime runtime;

        beginTest("Sample history respects configured maximum and updates zoom");
        {
            AdvancedHeatmapComponentTestPeer heatmap("F0 History");
            heatmap.setBounds(0, 0, 640, 320);

            AdvancedHeatmapComponent::HeatmapConfig config;
            config.maxSamples = 5;
            heatmap.setConfig(config);
            heatmap.setZoomRange(0.0, 2.0);

            int zoomCallbackCount = 0;
            double lastStart = 0.0;
            double lastEnd = 0.0;
            heatmap.onZoomRangeChanged = [&](double start, double end) {
                ++zoomCallbackCount;
                lastStart = start;
                lastEnd = end;
            };

            for (int i = 0; i < 8; ++i) {
                heatmap.appendSample(static_cast<float>(i) * 0.1f, static_cast<double>(i));
            }

            expectEquals(static_cast<int>(heatmap.getSampleCount()), config.maxSamples,
                         "Heatmap should keep only the most recent samples");
            expectGreaterThan(zoomCallbackCount, 0, "Zoom callback should fire when new data exceeds range");
            expect(lastEnd >= 7.0, "Zoom end should follow the latest timestamp");
            expect(lastStart < lastEnd, "Zoom start must remain before zoom end");
        }

        beginTest("Resolution toggle is reflected in component state");
        {
            AdvancedHeatmapComponentTestPeer heatmap("CPP Heatmap");
            heatmap.setResolutionMode(true);
            expect(heatmap.isHighResolutionEnabled(), "High resolution mode should be enabled");
            heatmap.setResolutionMode(false);
            expect(!heatmap.isHighResolutionEnabled(), "High resolution mode should be disabled");
        }

        beginTest("Export image generates expected dimensions");
        {
            AdvancedHeatmapComponentTestPeer heatmap("Export");
            AdvancedHeatmapComponent::HeatmapConfig config;
            config.maxSamples = 2;
            heatmap.setConfig(config);
            heatmap.appendSample(0.2f, 0.0);
            heatmap.appendSample(0.4f, 0.5);

            constexpr int width = 320;
            constexpr int height = 180;
            auto image = heatmap.createExportImage(width, height, true);
            expectEquals(image.getWidth(), width, "Export width should match request");
            expectEquals(image.getHeight(), height, "Export height should match request");
        }

        beginTest("PNG export writes to disk when target is writable");
        {
            AdvancedHeatmapComponentTestPeer heatmap("DiskExport");
            heatmap.appendSample(0.25f, 0.0);
            heatmap.appendSample(0.5f, 0.5);

            juce::TemporaryFile temp;
            auto targetFile = temp.getFile();
            targetFile.deleteFile();

            const bool exported = heatmap.exportToPNG(targetFile, 120, 80);
            expect(exported, "Export should succeed when file is writable");
            expect(targetFile.existsAsFile(), "PNG file should exist after export");

            targetFile.deleteFile();
        }
    }
};

class ComparativeMetricsScenarioTest : public juce::UnitTest {
public:
    ComparativeMetricsScenarioTest()
        : juce::UnitTest("ComparativeMetricsComponent Scenarios", "Visualization") {}

    void runTest() override {
        ScopedJuceRuntime runtime;

        beginTest("Session management maintains visibility and alpha state");
        {
            ComparativeMetricsComponentTestPeer component;

            yvc::AnalysisResults baseline{};
            baseline.f0 = 210.0f;
            baseline.cpp = -12.0f;
            baseline.hnr = 12.0f;

            component.addSession("baseline", {baseline}, juce::Colours::red);
            component.addSession("reference", {baseline}, juce::Colours::blue);
            expectEquals(static_cast<int>(component.getSessions().size()), 2,
                         "Two sessions should be registered");

            component.setSessionVisibility("baseline", false);
            component.setSessionAlpha("reference", 0.4f);

            const auto& sessions = component.getSessions();
            auto baselineIt = std::find_if(sessions.begin(), sessions.end(), [](const auto& session) {
                return session.name == "baseline";
            });
            auto referenceIt = std::find_if(sessions.begin(), sessions.end(), [](const auto& session) {
                return session.name == "reference";
            });

            expect(baselineIt != sessions.end(), "Baseline session should exist");
            expect(referenceIt != sessions.end(), "Reference session should exist");
            if (baselineIt != sessions.end()) {
                expect(!baselineIt->visible, "Baseline session should be hidden");
            }
            if (referenceIt != sessions.end()) {
                expectWithinAbsoluteError(referenceIt->alpha, 0.4f, 1e-3f,
                                          "Reference alpha should be updated");
            }

            component.removeSession("baseline");
            expectEquals(static_cast<int>(component.getSessions().size()), 1,
                         "Removing a session should decrease the total");

            component.clearAllSessions();
            expect(component.getSessions().empty(), "All sessions should be cleared");
        }

        beginTest("Metric and time range configuration is retained");
        {
            ComparativeMetricsComponentTestPeer component;
            component.setDisplayedMetric("hnr");
            component.setTimeRange(5.0, 25.0);

            expectEquals(component.getCurrentMetric(), juce::String("hnr"),
                         "Metric selection should be stored");
            expectWithinAbsoluteError(component.getTimeStart(), 5.0, 1e-6,
                                      "Time range start should match request");
            expectWithinAbsoluteError(component.getTimeEnd(), 25.0, 1e-6,
                                      "Time range end should match request");
        }

        beginTest("Metric metadata helpers expose expected ranges and units");
        {
            ComparativeMetricsComponentTestPeer component;

            const auto cppRange = component.getRangeFor("cpp");
            expectWithinAbsoluteError(cppRange.first, -10.0f, 1e-3f, "CPP lower bound should match design spec");
            expectWithinAbsoluteError(cppRange.second, 30.0f, 1e-3f, "CPP upper bound should match design spec");

            const auto defaultRange = component.getRangeFor("unknown");
            expectWithinAbsoluteError(defaultRange.first, 0.0f, 1e-3f, "Unknown metric lower bound should default to 0");
            expectWithinAbsoluteError(defaultRange.second, 100.0f, 1e-3f, "Unknown metric upper bound should default to 100");

            const auto unit = component.getUnitFor("spectral_tilt");
            expectEquals(unit, juce::String("dB/oct"), "Spectral tilt unit should be displayed in dB/oct");
        }
    }
};

class SpectrumAnalyzerScenarioTest : public juce::UnitTest {
public:
    SpectrumAnalyzerScenarioTest()
        : juce::UnitTest("SpectrumAnalyzerComponent Scenarios", "Visualization") {}

    void runTest() override {
        ScopedJuceRuntime runtime;

        beginTest("Spectrum updates propagate to peak hold buffer");
        {
            SpectrumAnalyzerComponentTestPeer component;
            component.setFFTSize(1024);
            component.setSampleRate(48000.0f);
            component.setPeakHoldEnabled(true);

            std::vector<float> spectrum(512, -100.0f);
            spectrum[10] = -20.0f;
            spectrum[20] = -30.0f;

            component.updateSpectrum(spectrum.data(), static_cast<int>(spectrum.size()));

            expectEquals(static_cast<int>(component.getCurrentSpectrum().size()),
                         static_cast<int>(spectrum.size()), "Spectrum size should match input");
            expectWithinAbsoluteError(component.getPeakSpectrum()[10], -20.0f, 1e-3f,
                                      "Peak hold should capture the loudest bin");
        }

        beginTest("Fundamental frequency updates are tracked");
        {
            SpectrumAnalyzerComponentTestPeer component;
            component.updateF0(220.0f, true);
            expectWithinAbsoluteError(component.getCurrentF0(), 220.0f, 1e-3f,
                                      "F0 should be stored");
            expect(component.isF0Valid(), "F0 should be marked valid");

            component.updateF0(0.0f, false);
            expect(!component.isF0Valid(), "Invalid F0 should clear validity flag");
        }

        beginTest("Harmonics overlay toggle is stateful");
        {
            SpectrumAnalyzerComponentTestPeer component;
            component.setHarmonicsOverlayEnabled(true);
            expect(component.isHarmonicsOverlayEnabled(), "Harmonics overlay should be enabled");
            component.setHarmonicsOverlayEnabled(false);
            expect(!component.isHarmonicsOverlayEnabled(), "Harmonics overlay should be disabled");
        }

        beginTest("Frequency range adjustments update coordinate mapping");
        {
            SpectrumAnalyzerComponentTestPeer component;
            component.setBounds(0, 0, 800, 400);
            component.setFrequencyRange(80.0f, 12000.0f);

            juce::Rectangle<int> area(50, 0, 600, 300);
            const float lowX = component.mapFrequencyToX(80.0f, area);
            const float highX = component.mapFrequencyToX(12000.0f, area);

            expect(lowX >= area.getX(), "Low frequency should map inside axis bounds");
            expect(highX <= area.getRight(), "High frequency should map inside axis bounds");
            expect(lowX < highX, "Frequency mapping should be monotonic");

            const int bin = component.mapFrequencyToBin(1000.0f);
            expect(bin >= 0, "Frequency bin should be non-negative");
            expect(bin < component.getFFTSize() / 2 + 1, "Frequency bin should fall within spectrum size");
        }

        beginTest("Disabling peak hold clears accumulated spectrum");
        {
            SpectrumAnalyzerComponentTestPeer component;
            component.setFFTSize(1024);
            component.setPeakHoldEnabled(true);

            std::vector<float> spectrum(513, -90.0f);
            spectrum[32] = -10.0f;
            component.updateSpectrum(spectrum.data(), static_cast<int>(spectrum.size()));

            component.setPeakHoldEnabled(false);
            bool allCleared = std::all_of(component.getPeakSpectrum().begin(), component.getPeakSpectrum().end(),
                                          [](float value) { return std::abs(value) < 1e-5f; });
            expect(allCleared, "Disabling peak hold should reset stored spectrum");
        }
    }
};

static HeatmapScenarioTest heatmapScenarioTest;
static ComparativeMetricsScenarioTest comparativeMetricsScenarioTest;
static SpectrumAnalyzerScenarioTest spectrumAnalyzerScenarioTest;

} // namespace
} // namespace yvc::app::test

int main() {
    juce::UnitTestRunner runner;
    runner.runAllTests();
    return runner.getNumFailures() == 0 ? 0 : 1;
}
