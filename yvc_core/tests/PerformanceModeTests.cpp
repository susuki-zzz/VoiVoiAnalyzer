#include <gtest/gtest.h>

#include <yvc_core/PerformanceMode.h>

namespace yvc::test {
namespace {

struct PerformanceExpectation {
    PerformanceMode mode;
    uint32_t fft_size;
    uint32_t hop_size;
    bool cpp_enabled;
};

class PerformanceModeFixture : public ::testing::TestWithParam<PerformanceExpectation> {};

TEST_P(PerformanceModeFixture, ConfiguresAudioSettings) {
    const auto expectation = GetParam();
    AudioConfig base_config{};
    base_config.sample_rate = SAMPLE_RATE_48K;
    base_config.mode = expectation.mode;
    base_config.buffer_size = 64;

    const auto configured = configureForPerformanceMode(base_config);
    EXPECT_EQ(configured.fft_size, expectation.fft_size);
    EXPECT_EQ(configured.hop_size, expectation.hop_size);
    EXPECT_EQ(configured.mode, expectation.mode);
    EXPECT_GE(configured.buffer_size, configured.hop_size);

    PerformanceModeConfig config(expectation.mode);
    EXPECT_EQ(config.isFeatureEnabled("cpp"), expectation.cpp_enabled);
}

INSTANTIATE_TEST_SUITE_P(
    PerformanceModes,
    PerformanceModeFixture,
    ::testing::Values(
        PerformanceExpectation{PerformanceMode::Mode_Light, 1024, 512, false},
        PerformanceExpectation{PerformanceMode::Mode_Standard, 2048, 512, true},
        PerformanceExpectation{PerformanceMode::Mode_Diagnostic, 4096, 1024, true}));

} // namespace
} // namespace yvc::test
