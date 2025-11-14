// VoiVoi Core Library - F0 Detector
// License: MIT
// Purpose: Fundamental frequency (pitch) detection with YIN algorithm and stability features

#pragma once

#include "Types.h"
#include <vector>
#include <array>

namespace yvc {

class F0Detector {
public:
    explicit F0Detector(const AudioConfig& config);
    
    // Detect F0 from audio samples
    // Returns F0 in Hz, sets valid flag
    // Uses YIN algorithm with stability enhancements:
    // - Range constraint (min_f0_ to max_f0_)
    // - 5-point median filter for smoothing
    // - Hysteresis for voicing decision
    // - Semitone jump guard (prevents sudden large changes)
    float detect(const Sample* samples, size_t num_samples, bool& valid);
    // Overload that also outputs confidence in [0..1]
    float detect(const Sample* samples, size_t num_samples, bool& valid, float& confidence);
    
    // Get the valid F0 range
    float getMinF0() const { return min_f0_; }
    float getMaxF0() const { return max_f0_; }
    
    // Set F0 range (for voice: typically 80-400 Hz)
    void setF0Range(float min_f0, float max_f0);
    
    // Set hysteresis thresholds for voicing decision
    void setHysteresis(float voiced_threshold, float unvoiced_threshold);
    
    // Set maximum semitone jump allowed per frame
    void setMaxSemitoneJump(float max_jump_st);
    
    // Reset detector state (clears history)
    void reset();
    
private:
    AudioConfig config_;
    float min_f0_ = 80.0f;   // Minimum F0 in Hz
    float max_f0_ = 400.0f;  // Maximum F0 in Hz
    float voiced_threshold_ = 0.15f;    // Lower = more sensitive to voice
    float unvoiced_threshold_ = 0.25f;  // Hysteresis upper bound
    float max_semitone_jump_ = 6.0f;    // Maximum semitone change per frame
    
    // State for continuity
    bool was_voiced_ = false;
    float last_f0_ = 0.0f;
    std::array<float, 5> f0_history_{};  // For median filter
    size_t history_index_ = 0;
    
    std::vector<float> yin_buffer_;
    std::vector<float> autocorr_buffer_;
    
    // YIN algorithm implementation
    float computeYIN(const Sample* samples, size_t num_samples, float& confidence);
    
    // Fallback autocorrelation-based pitch detection
    float computeAutocorrelation(const Sample* samples, size_t num_samples);
    
    // Apply 5-point median filter
    float medianFilter(float new_value);
    
    // Check if semitone jump is acceptable
    bool isJumpAcceptable(float new_f0) const;
    
    // Convert frequency to semitones
    static float hzToSemitones(float hz);
    static float semitonesToHz(float semitones);
};

} // namespace yvc
