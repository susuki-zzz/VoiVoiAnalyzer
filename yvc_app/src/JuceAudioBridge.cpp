// VoiVoi GUI Application - JUCE Audio Bridge Implementation
// License: GPLv3

#include "JuceAudioBridge.h"
#include <yvc_core/Logger.h>
#include <algorithm>

namespace yvc::app {

JuceAudioBridge::JuceAudioBridge(yvc::MetricsBus& metricsBus, const yvc::AudioConfig& config)
    : metricsBus_(metricsBus) {
    engine_ = std::make_unique<yvc::AnalyzerEngine>(config, metricsBus_);
    monoBuffer_.reserve(config.buffer_size * 4); // Pre-allocate with headroom
    LOG_INFO("JuceAudioBridge created");
}

JuceAudioBridge::~JuceAudioBridge() {
    LOG_INFO("JuceAudioBridge destroyed");
}

void JuceAudioBridge::audioDeviceIOCallbackWithContext(
    const float* const* inputChannelData,
    int numInputChannels,
    float* const* outputChannelData,
    int numOutputChannels,
    int numSamples,
    const juce::AudioIODeviceCallbackContext& context) {
    
    // Clear output (we're input-only)
    for (int ch = 0; ch < numOutputChannels; ++ch) {
        if (outputChannelData[ch] != nullptr) {
            juce::FloatVectorOperations::clear(outputChannelData[ch], numSamples);
        }
    }

    // No input available
    if (numInputChannels == 0 || inputChannelData == nullptr || numSamples <= 0) {
        return;
    }

    auto callbackStart = juce::Time::getHighResolutionTicks();

    // Convert to mono
    monoBuffer_.resize(static_cast<size_t>(numSamples));
    
    if (numInputChannels == 1 && inputChannelData[0] != nullptr) {
        // Already mono - direct copy
        std::copy_n(inputChannelData[0], numSamples, monoBuffer_.data());
    } else {
        // Mix down to mono
        juce::FloatVectorOperations::clear(monoBuffer_.data(), numSamples);
        
        int activeChannels = 0;
        for (int ch = 0; ch < numInputChannels; ++ch) {
            if (inputChannelData[ch] != nullptr) {
                juce::FloatVectorOperations::add(monoBuffer_.data(), inputChannelData[ch], numSamples);
                ++activeChannels;
            }
        }
        
        if (activeChannels > 1) {
            juce::FloatVectorOperations::multiply(monoBuffer_.data(), 1.0f / static_cast<float>(activeChannels), numSamples);
        }
    }

    // Calculate timestamp from stream time
    double timestamp = 0.0;
    if (stats_.isRunning && streamStartTime_.toMilliseconds() > 0) {
        auto elapsed = juce::Time::getCurrentTime() - streamStartTime_;
        timestamp = elapsed.inSeconds();
    }

    // Feed to analyzer engine
    engine_->process(monoBuffer_.data(), static_cast<size_t>(numSamples), timestamp);

    // Update statistics
    {
        std::lock_guard<std::mutex> lock(statsMutex_);
        stats_.totalSamplesProcessed += static_cast<uint64_t>(numSamples);
        stats_.totalCallbacks++;
        
        auto callbackEnd = juce::Time::getHighResolutionTicks();
        double latencyMs = juce::Time::highResolutionTicksToSeconds(callbackEnd - callbackStart) * 1000.0;
        
        // Exponential moving average for latency
        const double alpha = 0.1;
        if (stats_.totalCallbacks == 1) {
            stats_.averageLatencyMs = latencyMs;
        } else {
            stats_.averageLatencyMs = alpha * latencyMs + (1.0 - alpha) * stats_.averageLatencyMs;
        }
    }
}

void JuceAudioBridge::audioDeviceAboutToStart(juce::AudioIODevice* device) {
    if (device == nullptr) {
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

void JuceAudioBridge::audioDeviceStopped() {
    LOG_INFO("Audio device stopped");
    stats_.isRunning = false;
}

void JuceAudioBridge::audioDeviceError(const juce::String& errorMessage) {
    LOG_ERRORF("Audio device error: %s", errorMessage.toRawUTF8());
    std::lock_guard<std::mutex> lock(statsMutex_);
    stats_.xruns++;
}

void JuceAudioBridge::updateConfig(const yvc::AudioConfig& config) {
    LOG_INFOF("Updating audio config: SR=%u, buffer=%u, mode=%d",
              config.sample_rate, config.buffer_size, static_cast<int>(config.mode));
    
    // Recreate engine with new config
    engine_ = std::make_unique<yvc::AnalyzerEngine>(config, metricsBus_);
    monoBuffer_.reserve(config.buffer_size * 4);
    resetStatistics();
}

yvc::PerformanceMode JuceAudioBridge::getPerformanceMode() const {
    return engine_->getPerformanceMode();
}

void JuceAudioBridge::setPerformanceMode(yvc::PerformanceMode mode) {
    LOG_INFOF("Setting performance mode: %d", static_cast<int>(mode));
    engine_->setPerformanceMode(mode);
}

JuceAudioBridge::Statistics JuceAudioBridge::getStatistics() const {
    std::lock_guard<std::mutex> lock(statsMutex_);
    return stats_;
}

void JuceAudioBridge::resetStatistics() {
    std::lock_guard<std::mutex> lock(statsMutex_);
    stats_.totalSamplesProcessed = 0;
    stats_.totalCallbacks = 0;
    stats_.xruns = 0;
    stats_.averageLatencyMs = 0.0;
}

} // namespace yvc::app
