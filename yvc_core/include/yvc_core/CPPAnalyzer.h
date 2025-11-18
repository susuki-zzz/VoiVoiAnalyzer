// VoiVoi Core Library - CPP Analyzer
// License: MIT
// Purpose: Cepstral Peak Prominence analysis for voice quality

#pragma once

#include "Types.h"
#include <memory>

namespace yvc {

    /// <summary>
    /// Analyzes Cepstral Peak Prominence (CPP) for voice quality assessment.
    /// CPP measures the ratio of the cepstral peak to the surrounding noise floor,
    /// providing a metric for voice periodicity and quality.
    /// </summary>
    class CPPAnalyzer {
    public:
        /// <summary>
        /// Constructs a CPP analyzer with the specified configuration.
        /// </summary>
        /// <param name="config">Audio configuration parameters</param>
        explicit CPPAnalyzer(const AudioConfig& config);

        /// <summary>
        /// Destructor.
        /// </summary>
        ~CPPAnalyzer();

        /// <summary>
        /// Move constructor.
        /// </summary>
        CPPAnalyzer(CPPAnalyzer&&) noexcept;

        /// <summary>
        /// Move assignment operator.
        /// </summary>
        CPPAnalyzer& operator=(CPPAnalyzer&&) noexcept;

        // Delete copy operations
        CPPAnalyzer(const CPPAnalyzer&) = delete;
        CPPAnalyzer& operator=(const CPPAnalyzer&) = delete;

        /// <summary>
        /// Computes CPP (Cepstral Peak Prominence) in dB.
        /// </summary>
        /// <param name="samples">Pointer to audio samples</param>
        /// <param name="num_samples">Number of samples to analyze</param>
        /// <returns>CPP value in dB</returns>
        float analyze(const Sample* samples, size_t num_samples);

    private:
        struct Impl;                  // Pimpl to hide kissfft dependency
        AudioConfig config_;
        std::unique_ptr<Impl> impl_;  // Implementation storage

        /// <summary>
        /// Computes cepstrum and finds peak prominence.
        /// </summary>
        /// <param name="samples">Pointer to audio samples</param>
        /// <param name="num_samples">Number of samples to analyze</param>
        /// <returns>CPP value in dB</returns>
        float computeCPP(const Sample* samples, size_t num_samples);
    };

} // namespace yvc
