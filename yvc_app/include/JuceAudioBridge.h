// VoiVoi GUI Application - JUCE Audio Bridge
// License: GPLv3
// Purpose: Bridge between JUCE AudioDeviceManager and yvc_core analysis engine.
// Notes:
//  - Real-time path must avoid heap allocations and blocking. Only minimal locking is used.
//  - Provides a small mono ring buffer snapshot for UI waveform with end timestamp.

#pragma once

#include <juce_audio_devices/juce_audio_devices.h>
#include <yvc_core/AnalyzerEngine.h>
#include <yvc_core/MetricsBus.h>
#include <yvc_core/Types.h>
#include <memory>
#include <atomic>
#include <vector>
#include <mutex>

namespace yvc::app {

/// <summary>
/// JUCE AudioIODeviceCallback implementation that feeds audio to AnalyzerEngine.
/// Owns a core AnalyzerEngine and publishes metrics to the shared MetricsBus.
/// </summary>
class JuceAudioBridge : public juce::AudioIODeviceCallback {
public:
    /// <summary>
    /// Constructs the bridge.
    /// </summary>
    /// <param name="metricsBus">Reference to shared metrics bus</param>
    /// <param name="config">Initial audio configuration</param>
    JuceAudioBridge(yvc::MetricsBus& metricsBus, const yvc::AudioConfig& config);

    /// <summary>
    /// Destructor.
    /// </summary>
    ~JuceAudioBridge() override;

    // AudioIODeviceCallback interface
    /// <summary>
    /// Real-time audio callback entry point.
    /// </summary>
    void audioDeviceIOCallbackWithContext(
        const float* const* inputChannelData,
        int numInputChannels,
        float* const* outputChannelData,
        int numOutputChannels,
        int numSamples,
        const juce::AudioIODeviceCallbackContext& context) override;

    /// <summary>
    /// Called by JUCE when the device is starting.
    /// </summary>
    void audioDeviceAboutToStart(juce::AudioIODevice* device) override;

    /// <summary>
    /// Called by JUCE when the device has stopped.
    /// </summary>
    void audioDeviceStopped() override;

    /// <summary>
    /// Called by JUCE when a device error occurs.
    /// </summary>
    void audioDeviceError(const juce::String& errorMessage) override;

    // Configuration
    /// <summary>
    /// Updates the analyzer configuration.
    /// </summary>
    void updateConfig(const yvc::AudioConfig& config);

    /// <summary>
    /// Gets current performance mode.
    /// </summary>
    yvc::PerformanceMode getPerformanceMode() const;

    /// <summary>
    /// Sets performance mode.
    /// </summary>
    void setPerformanceMode(yvc::PerformanceMode mode);

    // Statistics
    /// <summary>
    /// Runtime statistics for the bridge.
    /// </summary>
    struct Statistics {
        uint64_t totalSamplesProcessed = 0;
        uint64_t totalCallbacks = 0;
        uint32_t xruns = 0;
        double averageLatencyMs = 0.0;
        std::atomic<bool> isRunning{false};

        Statistics() = default;
        Statistics(const Statistics& other) noexcept
            : totalSamplesProcessed(other.totalSamplesProcessed),
              totalCallbacks(other.totalCallbacks),
              xruns(other.xruns),
              averageLatencyMs(other.averageLatencyMs),
              isRunning(other.isRunning.load(std::memory_order_relaxed)) {}
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

    /// <summary>
    /// Small slice of mono audio for UI waveforms.
    /// </summary>
    struct AudioSlice { std::vector<float> samples; double endTimestamp = 0.0; };

    /// <summary>
    /// Gets current statistics snapshot.
    /// </summary>
    Statistics getStatistics() const;

    /// <summary>
    /// Resets statistics.
    /// </summary>
    void resetStatistics();

    /// <summary>
    /// Fetches up to maxSamples of latest mono samples with the block end timestamp.
    /// </summary>
    void getRecentMonoSamples(AudioSlice& out, size_t maxSamples) const;

    /// <summary>
    /// Backward-compatible helper that returns samples only (no timestamp).
    /// </summary>
    void getRecentMonoSamples(std::vector<float>& out, size_t maxSamples) const;

    /// <summary>
    /// Gets current sample rate of underlying engine.
    /// </summary>
    uint32_t getSampleRate() const { return engine_ ? engine_->getConfig().sample_rate : 0; }

private:
    yvc::MetricsBus& metricsBus_;
    std::unique_ptr<yvc::AnalyzerEngine> engine_;
    
    // Mono conversion buffer (RT path)
    std::vector<yvc::Sample> monoBuffer_;
    
    // Statistics
    mutable std::mutex statsMutex_;
    Statistics stats_;
    juce::Time streamStartTime_;

    // Recent mono waveform ring buffer for UI (1 sec by default)
    mutable std::mutex recentMutex_;
    std::vector<float> recentSamples_;
    size_t recentWritePos_ = 0;
    size_t recentCapacity_ = 0;
    double recentLastTimestamp_ = 0.0;
};

} // namespace yvc::app
