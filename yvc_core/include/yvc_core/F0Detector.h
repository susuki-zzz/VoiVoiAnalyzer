// VoiVoi Core Library - F0 Detector
// License: MIT
// Purpose: Fundamental frequency (pitch) detection with YIN algorithm and stability features

#pragma once

#include "Types.h"
#include <vector>
#include <array>

namespace yvc {

/// <summary>
/// Detects fundamental frequency (F0/pitch) using the YIN algorithm with stability enhancements.
/// Features include:
/// - Range constraint (min_f0_ to max_f0_)
/// - 5-point median filter for smoothing
/// - Hysteresis for voicing decision
/// - Semitone jump guard (prevents sudden large changes)
/// </summary>
class F0Detector {
public:
    /// <summary>
    /// Constructs an F0 detector with the specified configuration.
    /// </summary>
    /// <param name="config">Audio configuration parameters</param>
    explicit F0Detector(const AudioConfig& config);
    
    /// <summary>
    /// Detects F0 from audio samples.
    /// </summary>
    /// <param name="samples">Pointer to audio samples</param>
    /// <param name="num_samples">Number of samples to analyze</param>
    /// <param name="valid">Output parameter indicating if F0 is valid/voiced</param>
    /// <returns>F0 in Hz (0 if unvoiced)</returns>
    float detect(const Sample* samples, size_t num_samples, bool& valid);

    /// <summary>
    /// Detects F0 from audio samples with confidence output.
    /// </summary>
    /// <param name="samples">Pointer to audio samples</param>
    /// <param name="num_samples">Number of samples to analyze</param>
    /// <param name="valid">Output parameter indicating if F0 is valid/voiced</param>
    /// <param name="confidence">Output parameter for confidence in range [0..1]</param>
    /// <returns>F0 in Hz (0 if unvoiced)</returns>
    float detect(const Sample* samples, size_t num_samples, bool& valid, float& confidence);
    
    /// <summary>
    /// Gets the minimum F0 in the valid range.
    /// </summary>
    /// <returns>Minimum F0 in Hz</returns>
    float getMinF0() const { return min_f0_; }

    /// <summary>
    /// Gets the maximum F0 in the valid range.
    /// </summary>
    /// <returns>Maximum F0 in Hz</returns>
    float getMaxF0() const { return max_f0_; }
    
    /// <summary>
    /// Sets the F0 range (for voice: typically 80-400 Hz).
    /// </summary>
    /// <param name="min_f0">Minimum F0 in Hz</param>
    /// <param name="max_f0">Maximum F0 in Hz</param>
    void setF0Range(float min_f0, float max_f0);
    
    /// <summary>
    /// Sets hysteresis thresholds for voicing decision.
    /// </summary>
    /// <param name="voiced_threshold">Lower threshold for voicing (lower = more sensitive)</param>
    /// <param name="unvoiced_threshold">Upper threshold for voicing (hysteresis bound)</param>
    void setHysteresis(float voiced_threshold, float unvoiced_threshold);
    
    /// <summary>
    /// Sets maximum semitone jump allowed per frame.
    /// </summary>
    /// <param name="max_jump_st">Maximum semitone jump</param>
    void setMaxSemitoneJump(float max_jump_st);
    
    /// <summary>
    /// Resets detector state (clears history).
    /// </summary>
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
    
    /// <summary>
    /// YIN algorithm implementation.
    /// </summary>
    /// <param name="samples">Pointer to audio samples</param>
    /// <param name="num_samples">Number of samples to analyze</param>
    /// <param name="confidence">Output parameter for confidence</param>
    /// <returns>Detected F0 in Hz</returns>
    float computeYIN(const Sample* samples, size_t num_samples, float& confidence);
    
    /// <summary>
    /// Fallback autocorrelation-based pitch detection.
    /// </summary>
    /// <param name="samples">Pointer to audio samples</param>
    /// <param name="num_samples">Number of samples to analyze</param>
    /// <returns>Detected F0 in Hz</returns>
    float computeAutocorrelation(const Sample* samples, size_t num_samples);
    
    /// <summary>
    /// Applies 5-point median filter.
    /// </summary>
    /// <param name="new_value">New F0 value to filter</param>
    /// <returns>Filtered F0 value</returns>
    float medianFilter(float new_value);
    
    /// <summary>
    /// Checks if semitone jump is acceptable.
    /// </summary>
    /// <param name="new_f0">New F0 value to check</param>
    /// <returns>True if jump is acceptable, false otherwise</returns>
    bool isJumpAcceptable(float new_f0) const;
    
    /// <summary>
    /// Converts frequency to semitones.
    /// </summary>
    /// <param name="hz">Frequency in Hz</param>
    /// <returns>Semitone value</returns>
    static float hzToSemitones(float hz);

    /// <summary>
    /// Converts semitones to frequency.
    /// </summary>
    /// <param name="semitones">Semitone value</param>
    /// <returns>Frequency in Hz</returns>
    static float semitonesToHz(float semitones);
};

} // namespace yvc
