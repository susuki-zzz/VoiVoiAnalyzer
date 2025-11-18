// VoiVoi Core Library - AI Coach Provider Interface
// License: MIT
// Purpose: Base interface for AI coaching services

#pragma once

#include <string>
#include "Types.h"

namespace yvc {

    /// <summary>
    /// Summary snapshot containing aggregated metrics only (no raw audio).
    /// This is what gets sent to AI coach services for privacy protection.
    /// </summary>
    struct SummarySnapshot {
        /// <summary>
        /// Statistical aggregates over analysis window.
        /// </summary>
        struct Stats {
            float mean = 0.0f;
            float std_dev = 0.0f;
            float min = 0.0f;
            float max = 0.0f;
            float p50 = 0.0f;  // median
            float p95 = 0.0f;
        };

        Stats f0;                    // F0 statistics (Hz)
        Stats rms;                   // RMS level
        Stats cpp;                   // CPP statistics (dB)
        Stats hnr;                   // HNR statistics (dB)
        Stats spectral_tilt;         // Spectral tilt (dB/octave)
        Stats s_centroid;            // /s/ centroid (Hz)

        float speech_rate = 0.0f;    // Average speech rate (syllables/sec)
        float pause_ratio = 0.0f;    // Ratio of pause time to total time
        float voice_active_ratio = 0.0f;  // Ratio of voice active time

        double duration_seconds = 0.0;    // Total duration of analyzed audio
        size_t num_frames = 0;            // Number of frames analyzed

        // Target ranges for comparison (from preset)
        float target_f0_min = 0.0f;
        float target_f0_max = 0.0f;
    };

    /// <summary>
    /// AI Coach provider interface.
    /// Privacy guarantee: Only metrics are sent, NEVER raw audio data.
    /// </summary>
    class ICoachProvider {
    public:
        virtual ~ICoachProvider() = default;

        /// <summary>
        /// Sets API key for the coach service.
        /// Key should be stored securely (e.g., DPAPI on Windows).
        /// </summary>
        /// <param name="key">API key string</param>
        virtual void setApiKey(const std::string& key) = 0;

        /// <summary>
        /// Gets advice based on metrics summary.
        /// </summary>
        /// <param name="snapshot">Aggregated metrics (NO audio data)</param>
        /// <returns>Advice text from coach</returns>
        virtual std::string advise(const SummarySnapshot& snapshot) = 0;

        /// <summary>
        /// Checks if coach is available/configured.
        /// </summary>
        /// <returns>True if available, false otherwise</returns>
        virtual bool isAvailable() const = 0;

        /// <summary>
        /// Gets provider name (e.g., "OpenAI", "Local").
        /// </summary>
        /// <returns>Provider name as C-string</returns>
        virtual const char* name() const = 0;

        /// <summary>
        /// Sets minimum interval between advice requests (rate limiting).
        /// </summary>
        /// <param name="ms">Minimum interval in milliseconds</param>
        virtual void setMinIntervalMs(int ms) = 0;
    };

} // namespace yvc
