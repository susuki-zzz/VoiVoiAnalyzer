// VoiVoi Core Library - F0 Detector Implementation
// License: MIT

#include "yvc_core/F0Detector.h"
#include <algorithm>
#include <cmath>
#include <numeric>

namespace yvc {

F0Detector::F0Detector(const AudioConfig& config)
    : config_(config) {
    autocorr_buffer_.resize(config.sample_rate / 40);  // For lowest F0
}

float F0Detector::detect(const Sample* samples, size_t num_samples, bool& valid) {
    valid = false;
    
    if (num_samples < 2 * (config_.sample_rate / min_f0_)) {
        return 0.0f;  // Not enough samples
    }
    
    float f0 = computeAutocorrelation(samples, num_samples);
    
    // Validate F0 is in expected range
    if (f0 >= min_f0_ && f0 <= max_f0_) {
        valid = true;
        return f0;
    }
    
    return 0.0f;
}

void F0Detector::setF0Range(float min_f0, float max_f0) {
    min_f0_ = min_f0;
    max_f0_ = max_f0;
}

float F0Detector::computeAutocorrelation(const Sample* samples, size_t num_samples) {
    // Autocorrelation-based pitch detection
    const size_t min_lag = static_cast<size_t>(config_.sample_rate / max_f0_);
    const size_t max_lag = static_cast<size_t>(config_.sample_rate / min_f0_);
    
    float max_corr = 0.0f;
    size_t best_lag = 0;
    
    // Compute autocorrelation for each lag
    for (size_t lag = min_lag; lag < max_lag && lag < num_samples / 2; ++lag) {
        float corr = 0.0f;
        float energy = 0.0f;
        
        for (size_t i = 0; i < num_samples - lag; ++i) {
            corr += samples[i] * samples[i + lag];
            energy += samples[i] * samples[i];
        }
        
        // Normalize correlation
        if (energy > 0.0f) {
            corr /= energy;
        }
        
        if (corr > max_corr) {
            max_corr = corr;
            best_lag = lag;
        }
    }
    
    // Convert lag to frequency
    if (best_lag > 0 && max_corr > 0.3f) {  // Minimum correlation threshold
        return static_cast<float>(config_.sample_rate) / static_cast<float>(best_lag);
    }
    
    return 0.0f;
}

} // namespace yvc
