#include <gtest/gtest.h>

#include <yvc_core/VADAnalyzer.h>
#include "TestSignalHelpers.h"

#include <vector>

namespace yvc::test {

class VADAnalyzerTests : public ::testing::Test {
protected:
    void SetUp() override {
        config_.sample_rate = 48000;
        config_.buffer_size = 2048;
        config_.hop_size = 480; // 10ms
    }

    AudioConfig config_;
};

TEST_F(VADAnalyzerTests, DetectsSilenceAsInactive) {
    VADAnalyzer analyzer(config_);
    
    std::vector<Sample> silence(2048, 0.0f);
    auto results = analyzer.analyze(silence.data(), silence.size(), 0.0f);
    
    EXPECT_FALSE(results.voice_active);
}

TEST_F(VADAnalyzerTests, DetectsLoudSignalAsActive) {
    VADAnalyzer analyzer(config_);
    
    auto samples = generateSineWave(200.0f, 2048, config_.sample_rate, 0.5f);
    const float rms = 0.35f; // Approximate RMS for sine wave
    
    auto results = analyzer.analyze(samples.data(), samples.size(), rms);
    
    EXPECT_TRUE(results.voice_active);
}

TEST_F(VADAnalyzerTests, QuietSignalIsInactive) {
    VADAnalyzer analyzer(config_);
    
    auto samples = generateSineWave(200.0f, 2048, config_.sample_rate, 0.001f);
    const float rms = 0.0007f; // Very low RMS
    
    auto results = analyzer.analyze(samples.data(), samples.size(), rms);
    
    EXPECT_FALSE(results.voice_active);
}

TEST_F(VADAnalyzerTests, AlternatingVoicedSilencePatterns) {
    VADAnalyzer analyzer(config_);
    
    const size_t frame_size = 480; // 10ms at 48kHz
    
    // Voiced frame
    auto voiced = generateSineWave(200.0f, frame_size, config_.sample_rate, 0.5f);
    auto results_voiced = analyzer.analyze(voiced.data(), voiced.size(), 0.35f);
    EXPECT_TRUE(results_voiced.voice_active);
    
    // Silent frame
    std::vector<Sample> silence(frame_size, 0.0f);
    auto results_silent = analyzer.analyze(silence.data(), silence.size(), 0.0f);
    EXPECT_FALSE(results_silent.voice_active);
    
    // Voiced frame again
    auto results_voiced2 = analyzer.analyze(voiced.data(), voiced.size(), 0.35f);
    EXPECT_TRUE(results_voiced2.voice_active);
}

TEST_F(VADAnalyzerTests, CalculatesPauseRatio) {
    VADAnalyzer analyzer(config_);
    
    const size_t frame_size = 480;
    auto voiced = generateSineWave(200.0f, frame_size, config_.sample_rate, 0.5f);
    std::vector<Sample> silence(frame_size, 0.0f);
    
    // Process several frames: voiced, silence, voiced, silence
    analyzer.analyze(voiced.data(), voiced.size(), 0.35f);
    analyzer.analyze(silence.data(), silence.size(), 0.0f);
    analyzer.analyze(voiced.data(), voiced.size(), 0.35f);
    auto results = analyzer.analyze(silence.data(), silence.size(), 0.0f);
    
    // Should have some pause ratio
    EXPECT_GE(results.pause_ratio, 0.0f);
    EXPECT_LE(results.pause_ratio, 1.0f);
}

TEST_F(VADAnalyzerTests, ContinuousSpeechHasLowPauseRatio) {
    VADAnalyzer analyzer(config_);
    
    const size_t frame_size = 480;
    auto voiced = generateSineWave(200.0f, frame_size, config_.sample_rate, 0.5f);
    
    // Process many continuous voiced frames
    for (int i = 0; i < 20; ++i) {
        analyzer.analyze(voiced.data(), voiced.size(), 0.35f);
    }
    
    auto results = analyzer.analyze(voiced.data(), voiced.size(), 0.35f);
    
    // Continuous speech should have low pause ratio
    EXPECT_LT(results.pause_ratio, 0.2f);
}

TEST_F(VADAnalyzerTests, MostlySilenceHasHighPauseRatio) {
    VADAnalyzer analyzer(config_);
    
    const size_t frame_size = 480;
    std::vector<Sample> silence(frame_size, 0.0f);
    auto voiced = generateSineWave(200.0f, frame_size, config_.sample_rate, 0.5f);
    
    // Process mostly silent frames with occasional voiced
    for (int i = 0; i < 20; ++i) {
        analyzer.analyze(silence.data(), silence.size(), 0.0f);
    }
    analyzer.analyze(voiced.data(), voiced.size(), 0.35f);
    for (int i = 0; i < 20; ++i) {
        analyzer.analyze(silence.data(), silence.size(), 0.0f);
    }
    
    auto results = analyzer.analyze(silence.data(), silence.size(), 0.0f);
    
    // Mostly silence should have high pause ratio
    EXPECT_GT(results.pause_ratio, 0.7f);
}

TEST_F(VADAnalyzerTests, ResetClearsState) {
    VADAnalyzer analyzer(config_);
    
    const size_t frame_size = 480;
    auto voiced = generateSineWave(200.0f, frame_size, config_.sample_rate, 0.5f);
    
    // Build up some state
    for (int i = 0; i < 10; ++i) {
        analyzer.analyze(voiced.data(), voiced.size(), 0.35f);
    }
    
    // Reset
    analyzer.reset();
    
    // After reset, state should be cleared
    std::vector<Sample> silence(frame_size, 0.0f);
    auto results = analyzer.analyze(silence.data(), silence.size(), 0.0f);
    
    // Reset clears internal state
    EXPECT_FALSE(results.voice_active);
}

TEST_F(VADAnalyzerTests, SpeechRateIsReasonable) {
    VADAnalyzer analyzer(config_);
    
    const size_t frame_size = 480;
    auto voiced = generateSineWave(200.0f, frame_size, config_.sample_rate, 0.5f);
    std::vector<Sample> silence(frame_size, 0.0f);
    
    // Simulate speech pattern: voiced-voiced-silence pattern
    for (int i = 0; i < 10; ++i) {
        analyzer.analyze(voiced.data(), voiced.size(), 0.35f);
        analyzer.analyze(voiced.data(), voiced.size(), 0.35f);
        analyzer.analyze(silence.data(), silence.size(), 0.0f);
    }
    
    auto results = analyzer.analyze(voiced.data(), voiced.size(), 0.35f);
    
    // Speech rate should be non-negative
    // (exact value depends on implementation)
    EXPECT_GE(results.speech_rate, 0.0f);
}

TEST_F(VADAnalyzerTests, VeryQuietSignalBelowThreshold) {
    VADAnalyzer analyzer(config_);
    
    auto samples = generateSineWave(200.0f, 2048, config_.sample_rate, 0.005f);
    const float rms = 0.0035f; // Below typical threshold
    
    auto results = analyzer.analyze(samples.data(), samples.size(), rms);
    
    EXPECT_FALSE(results.voice_active);
}

TEST_F(VADAnalyzerTests, ModerateSignalAboveThreshold) {
    VADAnalyzer analyzer(config_);
    
    auto samples = generateSineWave(200.0f, 2048, config_.sample_rate, 0.3f);
    const float rms = 0.21f; // Above typical threshold
    
    auto results = analyzer.analyze(samples.data(), samples.size(), rms);
    
    EXPECT_TRUE(results.voice_active);
}

} // namespace yvc::test
