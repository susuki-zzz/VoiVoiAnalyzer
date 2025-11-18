// VoiVoi Core Library - Spectral Analyzer
// License: MIT
// Purpose: Spectral Tilt and /s/ Centroid analysis

#pragma once

#include "Types.h"
#include <vector>

namespace yvc {

    /// <summary>
    /// Analyzes spectral characteristics including spectral tilt and sibilant detection.
    /// </summary>
    class SpectralAnalyzer {
    public:
        /// <summary>
        /// Constructs a spectral analyzer with the specified configuration.
        /// </summary>
        /// <param name="config">Audio configuration parameters</param>
        explicit SpectralAnalyzer(const AudioConfig& config);

        /// <summary>
        /// Spectral analysis results structure.
        /// </summary>
        struct SpectralResults {
            float spectral_tilt = 0.0f;  // in dB/octave
            float s_centroid = 0.0f;      // in Hz
            bool s_detected = false;
        };

        /// <summary>
        /// Analyzes spectral characteristics of audio samples.
        /// </summary>
        /// <param name="samples">Pointer to audio samples</param>
        /// <param name="num_samples">Number of samples to analyze</param>
        /// <returns>Spectral results structure</returns>
        SpectralResults analyze(const Sample* samples, size_t num_samples);

    private:
        AudioConfig config_;
        std::vector<float> fft_buffer_;
        std::vector<float> magnitude_spectrum_;

        /// <summary>
        /// Computes spectral tilt (regression of spectrum slope).
        /// </summary>
        /// <param name="spectrum">Magnitude spectrum</param>
        /// <param name="spectrum_size">Size of spectrum</param>
        /// <returns>Spectral tilt in dB/octave</returns>
        float computeSpectralTilt(const float* spectrum, size_t spectrum_size);

        /// <summary>
        /// Detects /s/ sound and computes centroid.
        /// </summary>
        /// <param name="spectrum">Magnitude spectrum</param>
        /// <param name="spectrum_size">Size of spectrum</param>
        /// <returns>Spectral results with sibilant information</returns>
        SpectralResults analyzeSibilant(const float* spectrum, size_t spectrum_size);
    };

} // namespace yvc
