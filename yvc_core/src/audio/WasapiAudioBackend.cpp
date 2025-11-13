// VoiVoi Core Library - WASAPI Audio Backend
// License: MIT
// Purpose: Windows Audio Session API (WASAPI) implementation for low-latency audio input

#include "yvc_core/AudioBackend.h"
#include "yvc_core/Logger.h"

#ifdef _WIN32

#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <functiondiscoverykeys_devpkey.h>
#include <combaseapi.h>

#include <algorithm>
#include <thread>
#include <atomic>
#include <vector>

#pragma comment(lib, "ole32.lib")

namespace yvc::audio {

namespace {

class ComInitializer {
public:
    ComInitializer() {
        hr_ = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        initialized_ = SUCCEEDED(hr_);
    }

    ~ComInitializer() {
        if (initialized_) {
            CoUninitialize();
        }
    }

    bool isInitialized() const { return initialized_; }
    HRESULT hr() const { return hr_; }

private:
    HRESULT hr_;
    bool initialized_;
};

std::string wideToUtf8(const wchar_t* wide) {
    if (!wide) return {};
    int size = WideCharToMultiByte(CP_UTF8, 0, wide, -1, nullptr, 0, nullptr, nullptr);
    if (size <= 0) return {};
    std::string result(size - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wide, -1, &result[0], size, nullptr, nullptr);
    return result;
}

} // namespace

class WasapiAudioDevice : public AudioDevice {
public:
    WasapiAudioDevice(
        IMMDevice* device,
        const AudioDeviceInfo& info,
        const AudioStreamParameters& params,
        AudioInputCallback callback)
        : device_(device)
        , info_(info)
        , params_(params)
        , callback_(std::move(callback))
        , running_(false)
        , stop_event_(nullptr) {
        
        device_->AddRef();
    }

    ~WasapiAudioDevice() override {
        stop();
        if (device_) {
            device_->Release();
        }
        if (stop_event_) {
            CloseHandle(stop_event_);
        }
    }

    void start() override {
        if (running_.load()) {
            return;
        }

        HRESULT hr = device_->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr,
                                       reinterpret_cast<void**>(&audio_client_));
        if (FAILED(hr)) {
            LOG_ERRORF("Failed to activate audio client: 0x%08lx", hr);
            return;
        }

        // Get mix format and negotiate sample rate
        WAVEFORMATEX* mix_format = nullptr;
        hr = audio_client_->GetMixFormat(&mix_format);
        if (FAILED(hr)) {
            LOG_ERRORF("Failed to get mix format: 0x%08lx", hr);
            audio_client_->Release();
            audio_client_ = nullptr;
            return;
        }

        // Prefer 48kHz, fallback to device default
        WAVEFORMATEX desired_format = *mix_format;
        desired_format.nSamplesPerSec = static_cast<DWORD>(params_.sampleRate);
        desired_format.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
        desired_format.wBitsPerSample = 32;
        desired_format.nBlockAlign = static_cast<WORD>((desired_format.nChannels * desired_format.wBitsPerSample) / 8);
        desired_format.nAvgBytesPerSec = desired_format.nSamplesPerSec * desired_format.nBlockAlign;
        desired_format.cbSize = 0;

        WAVEFORMATEX* closest = nullptr;
        hr = audio_client_->IsFormatSupported(AUDCLNT_SHAREMODE_SHARED, &desired_format, &closest);
        
        WAVEFORMATEX* use_format = &desired_format;
        if (hr == S_FALSE && closest) {
            LOG_WARNF("Requested format not supported, using closest: %lu Hz", closest->nSamplesPerSec);
            use_format = closest;
        } else if (FAILED(hr)) {
            LOG_WARN("Desired format not supported, using device default");
            use_format = mix_format;
        }

        actual_sample_rate_ = use_format->nSamplesPerSec;
        actual_channels_ = use_format->nChannels;

        // Initialize audio client with shared mode
        REFERENCE_TIME default_period = 0;
        REFERENCE_TIME min_period = 0;
        audio_client_->GetDevicePeriod(&default_period, &min_period);

        hr = audio_client_->Initialize(
            AUDCLNT_SHAREMODE_SHARED,
            AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
            default_period,
            0,
            use_format,
            nullptr);

        CoTaskMemFree(mix_format);
        if (closest) {
            CoTaskMemFree(closest);
        }

        if (FAILED(hr)) {
            LOG_ERRORF("Failed to initialize audio client: 0x%08lx", hr);
            audio_client_->Release();
            audio_client_ = nullptr;
            return;
        }

        // Get actual buffer size
        UINT32 buffer_frames = 0;
        hr = audio_client_->GetBufferSize(&buffer_frames);
        if (SUCCEEDED(hr)) {
            actual_buffer_size_ = buffer_frames;
            LOG_INFOF("WASAPI buffer size: %u frames", buffer_frames);
        }

        // Create event for buffer ready notification
        stop_event_ = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        hr = audio_client_->SetEventHandle(stop_event_);
        if (FAILED(hr)) {
            LOG_ERRORF("Failed to set event handle: 0x%08lx", hr);
            audio_client_->Release();
            audio_client_ = nullptr;
            return;
        }

        // Get capture client
        hr = audio_client_->GetService(__uuidof(IAudioCaptureClient),
                                       reinterpret_cast<void**>(&capture_client_));
        if (FAILED(hr)) {
            LOG_ERRORF("Failed to get capture client: 0x%08lx", hr);
            audio_client_->Release();
            audio_client_ = nullptr;
            return;
        }

        // Start audio client
        hr = audio_client_->Start();
        if (FAILED(hr)) {
            LOG_ERRORF("Failed to start audio client: 0x%08lx", hr);
            capture_client_->Release();
            capture_client_ = nullptr;
            audio_client_->Release();
            audio_client_ = nullptr;
            return;
        }

        running_.store(true);
        capture_thread_ = std::thread(&WasapiAudioDevice::captureLoop, this);
        LOG_INFO("WASAPI audio device started");
    }

    void stop() override {
        if (!running_.load()) {
            return;
        }

        running_.store(false);
        SetEvent(stop_event_);
        
        if (capture_thread_.joinable()) {
            capture_thread_.join();
        }

        if (audio_client_) {
            audio_client_->Stop();
        }

        if (capture_client_) {
            capture_client_->Release();
            capture_client_ = nullptr;
        }

        if (audio_client_) {
            audio_client_->Release();
            audio_client_ = nullptr;
        }

        LOG_INFO("WASAPI audio device stopped");
    }

    bool isRunning() const override {
        return running_.load();
    }

    const AudioDeviceInfo& info() const override {
        return info_;
    }

    AudioStreamParameters streamParameters() const override {
        AudioStreamParameters actual = params_;
        actual.sampleRate = actual_sample_rate_;
        actual.channels = actual_channels_;
        actual.framesPerBuffer = actual_buffer_size_;
        return actual;
    }

private:
    void captureLoop() {
        ComInitializer com;
        if (!com.isInitialized()) {
            LOG_ERROR("Failed to initialize COM in capture thread");
            return;
        }

        std::vector<float> mono_buffer;
        mono_buffer.reserve(4096);

        while (running_.load()) {
            DWORD wait_result = WaitForSingleObject(stop_event_, 100);
            if (wait_result != WAIT_OBJECT_0 && wait_result != WAIT_TIMEOUT) {
                break;
            }

            if (!running_.load()) {
                break;
            }

            UINT32 packet_length = 0;
            HRESULT hr = capture_client_->GetNextPacketSize(&packet_length);
            if (FAILED(hr)) {
                LOG_ERRORF("GetNextPacketSize failed: 0x%08lx", hr);
                break;
            }

            while (packet_length > 0) {
                BYTE* data = nullptr;
                UINT32 frames_available = 0;
                DWORD flags = 0;

                hr = capture_client_->GetBuffer(&data, &frames_available, &flags, nullptr, nullptr);
                if (FAILED(hr)) {
                    LOG_ERRORF("GetBuffer failed: 0x%08lx", hr);
                    break;
                }

                if (frames_available > 0 && callback_) {
                    const float* samples = reinterpret_cast<const float*>(data);
                    
                    // Convert to mono if needed
                    if (actual_channels_ == 1) {
                        callback_(samples, frames_available, actual_sample_rate_);
                    } else {
                        mono_buffer.resize(frames_available);
                        for (UINT32 i = 0; i < frames_available; ++i) {
                            float sum = 0.0f;
                            for (UINT32 ch = 0; ch < actual_channels_; ++ch) {
                                sum += samples[i * actual_channels_ + ch];
                            }
                            mono_buffer[i] = sum / static_cast<float>(actual_channels_);
                        }
                        callback_(mono_buffer.data(), frames_available, actual_sample_rate_);
                    }
                }

                hr = capture_client_->ReleaseBuffer(frames_available);
                if (FAILED(hr)) {
                    LOG_ERRORF("ReleaseBuffer failed: 0x%08lx", hr);
                    break;
                }

                hr = capture_client_->GetNextPacketSize(&packet_length);
                if (FAILED(hr)) {
                    break;
                }
            }
        }
    }

    IMMDevice* device_;
    IAudioClient* audio_client_ = nullptr;
    IAudioCaptureClient* capture_client_ = nullptr;
    AudioDeviceInfo info_;
    AudioStreamParameters params_;
    AudioInputCallback callback_;
    std::atomic<bool> running_;
    std::thread capture_thread_;
    HANDLE stop_event_;
    uint32_t actual_sample_rate_ = 48000;
    uint32_t actual_channels_ = 1;
    uint32_t actual_buffer_size_ = 512;
};

class WasapiAudioBackend : public IAudioBackend {
public:
    WasapiAudioBackend() {
        com_ = std::make_unique<ComInitializer>();
        if (!com_->isInitialized()) {
            LOG_ERRORF("Failed to initialize COM: 0x%08lx", com_->hr());
        }
    }

    std::vector<AudioDeviceInfo> enumerateInputDevices() const override {
        std::vector<AudioDeviceInfo> devices;
        
        if (!com_->isInitialized()) {
            return devices;
        }

        IMMDeviceEnumerator* enumerator = nullptr;
        HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
                                      __uuidof(IMMDeviceEnumerator),
                                      reinterpret_cast<void**>(&enumerator));
        if (FAILED(hr)) {
            LOG_ERRORF("Failed to create device enumerator: 0x%08lx", hr);
            return devices;
        }

        IMMDeviceCollection* collection = nullptr;
        hr = enumerator->EnumAudioEndpoints(eCapture, DEVICE_STATE_ACTIVE, &collection);
        if (FAILED(hr)) {
            enumerator->Release();
            return devices;
        }

        UINT count = 0;
        collection->GetCount(&count);

        for (UINT i = 0; i < count; ++i) {
            IMMDevice* device = nullptr;
            hr = collection->Item(i, &device);
            if (FAILED(hr)) {
                continue;
            }

            AudioDeviceInfo info = getDeviceInfo(device);
            devices.push_back(info);
            device->Release();
        }

        collection->Release();
        enumerator->Release();

        return devices;
    }

    std::unique_ptr<AudioDevice> createInputDevice(
        const std::string& deviceId,
        const AudioStreamParameters& params,
        AudioInputCallback callback) override {
        
        if (!com_->isInitialized()) {
            return nullptr;
        }

        IMMDeviceEnumerator* enumerator = nullptr;
        HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
                                      __uuidof(IMMDeviceEnumerator),
                                      reinterpret_cast<void**>(&enumerator));
        if (FAILED(hr)) {
            return nullptr;
        }

        std::wstring wide_id(deviceId.begin(), deviceId.end());
        IMMDevice* device = nullptr;
        hr = enumerator->GetDevice(wide_id.c_str(), &device);
        enumerator->Release();

        if (FAILED(hr)) {
            return nullptr;
        }

        AudioDeviceInfo info = getDeviceInfo(device);
        auto audio_device = std::make_unique<WasapiAudioDevice>(device, info, params, std::move(callback));
        device->Release();
        return audio_device;
    }

    std::unique_ptr<AudioDevice> createDefaultInputDevice(
        const AudioStreamParameters& params,
        AudioInputCallback callback) override {
        
        if (!com_->isInitialized()) {
            return nullptr;
        }

        IMMDeviceEnumerator* enumerator = nullptr;
        HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
                                      __uuidof(IMMDeviceEnumerator),
                                      reinterpret_cast<void**>(&enumerator));
        if (FAILED(hr)) {
            return nullptr;
        }

        IMMDevice* device = nullptr;
        hr = enumerator->GetDefaultAudioEndpoint(eCapture, eConsole, &device);
        enumerator->Release();

        if (FAILED(hr)) {
            return nullptr;
        }

        AudioDeviceInfo info = getDeviceInfo(device);
        info.isDefault = true;
        auto audio_device = std::make_unique<WasapiAudioDevice>(device, info, params, std::move(callback));
        device->Release();
        return audio_device;
    }

private:
    AudioDeviceInfo getDeviceInfo(IMMDevice* device) const {
        AudioDeviceInfo info;

        LPWSTR device_id = nullptr;
        if (SUCCEEDED(device->GetId(&device_id))) {
            info.id = wideToUtf8(device_id);
            CoTaskMemFree(device_id);
        }

        IPropertyStore* props = nullptr;
        if (SUCCEEDED(device->OpenPropertyStore(STGM_READ, &props))) {
            PROPVARIANT var_name;
            PropVariantInit(&var_name);
            if (SUCCEEDED(props->GetValue(PKEY_Device_FriendlyName, &var_name))) {
                info.name = wideToUtf8(var_name.pwszVal);
                PropVariantClear(&var_name);
            }
            props->Release();
        }

        return info;
    }

    std::unique_ptr<ComInitializer> com_;
};

std::unique_ptr<IAudioBackend> createWasapiBackend() {
    return std::make_unique<WasapiAudioBackend>();
}

bool hasWasapiBackend() {
    return true;
}

} // namespace yvc::audio

#endif // _WIN32
