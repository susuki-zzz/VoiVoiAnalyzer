#include <gtest/gtest.h>

#include <yvc_core/VADAnalyzer.h>

#include "TestSignalHelpers.h"

#include <vector>

namespace yvc::test {
namespace {

TEST(VADAnalyzerWindowTests, SlidingWindowForgetsStaleActivity) {
    AudioConfig config;
    config.sample_rate = SAMPLE_RATE_48K;

    VADAnalyzer analyzer(config);
    const size_t frame_samples = static_cast<size_t>(0.1 * static_cast<double>(config.sample_rate));
    std::vector<Sample> frame(frame_samples, 0.0f);

    VADAnalyzer::VADResults active_results{};
    for (size_t frame_index = 0; frame_index < 120; ++frame_index) {
        const bool speaking_frame = (frame_index % 2) == 0;
        const float rms = speaking_frame ? 0.05f : 0.0f;
        active_results = analyzer.analyze(frame.data(), frame.size(), rms);
    }

    EXPECT_NEAR(active_results.pause_ratio, 0.5f, 0.05f);
    EXPECT_NEAR(active_results.speech_rate, 5.0f, 0.5f);

    VADAnalyzer::VADResults cooled_results{};
    for (size_t frame_index = 0; frame_index < 60; ++frame_index) {
        cooled_results = analyzer.analyze(frame.data(), frame.size(), 0.0f);
    }

    EXPECT_FALSE(cooled_results.voice_active);
    EXPECT_LT(cooled_results.speech_rate, 0.5f);
    EXPECT_GT(cooled_results.pause_ratio, 0.6f);
}

TEST(VADAnalyzerWindowTests, ResetClearsHistoryAndTiming) {
    AudioConfig config;
    config.sample_rate = SAMPLE_RATE_48K;

    VADAnalyzer analyzer(config);
    const size_t frame_samples = static_cast<size_t>(0.05 * static_cast<double>(config.sample_rate));
    std::vector<Sample> frame(frame_samples, 0.0f);

    VADAnalyzer::VADResults results{};
    for (size_t frame_index = 0; frame_index < 80; ++frame_index) {
        const float rms = (frame_index % 3 == 0) ? 0.08f : 0.0f;
        results = analyzer.analyze(frame.data(), frame.size(), rms);
    }

    EXPECT_GT(results.speech_rate, 0.0f);
    EXPECT_GT(results.pause_ratio, 0.0f);

    analyzer.reset();
    auto reset_results = analyzer.analyze(frame.data(), frame.size(), 0.05f);

    EXPECT_TRUE(reset_results.voice_active);
    EXPECT_FLOAT_EQ(reset_results.speech_rate, 0.0f);
    EXPECT_FLOAT_EQ(reset_results.pause_ratio, 0.0f);
}

} // namespace
} // namespace yvc::test
