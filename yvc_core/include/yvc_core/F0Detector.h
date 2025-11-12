// VoiVoi Core Library - F0 Detector
// License: MIT
// Purpose: Fundamental frequency (pitch) detection using autocorrelation

#pragma once

#include "Types.h"
#include <vector>

namespace yvc {

class F0Detector {
public:
    explicit F0Detector(const AudioConfig& config);
    
    // Detect F0 from audio samples
    // Returns F0 in Hz, sets valid flag
    float detect(const Sample* samples, size_t num_samples, bool& valid);
    
    // Get the valid F0 range
    float getMinF0() const { return min_f0_; }
    float getMaxF0() const { return max_f0_; }
    
    // Set F0 range (for voice: typically 80-400 Hz)
    void setF0Range(float min_f0, float max_f0);
    
private:
    AudioConfig config_;
    float min_f0_ = 80.0f;   // Minimum F0 in Hz
    float max_f0_ = 400.0f;  // Maximum F0 in Hz
    std::vector<float> autocorr_buffer_;
    
    // Autocorrelation-based pitch detection
    float computeAutocorrelation(const Sample* samples, size_t num_samples);
};

} // namespace yvc
