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

void AudioBuffer::addSamples(const Sample* samples, size_t num_samples) {
    // Append samples to buffer
    buffer_.insert(buffer_.end(), samples, samples + num_samples);
    
    // Limit buffer size to prevent unbounded growth
    const size_t max_buffer_size = config_.sample_rate * 2;  // 2 seconds max
    if (buffer_.size() > max_buffer_size) {
        buffer_.erase(buffer_.begin(), buffer_.begin() + (buffer_.size() - max_buffer_size));
    }
}

bool AudioBuffer::hasEnoughSamples(size_t required_samples) const {
    return buffer_.size() >= required_samples;
}

void AudioBuffer::clear() {
    buffer_.clear();
    write_pos_ = 0;
    latency_compensation_.clear();
}

void AudioBuffer::setLatencyCompensation(PreprocessTarget target, size_t samples) {
    if (!any(target)) {
        return;
    }
    latency_compensation_[target] = samples;
}

size_t AudioBuffer::getLatencyCompensation(PreprocessTarget target) const {
    auto it = latency_compensation_.find(target);
    if (it == latency_compensation_.end()) {
        return 0;
    }
    return it->second;
}

void AudioBuffer::clearLatencyCompensation() {
    latency_compensation_.clear();
}

} // namespace yvc
