// VoiVoi Core Library - F0 Detector Implementation
// License: MIT

#include "yvc_core/F0Detector.h"
#include "yvc_core/Logger.h"
#include <algorithm>
#include <cmath>
#include <numeric>

namespace yvc {

F0Detector::F0Detector(const AudioConfig& config)
    : config_(config) {
    const size_t max_lag = config.sample_rate / static_cast<size_t>(min_f0_);
    yin_buffer_.resize(max_lag);
    autocorr_buffer_.resize(max_lag);
    f0_history_.fill(0.0f);
    
    LOG_DEBUGF("F0Detector initialized: SR=%d, buffer_size=%zu", 
               config.sample_rate, yin_buffer_.size());
}

void F0Detector::reset() {
    was_voiced_ = false;
    last_f0_ = 0.0f;
    f0_history_.fill(0.0f);
    history_index_ = 0;
}

void F0Detector::setF0Range(float min_f0, float max_f0) {
    min_f0_ = min_f0;
    max_f0_ = max_f0;
    const size_t max_lag = config_.sample_rate / static_cast<size_t>(min_f0_);
    yin_buffer_.resize(max_lag);
    autocorr_buffer_.resize(max_lag);
    LOG_DEBUGF("F0Detector: Range set to [%.1f, %.1f] Hz", min_f0, max_f0);
}

void F0Detector::setHysteresis(float voiced_threshold, float unvoiced_threshold) {
    voiced_threshold_ = voiced_threshold;
    unvoiced_threshold_ = unvoiced_threshold;
}

void F0Detector::setMaxSemitoneJump(float max_jump_st) {
    max_semitone_jump_ = max_jump_st;
}

float F0Detector::detect(const Sample* samples, size_t num_samples, bool& valid) {
    valid = false;
    
    const size_t min_samples = 2 * (config_.sample_rate / static_cast<size_t>(min_f0_));
    if (num_samples < min_samples) {
        LOG_TRACE("F0Detector: Not enough samples for detection");
        return 0.0f;
    }
    
    // Try YIN algorithm first
    float confidence = 0.0f;
    float f0 = computeYIN(samples, num_samples, confidence);
    
    // Fallback to autocorrelation if YIN fails
    if (f0 <= 0.0f || f0 < min_f0_ || f0 > max_f0_) {
        f0 = computeAutocorrelation(samples, num_samples);
    }
    
    // Validate F0 is in expected range
    if (f0 < min_f0_ || f0 > max_f0_) {
        was_voiced_ = false;
        return 0.0f;
    }
    
    // Apply hysteresis for voicing decision
    const float threshold = was_voiced_ ? unvoiced_threshold_ : voiced_threshold_;
    if (confidence < threshold) {
        was_voiced_ = false;
        return 0.0f;
    }
    
    // Check semitone jump guard
    if (was_voiced_ && !isJumpAcceptable(f0)) {
        LOG_TRACEF("F0Detector: Jump rejected: %.2f -> %.2f Hz (%.1f ST)", 
                   last_f0_, f0, std::abs(hzToSemitones(f0) - hzToSemitones(last_f0_)));
        return last_f0_;  // Return previous stable value
    }
    
    // Apply median filter
    f0 = medianFilter(f0);
    
    valid = true;
    was_voiced_ = true;
    last_f0_ = f0;
    
    LOG_TRACEF("F0Detector: Detected F0=%.2f Hz (confidence=%.3f)", f0, confidence);
    return f0;
}

float F0Detector::computeYIN(const Sample* samples, size_t num_samples, float& confidence) {
    LOG_SCOPE_TIMER_TRACE("F0Detector::computeYIN");
    
    const size_t min_lag = config_.sample_rate / static_cast<size_t>(max_f0_);
    const size_t max_lag = std::min(
        config_.sample_rate / static_cast<size_t>(min_f0_),
        num_samples / 2
    );
    
    if (max_lag >= yin_buffer_.size()) {
        confidence = 0.0f;
        return 0.0f;
    }
    
    // Step 1: Difference function
    yin_buffer_[0] = 0.0f;
    for (size_t tau = 1; tau < max_lag; ++tau) {
        float sum = 0.0f;
        for (size_t i = 0; i < num_samples - tau; ++i) {
            const float delta = samples[i] - samples[i + tau];
            sum += delta * delta;
        }
        yin_buffer_[tau] = sum;
    }
    
    // Step 2: Cumulative mean normalized difference
    float running_sum = 0.0f;
    yin_buffer_[0] = 1.0f;
    for (size_t tau = 1; tau < max_lag; ++tau) {
        running_sum += yin_buffer_[tau];
        if (running_sum > 0.0f) {
            yin_buffer_[tau] *= static_cast<float>(tau) / running_sum;
        } else {
            yin_buffer_[tau] = 1.0f;
        }
    }
    
    // Step 3: Find first minimum below threshold
    size_t best_tau = 0;
    float min_val = 1.0f;
    
    for (size_t tau = min_lag; tau < max_lag; ++tau) {
        if (yin_buffer_[tau] < voiced_threshold_) {
            // Find local minimum
            if (tau + 1 < max_lag && yin_buffer_[tau] < yin_buffer_[tau + 1]) {
                best_tau = tau;
                min_val = yin_buffer_[tau];
                break;
            }
        }
        if (yin_buffer_[tau] < min_val) {
            min_val = yin_buffer_[tau];
            best_tau = tau;
        }
    }
    
    if (best_tau == 0) {
        confidence = 0.0f;
        return 0.0f;
    }
    
    // Step 4: Parabolic interpolation for sub-sample accuracy
    float interpolated_tau = static_cast<float>(best_tau);
    if (best_tau > 0 && best_tau < max_lag - 1) {
        const float s0 = yin_buffer_[best_tau - 1];
        const float s1 = yin_buffer_[best_tau];
        const float s2 = yin_buffer_[best_tau + 1];
        const float adjustment = (s2 - s0) / (2.0f * (2.0f * s1 - s2 - s0));
        interpolated_tau += adjustment;
    }
    
    confidence = 1.0f - min_val;
    const float f0 = static_cast<float>(config_.sample_rate) / interpolated_tau;
    
    LOG_TRACEF("F0Detector YIN: tau=%zu (%.2f), f0=%.2f Hz, confidence=%.3f", 
               best_tau, interpolated_tau, f0, confidence);
    
    return f0;
}

float F0Detector::computeAutocorrelation(const Sample* samples, size_t num_samples) {
    LOG_SCOPE_TIMER_TRACE("F0Detector::computeAutocorrelation");
    
    const size_t min_lag = config_.sample_rate / static_cast<size_t>(max_f0_);
    const size_t max_lag = std::min(
        config_.sample_rate / static_cast<size_t>(min_f0_),
        num_samples / 2
    );
    
    float max_corr = 0.0f;
    size_t best_lag = 0;
    
    for (size_t lag = min_lag; lag < max_lag; ++lag) {
        float corr = 0.0f;
        float energy = 0.0f;
        
        for (size_t i = 0; i < num_samples - lag; ++i) {
            corr += samples[i] * samples[i + lag];
            energy += samples[i] * samples[i];
        }
        
        if (energy > 0.0f) {
            corr /= energy;
        }
        
        if (corr > max_corr) {
            max_corr = corr;
            best_lag = lag;
        }
    }
    
    if (best_lag > 0 && max_corr > 0.3f) {
        const float f0 = static_cast<float>(config_.sample_rate) / static_cast<float>(best_lag);
        LOG_TRACEF("F0Detector ACF: best_lag=%zu, correlation=%.3f, f0=%.2f", 
                   best_lag, max_corr, f0);
        return f0;
    }
    
    LOG_TRACE("F0Detector ACF: No reliable pitch detected");
    return 0.0f;
}

float F0Detector::medianFilter(float new_value) {
    f0_history_[history_index_] = new_value;
    history_index_ = (history_index_ + 1) % f0_history_.size();
    
    std::array<float, 5> sorted = f0_history_;
    std::sort(sorted.begin(), sorted.end());
    
    return sorted[2];  // Median of 5 values
}

bool F0Detector::isJumpAcceptable(float new_f0) const {
    if (last_f0_ <= 0.0f || new_f0 <= 0.0f) {
        return true;
    }
    
    const float st_diff = std::abs(hzToSemitones(new_f0) - hzToSemitones(last_f0_));
    return st_diff <= max_semitone_jump_;
}

float F0Detector::hzToSemitones(float hz) {
    return 12.0f * std::log2(hz / 440.0f);
}

float F0Detector::semitonesToHz(float semitones) {
    return 440.0f * std::pow(2.0f, semitones / 12.0f);
}

} // namespace yvc
