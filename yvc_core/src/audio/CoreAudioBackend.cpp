#include "yvc_core/AudioBackend.h"

#if YVC_CORE_HAVE_COREAUDIO
#include <AudioToolbox/AudioQueue.h>
#include <CoreAudio/CoreAudio.h>
#include <CoreFoundation/CoreFoundation.h>

#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace yvc::audio {
namespace {

AudioDeviceID defaultInputDeviceId() {
    AudioDeviceID device = kAudioObjectUnknown;
    UInt32 size = sizeof(device);
    const AudioObjectPropertyAddress address{
        kAudioHardwarePropertyDefaultInputDevice,
        kAudioObjectPropertyScopeGlobal,
        kAudioObjectPropertyElementMain};
    if (AudioObjectGetPropertyData(kAudioObjectSystemObject, &address, 0, nullptr, &size, &device) != noErr) {
        return kAudioObjectUnknown;
    }
    return device;
}

std::string cfStringToStdString(CFStringRef value) {
    if (!value) {
        return {};
    }
    const CFIndex length = CFStringGetLength(value);
    const CFIndex maxSize = CFStringGetMaximumSizeForEncoding(length, kCFStringEncodingUTF8) + 1;
    std::string result(static_cast<size_t>(maxSize), '\0');
    if (!CFStringGetCString(value, result.data(), maxSize, kCFStringEncodingUTF8)) {
        return {};
    }
    result.resize(std::char_traits<char>::length(result.c_str()));
    return result;
}

std::string defaultInputDeviceName() {
    const AudioDeviceID device = defaultInputDeviceId();
    if (device == kAudioObjectUnknown) {
        return "Default Input";
    }

    CFStringRef cfName = nullptr;
    UInt32 size = sizeof(cfName);
    const AudioObjectPropertyAddress address{
        kAudioObjectPropertyName,
        kAudioObjectPropertyScopeInput,
        kAudioObjectPropertyElementMain};
    if (AudioObjectGetPropertyData(device, &address, 0, nullptr, &size, &cfName) != noErr || !cfName) {
        return "Default Input";
    }
    std::string name = cfStringToStdString(cfName);
    CFRelease(cfName);
    if (name.empty()) {
        name = "Default Input";
    }
    return name;
}

double defaultInputDeviceSampleRate() {
    const AudioDeviceID device = defaultInputDeviceId();
    if (device == kAudioObjectUnknown) {
        return 48000.0;
    }
    double sampleRate = 48000.0;
    UInt32 size = sizeof(sampleRate);
    const AudioObjectPropertyAddress address{
        kAudioDevicePropertyNominalSampleRate,
        kAudioObjectPropertyScopeInput,
        kAudioObjectPropertyElementMain};
    if (AudioObjectGetPropertyData(device, &address, 0, nullptr, &size, &sampleRate) != noErr) {
        return 48000.0;
    }
    return sampleRate;
}

class CoreAudioDevice final : public AudioDevice {
public:
    CoreAudioDevice(AudioDeviceInfo info,
                    AudioStreamParameters params,
                    AudioInputCallback callback)
        : info_(std::move(info)),
          params_(params),
          callback_(std::move(callback)) {
        if (!callback_) {
            throw std::invalid_argument("AudioInputCallback must not be empty");
        }

        AudioStreamBasicDescription format{};
        format.mSampleRate = params_.sampleRate;
        format.mFormatID = kAudioFormatLinearPCM;
        format.mFormatFlags = kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked;
        format.mBitsPerChannel = static_cast<UInt32>(8U * sizeof(float));
        format.mChannelsPerFrame = params_.channels;
        format.mBytesPerFrame = static_cast<UInt32>(sizeof(float) * params_.channels);
        format.mFramesPerPacket = 1;
        format.mBytesPerPacket = format.mBytesPerFrame;

        const OSStatus status = AudioQueueNewInput(&format, &CoreAudioDevice::handleInput,
                                                   this, nullptr, nullptr, 0, &queue_);
        if (status != noErr) {
            throw std::runtime_error("AudioQueueNewInput failed");
        }
        allocateBuffers();
    }

    ~CoreAudioDevice() override {
        try {
            stop();
        } catch (...) {
        }
        if (queue_) {
            AudioQueueDispose(queue_, true);
        }
    }

    void start() override {
        if (running_) {
            return;
        }
        const OSStatus status = AudioQueueStart(queue_, nullptr);
        if (status != noErr) {
            throw std::runtime_error("AudioQueueStart failed");
        }
        running_ = true;
    }

    void stop() override {
        if (!running_) {
            return;
        }
        AudioQueueStop(queue_, true);
        running_ = false;
    }

    bool isRunning() const override { return running_; }

    const AudioDeviceInfo& info() const override { return info_; }

    AudioStreamParameters streamParameters() const override { return params_; }

private:
    void allocateBuffers() {
        const size_t bufferSize = static_cast<size_t>(params_.framesPerBuffer) * params_.channels * sizeof(float);
        for (int i = 0; i < 3; ++i) {
            AudioQueueBufferRef buffer = nullptr;
            const OSStatus status = AudioQueueAllocateBuffer(queue_, bufferSize, &buffer);
            if (status != noErr || !buffer) {
                throw std::runtime_error("AudioQueueAllocateBuffer failed");
            }
            buffer->mAudioDataByteSize = static_cast<UInt32>(bufferSize);
            AudioQueueEnqueueBuffer(queue_, buffer, 0, nullptr);
            buffers_.push_back(buffer);
        }
    }

    static void handleInput(void* userData,
                             AudioQueueRef inAQ,
                             AudioQueueBufferRef inBuffer,
                             const AudioTimeStamp* /*inStartTime*/,
                             UInt32 inNumPackets,
                             const AudioStreamPacketDescription* /*inPacketDesc*/) {
        auto* self = static_cast<CoreAudioDevice*>(userData);
        if (!self->callback_) {
            AudioQueueEnqueueBuffer(inAQ, inBuffer, 0, nullptr);
            return;
        }

        size_t frames = inNumPackets;
        if (frames == 0 && self->params_.channels > 0) {
            frames = inBuffer->mAudioDataByteSize / (sizeof(float) * self->params_.channels);
        }
        auto* samples = static_cast<float*>(inBuffer->mAudioData);
        self->callback_(samples, frames, self->params_.sampleRate);
        AudioQueueEnqueueBuffer(inAQ, inBuffer, 0, nullptr);
    }

    AudioQueueRef queue_{nullptr};
    std::vector<AudioQueueBufferRef> buffers_;
    AudioDeviceInfo info_;
    AudioStreamParameters params_;
    AudioInputCallback callback_;
    bool running_ = false;
};

class CoreAudioBackend final : public IAudioBackend {
public:
    std::vector<AudioDeviceInfo> enumerateInputDevices() const override {
        AudioDeviceInfo info;
        info.id = "default";
        info.name = defaultInputDeviceName();
        info.channels = 1;
        info.sampleRate = defaultInputDeviceSampleRate();
        info.isDefault = true;
        return {info};
    }

    std::unique_ptr<AudioDevice> createInputDevice(const std::string& deviceId,
                                                   const AudioStreamParameters& params,
                                                   AudioInputCallback callback) override {
        AudioDeviceInfo info;
        info.id = deviceId.empty() ? std::string("default") : deviceId;
        info.name = defaultInputDeviceName();
        info.channels = params.channels;
        info.sampleRate = params.sampleRate;
        info.isDefault = info.id == "default";
        return std::make_unique<CoreAudioDevice>(std::move(info), params, std::move(callback));
    }

    std::unique_ptr<AudioDevice> createDefaultInputDevice(const AudioStreamParameters& params,
                                                          AudioInputCallback callback) override {
        return createInputDevice("default", params, std::move(callback));
    }
};

}  // namespace

std::unique_ptr<IAudioBackend> createCoreAudioBackend() {
    return std::make_unique<CoreAudioBackend>();
}

}  // namespace yvc::audio

#else  // YVC_CORE_HAVE_COREAUDIO

namespace yvc::audio {
std::unique_ptr<IAudioBackend> createCoreAudioBackend() { return {}; }
}  // namespace yvc::audio

#endif
