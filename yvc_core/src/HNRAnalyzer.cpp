// VoiVoi Core Library - HNR Analyzer Implementation
// License: MIT

#include "yvc_core/HNRAnalyzer.h"
#include <cmath>
#include <algorithm>

namespace yvc {

HNRAnalyzer::HNRAnalyzer(const AudioConfig& config)
    : config_(config) {
    autocorr_buffer_.resize(config.sample_rate / 40);
}

float HNRAnalyzer::analyze(const Sample* samples, size_t num_samples, float f0) {
    if (f0 <= 0.0f || num_samples == 0) {
        return 0.0f;
    }
    
    return computeHNR(samples, num_samples, f0);
}

float HNRAnalyzer::computeHNR(const Sample* samples, size_t num_samples, float f0) {
    // Compute autocorrelation at the pitch period
    const size_t period = static_cast<size_t>(config_.sample_rate / f0);
    
    if (period >= num_samples) {
        return 0.0f;
    }
    
    // Compute autocorrelation at lag=0 (signal energy)
    float r0 = 0.0f;
    for (size_t i = 0; i < num_samples; ++i) {
        r0 += samples[i] * samples[i];
    }
    
    // Compute autocorrelation at lag=period (harmonic energy)
    float rp = 0.0f;
    for (size_t i = 0; i < num_samples - period; ++i) {
        rp += samples[i] * samples[i + period];
    }
    
    // HNR calculation
    // HNR = 10 * log10(rp / (r0 - rp))
    float harmonic_energy = rp;
    float noise_energy = r0 - rp;
    
    if (noise_energy > 0.0f && harmonic_energy > 0.0f) {
        float hnr = 10.0f * std::log10(harmonic_energy / noise_energy);
        return hnr;
    }
    
    return 0.0f;
}

} // namespace yvc
