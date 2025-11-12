#include <gtest/gtest.h>

#include <yvc_core/AudioBuffer.h>

#include "TestSignalHelpers.h"

#include <array>

namespace yvc::test {
namespace {

class AudioBufferFixture : public ::testing::Test {
protected:
    AudioBufferFixture() {
        config.sample_rate = SAMPLE_RATE_48K;
        config.buffer_size = 256;
    }

    AudioConfig config{};
};

TEST_F(AudioBufferFixture, AddsAndConsumesSamples) {
    AudioBuffer buffer(config);
    std::array<Sample, 128> samples{};
    samples.fill(0.1f);

    buffer.addSamples(samples.data(), samples.size());
    EXPECT_TRUE(buffer.hasEnoughSamples(64));

    buffer.consumeSamples(64);
    EXPECT_TRUE(buffer.hasEnoughSamples(64));

    buffer.consumeSamples(64);
    EXPECT_FALSE(buffer.hasEnoughSamples(1));
}

TEST_F(AudioBufferFixture, EnforcesMaximumCapacity) {
    AudioBuffer buffer(config);
    const auto sine = generateSineWave(220.0f, static_cast<size_t>(config.sample_rate * 3), config.sample_rate);

    buffer.addSamples(sine.data(), sine.size());
    EXPECT_LE(buffer.getSize(), static_cast<size_t>(config.sample_rate * 2));
}

TEST_F(AudioBufferFixture, TracksLatencyCompensationPerTarget) {
    AudioBuffer buffer(config);
    buffer.setLatencyCompensation(PreprocessTarget::Monitor, 128);
    buffer.setLatencyCompensation(PreprocessTarget::Record, 256);

    EXPECT_EQ(buffer.getLatencyCompensation(PreprocessTarget::Monitor), 128u);
    EXPECT_EQ(buffer.getLatencyCompensation(PreprocessTarget::Record), 256u);

    buffer.clearLatencyCompensation();
    EXPECT_EQ(buffer.getLatencyCompensation(PreprocessTarget::Monitor), 0u);
}

} // namespace
} // namespace yvc::test
