// VoiVoi Core Library - Level Analyzer Implementation
// License: MIT

#include "yvc_core/LevelAnalyzer.h"
#include <algorithm>
#include <cmath>

namespace yvc {

    LevelAnalyzer::LevelAnalyzer(const AudioConfig& config)
        : config_(config) { }

    /// <summary>
    /// Calculates RMS, peak, and crest factor for the frame.
    /// </summary>
    LevelAnalyzer::LevelResults LevelAnalyzer::analyze(const Sample* samples,
                                                       size_t num_samples) {
        LevelResults results;
        if(num_samples == 0) { return results; }
        results.rms = calculateRMS(samples, num_samples);
        results.peak = calculatePeak(samples, num_samples);
        if(results.rms > 0.0f) {
            results.crest_factor = results.peak / results.rms;
        }
        return results;
    }

    /// <summary>
    /// RMS = sqrt( mean(x^2) ) for the buffer.
    /// </summary>
    float LevelAnalyzer::calculateRMS(const Sample* samples,
                                      size_t num_samples) {
        float sum_squares = 0.0f;
        for(size_t i = 0; i < num_samples; ++i) sum_squares += samples[i] * samples[i];
        return std::sqrt(sum_squares / static_cast<float>(num_samples));
    }

    /// <summary>
    /// Peak absolute sample value over the frame.
    /// </summary>
    float LevelAnalyzer::calculatePeak(const Sample* samples,
                                       size_t num_samples) {
        float peak = 0.0f;
        for(size_t i = 0; i < num_samples; ++i) {
            float abs_sample = std::abs(samples[i]);
            if(abs_sample > peak) peak = abs_sample;
        }
        return peak;
    }

} // namespace yvc
