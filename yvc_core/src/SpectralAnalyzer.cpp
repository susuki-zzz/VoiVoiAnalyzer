// VoiVoi Core Library - Spectral Analyzer Implementation
// License: MIT

#include "yvc_core/SpectralAnalyzer.h"
#include <kiss_fft.h>
#include <kiss_fftr.h>
#include <cmath>
#include <algorithm>
#include <numeric>

namespace yvc {

SpectralAnalyzer::SpectralAnalyzer(const AudioConfig& config)
    : config_(config) {
    fft_buffer_.resize(config_.fft_size);
    magnitude_spectrum_.resize(config_.fft_size / 2 + 1);
}

SpectralAnalyzer::SpectralResults SpectralAnalyzer::analyze(const Sample* samples, size_t num_samples) {
    SpectralResults results;

    const size_t fft_size = std::min(static_cast<size_t>(config_.fft_size), num_samples);

    // Prepare FFT
    kiss_fftr_cfg fft_cfg = kiss_fftr_alloc(static_cast<int>(fft_size), 0, nullptr, nullptr);
    std::vector<kiss_fft_cpx> fft_out(fft_size / 2 + 1);
    
    // Copy and window samples
    std::vector<float> windowed(fft_size);
    for (size_t i = 0; i < fft_size; ++i) {
        // Hamming window
        float window = 0.54f - 0.46f * std::cos(2.0f * M_PI * i / (fft_size - 1));
        windowed[i] = samples[i] * window;
    }
    
    // Compute FFT
    kiss_fftr(fft_cfg, windowed.data(), fft_out.data());

    // Compute magnitude spectrum
    magnitude_spectrum_.resize(fft_size / 2 + 1);
    for (size_t i = 0; i < fft_size / 2 + 1; ++i) {
        magnitude_spectrum_[i] = std::sqrt(fft_out[i].r * fft_out[i].r + fft_out[i].i * fft_out[i].i);
    }

    // Compute spectral tilt
    results.spectral_tilt = computeSpectralTilt(magnitude_spectrum_.data(), magnitude_spectrum_.size());
    
    // Analyze sibilants
    auto sibilant_results = analyzeSibilant(magnitude_spectrum_.data(), magnitude_spectrum_.size());
    results.s_centroid = sibilant_results.s_centroid;
    results.s_detected = sibilant_results.s_detected;
    
    kiss_fft_free(fft_cfg);
    
    return results;
}

float SpectralAnalyzer::computeSpectralTilt(const float* spectrum, size_t spectrum_size) {
    // Compute spectral tilt using linear regression on log-magnitude spectrum
    std::vector<float> log_freq(spectrum_size);
    std::vector<float> log_mag(spectrum_size);
    
    for (size_t i = 1; i < spectrum_size; ++i) {  // Start from 1 to avoid log(0)
        float freq = static_cast<float>(i) * config_.sample_rate / (2.0f * spectrum_size);
        log_freq[i] = std::log10(freq + 1.0f);
        log_mag[i] = 20.0f * std::log10(spectrum[i] + 1e-10f);
    }
    
    // Simple linear regression
    float mean_x = 0.0f, mean_y = 0.0f;
    for (size_t i = 1; i < spectrum_size; ++i) {
        mean_x += log_freq[i];
        mean_y += log_mag[i];
    }
    mean_x /= (spectrum_size - 1);
    mean_y /= (spectrum_size - 1);
    
    float numerator = 0.0f, denominator = 0.0f;
    for (size_t i = 1; i < spectrum_size; ++i) {
        numerator += (log_freq[i] - mean_x) * (log_mag[i] - mean_y);
        denominator += (log_freq[i] - mean_x) * (log_freq[i] - mean_x);
    }
    
    float slope = 0.0f;
    if (denominator > 0.0f) {
        slope = numerator / denominator;
    }
    
    return slope;  // dB/octave
}

SpectralAnalyzer::SpectralResults SpectralAnalyzer::analyzeSibilant(const float* spectrum, size_t spectrum_size) {
    SpectralResults results;

    // /s/ sound is typically concentrated in 4-8 kHz range
    const size_t min_bin = static_cast<size_t>(4000.0f * spectrum_size * 2 / config_.sample_rate);
    const size_t max_bin = static_cast<size_t>(8000.0f * spectrum_size * 2 / config_.sample_rate);
    
    // Compute energy in sibilant range
    float sibilant_energy = 0.0f;
    float total_energy = 0.0f;
    
    for (size_t i = 0; i < spectrum_size; ++i) {
        float energy = spectrum[i] * spectrum[i];
        total_energy += energy;
        
        if (i >= min_bin && i <= max_bin) {
            sibilant_energy += energy;
        }
    }
    
    // Detect /s/ if high frequency energy is prominent
    const float sibilant_ratio = sibilant_energy / (total_energy + 1e-10f);
    results.s_detected = (sibilant_ratio > 0.3f);
    
    if (results.s_detected) {
        // Compute spectral centroid in sibilant range
        float weighted_sum = 0.0f;
        float weight_total = 0.0f;
        
        for (size_t i = min_bin; i <= max_bin && i < spectrum_size; ++i) {
            float freq = static_cast<float>(i) * config_.sample_rate / (2.0f * spectrum_size);
            float weight = spectrum[i];
            weighted_sum += freq * weight;
            weight_total += weight;
        }
        
        if (weight_total > 0.0f) {
            results.s_centroid = weighted_sum / weight_total;
        }
    }
    
    return results;
}

} // namespace yvc
