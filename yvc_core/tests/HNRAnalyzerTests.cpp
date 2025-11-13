#include <gtest/gtest.h>

#include <yvc_core/HNRAnalyzer.h>
#include "TestSignalHelpers.h"

#include <cmath>
#include <random>
#include <vector>

namespace yvc::test {

class HNRAnalyzerTests : public ::testing::Test {
protected:
    void SetUp() override {
        config_.sample_rate = 48000;
        config_.buffer_size = 2048;
        config_.hop_size = 480;
    }

    AudioConfig config_;
    
    // Helper to add white noise to a signal
    std::vector<Sample> addNoise(const std::vector<Sample>& signal, float noise_level) {
        std::vector<Sample> noisy = signal;
        std::mt19937 gen(12345); // Fixed seed for reproducibility
        std::uniform_real_distribution<float> dist(-noise_level, noise_level);
        
        for (auto& sample : noisy) {
            sample += dist(gen);
        }
        return noisy;
    }
};

TEST_F(HNRAnalyzerTests, PureToneHasHighHNR) {
    HNRAnalyzer analyzer(config_);
    
    const float f0 = 200.0f;
    auto samples = generateSineWave(f0, 4096, config_.sample_rate);
    
    float hnr = analyzer.analyze(samples.data(), samples.size(), f0);
    
    // Pure sine wave should have reasonably high HNR (implementation dependent)
    // Just check it's above noise floor
    EXPECT_GT(hnr, 5.0f);
}

TEST_F(HNRAnalyzerTests, NoisySignalHasLowerHNR) {
    HNRAnalyzer analyzer(config_);
    
    const float f0 = 200.0f;
    auto clean = generateSineWave(f0, 4096, config_.sample_rate);
    auto noisy = addNoise(clean, 0.2f); // Add 20% noise
    
    float hnr_clean = analyzer.analyze(clean.data(), clean.size(), f0);
    float hnr_noisy = analyzer.analyze(noisy.data(), noisy.size(), f0);
    
    // Noisy signal should have lower HNR than clean
    EXPECT_LT(hnr_noisy, hnr_clean);
    EXPECT_GT(hnr_noisy, 0.0f); // Should still be positive
}

TEST_F(HNRAnalyzerTests, WhiteNoiseHasNegativeOrLowHNR) {
    HNRAnalyzer analyzer(config_);
    
    std::vector<Sample> noise(4096);
    std::mt19937 gen(54321);
    std::normal_distribution<float> dist(0.0f, 0.3f);
    
    for (auto& sample : noise) {
        sample = dist(gen);
    }
    
    float hnr = analyzer.analyze(noise.data(), noise.size(), 200.0f);
    
    // Pure noise should have very low or negative HNR
    EXPECT_LT(hnr, 5.0f);
}

TEST_F(HNRAnalyzerTests, AnalyzesDifferentF0Values) {
    HNRAnalyzer analyzer(config_);
    
    const float f0_low = 100.0f;
    const float f0_high = 300.0f;
    
    auto samples_low = generateSineWave(f0_low, 4096, config_.sample_rate);
    auto samples_high = generateSineWave(f0_high, 4096, config_.sample_rate);
    
    float hnr_low = analyzer.analyze(samples_low.data(), samples_low.size(), f0_low);
    float hnr_high = analyzer.analyze(samples_high.data(), samples_high.size(), f0_high);
    
    // Both should have reasonable HNR for pure tones
    // Values depend on implementation
    EXPECT_GT(hnr_low, 0.0f);
    EXPECT_GT(hnr_high, 0.0f);
}

TEST_F(HNRAnalyzerTests, HandlesZeroF0) {
    HNRAnalyzer analyzer(config_);
    
    auto samples = generateSineWave(200.0f, 2048, config_.sample_rate);
    
    float hnr = analyzer.analyze(samples.data(), samples.size(), 0.0f);
    
    // Should handle gracefully (may return 0 or very low value)
    EXPECT_GE(hnr, -50.0f); // Reasonable lower bound
}

TEST_F(HNRAnalyzerTests, HandlesSilence) {
    HNRAnalyzer analyzer(config_);
    
    std::vector<Sample> silence(2048, 0.0f);
    
    float hnr = analyzer.analyze(silence.data(), silence.size(), 200.0f);
    
    // Silence should produce low or zero HNR
    EXPECT_LE(hnr, 5.0f);
}

TEST_F(HNRAnalyzerTests, IncreasingNoiseDecreasesHNR) {
    HNRAnalyzer analyzer(config_);
    
    const float f0 = 200.0f;
    auto clean = generateSineWave(f0, 4096, config_.sample_rate);
    
    auto noisy_low = addNoise(clean, 0.05f);
    auto noisy_med = addNoise(clean, 0.15f);
    auto noisy_high = addNoise(clean, 0.30f);
    
    float hnr_low = analyzer.analyze(noisy_low.data(), noisy_low.size(), f0);
    float hnr_med = analyzer.analyze(noisy_med.data(), noisy_med.size(), f0);
    float hnr_high = analyzer.analyze(noisy_high.data(), noisy_high.size(), f0);
    
    // More noise should progressively decrease HNR
    EXPECT_GT(hnr_low, hnr_med);
    EXPECT_GT(hnr_med, hnr_high);
}

TEST_F(HNRAnalyzerTests, ComplexHarmonicSignalHasGoodHNR) {
    HNRAnalyzer analyzer(config_);
    
    const float f0 = 150.0f;
    std::vector<Sample> harmonic(4096, 0.0f);
    
    // Generate signal with fundamental and harmonics
    auto fundamental = generateSineWave(f0, 4096, config_.sample_rate, 1.0f);
    auto harmonic2 = generateSineWave(f0 * 2.0f, 4096, config_.sample_rate, 0.5f);
    auto harmonic3 = generateSineWave(f0 * 3.0f, 4096, config_.sample_rate, 0.25f);
    
    for (size_t i = 0; i < harmonic.size(); ++i) {
        harmonic[i] = fundamental[i] + harmonic2[i] + harmonic3[i];
    }
    
    float hnr = analyzer.analyze(harmonic.data(), harmonic.size(), f0);
    
    // Complex harmonic signal should still have good HNR
    EXPECT_GT(hnr, 10.0f);
}

} // namespace yvc::test
