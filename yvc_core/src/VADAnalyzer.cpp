// VoiVoi Core Library - VAD Analyzer Implementation
// License: MIT

#include "yvc_core/VADAnalyzer.h"
#include <algorithm>

namespace yvc {

VADAnalyzer::VADAnalyzer(const AudioConfig& config)
    : config_(config) {
}

VADAnalyzer::VADResults VADAnalyzer::analyze(const Sample* samples, size_t num_samples, float rms) {
    VADResults results;
    
    // Detect voice activity based on RMS level
    results.voice_active = detectVoiceActivity(rms);
    
    // Update time tracking
    double frame_duration = static_cast<double>(num_samples) / config_.sample_rate;
    total_time_ += frame_duration;
    
    if (results.voice_active) {
        speech_time_ += frame_duration;
        voice_activity_history_.push_back(total_time_);
        
        // Keep history limited to last 5 seconds
        while (!voice_activity_history_.empty() && 
               (total_time_ - voice_activity_history_.front()) > 5.0) {
            voice_activity_history_.pop_front();
        }
    }
    
    // Calculate pause ratio
    if (total_time_ > 0.0) {
        results.pause_ratio = static_cast<float>(1.0 - (speech_time_ / total_time_));
    }
    
    // Estimate speech rate
    results.speech_rate = estimateSpeechRate();
    
    return results;
}

void VADAnalyzer::reset() {
    voice_activity_history_.clear();
    syllable_times_.clear();
    total_time_ = 0.0;
    speech_time_ = 0.0;
}

bool VADAnalyzer::detectVoiceActivity(float rms) {
    return rms > vad_threshold_;
}

float VADAnalyzer::estimateSpeechRate() {
    if (voice_activity_history_.size() < 2) {
        return 0.0f;
    }
    
    // Simple syllable rate estimation based on voice activity transitions
    // Assuming each transition corresponds to a syllable boundary
    const size_t num_transitions = voice_activity_history_.size();
    const double time_span = voice_activity_history_.back() - voice_activity_history_.front();
    
    if (time_span > 0.0) {
        // Estimate syllables per second
        // Typical speech is 3-6 syllables per second
        return static_cast<float>(num_transitions / time_span);
    }
    
    return 0.0f;
}

} // namespace yvc
