// VoiVoi Core Library - VAD Analyzer
// License: MIT
// Purpose: Voice Activity Detection with speech rate and pause analysis

#pragma once

#include "Types.h"
#include <vector>
#include <deque>

namespace yvc {

class VADAnalyzer {
public:
    explicit VADAnalyzer(const AudioConfig& config);
    
    // VAD analysis results
    struct VADResults {
        bool voice_active = false;
        float speech_rate = 0.0f;   // syllables per second
        float pause_ratio = 0.0f;   // ratio of pauses to total time
    };
    
    VADResults analyze(const Sample* samples, size_t num_samples, float rms);
    
    // Reset the analyzer state
    void reset();
    
private:
    AudioConfig config_;
    
    // Voice activity detection threshold
    float vad_threshold_ = 0.01f;
    
    // State tracking for speech rate
    std::deque<double> voice_activity_history_;
    std::deque<double> syllable_times_;
    double total_time_ = 0.0;
    double speech_time_ = 0.0;
    
    // Detect voice activity
    bool detectVoiceActivity(float rms);
    
    // Estimate speech rate from activity history
    float estimateSpeechRate();
};

} // namespace yvc
