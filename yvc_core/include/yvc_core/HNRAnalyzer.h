// VoiVoi Core Library - HNR Analyzer
// License: MIT
// Purpose: Harmonics-to-Noise Ratio analysis

#pragma once

#include "Types.h"
#include <vector>

namespace yvc {

class HNRAnalyzer {
public:
    explicit HNRAnalyzer(const AudioConfig& config);
    
    // Compute HNR (Harmonics-to-Noise Ratio) in dB
    float analyze(const Sample* samples, size_t num_samples, float f0);
    
private:
    AudioConfig config_;
    std::vector<float> autocorr_buffer_;
    
    // Compute HNR using autocorrelation method
    float computeHNR(const Sample* samples, size_t num_samples, float f0);
};

} // namespace yvc
