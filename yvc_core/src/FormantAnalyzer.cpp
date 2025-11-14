// VoiVoi Core Library - Formant Analyzer Implementation
// License: MIT

#include "yvc_core/FormantAnalyzer.h"
#include <cmath>
#include <algorithm>
#include <complex>

namespace yvc {

namespace {
constexpr size_t kMaxLpcOrder = 16; // Reasonable for speech at 48kHz with downsampling potential
constexpr double kPi = 3.14159265358979323846; // avoid M_PI macro
}

FormantAnalyzer::FormantAnalyzer(const AudioConfig& config)
    : config_(config) {
}

void FormantAnalyzer::applyPreEmphasis(const Sample* in, size_t n, float* out) {
    const float coef = 0.97f;
    if (n == 0) return;
    out[0] = in[0];
    for (size_t i = 1; i < n; ++i)
        out[i] = in[i] - coef * in[i - 1];
}

void FormantAnalyzer::applyWindow(float* data, size_t n) {
    if (n < 2) return;
    for (size_t i = 0; i < n; ++i) {
        float w = 0.54f - 0.46f * std::cos(2.0 * kPi * static_cast<double>(i) / static_cast<double>(n - 1));
        data[i] *= w;
    }
}

void FormantAnalyzer::computeAutocorrelation(const float* data, size_t n, size_t order) {
    autocorr_.assign(order + 1, 0.0f);
    for (size_t lag = 0; lag <= order; ++lag) {
        double sum = 0.0;
        for (size_t i = 0; i + lag < n; ++i)
            sum += double(data[i]) * double(data[i + lag]);
        autocorr_[lag] = float(sum);
    }
}

bool FormantAnalyzer::levinsonDurbin(size_t order) {
    lpc_coeffs_.assign(order + 1, 0.0f);
    std::vector<float> refl(order + 1, 0.0f);

    if (autocorr_.empty() || autocorr_[0] <= 1e-12f)
        return false;

    float error = autocorr_[0];
    lpc_coeffs_[0] = 1.0f;

    for (size_t i = 1; i <= order; ++i) {
        float acc = 0.0f;
        for (size_t j = 1; j < i; ++j)
            acc += lpc_coeffs_[j] * autocorr_[i - j];
        float k = (autocorr_[i] - acc) / error;
        if (std::fabs(k) > 0.99f)
            return false; // Unstable
        refl[i] = k;

        // Update LPC coeffs
        for (size_t j = 1; j < i; ++j)
            lpc_coeffs_[j] = lpc_coeffs_[j] - k * lpc_coeffs_[i - j];
        lpc_coeffs_[i] = k;
        error *= (1.0f - k * k);
        if (error <= 1e-12f)
            return false;
    }
    return true;
}

std::vector<float> FormantAnalyzer::rootsToFormants(size_t order) {
    const size_t fftN = 1024;
    std::vector<float> mag(fftN, 0.0f);
    for (size_t k = 0; k < fftN; ++k) {
        double omega = kPi * double(k) / double(fftN);
        std::complex<double> acc(1.0, 0.0);
        for (size_t i = 1; i <= order; ++i) {
            acc += std::complex<double>(lpc_coeffs_[i], 0.0) * std::exp(std::complex<double>(0.0, -omega * double(i)));
        }
        mag[k] = float(1.0 / std::abs(acc));
    }
    std::vector<std::pair<float,float>> peaks; // (freq, magnitude)
    for (size_t k = 2; k + 2 < fftN; ++k) {
        if (mag[k] > mag[k-1] && mag[k] > mag[k+1]) {
            float freq = float(k) * (float(config_.sample_rate) / 2.0f) / float(fftN);
            if (freq >= 200.0f && freq <= 3500.0f)
                peaks.emplace_back(freq, mag[k]);
        }
    }
    std::sort(peaks.begin(), peaks.end(), [](auto& a, auto& b){ return a.second > b.second; });
    std::vector<float> formants;
    for (auto& p : peaks) {
        bool farEnough = true;
        for (auto f : formants)
            if (std::fabs(f - p.first) < 120.0f) { farEnough = false; break; }
        if (farEnough) formants.push_back(p.first);
        if (formants.size() >= 4) break; // now capture up to 4
    }
    while (formants.size() < 4) formants.push_back(0.0f);
    return formants;
}

FormantAnalyzer::FormantResults FormantAnalyzer::analyze(const Sample* samples, size_t num_samples) {
    FormantResults out;
    if (samples == nullptr || num_samples < 256)
        return out;

    size_t N = std::min<size_t>(num_samples, size_t(config_.fft_size));
    windowed_.resize(N);
    applyPreEmphasis(samples, N, windowed_.data());
    applyWindow(windowed_.data(), N);

    size_t order = std::min<size_t>(kMaxLpcOrder, (config_.sample_rate / 1000) + 2);
    computeAutocorrelation(windowed_.data(), N, order);
    if (!levinsonDurbin(order))
        return out;

    auto freqs = rootsToFormants(order);
    out.f1 = freqs[0];
    out.f2 = freqs[1];
    out.f3 = freqs[2];
    out.f4 = freqs[3];
    out.valid = (out.f1 > 0.0f && out.f2 > 0.0f);
    return out;
}

} // namespace yvc
