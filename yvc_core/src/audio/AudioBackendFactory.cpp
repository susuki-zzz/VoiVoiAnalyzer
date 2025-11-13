#include "yvc_core/AudioBackend.h"

namespace yvc::audio {

std::unique_ptr<IAudioBackend> createPlatformBackend() {
#if YVC_CORE_HAVE_WASAPI
    return createWasapiBackend();
#elif YVC_CORE_HAVE_COREAUDIO
    return createCoreAudioBackend();
#elif YVC_CORE_HAVE_ALSA
    return createAlsaBackend();
#else
    return {};
#endif
}

#if !YVC_CORE_HAVE_WASAPI
std::unique_ptr<IAudioBackend> createWasapiBackend() { return {}; }
bool hasWasapiBackend() { return false; }
#endif

bool hasCoreAudioBackend() {
#if YVC_CORE_HAVE_COREAUDIO
    return true;
#else
    return false;
#endif
}

bool hasAlsaBackend() {
#if YVC_CORE_HAVE_ALSA
    return true;
#else
    return false;
#endif
}

}  // namespace yvc::audio
