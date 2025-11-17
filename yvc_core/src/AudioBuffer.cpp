// VoiVoi Core Library - Audio Buffer Implementation
// License: MIT

#include "yvc_core/AudioBuffer.h"
#include <algorithm>
#include <cstring>

namespace yvc {

AudioBuffer::AudioBuffer(const AudioConfig& config)
    : config_(config) {
    // Pre-allocate buffer for efficiency
    buffer_.reserve(config.buffer_size * 4);
}

/// <summary>
/// Appends incoming samples to the rolling buffer (clamped to ~2 seconds).
/// </summary>
void AudioBuffer::addSamples(const Sample* samples, size_t num_samples) {
    // Append samples to buffer
    buffer_.insert(buffer_.end(), samples, samples + num_samples);
    
    // Limit buffer size to prevent unbounded growth (privacy + memory)
    const size_t max_buffer_size = config_.sample_rate * 2;  // 2 seconds max
    if (buffer_.size() > max_buffer_size) {
        buffer_.erase(buffer_.begin(), buffer_.begin() + (buffer_.size() - max_buffer_size));
    }
}

/// <summary>
/// Returns true when at least required_samples have accumulated.
/// </summary>
bool AudioBuffer::hasEnoughSamples(size_t required_samples) const {
    return buffer_.size() >= required_samples;
}

/// <summary>
/// Removes num_samples from the front of the buffer; clears if consuming all.
/// </summary>
void AudioBuffer::consumeSamples(size_t num_samples) {
    if (num_samples == 0 || buffer_.empty()) {
        return;
    }

    if (num_samples >= buffer_.size()) {
        clear();
        return;
    }

    buffer_.erase(buffer_.begin(), buffer_.begin() + num_samples);
}

/// <summary>
/// Clears all buffered samples and latency metadata.
/// </summary>
void AudioBuffer::clear() {
    buffer_.clear();
    write_pos_ = 0;
    latency_compensation_.clear();
}

/// <summary>
/// Sets latency compensation (in samples) for a preprocess target.
/// </summary>
void AudioBuffer::setLatencyCompensation(PreprocessTarget target, size_t samples) {
    if (!any(target)) {
        return;
    }
    latency_compensation_[target] = samples;
}

/// <summary>
/// Gets latency compensation for a target (0 if none configured).
/// </summary>
size_t AudioBuffer::getLatencyCompensation(PreprocessTarget target) const {
    auto it = latency_compensation_.find(target);
    if (it == latency_compensation_.end()) {
        return 0;
    }
    return it->second;
}

/// <summary>
/// Clears all latency compensation entries.
/// </summary>
void AudioBuffer::clearLatencyCompensation() {
    latency_compensation_.clear();
}

} // namespace yvc
