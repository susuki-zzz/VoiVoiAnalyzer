// VoiVoi Core Library - CPP Analyzer
// License: MIT
// Purpose: Cepstral Peak Prominence analysis for voice quality

#pragma once

#include "Types.h"
#include <vector>

namespace yvc {

class CPPAnalyzer {
public:
    explicit CPPAnalyzer(const AudioConfig& config);
    
    // Compute CPP (Cepstral Peak Prominence) in dB
    float analyze(const Sample* samples, size_t num_samples);
    
private:
    AudioConfig config_;
    std::vector<float> fft_buffer_;
    std::vector<float> cepstrum_buffer_;
    
    // Compute cepstrum and find peak prominence
    float computeCPP(const Sample* samples, size_t num_samples);
};

} // namespace yvc
