// VoiVoi Core Library - Level Analyzer
// License: MIT
// Purpose: RMS, Peak, and Crest Factor analysis

#pragma once

#include "Types.h"

namespace yvc {

class LevelAnalyzer {
public:
    explicit LevelAnalyzer(const AudioConfig& config);
    
    // Analyze audio levels
    struct LevelResults {
        float rms = 0.0f;
        float peak = 0.0f;
        float crest_factor = 0.0f;
    };
    
    LevelResults analyze(const Sample* samples, size_t num_samples);
    
private:
    AudioConfig config_;
    
    // Calculate RMS (Root Mean Square) level
    float calculateRMS(const Sample* samples, size_t num_samples);
    
    // Calculate peak level
    float calculatePeak(const Sample* samples, size_t num_samples);
};

} // namespace yvc
