#include "yvc_core/AudioBackend.h"

#include <gtest/gtest.h>

namespace {

template <typename Backend>
void expectDevices(Backend& backend) {
    const auto devices = backend.enumerateInputDevices();
    EXPECT_FALSE(devices.empty());
    for (const auto& info : devices) {
        EXPECT_FALSE(info.id.empty());
        EXPECT_FALSE(info.name.empty());
        EXPECT_GE(info.channels, 1u);
        EXPECT_GT(info.sampleRate, 0.0);
    }
}

}  // namespace

TEST(AudioBackend, PlatformBackendFactory) {
#if YVC_CORE_HAVE_COREAUDIO || YVC_CORE_HAVE_ALSA
    auto backend = yvc::audio::createPlatformBackend();
    ASSERT_NE(backend, nullptr);
    expectDevices(*backend);
#else
    EXPECT_EQ(nullptr, yvc::audio::createPlatformBackend());
#endif
}
