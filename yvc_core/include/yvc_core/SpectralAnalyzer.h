// VoiVoi Core Library - Spectral Analyzer
// License: MIT
// Purpose: Spectral Tilt and /s/ Centroid analysis

#pragma once

#include "Types.h"
#include <vector>

namespace yvc {

class SpectralAnalyzer {
public:
    explicit SpectralAnalyzer(const AudioConfig& config);
    
    // Spectral analysis results
    struct SpectralResults {
        float spectral_tilt = 0.0f;  // in dB/octave
        float s_centroid = 0.0f;      // in Hz
        bool s_detected = false;
    };
    
    SpectralResults analyze(const Sample* samples, size_t num_samples);
    
private:
    AudioConfig config_;
    std::vector<float> fft_buffer_;
    std::vector<float> magnitude_spectrum_;
    
    // Compute spectral tilt (regression of spectrum slope)
    float computeSpectralTilt(const float* spectrum, size_t spectrum_size);
    
    // Detect /s/ sound and compute centroid
    SpectralResults analyzeSibilant(const float* spectrum, size_t spectrum_size);
};

} // namespace yvc
