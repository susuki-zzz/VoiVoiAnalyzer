#include <gtest/gtest.h>

#include "TestSignalHelpers.h"

#include <yvc_core/AnalyzerEngine.h>
#include <yvc_core/CPPAnalyzer.h>
#include <yvc_core/F0Detector.h>
#include <yvc_core/HNRAnalyzer.h>
#include <yvc_core/LevelAnalyzer.h>
#include <yvc_core/MetricsBus.h>
#include <yvc_core/PerformanceMode.h>
#include <yvc_core/SpectralAnalyzer.h>
#include <yvc_core/VADAnalyzer.h>

namespace yvc::test {
namespace {

class AnalyzerEngineFixture : public ::testing::TestWithParam<PerformanceMode> {
protected:
    void SetUp() override {
        config_.sample_rate = SAMPLE_RATE_48K;
        engine_ = std::make_unique<AnalyzerEngine>(config_, bus_, GetParam());
    }

    void TearDown() override {
        AnalysisResults results{};
        while (bus_.read(results)) {
        }
    }

    AudioConfig config_{};
    MetricsBus bus_{};
    std::unique_ptr<AnalyzerEngine> engine_;
};

TEST_P(AnalyzerEngineFixture, PublishesConsistentMetricsForSineWave) {
    const auto& engine_config = engine_->getConfig();
    const size_t frame_samples = engine_config.fft_size;
    auto sine = generateSineWave(440.0f, frame_samples, engine_config.sample_rate);

    engine_->process(sine.data(), sine.size(), 0.0);

    AnalysisResults results{};
    ASSERT_TRUE(bus_.read(results));

    F0Detector direct_f0(engine_config);
    bool f0_valid = false;
    const float expected_f0 = direct_f0.detect(sine.data(), frame_samples, f0_valid);
    EXPECT_EQ(results.f0_valid, f0_valid);
    EXPECT_TRUE(approximatelyEqual(results.f0, expected_f0, 1e-3f));

    LevelAnalyzer level(engine_config);
    const auto expected_level = level.analyze(sine.data(), frame_samples);
    EXPECT_TRUE(approximatelyEqual(results.rms, expected_level.rms, 1e-4f));
    EXPECT_TRUE(approximatelyEqual(results.peak, expected_level.peak, 1e-4f));
    EXPECT_TRUE(approximatelyEqual(results.crest_factor, expected_level.crest_factor, 1e-3f));

    CPPAnalyzer cpp(engine_config);
    const float expected_cpp = cpp.analyze(sine.data(), frame_samples);
    EXPECT_TRUE(approximatelyEqual(results.cpp, expected_cpp, 1e-3f));

    if (results.f0_valid) {
        HNRAnalyzer hnr(engine_config);
        const float expected_hnr = hnr.analyze(sine.data(), frame_samples, expected_f0);
        EXPECT_TRUE(approximatelyEqual(results.hnr, expected_hnr, 1e-3f));
    }

    SpectralAnalyzer spectral(engine_config);
    const auto expected_spectral = spectral.analyze(sine.data(), frame_samples);
    EXPECT_TRUE(approximatelyEqual(results.spectral_tilt, expected_spectral.spectral_tilt, 1e-3f));
    EXPECT_TRUE(approximatelyEqual(results.s_centroid, expected_spectral.s_centroid, 1e-3f));
    EXPECT_EQ(results.s_detected, expected_spectral.s_detected);

    VADAnalyzer vad(engine_config);
    const auto expected_vad = vad.analyze(sine.data(), frame_samples, expected_level.rms);
    EXPECT_EQ(results.voice_active, expected_vad.voice_active);
    EXPECT_TRUE(approximatelyEqual(results.speech_rate, expected_vad.speech_rate, 1e-3f));
    EXPECT_TRUE(approximatelyEqual(results.pause_ratio, expected_vad.pause_ratio, 1e-3f));

    EXPECT_FALSE(bus_.hasNewData());
    const auto history = bus_.getHistory();
    ASSERT_FALSE(history.empty());
    EXPECT_TRUE(approximatelyEqual(history.back().rms, results.rms, 1e-6f));
}

TEST_P(AnalyzerEngineFixture, MaintainsHistoryAcrossMultipleBlocks) {
    const auto fft_size = static_cast<size_t>(engine_->getFFTSize());
    const auto hop_size = static_cast<size_t>(engine_->getHopSize());

    std::vector<Sample> block(fft_size + hop_size, 0.0f);
    const double start_timestamp = 0.25;

    engine_->process(block.data(), block.size(), start_timestamp);

    const auto history = bus_.getHistory();
    ASSERT_EQ(history.size(), 2u);

    const double sample_rate = static_cast<double>(engine_->getConfig().sample_rate);
    const double hop_duration = static_cast<double>(hop_size) / sample_rate;

    EXPECT_TRUE(approximatelyEqual(history.front().timestamp, start_timestamp, 1e-6));
    EXPECT_TRUE(approximatelyEqual(history.back().timestamp, start_timestamp + hop_duration, 1e-6));
}

TEST(AnalyzerEngineVADTest, SustainedSpeechRateRemainsInExpectedRange) {
    AudioConfig config;
    config.sample_rate = SAMPLE_RATE_48K;

    VADAnalyzer analyzer(config);
    const size_t frame_samples = static_cast<size_t>(0.1 * static_cast<double>(config.sample_rate));

    auto sequence = generateAlternatingFrames(frame_samples, 40);
    VADAnalyzer::VADResults results{};
    for (size_t offset = 0; offset < sequence.size(); offset += frame_samples) {
        results = analyzer.analyze(sequence.data() + offset, frame_samples, offset % (2 * frame_samples) ? 0.0f : 0.05f);
    }

    EXPECT_GE(results.speech_rate, 3.0f);
    EXPECT_LE(results.speech_rate, 6.0f);
}

} // namespace
} // namespace yvc::test

INSTANTIATE_TEST_SUITE_P(AllPerformanceModes,
                         yvc::test::AnalyzerEngineFixture,
                         ::testing::Values(yvc::PerformanceMode::Light,
                                           yvc::PerformanceMode::Standard,
                                           yvc::PerformanceMode::Diagnostic));
