// VoiVoi Core Library - HNR Analyzer
// License: MIT
// Purpose: Harmonics-to-Noise Ratio analysis

#pragma once

#include "Types.h"
#include <vector>

namespace yvc {

    /// <summary>
    /// Analyzes Harmonics-to-Noise Ratio (HNR) for voice quality assessment.
    /// HNR measures the ratio of harmonic energy to noise energy in the voice signal,
    /// providing an indication of voice clarity and stability.
    /// </summary>
    class HNRAnalyzer {
    public:
        /// <summary>
        /// Constructs an HNR analyzer with the specified configuration.
        /// </summary>
        /// <param name="config">Audio configuration parameters</param>
        explicit HNRAnalyzer(const AudioConfig& config);

        /// <summary>
        /// Computes HNR (Harmonics-to-Noise Ratio) in dB.
        /// </summary>
        /// <param name="samples">Pointer to audio samples</param>
        /// <param name="num_samples">Number of samples to analyze</param>
        /// <param name="f0">Fundamental frequency in Hz</param>
        /// <returns>HNR value in dB</returns>
        float analyze(const Sample* samples, size_t num_samples, float f0);

    private:
        AudioConfig config_;
        std::vector<float> autocorr_buffer_;

        /// <summary>
        /// Computes HNR using autocorrelation method.
        /// </summary>
        /// <param name="samples">Pointer to audio samples</param>
        /// <param name="num_samples">Number of samples to analyze</param>
        /// <param name="f0">Fundamental frequency in Hz</param>
        /// <returns>HNR value in dB</returns>
        float computeHNR(const Sample* samples, size_t num_samples, float f0);
    };

} // namespace yvc
