// VoiVoi GUI Application - JUCE Audio Bridge
// License: GPLv3
// Purpose: Bridge between JUCE AudioDeviceManager and yvc_core analysis engine

#pragma once

#include <juce_audio_devices/juce_audio_devices.h>
#include <yvc_core/AnalyzerEngine.h>
#include <yvc_core/MetricsBus.h>
#include <yvc_core/Types.h>
#include <memory>
#include <atomic>
#include <vector>

namespace yvc::app {

/// JUCE AudioIODeviceCallback implementation that feeds audio to AnalyzerEngine
class JuceAudioBridge : public juce::AudioIODeviceCallback {
public:
    JuceAudioBridge(yvc::MetricsBus& metricsBus, const yvc::AudioConfig& config);
    ~JuceAudioBridge() override;

    // AudioIODeviceCallback interface
    void audioDeviceIOCallbackWithContext(
        const float* const* inputChannelData,
        int numInputChannels,
        float* const* outputChannelData,
        int numOutputChannels,
        int numSamples,
        const juce::AudioIODeviceCallbackContext& context) override;

    void audioDeviceAboutToStart(juce::AudioIODevice* device) override;
    void audioDeviceStopped() override;
    void audioDeviceError(const juce::String& errorMessage) override;

    // Configuration
    void updateConfig(const yvc::AudioConfig& config);
    yvc::PerformanceMode getPerformanceMode() const;
    void setPerformanceMode(yvc::PerformanceMode mode);

    // Statistics
    struct Statistics {
        uint64_t totalSamplesProcessed = 0;
        uint64_t totalCallbacks = 0;
        uint32_t xruns = 0;
        double averageLatencyMs = 0.0;
        std::atomic<bool> isRunning{false};

        Statistics() = default;
        // Explicit copy constructor (std::atomic is non-copyable by default)
        Statistics(const Statistics& other) noexcept
            : totalSamplesProcessed(other.totalSamplesProcessed),
              totalCallbacks(other.totalCallbacks),
              xruns(other.xruns),
              averageLatencyMs(other.averageLatencyMs),
              isRunning(other.isRunning.load(std::memory_order_relaxed)) {}
        // Explicit copy assignment
        Statistics& operator=(const Statistics& other) noexcept {
            if (this != &other) {
                totalSamplesProcessed = other.totalSamplesProcessed;
                totalCallbacks = other.totalCallbacks;
                xruns = other.xruns;
                averageLatencyMs = other.averageLatencyMs;
                isRunning.store(other.isRunning.load(std::memory_order_relaxed), std::memory_order_relaxed);
            }
            return *this;
        }
        // Move operations (treat atomic via load/store)
        Statistics(Statistics&& other) noexcept
            : totalSamplesProcessed(other.totalSamplesProcessed),
              totalCallbacks(other.totalCallbacks),
              xruns(other.xruns),
              averageLatencyMs(other.averageLatencyMs),
              isRunning(other.isRunning.load(std::memory_order_relaxed)) {}
        Statistics& operator=(Statistics&& other) noexcept {
            if (this != &other) {
                totalSamplesProcessed = other.totalSamplesProcessed;
                totalCallbacks = other.totalCallbacks;
                xruns = other.xruns;
                averageLatencyMs = other.averageLatencyMs;
                isRunning.store(other.isRunning.load(std::memory_order_relaxed), std::memory_order_relaxed);
            }
            return *this;
        }
    };

    Statistics getStatistics() const;
    void resetStatistics();

private:
    yvc::MetricsBus& metricsBus_;
    std::unique_ptr<yvc::AnalyzerEngine> engine_;
    
    // Mono conversion buffer
    std::vector<yvc::Sample> monoBuffer_;
    
    // Statistics
    mutable std::mutex statsMutex_;
    Statistics stats_;
    juce::Time streamStartTime_;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(JuceAudioBridge)
};

} // namespace yvc::app
