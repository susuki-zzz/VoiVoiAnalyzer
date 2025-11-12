// VoiVoi Core Library - CPP Analyzer Implementation
// License: MIT

#include "yvc_core/CPPAnalyzer.h"
#include <kiss_fft.h>
#include <kiss_fftr.h>
#include <cmath>
#include <algorithm>

namespace yvc {

CPPAnalyzer::CPPAnalyzer(const AudioConfig& config)
    : config_(config) {
    fft_buffer_.resize(config_.fft_size);
    cepstrum_buffer_.resize(config_.fft_size);
}

float CPPAnalyzer::analyze(const Sample* samples, size_t num_samples) {
    return computeCPP(samples, num_samples);
}

float CPPAnalyzer::computeCPP(const Sample* samples, size_t num_samples) {
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
    
    // Compute log magnitude spectrum
    std::vector<float> log_spectrum(fft_size / 2 + 1);
    for (size_t i = 0; i < fft_size / 2 + 1; ++i) {
        float magnitude = std::sqrt(fft_out[i].r * fft_out[i].r + fft_out[i].i * fft_out[i].i);
        log_spectrum[i] = std::log(magnitude + 1e-10f);  // Add small value to avoid log(0)
    }
    
    // Compute inverse FFT to get cepstrum
    kiss_fft_cfg ifft_cfg = kiss_fft_alloc(static_cast<int>(fft_size / 2 + 1), 1, nullptr, nullptr);
    std::vector<kiss_fft_cpx> cepstrum_cpx(fft_size / 2 + 1);
    
    for (size_t i = 0; i < fft_size / 2 + 1; ++i) {
        cepstrum_cpx[i].r = log_spectrum[i];
        cepstrum_cpx[i].i = 0.0f;
    }
    
    std::vector<kiss_fft_cpx> cepstrum_out(fft_size / 2 + 1);
    kiss_fft(ifft_cfg, cepstrum_cpx.data(), cepstrum_out.data());
    
    // Find peak in quefrency range corresponding to typical F0 (80-400 Hz)
    const size_t min_quefrency = static_cast<size_t>(config_.sample_rate / 400);
    const size_t max_quefrency = static_cast<size_t>(config_.sample_rate / 80);
    
    float peak_value = 0.0f;
    float baseline = 0.0f;
    
    for (size_t i = min_quefrency; i < max_quefrency && i < cepstrum_out.size(); ++i) {
        float magnitude = std::sqrt(cepstrum_out[i].r * cepstrum_out[i].r);
        if (magnitude > peak_value) {
            peak_value = magnitude;
        }
        baseline += magnitude;
    }
    
    baseline /= static_cast<float>(max_quefrency - min_quefrency);
    
    // CPP is the peak prominence above baseline
    float cpp = 0.0f;
    if (baseline > 0.0f) {
        cpp = 20.0f * std::log10((peak_value + 1e-10f) / (baseline + 1e-10f));
    }
    
    // Cleanup
    kiss_fft_free(fft_cfg);
    kiss_fft_free(ifft_cfg);
    
    return cpp;
}

} // namespace yvc
