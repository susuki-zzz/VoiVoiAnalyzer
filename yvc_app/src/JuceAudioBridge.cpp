// VoiVoi GUI Application - JUCE Audio Bridge Implementation
// License: GPLv3
// RT notes: keep audio callback free of blocking work; analyzer call is expected to be lock-free.

#include "JuceAudioBridge.h"
#include <yvc_core/Logger.h>
#include <algorithm>

namespace yvc::app {

    /// <summary>
    /// Constructs the bridge and initializes analyzer engine and waveform buffer.
    /// </summary>
    JuceAudioBridge::JuceAudioBridge(yvc::MetricsBus& metricsBus, const yvc::AudioConfig& config)
        : metricsBus_(metricsBus) {
        engine_ = std::make_unique<yvc::AnalyzerEngine>(config, metricsBus_);
        monoBuffer_.reserve(config.buffer_size * 4); // Pre-allocate with headroom
        recentCapacity_ = static_cast<size_t>(config.sample_rate); // 1 second of audio
        recentSamples_.assign(recentCapacity_, 0.0f);
        LOG_INFO("JuceAudioBridge created");
    }

    /// <summary>
    /// Destructor logs destruction.
    /// </summary>
    JuceAudioBridge::~JuceAudioBridge() {
        LOG_INFO("JuceAudioBridge destroyed");
    }

    /// <summary>
    /// Real-time audio callback invoked by JUCE. Mixes input to mono, timestamps, and processes.
    /// </summary>
    void JuceAudioBridge::audioDeviceIOCallbackWithContext(
        const float* const* inputChannelData,
        int numInputChannels,
        float* const* outputChannelData,
        int numOutputChannels,
        int numSamples,
        const juce::AudioIODeviceCallbackContext& context) {

        // Output must be silent (input-only app)
        for(int ch = 0; ch < numOutputChannels; ++ch) {
            if(outputChannelData[ch] != nullptr) {
                juce::FloatVectorOperations::clear(outputChannelData[ch], numSamples);
            }
        }

        // No input available
        if(numInputChannels == 0 || inputChannelData == nullptr || numSamples <= 0) {
            return;
        }

        auto callbackStart = juce::Time::getHighResolutionTicks();

        // Convert to mono
        monoBuffer_.resize(static_cast<size_t>(numSamples));

        if(numInputChannels == 1 && inputChannelData[0] != nullptr) {
            // Already mono - direct copy
            std::copy_n(inputChannelData[0], numSamples, monoBuffer_.data());
        }
        else {
            // Mix down to mono
            juce::FloatVectorOperations::clear(monoBuffer_.data(), numSamples);

            int activeChannels = 0;
            for(int ch = 0; ch < numInputChannels; ++ch) {
                if(inputChannelData[ch] != nullptr) {
                    juce::FloatVectorOperations::add(monoBuffer_.data(), inputChannelData[ch], numSamples);
                    ++activeChannels;
                }
            }

            if(activeChannels > 1) {
                juce::FloatVectorOperations::multiply(monoBuffer_.data(), 1.0f / static_cast<float>(activeChannels), numSamples);
            }
        }

        // Calculate timestamp from stream time (seconds since start)
        double timestamp = 0.0;
        if(stats_.isRunning && streamStartTime_.toMilliseconds() > 0) {
            auto elapsed = juce::Time::getCurrentTime() - streamStartTime_;
            timestamp = elapsed.inSeconds();
        }

        // Feed to analyzer engine
        engine_->process(monoBuffer_.data(), static_cast<size_t>(numSamples), timestamp);

        // Store into recent ring buffer for UI waveform
        {
            std::lock_guard<std::mutex> lk(recentMutex_);
            if(recentCapacity_ == 0) {
                recentCapacity_ = static_cast<size_t>(engine_->getConfig().sample_rate);
                recentSamples_.assign(recentCapacity_, 0.0f);
                recentWritePos_ = 0;
                recentLastTimestamp_ = 0.0;
            }
            for(int i = 0; i < numSamples; ++i) {
                recentSamples_[recentWritePos_] = monoBuffer_[static_cast<size_t>(i)];
                recentWritePos_ = (recentWritePos_ + 1) % recentCapacity_;
            }
            // Timestamp corresponds to the end of this block
            recentLastTimestamp_ = timestamp;
        }

        // Update statistics (non-RT critical)
        {
            std::lock_guard<std::mutex> lock(statsMutex_);
            stats_.totalSamplesProcessed += static_cast<uint64_t>(numSamples);
            stats_.totalCallbacks++;

            auto callbackEnd = juce::Time::getHighResolutionTicks();
            double latencyMs = juce::Time::highResolutionTicksToSeconds(callbackEnd - callbackStart) * 1000.0;

            // Exponential moving average for latency
            const double alpha = 0.1;
            if(stats_.totalCallbacks == 1) {
                stats_.averageLatencyMs = latencyMs;
            }
            else {
                stats_.averageLatencyMs = alpha * latencyMs + (1.0 - alpha) * stats_.averageLatencyMs;
            }
        }
    }

    /// <summary>
    /// Called when JUCE device is starting; resets timing and statistics.
    /// </summary>
    void JuceAudioBridge::audioDeviceAboutToStart(juce::AudioIODevice* device) {
        if(device == nullptr) {
            LOG_WARN("audioDeviceAboutToStart called with null device");
            return;
        }

        LOG_INFOF("Audio device starting: %s @ %.0f Hz, buffer %d",
                  device->getName().toRawUTF8(),
                  device->getCurrentSampleRate(),
                  device->getCurrentBufferSizeSamples());

        streamStartTime_ = juce::Time::getCurrentTime();
        stats_.isRunning = true;
        resetStatistics();
    }

    /// <summary>
    /// Called when JUCE device stops.
    /// </summary>
    void JuceAudioBridge::audioDeviceStopped() {
        LOG_INFO("Audio device stopped");
        stats_.isRunning = false;
    }

    /// <summary>
    /// Called by JUCE on device error.
    /// </summary>
    void JuceAudioBridge::audioDeviceError(const juce::String& errorMessage) {
        LOG_ERRORF("Audio device error: %s", errorMessage.toRawUTF8());
        std::lock_guard<std::mutex> lock(statsMutex_);
        stats_.xruns++;
    }

    /// <summary>
    /// Updates analyzer engine with new audio configuration.
    /// </summary>
    void JuceAudioBridge::updateConfig(const yvc::AudioConfig& config) {
        LOG_INFOF("Updating audio config: SR=%u, buffer=%u, mode=%d",
                  config.sample_rate, config.buffer_size, static_cast<int>(config.mode));

        // Recreate engine with new config
        engine_ = std::make_unique<yvc::AnalyzerEngine>(config, metricsBus_);
        monoBuffer_.reserve(config.buffer_size * 4);
        resetStatistics();
    }

    /// <summary>
    /// Gets current performance mode.
    /// </summary>
    yvc::PerformanceMode JuceAudioBridge::getPerformanceMode() const {
        return engine_->getPerformanceMode();
    }

    /// <summary>
    /// Sets performance mode.
    /// </summary>
    void JuceAudioBridge::setPerformanceMode(yvc::PerformanceMode mode) {
        LOG_INFOF("Setting performance mode: %d", static_cast<int>(mode));
        engine_->setPerformanceMode(mode);
    }

    /// <summary>
    /// Gets a snapshot of current runtime statistics.
    /// </summary>
    JuceAudioBridge::Statistics JuceAudioBridge::getStatistics() const {
        std::lock_guard<std::mutex> lock(statsMutex_);
        return stats_;
    }

    /// <summary>
    /// Resets statistics counters.
    /// </summary>
    void JuceAudioBridge::resetStatistics() {
        std::lock_guard<std::mutex> lock(statsMutex_);
        stats_.totalSamplesProcessed = 0;
        stats_.totalCallbacks = 0;
        stats_.xruns = 0;
        stats_.averageLatencyMs = 0.0;
    }

    /// <summary>
    /// Retrieves recent mono samples (oldest first) without timestamp.
    /// </summary>
    void JuceAudioBridge::getRecentMonoSamples(std::vector<float>& out, size_t maxSamples) const {
        std::lock_guard<std::mutex> lk(recentMutex_);
        if(recentCapacity_ == 0 || recentSamples_.empty()) {
            out.clear();
            return;
        }
        size_t n = std::min(maxSamples, recentCapacity_);
        out.resize(n);
        // Oldest first: start from writePos - n (mod capacity)
        size_t start = (recentWritePos_ + recentCapacity_ - n) % recentCapacity_;
        for(size_t i = 0; i < n; ++i) {
            out[i] = recentSamples_[(start + i) % recentCapacity_];
        }
    }

    /// <summary>
    /// Retrieves recent mono samples with end timestamp of last block.
    /// </summary>
    void JuceAudioBridge::getRecentMonoSamples(AudioSlice& out, size_t maxSamples) const {
        std::lock_guard<std::mutex> lk(recentMutex_);
        if(recentCapacity_ == 0 || recentSamples_.empty()) {
            out.samples.clear();
            out.endTimestamp = 0.0;
            return;
        }
        size_t n = std::min(maxSamples, recentCapacity_);
        out.samples.resize(n);
        size_t start = (recentWritePos_ + recentCapacity_ - n) % recentCapacity_;
        for(size_t i = 0; i < n; ++i) {
            out.samples[i] = recentSamples_[(start + i) % recentCapacity_];
        }
        out.endTimestamp = recentLastTimestamp_;
    }

} // namespace yvc::app
