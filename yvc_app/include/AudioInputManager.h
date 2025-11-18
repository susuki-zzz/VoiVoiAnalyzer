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

    /// <summary>
    /// Manages opening, starting, and reading from audio input devices.
    /// Provides a lock-free ring buffer for passing audio to analysis/UI threads.
    /// </summary>
    class AudioInputManager {
    public:
        /// <summary>
        /// Constructs the audio input manager.
        /// </summary>
        AudioInputManager();

        /// <summary>
        /// Destructor.
        /// </summary>
        ~AudioInputManager();

        // Device management
        /// <summary>
        /// Opens a specific input device.
        /// </summary>
        /// <param name="deviceId">Device identifier</param>
        /// <param name="sampleRate">Requested sample rate</param>
        /// <param name="bufferSize">Requested buffer size (frames)</param>
        /// <returns>True if device opened</returns>
        bool openDevice(const std::string& deviceId,
                        yvc::SampleRate sampleRate,
                        size_t bufferSize);

        /// <summary>
        /// Opens the default input device.
        /// </summary>
        /// <param name="sampleRate">Requested sample rate</param>
        /// <param name="bufferSize">Requested buffer size (frames)</param>
        /// <returns>True if device opened</returns>
        bool openDefaultDevice(yvc::SampleRate sampleRate,
                               size_t bufferSize);

        /// <summary>
        /// Closes the current device.
        /// </summary>
        void closeDevice();

        /// <summary>
        /// Checks if a device is open.
        /// </summary>
        bool isDeviceOpen() const { return device_ != nullptr; }

        /// <summary>
        /// Checks if the device stream is running.
        /// </summary>
        bool isRunning() const { return device_ && device_->isRunning(); }

        // Audio control
        /// <summary>
        /// Starts audio streaming.
        /// </summary>
        void start();

        /// <summary>
        /// Stops audio streaming.
        /// </summary>
        void stop();

        // Get actual stream parameters
        /// <summary>
        /// Gets the actual sample rate.
        /// </summary>
        yvc::SampleRate getActualSampleRate() const;

        /// <summary>
        /// Gets the actual buffer size (frames).
        /// </summary>
        size_t getActualBufferSize() const;

        /// <summary>
        /// Gets the actual channel count.
        /// </summary>
        size_t getActualChannels() const;

        // Get device info
        /// <summary>
        /// Gets the opened device name.
        /// </summary>
        std::string getDeviceName() const;

        // Audio data access (for analysis thread)
        // Returns number of samples read
        /// <summary>
        /// Reads mono samples from the internal ring buffer.
        /// </summary>
        /// <param name="buffer">Destination buffer</param>
        /// <param name="numSamples">Max samples to read</param>
        /// <returns>Number of samples actually read</returns>
        size_t readAudioData(float* buffer,
                             size_t numSamples);

        /// <summary>
        /// Gets number of samples currently available to read.
        /// </summary>
        size_t getAvailableSamples() const;

        // Statistics
        /// <summary>
        /// Runtime statistics for monitoring.
        /// </summary>
        struct Statistics {
            uint64_t totalSamplesReceived = 0;
            uint64_t totalCallbacks = 0;
            uint32_t xruns = 0;  // Buffer overruns
            double averageCallbackTime = 0.0;
            double peakLevel = 0.0;
            double rmsLevel = 0.0;
        };

        /// <summary>
        /// Gets current statistics snapshot.
        /// </summary>
        Statistics getStatistics() const;

        /// <summary>
        /// Resets statistics.
        /// </summary>
        void resetStatistics();

        // Callbacks
        /// <summary>
        /// Optional callback invoked on audio data arrival (RT path).
        /// </summary>
        std::function<void(const float* samples, size_t numSamples, double sampleRate)> onAudioData;

        /// <summary>
        /// Callback invoked on device error.
        /// </summary>
        std::function<void()> onDeviceError;

        /// <summary>
        /// Callback invoked on buffer overrun/underrun.
        /// </summary>
        std::function<void()> onXRun;

    private:
        void audioCallback(const float* samples,
                           size_t frames,
                           double sampleRate);
        void updateStatistics(const float* samples,
                              size_t frames);

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
