// VoiVoi Core Library - Formant Analyzer
// License: MIT
// Purpose: Extract first three formant frequencies (F1,F2,F3) using LPC

#pragma once

#define _USE_MATH_DEFINES
#include "Types.h"
#include <vector>
#include <optional>

namespace yvc {

    /// <summary>
    /// Analyzes formant frequencies (F1, F2, F3, F4) using Linear Predictive Coding (LPC).
    /// Formants are resonant frequencies of the vocal tract and are crucial for vowel identification.
    /// </summary>
    class FormantAnalyzer {
    public:
        /// <summary>
        /// Constructs a formant analyzer with the specified configuration.
        /// </summary>
        /// <param name="config">Audio configuration parameters</param>
        explicit FormantAnalyzer(const AudioConfig& config);

        /// <summary>
        /// Results structure containing formant frequencies.
        /// </summary>
        struct FormantResults {
            float f1 = 0.0f;
            float f2 = 0.0f;
            float f3 = 0.0f;
            float f4 = 0.0f;
            bool valid = false;
        };

        /// <summary>
        /// Analyzes audio samples to extract formant frequencies.
        /// </summary>
        /// <param name="samples">Pointer to audio samples</param>
        /// <param name="num_samples">Number of samples to analyze</param>
        /// <returns>Formant results structure</returns>
        FormantResults analyze(const Sample* samples, size_t num_samples);

    private:
        AudioConfig config_;
        std::vector<float> windowed_;
        std::vector<float> autocorr_;
        std::vector<float> lpc_coeffs_;

        /// <summary>
        /// Applies pre-emphasis filter to enhance higher frequencies.
        /// </summary>
        /// <param name="in">Input samples</param>
        /// <param name="n">Number of samples</param>
        /// <param name="out">Output samples</param>
        void applyPreEmphasis(const Sample* in, size_t n, float* out);

        /// <summary>
        /// Applies window function to the data.
        /// </summary>
        /// <param name="data">Data to window</param>
        /// <param name="n">Number of samples</param>
        void applyWindow(float* data, size_t n);

        /// <summary>
        /// Computes autocorrelation for LPC analysis.
        /// </summary>
        /// <param name="data">Input data</param>
        /// <param name="n">Number of samples</param>
        /// <param name="order">LPC order</param>
        void computeAutocorrelation(const float* data, size_t n, size_t order);

        /// <summary>
        /// Solves LPC coefficients using Levinson-Durbin recursion.
        /// </summary>
        /// <param name="order">LPC order</param>
        /// <returns>True if successful, false otherwise</returns>
        bool levinsonDurbin(size_t order);

        /// <summary>
        /// Extracts formant frequencies from LPC roots.
        /// </summary>
        /// <param name="order">LPC order</param>
        /// <returns>Vector of formant frequencies</returns>
        std::vector<float> rootsToFormants(size_t order);
    };

} // namespace yvc
