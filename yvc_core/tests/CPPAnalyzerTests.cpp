#include <gtest/gtest.h>

#include <yvc_core/CPPAnalyzer.h>

#include "TestSignalHelpers.h"

#include <algorithm>
#include <cmath>
#include <complex>
#include <numbers>
#include <vector>

namespace yvc::test {
namespace {

std::vector<double> computeExpectedCepstrum(const std::vector<Sample>& samples, const AudioConfig& config) {
    const size_t fft_size = static_cast<size_t>(config.fft_size);
    std::vector<double> windowed(fft_size, 0.0);
    const double window_denominator = fft_size > 1 ? static_cast<double>(fft_size - 1) : 1.0;

    for (size_t n = 0; n < fft_size; ++n) {
        const double window = 0.54 - 0.46 * std::cos(2.0 * std::numbers::pi * static_cast<double>(n) /
                                                     window_denominator);
        windowed[n] = static_cast<double>(samples[n]) * window;
    }

    std::vector<std::complex<double>> spectrum(fft_size, std::complex<double>(0.0, 0.0));
    for (size_t k = 0; k < fft_size; ++k) {
        std::complex<double> sum(0.0, 0.0);
        for (size_t n = 0; n < fft_size; ++n) {
            const double angle = -2.0 * std::numbers::pi * static_cast<double>(k * n) / static_cast<double>(fft_size);
            sum += windowed[n] * std::complex<double>(std::cos(angle), std::sin(angle));
        }
        spectrum[k] = sum;
    }

    std::vector<double> log_spectrum(fft_size, 0.0);
    std::transform(spectrum.begin(), spectrum.end(), log_spectrum.begin(), [](const auto& bin) {
        return std::log(std::abs(bin) + 1e-10);
    });

    std::vector<double> cepstrum(fft_size, 0.0);
    for (size_t n = 0; n < fft_size; ++n) {
        std::complex<double> sum(0.0, 0.0);
        for (size_t k = 0; k < fft_size; ++k) {
            const double angle = 2.0 * std::numbers::pi * static_cast<double>(k * n) / static_cast<double>(fft_size);
            sum += std::complex<double>(log_spectrum[k], 0.0) * std::complex<double>(std::cos(angle), std::sin(angle));
        }
        cepstrum[n] = (sum / static_cast<double>(fft_size)).real();
    }

    return cepstrum;
}

TEST(CPPAnalyzerTests, CepstralPeakMatchesAnalyticalReference) {
    AudioConfig config{};
    config.sample_rate = 8000;
    config.fft_size = 256;

    CPPAnalyzer analyzer(config);

    std::vector<Sample> samples(config.fft_size, 0.0f);
    const float frequency = 160.0f;
    for (size_t n = 0; n < samples.size(); ++n) {
        samples[n] = std::sin(2.0f * std::numbers::pi_v<float> * frequency *
                              static_cast<float>(n) / static_cast<float>(config.sample_rate));
    }

    const float cpp = analyzer.analyze(samples.data(), samples.size());

    const auto expected_cepstrum = computeExpectedCepstrum(samples, config);
    const size_t min_quefrency = static_cast<size_t>(config.sample_rate / 400);
    const size_t max_quefrency = static_cast<size_t>(config.sample_rate / 80);
    const size_t quefrency_end = std::min(max_quefrency, expected_cepstrum.size());

    double peak_value = 0.0;
    double baseline_sum = 0.0;
    size_t count = 0;

    for (size_t i = min_quefrency; i < quefrency_end; ++i) {
        const double value = std::fabs(expected_cepstrum[i]);
        peak_value = std::max(peak_value, value);
        baseline_sum += value;
        ++count;
    }

    ASSERT_GT(count, 0u);

    const double baseline = baseline_sum / static_cast<double>(count);
    const double expected_cpp = 20.0 * std::log10((peak_value + 1e-10) / (baseline + 1e-10));

    EXPECT_TRUE(approximatelyEqual(static_cast<double>(cpp), expected_cpp, 1e-3));
}

} // namespace
} // namespace yvc::test
