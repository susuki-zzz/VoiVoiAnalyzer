// VoiVoi GUI Application - Audio Input Manager
// License: GPLv3

#pragma once

#include <juce_core/juce_core.h>
#include <yvc_core/AudioBackend.h>
#include <yvc_core/LockFreeRingBuffer.h>
#include <yvc_core/Types.h>
#include <memory>
#include <atomic>
#include <functional>
#include <mutex>

namespace yvc::app {

class AudioInputManager {
public:
    AudioInputManager();
    ~AudioInputManager();

    // Device management
    bool openDevice(const std::string& deviceId, yvc::SampleRate sampleRate, size_t bufferSize);
    bool openDefaultDevice(yvc::SampleRate sampleRate, size_t bufferSize);
    void closeDevice();
    
    bool isDeviceOpen() const { return device_ != nullptr; }
    bool isRunning() const { return device_ && device_->isRunning(); }
    
    // Audio control
    void start();
    void stop();
    
    // Get actual stream parameters
    yvc::SampleRate getActualSampleRate() const;
    size_t getActualBufferSize() const;
    size_t getActualChannels() const;
    
    // Get device info
    std::string getDeviceName() const;
    
    // Audio data access (for analysis thread)
    // Returns number of samples read
    size_t readAudioData(float* buffer, size_t numSamples);
    size_t getAvailableSamples() const;
    
    // Statistics
    struct Statistics {
        uint64_t totalSamplesReceived = 0;
        uint64_t totalCallbacks = 0;
        uint32_t xruns = 0;  // Buffer overruns
        double averageCallbackTime = 0.0;
        double peakLevel = 0.0;
        double rmsLevel = 0.0;
    };
    
    Statistics getStatistics() const;
    void resetStatistics();
    
    // Callbacks
    std::function<void(const float* samples, size_t numSamples, double sampleRate)> onAudioData;
    std::function<void()> onDeviceError;
    std::function<void()> onXRun;

private:
    void audioCallback(const float* samples, size_t frames, double sampleRate);
    void updateStatistics(const float* samples, size_t frames);
    
    std::unique_ptr<yvc::audio::IAudioBackend> backend_;
    std::unique_ptr<yvc::audio::AudioDevice> device_;
    
    // Ring buffer for thread-safe audio transfer
    static constexpr size_t kRingBufferSize = 96000 * 2;  // 2 seconds at 48kHz
    std::unique_ptr<yvc::LockFreeRingBuffer<float>> ringBuffer_;
    
    // Device info
    std::string deviceName_;
    yvc::SampleRate actualSampleRate_ = 0;
    size_t actualBufferSize_ = 0;
    size_t actualChannels_ = 0;
    
    // Statistics
    mutable std::mutex statsMutex_;
    Statistics stats_;
    juce::Time lastCallbackTime_;
    std::vector<double> callbackDurations_;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioInputManager)
};

} // namespace yvc::app
