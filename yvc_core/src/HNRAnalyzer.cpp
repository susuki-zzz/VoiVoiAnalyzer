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

    /// <summary>
    /// Public entry computing harmonics-to-noise ratio for voiced segments.
    /// </summary>
    float HNRAnalyzer::analyze(const Sample* samples,
                               size_t num_samples,
                               float f0) {
        if(f0 <= 0.0f || num_samples == 0) {
            return 0.0f;
        }

        return computeHNR(samples, num_samples, f0);
    }

    /// <summary>
    /// Autocorrelation-based HNR estimation: 10*log10(harmonic_energy / noise_energy).
    /// </summary>
    float HNRAnalyzer::computeHNR(const Sample* samples,
                                  size_t num_samples,
                                  float f0) {
        const size_t period = static_cast<size_t>(config_.sample_rate / f0);

        if(period >= num_samples) {
            return 0.0f;
        }

        float r0 = 0.0f; // total energy
        for(size_t i = 0; i < num_samples; ++i) r0 += samples[i] * samples[i];

        float rp = 0.0f; // harmonic energy at pitch lag
        for(size_t i = 0; i < num_samples - period; ++i) rp += samples[i] * samples[i + period];

        float harmonic_energy = rp; float noise_energy = r0 - rp;
        if(noise_energy > 0.0f && harmonic_energy > 0.0f) {
            return 10.0f * std::log10(harmonic_energy / noise_energy);
        }

        return 0.0f;
    }

} // namespace yvc
