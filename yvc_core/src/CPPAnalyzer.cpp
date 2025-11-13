// VoiVoi Core Library - CPP Analyzer Implementation
// License: MIT

#include "yvc_core/CPPAnalyzer.h"
#include <kiss_fft.h>
#include <kiss_fftr.h>
#include <cmath>
#include <algorithm>
#include <vector>

namespace yvc {

// Use constexpr PI to avoid dependency on M_PI macro
static constexpr float kPi = 3.14159265358979323846f;

struct CPPAnalyzer::Impl {
    std::vector<float> fft_buffer;
    std::vector<float> cepstrum_buffer;
    std::vector<kiss_fft_cpx> fft_out_buffer;
    std::vector<kiss_fft_cpx> log_spectrum_buffer;
    std::vector<kiss_fft_cpx> cepstrum_complex_buffer;
};

CPPAnalyzer::CPPAnalyzer(const AudioConfig& config)
    : config_(config), impl_(std::make_unique<Impl>()) {
    impl_->fft_buffer.resize(config_.fft_size);
    impl_->cepstrum_buffer.resize(config_.fft_size);
    impl_->fft_out_buffer.resize(config_.fft_size / 2 + 1);
    impl_->log_spectrum_buffer.resize(config_.fft_size);
    impl_->cepstrum_complex_buffer.resize(config_.fft_size);
}

CPPAnalyzer::~CPPAnalyzer() = default;
CPPAnalyzer::CPPAnalyzer(CPPAnalyzer&&) noexcept = default;
CPPAnalyzer& CPPAnalyzer::operator=(CPPAnalyzer&&) noexcept = default;

float CPPAnalyzer::analyze(const Sample* samples, size_t num_samples) {
    return computeCPP(samples, num_samples);
}

float CPPAnalyzer::computeCPP(const Sample* samples, size_t num_samples) {
    const size_t fft_size = std::min(static_cast<size_t>(config_.fft_size), num_samples);
    if (fft_size == 0) {
        return 0.0f;
    }

    kiss_fftr_cfg fft_cfg = kiss_fftr_alloc(static_cast<int>(fft_size), 0, nullptr, nullptr);
    if (!fft_cfg) {
        return 0.0f;
    }

    kiss_fft_cfg ifft_cfg = kiss_fft_alloc(static_cast<int>(fft_size), 1, nullptr, nullptr);
    if (!ifft_cfg) {
        kiss_fft_free(fft_cfg);
        return 0.0f;
    }

    // Copy and window samples using the reusable buffer
    const float window_denominator = fft_size > 1 ? static_cast<float>(fft_size - 1) : 1.0f;

    for (size_t i = 0; i < fft_size; ++i) {
        const float window = 0.54f - 0.46f * std::cos(kPi * 2.0f * static_cast<float>(i) / window_denominator);
        impl_->fft_buffer[i] = samples[i] * window;
    }

    // Compute FFT
    kiss_fftr(fft_cfg, impl_->fft_buffer.data(), impl_->fft_out_buffer.data());

    const size_t half = fft_size / 2;
    const size_t positive_limit = (fft_size % 2 == 0) ? half : half + 1;

    // Compute log magnitude spectrum for positive frequencies and store in reusable buffer
    for (size_t i = 0; i <= half; ++i) {
        const float real = impl_->fft_out_buffer[i].r;
        const float imag = impl_->fft_out_buffer[i].i;
        const float magnitude = std::sqrt(real * real + imag * imag);
        impl_->fft_buffer[i] = std::log(magnitude + 1e-10f);
    }

    // Build a full complex spectrum with conjugate symmetry
    for (size_t i = 0; i < fft_size; ++i) {
        impl_->log_spectrum_buffer[i].r = 0.0f;
        impl_->log_spectrum_buffer[i].i = 0.0f;
    }

    for (size_t i = 0; i <= half; ++i) {
        impl_->log_spectrum_buffer[i].r = impl_->fft_buffer[i];
    }

    for (size_t i = 1; i < positive_limit; ++i) {
        const size_t mirror = fft_size - i;
        impl_->log_spectrum_buffer[mirror].r = impl_->log_spectrum_buffer[i].r;
        impl_->log_spectrum_buffer[mirror].i = -impl_->log_spectrum_buffer[i].i;
    }

    // Inverse FFT to get cepstrum
    kiss_fft(ifft_cfg, impl_->log_spectrum_buffer.data(), impl_->cepstrum_complex_buffer.data());

    const float scale = 1.0f / static_cast<float>(fft_size);
    for (size_t i = 0; i < fft_size; ++i) {
        impl_->cepstrum_buffer[i] = impl_->cepstrum_complex_buffer[i].r * scale;
    }

    // Find peak in quefrency range corresponding to typical F0 (80-400 Hz)
    const size_t min_quefrency = static_cast<size_t>(config_.sample_rate / 400);
    const size_t max_quefrency = static_cast<size_t>(config_.sample_rate / 80);
    const size_t quefrency_end = std::min(max_quefrency, fft_size);

    float peak_value = 0.0f;
    float baseline_sum = 0.0f;
    size_t count = 0;

    for (size_t i = min_quefrency; i < quefrency_end; ++i) {
        const float value = std::fabs(impl_->cepstrum_buffer[i]);
        peak_value = std::max(peak_value, value);
        baseline_sum += value;
        ++count;
    }

    float cpp = 0.0f;
    if (count > 0) {
        const float baseline = baseline_sum / static_cast<float>(count);
        cpp = 20.0f * std::log10((peak_value + 1e-10f) / (baseline + 1e-10f));
    }

    kiss_fft_free(fft_cfg);
    kiss_fft_free(ifft_cfg);

    return cpp;
}

} // namespace yvc
