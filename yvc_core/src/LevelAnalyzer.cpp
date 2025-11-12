// VoiVoi Core Library - Level Analyzer Implementation
// License: MIT

#include "yvc_core/LevelAnalyzer.h"
#include <algorithm>
#include <cmath>

namespace yvc {

LevelAnalyzer::LevelAnalyzer(const AudioConfig& config)
    : config_(config) {
}

LevelAnalyzer::LevelResults LevelAnalyzer::analyze(const Sample* samples, size_t num_samples) {
    LevelResults results;
    
    if (num_samples == 0) {
        return results;
    }
    
    results.rms = calculateRMS(samples, num_samples);
    results.peak = calculatePeak(samples, num_samples);
    
    // Calculate crest factor (Peak/RMS ratio)
    if (results.rms > 0.0f) {
        results.crest_factor = results.peak / results.rms;
    }
    
    return results;
}

float LevelAnalyzer::calculateRMS(const Sample* samples, size_t num_samples) {
    float sum_squares = 0.0f;
    
    for (size_t i = 0; i < num_samples; ++i) {
        sum_squares += samples[i] * samples[i];
    }
    
    return std::sqrt(sum_squares / static_cast<float>(num_samples));
}

float LevelAnalyzer::calculatePeak(const Sample* samples, size_t num_samples) {
    float peak = 0.0f;
    
    for (size_t i = 0; i < num_samples; ++i) {
        float abs_sample = std::abs(samples[i]);
        if (abs_sample > peak) {
            peak = abs_sample;
        }
    }
    
    return peak;
}

} // namespace yvc
