// VoiVoi Core Library - Level Analyzer
// License: MIT
// Purpose: RMS, Peak, and Crest Factor analysis

#pragma once

#include "Types.h"

namespace yvc {

    /// <summary>
    /// Analyzes audio level metrics including RMS, Peak, and Crest Factor.
    /// </summary>
    class LevelAnalyzer {
    public:
        /// <summary>
        /// Constructs a level analyzer with the specified configuration.
        /// </summary>
        /// <param name="config">Audio configuration parameters</param>
        explicit LevelAnalyzer(const AudioConfig& config);

        /// <summary>
        /// Results structure containing level analysis metrics.
        /// </summary>
        struct LevelResults {
            float rms = 0.0f;
            float peak = 0.0f;
            float crest_factor = 0.0f;
        };

        /// <summary>
        /// Analyzes audio levels.
        /// </summary>
        /// <param name="samples">Pointer to audio samples</param>
        /// <param name="num_samples">Number of samples to analyze</param>
        /// <returns>Level results structure</returns>
        LevelResults analyze(const Sample* samples, size_t num_samples);

    private:
        AudioConfig config_;

        /// <summary>
        /// Calculates RMS (Root Mean Square) level.
        /// </summary>
        /// <param name="samples">Pointer to audio samples</param>
        /// <param name="num_samples">Number of samples</param>
        /// <returns>RMS level</returns>
        float calculateRMS(const Sample* samples, size_t num_samples);

        /// <summary>
        /// Calculates peak level.
        /// </summary>
        /// <param name="samples">Pointer to audio samples</param>
        /// <param name="num_samples">Number of samples</param>
        /// <returns>Peak level</returns>
        float calculatePeak(const Sample* samples, size_t num_samples);
    };

} // namespace yvc
