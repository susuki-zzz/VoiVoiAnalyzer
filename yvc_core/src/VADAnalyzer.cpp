// VoiVoi Core Library - VAD Analyzer Implementation
// License: MIT

#include "yvc_core/VADAnalyzer.h"
#include <algorithm>

namespace yvc {

    namespace {
        constexpr double kSyllableHistoryWindowSeconds = 5.0;
    }

    VADAnalyzer::VADAnalyzer(const AudioConfig& config)
        : config_(config) { }

    /// <summary>
    /// Analyzes frame for voice activity; updates speech rate and pause ratio statistics.
    /// </summary>
    VADAnalyzer::VADResults VADAnalyzer::analyze(const Sample* samples,
                                                 size_t num_samples,
                                                 float rms) {
        VADResults results;

        (void)samples;  // current trivial VAD uses RMS only

        // Detect voice activity based on RMS level
        results.voice_active = detectVoiceActivity(rms);

        // Update time tracking
        double frame_duration = static_cast<double>(num_samples) / config_.sample_rate;
        total_time_ += frame_duration;

        if(results.voice_active) {
            speech_time_ += frame_duration;
        }

        // Detect transitions from silence to voice and record their timestamps
        if(results.voice_active && !previous_voice_active_) {
            syllable_times_.push_back(total_time_);
        }

        previous_voice_active_ = results.voice_active;

        // Keep syllable history limited to a fixed window
        const double history_start = total_time_ - kSyllableHistoryWindowSeconds;
        while(!syllable_times_.empty() && syllable_times_.front() < history_start) {
            syllable_times_.pop_front();
        }

        // Calculate pause ratio
        if(total_time_ > 0.0) {
            results.pause_ratio = static_cast<float>(1.0 - (speech_time_ / total_time_));
        }

        // Estimate speech rate
        results.speech_rate = estimateSpeechRate();

        return results;
    }

    /// <summary>
    /// Resets internal time counters and history.
    /// </summary>
    void VADAnalyzer::reset() {
        syllable_times_.clear();
        previous_voice_active_ = false;
        total_time_ = 0.0;
        speech_time_ = 0.0;
    }

    /// <summary>
    /// Simple RMS threshold based VAD decision.
    /// </summary>
    bool VADAnalyzer::detectVoiceActivity(float rms) {
        return rms > vad_threshold_;
    }

    /// <summary>
    /// Estimates speech rate as transitions per second over recent window.
    /// </summary>
    float VADAnalyzer::estimateSpeechRate() {
        if(syllable_times_.size() < 2) {
            return 0.0f;
        }

        // Simple syllable rate estimation based on voice activity transitions
        // Assuming each transition corresponds to a syllable boundary
        const double time_span = syllable_times_.back() - syllable_times_.front();

        if(time_span > 0.0) {
            // Estimate syllables per second
            // Typical speech is 3-6 syllables per second
            const size_t intervals = syllable_times_.size() - 1;
            return static_cast<float>(intervals / time_span);
        }

        return 0.0f;
    }

} // namespace yvc
