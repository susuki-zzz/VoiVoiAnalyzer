#include "yvc_core/AudioBackend.h"

#include <atomic>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#if YVC_CORE_HAVE_ALSA
#include <alsa/asoundlib.h>
#include <errno.h>
#include <cstdlib>
#include <cstring>
#include <stdexcept>

namespace yvc::audio {
namespace {

class AlsaAudioDevice final : public AudioDevice {
public:
    AlsaAudioDevice(std::string deviceId,
                    AudioDeviceInfo info,
                    AudioStreamParameters params,
                    AudioInputCallback callback)
        : deviceId_(std::move(deviceId)),
          info_(std::move(info)),
          params_(params),
          callback_(std::move(callback)) {
        if (!callback_) {
            throw std::invalid_argument("AudioInputCallback must not be empty");
        }
    }

    ~AlsaAudioDevice() override {
        try {
            stop();
        } catch (...) {
        }
    }

    void start() override {
        if (running_.exchange(true)) {
            return;
        }
        worker_ = std::thread(&AlsaAudioDevice::captureLoop, this);
    }

    void stop() override {
        if (!running_.exchange(false)) {
            return;
        }
        if (worker_.joinable()) {
            worker_.join();
        }
    }

    bool isRunning() const override { return running_.load(); }

    const AudioDeviceInfo& info() const override { return info_; }

    AudioStreamParameters streamParameters() const override { return params_; }

private:
    void captureLoop() {
        snd_pcm_t* handle = nullptr;
        const char* device = deviceId_.empty() ? "default" : deviceId_.c_str();
        if (snd_pcm_open(&handle, device, SND_PCM_STREAM_CAPTURE, 0) < 0) {
            running_ = false;
            return;
        }

        snd_pcm_hw_params_t* params = nullptr;
        snd_pcm_hw_params_malloc(&params);
        snd_pcm_hw_params_any(handle, params);
        snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
        snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_FLOAT_LE);
        snd_pcm_hw_params_set_channels(handle, params, params_.channels);
        unsigned int rate = static_cast<unsigned int>(params_.sampleRate);
        snd_pcm_hw_params_set_rate_near(handle, params, &rate, nullptr);
        snd_pcm_uframes_t periodSize = params_.framesPerBuffer;
        snd_pcm_hw_params_set_period_size_near(handle, params, &periodSize, nullptr);
        snd_pcm_hw_params(handle, params);
        snd_pcm_hw_params_free(params);

        snd_pcm_prepare(handle);

        std::vector<float> buffer(static_cast<size_t>(params_.framesPerBuffer) * params_.channels);
        while (running_) {
            const snd_pcm_sframes_t frames = snd_pcm_readi(handle, buffer.data(), params_.framesPerBuffer);
            if (frames == -EPIPE) {
                snd_pcm_prepare(handle);
                continue;
            }
            if (frames < 0) {
                continue;
            }
            if (callback_) {
                callback_(buffer.data(), static_cast<size_t>(frames), params_.sampleRate);
            }
        }

        snd_pcm_drop(handle);
        snd_pcm_close(handle);
    }

    std::string deviceId_;
    AudioDeviceInfo info_;
    AudioStreamParameters params_;
    AudioInputCallback callback_;
    std::atomic<bool> running_{false};
    std::thread worker_;
};

class AlsaAudioBackend final : public IAudioBackend {
public:
    std::vector<AudioDeviceInfo> enumerateInputDevices() const override {
        std::vector<AudioDeviceInfo> devices;
        void** hints = nullptr;
        if (snd_device_name_hint(-1, "pcm", &hints) == 0 && hints) {
            for (void** hint = hints; *hint != nullptr; ++hint) {
                char* io = snd_device_name_get_hint(*hint, "IOID");
                if (io && std::strcmp(io, "Output") == 0) {
                    free(io);
                    continue;
                }
                char* name = snd_device_name_get_hint(*hint, "NAME");
                if (!name) {
                    if (io) {
                        free(io);
                    }
                    continue;
                }
                AudioDeviceInfo info;
                info.id = name;
                char* desc = snd_device_name_get_hint(*hint, "DESC");
                if (desc) {
                    info.name = desc;
                    free(desc);
                } else {
                    info.name = name;
                }
                info.channels = 1;
                info.sampleRate = 48000.0;
                info.isDefault = info.id == "default";
                devices.push_back(std::move(info));
                free(name);
                if (io) {
                    free(io);
                }
            }
            snd_device_name_free_hint(hints);
        }

        if (devices.empty()) {
            AudioDeviceInfo info;
            info.id = "default";
            info.name = "Default ALSA Capture";
            info.channels = 1;
            info.sampleRate = 48000.0;
            info.isDefault = true;
            devices.push_back(std::move(info));
        }
        return devices;
    }

    std::unique_ptr<AudioDevice> createInputDevice(const std::string& deviceId,
                                                   const AudioStreamParameters& params,
                                                   AudioInputCallback callback) override {
        AudioDeviceInfo info;
        info.id = deviceId.empty() ? std::string("default") : deviceId;
        info.name = info.id == "default" ? std::string("Default ALSA Capture") : info.id;
        info.channels = params.channels;
        info.sampleRate = params.sampleRate;
        info.isDefault = info.id == "default";
        return std::make_unique<AlsaAudioDevice>(info.id, std::move(info), params, std::move(callback));
    }

    std::unique_ptr<AudioDevice> createDefaultInputDevice(const AudioStreamParameters& params,
                                                          AudioInputCallback callback) override {
        return createInputDevice("default", params, std::move(callback));
    }
};

}  // namespace

std::unique_ptr<IAudioBackend> createAlsaBackend() {
    return std::make_unique<AlsaAudioBackend>();
}

}  // namespace yvc::audio

#else

namespace yvc::audio {
std::unique_ptr<IAudioBackend> createAlsaBackend() { return {}; }
}  // namespace yvc::audio

#endif
