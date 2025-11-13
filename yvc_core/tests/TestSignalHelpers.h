#pragma once

#include <vector>
#include <cmath>
#include <numbers>
#include <random>
#include <algorithm>

#include "yvc_core/Types.h"

namespace yvc::test {

// ============================================================================
// Basic Waveform Generators
// ============================================================================

/// Generate a pure sine wave
inline std::vector<Sample> generateSineWave(float frequency, size_t samples, SampleRate sample_rate,
                                            float amplitude = 1.0f) {
    std::vector<Sample> data(samples, 0.0f);
    const float sr = static_cast<float>(sample_rate);
    for (size_t i = 0; i < samples; ++i) {
        const float phase = 2.0f * std::numbers::pi_v<float> * frequency * static_cast<float>(i) / sr;
        data[i] = amplitude * std::sin(phase);
    }
    return data;
}

/// Generate a square wave (rectangular)
inline std::vector<Sample> generateSquareWave(float frequency, size_t samples, SampleRate sample_rate,
                                              float amplitude = 1.0f, float duty_cycle = 0.5f) {
    std::vector<Sample> data(samples, 0.0f);
    const float sr = static_cast<float>(sample_rate);
    const float period = sr / frequency;
    const float high_duration = period * duty_cycle;
    
    for (size_t i = 0; i < samples; ++i) {
        float phase = std::fmod(static_cast<float>(i), period);
        data[i] = (phase < high_duration) ? amplitude : -amplitude;
    }
    return data;
}

/// Generate a sawtooth wave
inline std::vector<Sample> generateSawtoothWave(float frequency, size_t samples, SampleRate sample_rate,
                                                float amplitude = 1.0f) {
    std::vector<Sample> data(samples, 0.0f);
    const float sr = static_cast<float>(sample_rate);
    const float period = sr / frequency;
    
    for (size_t i = 0; i < samples; ++i) {
        float phase = std::fmod(static_cast<float>(i), period);
        data[i] = amplitude * (2.0f * phase / period - 1.0f);
    }
    return data;
}

/// Generate a triangle wave
inline std::vector<Sample> generateTriangleWave(float frequency, size_t samples, SampleRate sample_rate,
                                                float amplitude = 1.0f) {
    std::vector<Sample> data(samples, 0.0f);
    const float sr = static_cast<float>(sample_rate);
    const float period = sr / frequency;
    
    for (size_t i = 0; i < samples; ++i) {
        float phase = std::fmod(static_cast<float>(i), period);
        float normalized = phase / period; // 0 to 1
        
        if (normalized < 0.5f) {
            // Rising edge: -1 to +1
            data[i] = amplitude * (4.0f * normalized - 1.0f);
        } else {
            // Falling edge: +1 to -1
            data[i] = amplitude * (3.0f - 4.0f * normalized);
        }
    }
    return data;
}

// ============================================================================
// Noise Generators
// ============================================================================

/// Generate white noise
inline std::vector<Sample> generateWhiteNoise(size_t samples, float amplitude = 1.0f, unsigned int seed = 42) {
    std::vector<Sample> data(samples);
    std::mt19937 gen(seed);
    std::normal_distribution<float> dist(0.0f, amplitude);
    
    for (size_t i = 0; i < samples; ++i) {
        data[i] = dist(gen);
    }
    return data;
}

/// Generate pink noise (1/f noise) - approximation using simple filter
inline std::vector<Sample> generatePinkNoise(size_t samples, float amplitude = 1.0f, unsigned int seed = 123) {
    std::vector<Sample> data(samples);
    std::mt19937 gen(seed);
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
    
    // Simple pink noise approximation using running sum
    float b0 = 0.0f, b1 = 0.0f, b2 = 0.0f, b3 = 0.0f, b4 = 0.0f, b5 = 0.0f, b6 = 0.0f;
    
    for (size_t i = 0; i < samples; ++i) {
        float white = dist(gen);
        b0 = 0.99886f * b0 + white * 0.0555179f;
        b1 = 0.99332f * b1 + white * 0.0750759f;
        b2 = 0.96900f * b2 + white * 0.1538520f;
        b3 = 0.86650f * b3 + white * 0.3104856f;
        b4 = 0.55000f * b4 + white * 0.5329522f;
        b5 = -0.7616f * b5 - white * 0.0168980f;
        float pink = b0 + b1 + b2 + b3 + b4 + b5 + b6 + white * 0.5362f;
        b6 = white * 0.115926f;
        
        data[i] = pink * amplitude * 0.11f; // Scale down
    }
    return data;
}

// ============================================================================
// Complex Signal Generators
// ============================================================================

/// Generate a chirp (frequency sweep)
inline std::vector<Sample> generateChirp(float start_freq, float end_freq, size_t samples, 
                                         SampleRate sample_rate, float amplitude = 1.0f) {
    std::vector<Sample> data(samples, 0.0f);
    const float sr = static_cast<float>(sample_rate);
    const float freq_delta = (end_freq - start_freq) / static_cast<float>(samples);
    
    float phase = 0.0f;
    for (size_t i = 0; i < samples; ++i) {
        float freq = start_freq + freq_delta * static_cast<float>(i);
        data[i] = amplitude * std::sin(phase);
        phase += 2.0f * std::numbers::pi_v<float> * freq / sr;
    }
    return data;
}

/// Generate a complex tone with harmonics
inline std::vector<Sample> generateHarmonicTone(float fundamental, size_t samples, SampleRate sample_rate,
                                                const std::vector<float>& harmonic_amplitudes) {
    std::vector<Sample> data(samples, 0.0f);
    const float sr = static_cast<float>(sample_rate);
    
    for (size_t harmonic = 0; harmonic < harmonic_amplitudes.size(); ++harmonic) {
        float freq = fundamental * (static_cast<float>(harmonic) + 1.0f);
        float amp = harmonic_amplitudes[harmonic];
        
        for (size_t i = 0; i < samples; ++i) {
            const float phase = 2.0f * std::numbers::pi_v<float> * freq * static_cast<float>(i) / sr;
            data[i] += amp * std::sin(phase);
        }
    }
    return data;
}

/// Generate an impulse (dirac delta function)
inline std::vector<Sample> generateImpulse(size_t samples, size_t position, float amplitude = 1.0f) {
    std::vector<Sample> data(samples, 0.0f);
    if (position < samples) {
        data[position] = amplitude;
    }
    return data;
}

/// Generate impulse train
inline std::vector<Sample> generateImpulseTrain(size_t samples, size_t period, float amplitude = 1.0f) {
    std::vector<Sample> data(samples, 0.0f);
    for (size_t i = 0; i < samples; i += period) {
        data[i] = amplitude;
    }
    return data;
}

// ============================================================================
// Envelope Generators
// ============================================================================

/// Apply ADSR envelope to a signal
inline void applyADSR(std::vector<Sample>& data, SampleRate sample_rate,
                     float attack_ms, float decay_ms, float sustain_level, float release_ms) {
    const size_t samples = data.size();
    const size_t attack_samples = static_cast<size_t>(attack_ms * sample_rate / 1000.0f);
    const size_t decay_samples = static_cast<size_t>(decay_ms * sample_rate / 1000.0f);
    const size_t release_samples = static_cast<size_t>(release_ms * sample_rate / 1000.0f);
    const size_t sustain_start = attack_samples + decay_samples;
    const size_t release_start = samples > release_samples ? samples - release_samples : 0;
    
    for (size_t i = 0; i < samples; ++i) {
        float envelope = 1.0f;
        
        if (i < attack_samples) {
            // Attack phase
            envelope = static_cast<float>(i) / static_cast<float>(attack_samples);
        } else if (i < sustain_start) {
            // Decay phase
            size_t decay_pos = i - attack_samples;
            float decay_progress = static_cast<float>(decay_pos) / static_cast<float>(decay_samples);
            envelope = 1.0f - decay_progress * (1.0f - sustain_level);
        } else if (i < release_start) {
            // Sustain phase
            envelope = sustain_level;
        } else {
            // Release phase
            size_t release_pos = i - release_start;
            float release_progress = static_cast<float>(release_pos) / static_cast<float>(release_samples);
            envelope = sustain_level * (1.0f - release_progress);
        }
        
        data[i] *= envelope;
    }
}

/// Apply linear fade in
inline void applyFadeIn(std::vector<Sample>& data, size_t fade_samples) {
    const size_t samples_to_fade = std::min(fade_samples, data.size());
    for (size_t i = 0; i < samples_to_fade; ++i) {
        float gain = static_cast<float>(i) / static_cast<float>(samples_to_fade);
        data[i] *= gain;
    }
}

/// Apply linear fade out
inline void applyFadeOut(std::vector<Sample>& data, size_t fade_samples) {
    const size_t samples_to_fade = std::min(fade_samples, data.size());
    const size_t start_pos = data.size() - samples_to_fade;
    for (size_t i = 0; i < samples_to_fade; ++i) {
        float gain = 1.0f - (static_cast<float>(i) / static_cast<float>(samples_to_fade));
        data[start_pos + i] *= gain;
    }
}

// ============================================================================
// Pattern Generators
// ============================================================================

/// Generate alternating voiced/silent frames
inline std::vector<Sample> generateAlternatingFrames(size_t samples_per_frame, int cycles,
                                                     float voiced_level = 0.1f, float silent_level = 0.0f) {
    std::vector<Sample> data;
    data.reserve(static_cast<size_t>(samples_per_frame) * static_cast<size_t>(cycles) * 2);
    auto voiced = std::vector<Sample>(samples_per_frame, voiced_level);
    auto silent = std::vector<Sample>(samples_per_frame, silent_level);
    for (int i = 0; i < cycles; ++i) {
        data.insert(data.end(), voiced.begin(), voiced.end());
        data.insert(data.end(), silent.begin(), silent.end());
    }
    return data;
}

// ============================================================================
// Signal Processing Utilities
// ============================================================================

/// Mix two signals together
inline std::vector<Sample> mixSignals(const std::vector<Sample>& signal1, 
                                     const std::vector<Sample>& signal2,
                                     float gain1 = 1.0f, float gain2 = 1.0f) {
    const size_t size = std::min(signal1.size(), signal2.size());
    std::vector<Sample> mixed(size);
    for (size_t i = 0; i < size; ++i) {
        mixed[i] = signal1[i] * gain1 + signal2[i] * gain2;
    }
    return mixed;
}

/// Add noise to a signal
inline std::vector<Sample> addNoise(const std::vector<Sample>& signal, float noise_level, unsigned int seed = 999) {
    auto noise = generateWhiteNoise(signal.size(), noise_level, seed);
    return mixSignals(signal, noise, 1.0f, 1.0f);
}

/// Normalize signal to peak amplitude
inline void normalizeSignal(std::vector<Sample>& data, float target_peak = 1.0f) {
    float max_abs = 0.0f;
    for (const auto& sample : data) {
        max_abs = std::max(max_abs, std::abs(sample));
    }
    
    if (max_abs > 0.0f) {
        float scale = target_peak / max_abs;
        for (auto& sample : data) {
            sample *= scale;
        }
    }
}

/// Calculate RMS of a signal
inline float calculateRMS(const std::vector<Sample>& data) {
    if (data.empty()) return 0.0f;
    
    float sum_squares = 0.0f;
    for (const auto& sample : data) {
        sum_squares += sample * sample;
    }
    return std::sqrt(sum_squares / static_cast<float>(data.size()));
}

/// Calculate peak amplitude
inline float calculatePeak(const std::vector<Sample>& data) {
    if (data.empty()) return 0.0f;
    
    float peak = 0.0f;
    for (const auto& sample : data) {
        peak = std::max(peak, std::abs(sample));
    }
    return peak;
}

// ============================================================================
// Comparison Utilities
// ============================================================================

/// Check if two floats are approximately equal
inline bool approximatelyEqual(float lhs, float rhs, float tolerance) {
    return std::fabs(lhs - rhs) <= tolerance;
}

/// Check if two doubles are approximately equal
inline bool approximatelyEqual(double lhs, double rhs, double tolerance) {
    return std::fabs(lhs - rhs) <= tolerance;
}

/// Check if a value is within a range
inline bool inRange(float value, float min_val, float max_val) {
    return value >= min_val && value <= max_val;
}

/// Check if signals are approximately equal (element-wise)
inline bool signalsEqual(const std::vector<Sample>& signal1, 
                        const std::vector<Sample>& signal2,
                        float tolerance = 1e-6f) {
    if (signal1.size() != signal2.size()) return false;
    
    for (size_t i = 0; i < signal1.size(); ++i) {
        if (!approximatelyEqual(signal1[i], signal2[i], tolerance)) {
            return false;
        }
    }
    return true;
}

} // namespace yvc::test
