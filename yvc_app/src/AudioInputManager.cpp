// VoiVoi GUI Application - Audio Input Manager Implementation
// License: GPLv3

#include "AudioInputManager.h"
#include <yvc_core/Logger.h>
#include <algorithm>
#include <cmath>
#include <mutex>
#include <juce_gui_basics/juce_gui_basics.h>

namespace yvc::app {

    /// <summary>
    /// Constructs the AudioInputManager and initializes the platform backend and ring buffer.
    /// </summary>
    AudioInputManager::AudioInputManager()
        : ringBuffer_(std::make_unique<yvc::LockFreeRingBuffer<float>>(kRingBufferSize))
    {
        backend_ = yvc::audio::createPlatformBackend();
        if(!backend_) {
            LOG_ERROR("Failed to create audio backend");
        }
        else {
            LOG_INFO("Audio input manager initialized");
        }
    }

    /// <summary>
    /// Destructor closes any open device.
    /// </summary>
    AudioInputManager::~AudioInputManager() { closeDevice(); }

    /// <summary>
    /// Opens a specific audio input device by id.
    /// </summary>
    bool AudioInputManager::openDevice(const std::string& deviceId, yvc::SampleRate sampleRate, size_t bufferSize) {
        closeDevice();
        if(!backend_) {
            LOG_ERROR("No audio backend available");
            return false;
        }
        yvc::audio::AudioStreamParameters params;
        params.sampleRate = static_cast<double>(sampleRate);
        params.channels = 1;
        params.framesPerBuffer = static_cast<uint32_t>(bufferSize);
        auto callback = [this](const float* samples, size_t frames, double sr)
        {
            audioCallback(samples, frames, sr);
        };
        device_ = backend_->createInputDevice(deviceId, params, callback);
        if(!device_) {
            LOG_ERRORF("Failed to open audio device: %s", deviceId.c_str());
            return false;
        }
        auto actualParams = device_->streamParameters();
        actualSampleRate_ = static_cast<yvc::SampleRate>(actualParams.sampleRate);
        actualBufferSize_ = actualParams.framesPerBuffer;
        actualChannels_ = actualParams.channels;
        deviceName_ = device_->info().name;
        LOG_INFOF("Opened audio device: %s (SR: %u Hz, Buffer: %zu, Channels: %zu)",
                  deviceName_.c_str(),
                  actualSampleRate_,
                  actualBufferSize_,
                  actualChannels_);
        return true;
    }

    /// <summary>
    /// Opens the default audio input device.
    /// </summary>
    bool AudioInputManager::openDefaultDevice(yvc::SampleRate sampleRate, size_t bufferSize) {
        closeDevice();
        if(!backend_) {
            LOG_ERROR("No audio backend available");
            return false;
        }
        yvc::audio::AudioStreamParameters params;
        params.sampleRate = static_cast<double>(sampleRate);
        params.channels = 1;
        params.framesPerBuffer = static_cast<uint32_t>(bufferSize);
        auto callback = [this](const float* samples, size_t frames, double sr)
        {
            audioCallback(samples, frames, sr);
        };
        device_ = backend_->createDefaultInputDevice(params, callback);
        if(!device_) {
            LOG_ERROR("Failed to open default audio device");
            return false;
        }
        auto actualParams = device_->streamParameters();
        actualSampleRate_ = static_cast<yvc::SampleRate>(actualParams.sampleRate);
        actualBufferSize_ = actualParams.framesPerBuffer;
        actualChannels_ = actualParams.channels;
        deviceName_ = device_->info().name;
        LOG_INFOF("Opened default audio device: %s (SR: %u Hz, Buffer: %zu, Channels: %zu)",
                  deviceName_.c_str(),
                  actualSampleRate_,
                  actualBufferSize_,
                  actualChannels_);
        return true;
    }

    /// <summary>
    /// Closes the currently open device and resets statistics and buffers.
    /// </summary>
    void AudioInputManager::closeDevice() {
        if(device_) {
            stop();
            device_.reset();
            LOG_INFO("Audio device closed");
        }
        deviceName_.clear();
        actualSampleRate_ = 0;
        actualBufferSize_ = 0;
        actualChannels_ = 0;
        ringBuffer_->clear();
        resetStatistics();
    }

    /// <summary>
    /// Starts streaming from the open device.
    /// </summary>
    void AudioInputManager::start() {
        if(!device_) {
            LOG_ERROR("Cannot start: no device open");
            return;
        }
        if(device_->isRunning()) {
            LOG_WARN("Device already running");
            return;
        }
        ringBuffer_->clear();
        resetStatistics();
        device_->start();
        LOG_INFO("Audio input started");
    }

    /// <summary>
    /// Stops streaming if currently running.
    /// </summary>
    void AudioInputManager::stop() {
        if(!device_) return;
        if(!device_->isRunning()) return;
        device_->stop();
        LOG_INFO("Audio input stopped");
    }

    /// <summary>
    /// Returns actual negotiated sample rate.
    /// </summary>
    yvc::SampleRate AudioInputManager::getActualSampleRate() const { return actualSampleRate_; }
    /// <summary>
    /// Returns actual buffer size in frames.
    /// </summary>
    size_t AudioInputManager::getActualBufferSize() const { return actualBufferSize_; }
    /// <summary>
    /// Returns actual channel count.
    /// </summary>
    size_t AudioInputManager::getActualChannels() const { return actualChannels_; }
    /// <summary>
    /// Returns device name.
    /// </summary>
    std::string AudioInputManager::getDeviceName() const { return deviceName_; }
    /// <summary>
    /// Reads available mono samples from ring buffer.
    /// </summary>
    size_t AudioInputManager::readAudioData(float* buffer, size_t numSamples) {
        return ringBuffer_->read(buffer, numSamples);
    }
    /// <summary>
    /// Returns number of samples available for reading.
    /// </summary>
    size_t AudioInputManager::getAvailableSamples() const { return ringBuffer_->getAvailableRead(); }
    /// <summary>
    /// Returns a snapshot of current statistics.
    /// </summary>
    AudioInputManager::Statistics AudioInputManager::getStatistics() const {
        std::lock_guard<std::mutex> lock(statsMutex_);
        return stats_;
    }
    /// <summary>
    /// Resets accumulated statistics.
    /// </summary>
    void AudioInputManager::resetStatistics() {
        std::lock_guard<std::mutex> lock(statsMutex_);
        stats_ = Statistics();
        callbackDurations_.clear();
    }

    /// <summary>
    /// Real-time audio callback invoked by backend; writes samples to ring buffer and updates stats.
    /// </summary>
    void AudioInputManager::audioCallback(const float* samples,
                                          size_t frames,
                                          double sampleRate) {
        auto callbackStart = juce::Time::getCurrentTime();
        size_t written = ringBuffer_->write(samples, frames);
        if(written < frames) {
            std::lock_guard<std::mutex> lock(statsMutex_); stats_.xruns++;
            if(onXRun) {
                if(auto* mm = juce::MessageManager::getInstance())
                {
                    mm->callAsync([this]
                    {
                        if(onXRun) onXRun();
                    });
                }
            }
            LOG_WARNF("Audio XRun detected: attempted %zu samples, written %zu", frames, written);
        }
        updateStatistics(samples, frames);
        auto callbackEnd = juce::Time::getCurrentTime();
        auto duration = (callbackEnd - callbackStart).inSeconds();
        std::lock_guard<std::mutex> lock(statsMutex_);
        callbackDurations_.push_back(duration);
        if(callbackDurations_.size() > 100)
            callbackDurations_.erase(callbackDurations_.begin());
        double sum = 0.0;
        for(double d : callbackDurations_) sum += d;
        stats_.averageCallbackTime = sum / static_cast<double>(callbackDurations_.size());
    }

    /// <summary>
    /// Updates internal statistics (peak, RMS, counters) from a block of samples.
    /// </summary>
    void AudioInputManager::updateStatistics(const float* samples, size_t frames) {
        std::lock_guard<std::mutex> lock(statsMutex_);
        stats_.totalSamplesReceived += frames;
        stats_.totalCallbacks++;
        float peak = 0.0f;
        double sumSquares = 0.0;
        for(size_t i = 0; i < frames; ++i) {
            float absVal = std::abs(samples[i]);
            peak = std::max(peak, absVal);
            sumSquares += static_cast<double>(samples[i]) * static_cast<double>(samples[i]);
        }
        stats_.peakLevel = std::max(stats_.peakLevel, static_cast<double>(peak));
        if(frames > 0) {
            double rms = std::sqrt(sumSquares / static_cast<double>(frames));
            const double alpha = 0.1;
            if(stats_.totalCallbacks == 1) {
                stats_.rmsLevel = rms;
            }
            else {
                stats_.rmsLevel = alpha * rms + (1.0 - alpha) * stats_.rmsLevel;
            }
        }
    }

} // namespace yvc::app
