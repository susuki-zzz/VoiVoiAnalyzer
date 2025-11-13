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

namespace yvc::audio {

struct AudioDeviceInfo {
    std::string id;
    std::string name;
    uint32_t channels = 1;
    double sampleRate = 48000.0;
    bool isDefault = false;
};

struct AudioStreamParameters {
    double sampleRate = 48000.0;
    uint32_t channels = 1;
    uint32_t framesPerBuffer = 512;
};

using AudioInputCallback = std::function<void(const float* samples, size_t frames, double sampleRate)>;

class AudioDevice {
public:
    virtual ~AudioDevice() = default;

    virtual void start() = 0;
    virtual void stop() = 0;
    virtual bool isRunning() const = 0;

    virtual const AudioDeviceInfo& info() const = 0;
    virtual AudioStreamParameters streamParameters() const = 0;
};

class IAudioBackend {
public:
    virtual ~IAudioBackend() = default;

    virtual std::vector<AudioDeviceInfo> enumerateInputDevices() const = 0;

    virtual std::unique_ptr<AudioDevice> createInputDevice(
        const std::string& deviceId,
        const AudioStreamParameters& params,
        AudioInputCallback callback) = 0;

    virtual std::unique_ptr<AudioDevice> createDefaultInputDevice(
        const AudioStreamParameters& params,
        AudioInputCallback callback) = 0;
};

std::unique_ptr<IAudioBackend> createCoreAudioBackend();
std::unique_ptr<IAudioBackend> createAlsaBackend();
std::unique_ptr<IAudioBackend> createPlatformBackend();

bool hasCoreAudioBackend();
bool hasAlsaBackend();

}  // namespace yvc::audio
