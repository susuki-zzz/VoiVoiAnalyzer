#include <gtest/gtest.h>
#include "TestSignalHelpers.h"
#include <cmath>

namespace yvc::test {

// Demonstration tests for the new helper functions
class TestSignalHelpersDemo : public ::testing::Test {
protected:
    void SetUp() override {
        sample_rate_ = 48000;
    }
    
    SampleRate sample_rate_;
};

TEST_F(TestSignalHelpersDemo, TriangleWaveGeneratesCorrectly) {
    auto signal = generateTriangleWave(100.0f, 480, sample_rate_);
    
    EXPECT_EQ(signal.size(), 480u);
    // Triangle wave should oscillate between -1 and +1
    float peak = calculatePeak(signal);
    EXPECT_TRUE(approximatelyEqual(peak, 1.0f, 0.1f));
}

TEST_F(TestSignalHelpersDemo, WhiteNoiseGeneratesRandomValues) {
    auto noise1 = generateWhiteNoise(1000, 0.5f, 42);
    auto noise2 = generateWhiteNoise(1000, 0.5f, 123);
    
    EXPECT_EQ(noise1.size(), 1000u);
    EXPECT_EQ(noise2.size(), 1000u);
    
    // Different seeds should produce different noise
    EXPECT_FALSE(signalsEqual(noise1, noise2, 0.001f));
    
    // RMS should be around the amplitude
    float rms = calculateRMS(noise1);
    EXPECT_GT(rms, 0.3f);
    EXPECT_LT(rms, 0.7f);
}

TEST_F(TestSignalHelpersDemo, PinkNoiseHasLowerHighFrequency) {
    auto pink = generatePinkNoise(4800, 1.0f);
    
    EXPECT_EQ(pink.size(), 4800u);
    // Pink noise should have finite values
    for (const auto& sample : pink) {
        EXPECT_TRUE(std::isfinite(sample));
    }
}

TEST_F(TestSignalHelpersDemo, ChirpSweepsFrequency) {
    auto chirp = generateChirp(100.0f, 1000.0f, 4800, sample_rate_);
    
    EXPECT_EQ(chirp.size(), 4800u);
    // Chirp should have varying instantaneous frequency
    EXPECT_TRUE(std::isfinite(calculatePeak(chirp)));
}

TEST_F(TestSignalHelpersDemo, HarmonicToneContainsMultipleFrequencies) {
    std::vector<float> harmonics = {1.0f, 0.5f, 0.25f}; // Fundamental + 2 harmonics
    auto signal = generateHarmonicTone(200.0f, 2400, sample_rate_, harmonics);
    
    EXPECT_EQ(signal.size(), 2400u);
    // Should be louder than a single sine wave
    float peak = calculatePeak(signal);
    EXPECT_GT(peak, 1.0f);
}

TEST_F(TestSignalHelpersDemo, ImpulseIsCorrectlyPlaced) {
    auto impulse = generateImpulse(1000, 500, 1.5f);
    
    EXPECT_EQ(impulse.size(), 1000u);
    EXPECT_FLOAT_EQ(impulse[500], 1.5f);
    EXPECT_FLOAT_EQ(impulse[499], 0.0f);
    EXPECT_FLOAT_EQ(impulse[501], 0.0f);
}

TEST_F(TestSignalHelpersDemo, ImpulseTrainHasRegularSpacing) {
    auto train = generateImpulseTrain(1000, 100, 0.8f);
    
    EXPECT_EQ(train.size(), 1000u);
    EXPECT_FLOAT_EQ(train[0], 0.8f);
    EXPECT_FLOAT_EQ(train[100], 0.8f);
    EXPECT_FLOAT_EQ(train[200], 0.8f);
    EXPECT_FLOAT_EQ(train[50], 0.0f);
}

TEST_F(TestSignalHelpersDemo, ADSREnvelopeShapesSignal) {
    auto signal = generateSineWave(440.0f, 4800, sample_rate_, 1.0f);
    applyADSR(signal, sample_rate_, 10.0f, 20.0f, 0.7f, 30.0f);
    
    // Signal should start at 0 (attack)
    EXPECT_LT(std::abs(signal[0]), 0.1f);
    
    // Should reach near peak during attack/decay
    float max_val = 0.0f;
    for (size_t i = 0; i < 480; ++i) { // First 10ms
        max_val = std::max(max_val, std::abs(signal[i]));
    }
    EXPECT_GT(max_val, 0.0f);
}

TEST_F(TestSignalHelpersDemo, FadeInGraduallyIncreasesVolume) {
    auto signal = generateSineWave(440.0f, 1000, sample_rate_, 1.0f);
    applyFadeIn(signal, 500);
    
    // Should start at 0
    EXPECT_FLOAT_EQ(signal[0], 0.0f);
    
    // Should gradually increase
    EXPECT_GT(std::abs(signal[250]), std::abs(signal[100]));
}

TEST_F(TestSignalHelpersDemo, FadeOutGraduallyDecreasesVolume) {
    auto signal = generateSineWave(440.0f, 1000, sample_rate_, 1.0f);
    applyFadeOut(signal, 500);
    
    // Should end near 0
    EXPECT_LT(std::abs(signal[999]), 0.1f);
    
    // Should gradually decrease
    EXPECT_LT(std::abs(signal[750]), std::abs(signal[500]));
}

TEST_F(TestSignalHelpersDemo, MixSignalsCombinesTwoSignals) {
    auto sine1 = generateSineWave(200.0f, 1000, sample_rate_, 0.5f);
    auto sine2 = generateSineWave(400.0f, 1000, sample_rate_, 0.5f);
    
    auto mixed = mixSignals(sine1, sine2, 1.0f, 1.0f);
    
    EXPECT_EQ(mixed.size(), 1000u);
    // Mixed signal should have higher peak than individual components
    EXPECT_GT(calculatePeak(mixed), calculatePeak(sine1));
}

TEST_F(TestSignalHelpersDemo, AddNoiseIncreasesSignalLevel) {
    auto clean = generateSineWave(440.0f, 1000, sample_rate_, 0.5f);
    auto noisy = addNoise(clean, 0.1f);
    
    EXPECT_EQ(noisy.size(), clean.size());
    // Noisy signal should differ from clean
    EXPECT_FALSE(signalsEqual(clean, noisy, 0.001f));
}

TEST_F(TestSignalHelpersDemo, NormalizeSignalScalesToTarget) {
    auto signal = generateSineWave(440.0f, 1000, sample_rate_, 0.3f);
    normalizeSignal(signal, 1.0f);
    
    float peak = calculatePeak(signal);
    EXPECT_TRUE(approximatelyEqual(peak, 1.0f, 0.01f));
}

TEST_F(TestSignalHelpersDemo, RMSCalculationIsAccurate) {
    auto signal = generateSineWave(440.0f, 2000, sample_rate_, 1.0f);
    float rms = calculateRMS(signal);
    
    // RMS of sine wave should be 1/sqrt(2)
    float expected = 1.0f / std::sqrt(2.0f);
    EXPECT_TRUE(approximatelyEqual(rms, expected, 0.01f));
}

TEST_F(TestSignalHelpersDemo, InRangeCheckWorks) {
    EXPECT_TRUE(inRange(5.0f, 0.0f, 10.0f));
    EXPECT_TRUE(inRange(0.0f, 0.0f, 10.0f));
    EXPECT_TRUE(inRange(10.0f, 0.0f, 10.0f));
    EXPECT_FALSE(inRange(-1.0f, 0.0f, 10.0f));
    EXPECT_FALSE(inRange(11.0f, 0.0f, 10.0f));
}

TEST_F(TestSignalHelpersDemo, SignalsEqualDetectsIdenticalSignals) {
    auto signal1 = generateSineWave(440.0f, 1000, sample_rate_);
    auto signal2 = generateSineWave(440.0f, 1000, sample_rate_);
    
    EXPECT_TRUE(signalsEqual(signal1, signal2, 1e-6f));
}

TEST_F(TestSignalHelpersDemo, SignalsEqualDetectsDifferentSignals) {
    auto signal1 = generateSineWave(440.0f, 1000, sample_rate_);
    auto signal2 = generateSineWave(880.0f, 1000, sample_rate_);
    
    EXPECT_FALSE(signalsEqual(signal1, signal2, 1e-6f));
}

} // namespace yvc::test
