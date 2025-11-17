// VoiVoi Core Library - VAD Analyzer
// License: MIT
// Purpose: Voice Activity Detection with speech rate and pause analysis

#pragma once

#include "Types.h"
#include <deque>

namespace yvc {

/// <summary>
/// Voice Activity Detection (VAD) analyzer with speech rate and pause metrics.
/// </summary>
class VADAnalyzer {
public:
    /// <summary>
    /// Constructs a VAD analyzer with the specified configuration.
    /// </summary>
    /// <param name="config">Audio configuration parameters</param>
    explicit VADAnalyzer(const AudioConfig& config);
    
    /// <summary>
    /// VAD analysis results structure.
    /// </summary>
    struct VADResults {
        bool voice_active = false;
        float speech_rate = 0.0f;   // syllables per second
        float pause_ratio = 0.0f;   // ratio of pauses to total time
    };
    
    /// <summary>
    /// Analyzes voice activity and speech characteristics.
    /// </summary>
    /// <param name="samples">Pointer to audio samples</param>
    /// <param name="num_samples">Number of samples to analyze</param>
    /// <param name="rms">RMS level of the audio</param>
    /// <returns>VAD results structure</returns>
    VADResults analyze(const Sample* samples, size_t num_samples, float rms);
    
    /// <summary>
    /// Resets the analyzer state.
    /// </summary>
    void reset();
    
private:
    AudioConfig config_;
    
    // Voice activity detection threshold
    float vad_threshold_ = 0.01f;
    
    // State tracking for speech rate
    std::deque<double> syllable_times_;
    bool previous_voice_active_ = false;
    double total_time_ = 0.0;
    double speech_time_ = 0.0;

    /// <summary>
    /// Detects voice activity based on RMS level.
    /// </summary>
    /// <param name="rms">RMS level</param>
    /// <returns>True if voice is detected</returns>
    bool detectVoiceActivity(float rms);
    
    /// <summary>
    /// Estimates speech rate from activity history.
    /// </summary>
    /// <returns>Estimated speech rate in syllables per second</returns>
    float estimateSpeechRate();
};

} // namespace yvc
