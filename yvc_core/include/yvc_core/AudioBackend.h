#pragma once

#include "yvc_core/Types.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

#ifndef YVC_CORE_HAVE_COREAUDIO
#define YVC_CORE_HAVE_COREAUDIO 0
#endif

#ifndef YVC_CORE_HAVE_ALSA
#define YVC_CORE_HAVE_ALSA 0
#endif

#ifndef YVC_CORE_HAVE_WASAPI
#ifdef _WIN32
#define YVC_CORE_HAVE_WASAPI 1
#else
#define YVC_CORE_HAVE_WASAPI 0
#endif
#endif

namespace yvc::audio {

/// <summary>
/// Information about an audio device.
/// </summary>
struct AudioDeviceInfo {
    std::string id;
    std::string name;
    uint32_t channels = 1;
    double sampleRate = 48000.0;
    bool isDefault = false;
};

/// <summary>
/// Parameters for configuring an audio stream.
/// </summary>
struct AudioStreamParameters {
    double sampleRate = 48000.0;
    uint32_t channels = 1;
    uint32_t framesPerBuffer = 512;
};

/// <summary>
/// Callback function type for audio input processing.
/// </summary>
using AudioInputCallback = std::function<void(const float* samples, size_t frames, double sampleRate)>;

/// <summary>
/// Abstract interface for an audio device.
/// </summary>
class AudioDevice {
public:
    virtual ~AudioDevice() = default;

    /// <summary>
    /// Starts the audio device.
    /// </summary>
    virtual void start() = 0;

    /// <summary>
    /// Stops the audio device.
    /// </summary>
    virtual void stop() = 0;

    /// <summary>
    /// Checks if the device is currently running.
    /// </summary>
    /// <returns>True if running, false otherwise</returns>
    virtual bool isRunning() const = 0;

    /// <summary>
    /// Gets device information.
    /// </summary>
    /// <returns>Reference to device info structure</returns>
    virtual const AudioDeviceInfo& info() const = 0;

    /// <summary>
    /// Gets current stream parameters.
    /// </summary>
    /// <returns>Current stream parameters</returns>
    virtual AudioStreamParameters streamParameters() const = 0;
};

/// <summary>
/// Abstract interface for platform-specific audio backend.
/// </summary>
class IAudioBackend {
public:
    virtual ~IAudioBackend() = default;

    /// <summary>
    /// Enumerates all available input devices.
    /// </summary>
    /// <returns>Vector of device information structures</returns>
    virtual std::vector<AudioDeviceInfo> enumerateInputDevices() const = 0;

    /// <summary>
    /// Creates an input device with the specified parameters.
    /// </summary>
    /// <param name="deviceId">Device identifier</param>
    /// <param name="params">Stream parameters</param>
    /// <param name="callback">Audio input callback</param>
    /// <returns>Unique pointer to the created device</returns>
    virtual std::unique_ptr<AudioDevice> createInputDevice(
        const std::string& deviceId,
        const AudioStreamParameters& params,
        AudioInputCallback callback) = 0;

    /// <summary>
    /// Creates the default input device with the specified parameters.
    /// </summary>
    /// <param name="params">Stream parameters</param>
    /// <param name="callback">Audio input callback</param>
    /// <returns>Unique pointer to the created device</returns>
    virtual std::unique_ptr<AudioDevice> createDefaultInputDevice(
        const AudioStreamParameters& params,
        AudioInputCallback callback) = 0;
};

/// <summary>
/// Creates a CoreAudio backend (macOS/iOS).
/// </summary>
/// <returns>Unique pointer to the backend</returns>
std::unique_ptr<IAudioBackend> createCoreAudioBackend();

/// <summary>
/// Creates an ALSA backend (Linux).
/// </summary>
/// <returns>Unique pointer to the backend</returns>
std::unique_ptr<IAudioBackend> createAlsaBackend();

/// <summary>
/// Creates a WASAPI backend (Windows).
/// </summary>
/// <returns>Unique pointer to the backend</returns>
std::unique_ptr<IAudioBackend> createWasapiBackend();

/// <summary>
/// Creates the default platform backend.
/// </summary>
/// <returns>Unique pointer to the backend</returns>
std::unique_ptr<IAudioBackend> createPlatformBackend();

/// <summary>
/// Checks if CoreAudio backend is available.
/// </summary>
/// <returns>True if available, false otherwise</returns>
bool hasCoreAudioBackend();

/// <summary>
/// Checks if ALSA backend is available.
/// </summary>
/// <returns>True if available, false otherwise</returns>
bool hasAlsaBackend();

/// <summary>
/// Checks if WASAPI backend is available.
/// </summary>
/// <returns>True if available, false otherwise</returns>
bool hasWasapiBackend();

}  // namespace yvc::audio
