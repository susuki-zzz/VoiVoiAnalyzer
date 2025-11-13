#include <gtest/gtest.h>

#include <yvc_core/LevelAnalyzer.h>
#include "TestSignalHelpers.h"

#include <cmath>
#include <vector>

namespace yvc::test {

class LevelAnalyzerTests : public ::testing::Test {
protected:
    void SetUp() override {
        config_.sample_rate = 48000;
        config_.buffer_size = 2048;
        config_.hop_size = 480;
    }

    AudioConfig config_;
};

TEST_F(LevelAnalyzerTests, AnalyzesSilence) {
    LevelAnalyzer analyzer(config_);
    
    std::vector<Sample> silence(2048, 0.0f);
    auto results = analyzer.analyze(silence.data(), silence.size());
    
    EXPECT_FLOAT_EQ(results.rms, 0.0f);
    EXPECT_FLOAT_EQ(results.peak, 0.0f);
    EXPECT_FLOAT_EQ(results.crest_factor, 0.0f);
}

TEST_F(LevelAnalyzerTests, AnalyzesConstantSignal) {
    LevelAnalyzer analyzer(config_);
    
    const float amplitude = 0.5f;
    std::vector<Sample> constant(2048, amplitude);
    auto results = analyzer.analyze(constant.data(), constant.size());
    
    EXPECT_FLOAT_EQ(results.rms, amplitude);
    EXPECT_FLOAT_EQ(results.peak, amplitude);
    EXPECT_FLOAT_EQ(results.crest_factor, 1.0f); // Peak/RMS for DC
}

TEST_F(LevelAnalyzerTests, AnalyzesSineWave) {
    LevelAnalyzer analyzer(config_);
    
    const float amplitude = 1.0f;
    auto samples = generateSineWave(440.0f, 2048, config_.sample_rate, amplitude);
    auto results = analyzer.analyze(samples.data(), samples.size());
    
    // RMS of sine wave is amplitude / sqrt(2)
    const float expected_rms = amplitude / std::sqrt(2.0f);
    EXPECT_TRUE(approximatelyEqual(results.rms, expected_rms, 0.01f));
    
    // Peak should be close to amplitude
    EXPECT_TRUE(approximatelyEqual(results.peak, amplitude, 0.01f));
    
    // Crest factor for sine wave is sqrt(2) ≈ 1.414
    const float expected_crest = std::sqrt(2.0f);
    EXPECT_TRUE(approximatelyEqual(results.crest_factor, expected_crest, 0.05f));
}

TEST_F(LevelAnalyzerTests, AnalyzesImpulseSignal) {
    LevelAnalyzer analyzer(config_);
    
    std::vector<Sample> impulse(2048, 0.0f);
    impulse[1024] = 1.0f; // Single peak
    
    auto results = analyzer.analyze(impulse.data(), impulse.size());
    
    EXPECT_FLOAT_EQ(results.peak, 1.0f);
    // RMS will be very small
    EXPECT_LT(results.rms, 0.1f);
    // Crest factor will be high
    EXPECT_GT(results.crest_factor, 10.0f);
}

TEST_F(LevelAnalyzerTests, HandlesNegativeValues) {
    LevelAnalyzer analyzer(config_);
    
    auto samples = generateSineWave(440.0f, 2048, config_.sample_rate, -0.8f);
    auto results = analyzer.analyze(samples.data(), samples.size());
    
    // RMS and peak should be positive regardless of polarity
    EXPECT_GT(results.rms, 0.0f);
    EXPECT_GT(results.peak, 0.0f);
    EXPECT_TRUE(approximatelyEqual(results.peak, 0.8f, 0.01f));
}

TEST_F(LevelAnalyzerTests, AnalyzesDifferentAmplitudes) {
    LevelAnalyzer analyzer(config_);
    
    const float amp1 = 0.25f;
    const float amp2 = 0.75f;
    
    auto samples1 = generateSineWave(440.0f, 2048, config_.sample_rate, amp1);
    auto samples2 = generateSineWave(440.0f, 2048, config_.sample_rate, amp2);
    
    auto results1 = analyzer.analyze(samples1.data(), samples1.size());
    auto results2 = analyzer.analyze(samples2.data(), samples2.size());
    
    // Higher amplitude should produce proportionally higher levels
    EXPECT_GT(results2.rms, results1.rms);
    EXPECT_GT(results2.peak, results1.peak);
    
    // Crest factor should be similar for same waveform
    EXPECT_TRUE(approximatelyEqual(results1.crest_factor, results2.crest_factor, 0.1f));
}

TEST_F(LevelAnalyzerTests, HandlesSmallBuffers) {
    LevelAnalyzer analyzer(config_);
    
    auto samples = generateSineWave(440.0f, 64, config_.sample_rate);
    auto results = analyzer.analyze(samples.data(), samples.size());
    
    // Should still produce valid results
    EXPECT_GT(results.rms, 0.0f);
    EXPECT_GT(results.peak, 0.0f);
    EXPECT_GT(results.crest_factor, 0.0f);
}

TEST_F(LevelAnalyzerTests, HandlesMixedSignal) {
    LevelAnalyzer analyzer(config_);
    
    std::vector<Sample> mixed(2048, 0.0f);
    
    // Mix two frequencies
    auto sine1 = generateSineWave(200.0f, 2048, config_.sample_rate, 0.5f);
    auto sine2 = generateSineWave(600.0f, 2048, config_.sample_rate, 0.3f);
    
    for (size_t i = 0; i < mixed.size(); ++i) {
        mixed[i] = sine1[i] + sine2[i];
    }
    
    auto results = analyzer.analyze(mixed.data(), mixed.size());
    
    // Mixed signal should have measurable levels
    EXPECT_GT(results.rms, 0.0f);
    EXPECT_GT(results.peak, 0.0f);
    EXPECT_GT(results.crest_factor, 1.0f);
}

} // namespace yvc::test
