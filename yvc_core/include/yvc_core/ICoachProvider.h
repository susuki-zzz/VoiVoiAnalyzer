// VoiVoi Core Library - AI Coach Provider Interface
// License: MIT
// Purpose: Base interface for AI coaching services

#pragma once

#include <string>
#include "Types.h"

namespace yvc {

// Summary snapshot containing aggregated metrics only (no raw audio)
// This is what gets sent to AI coach services
struct SummarySnapshot {
    // Statistical aggregates over analysis window
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

// AI Coach provider interface
// Privacy: Only metrics are sent, NEVER raw audio data
class ICoachProvider {
public:
    virtual ~ICoachProvider() = default;
    
    // Set API key for the coach service
    // Key should be stored securely (e.g., DPAPI on Windows)
    virtual void setApiKey(const std::string& key) = 0;
    
    // Get advice based on metrics summary
    // @param snapshot: Aggregated metrics (NO audio data)
    // @return: Advice text from coach
    virtual std::string advise(const SummarySnapshot& snapshot) = 0;
    
    // Check if coach is available/configured
    virtual bool isAvailable() const = 0;
    
    // Get provider name (e.g., "OpenAI", "Local")
    virtual const char* name() const = 0;
    
    // Set minimum interval between advice requests (rate limiting)
    virtual void setMinIntervalMs(int ms) = 0;
};

} // namespace yvc
