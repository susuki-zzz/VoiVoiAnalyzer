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

bool hasWasapiBackend() {
#if YVC_CORE_HAVE_WASAPI
    return true;
#else
    return false;
#endif
}

}  // namespace yvc::audio
