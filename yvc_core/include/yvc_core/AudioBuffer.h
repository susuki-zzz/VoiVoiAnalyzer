// VoiVoi Core Library - Audio Buffer
// License: MIT
// Purpose: Audio buffer management for real-time processing

#pragma once

#include "Types.h"
#include "IPreprocessor.h"
#include <vector>
#include <memory>
#include <unordered_map>

namespace yvc {

class AudioBuffer {
public:
    explicit AudioBuffer(const AudioConfig& config);
    
    // Add samples to the buffer
    void addSamples(const Sample* samples, size_t num_samples);
    
    // Get the current buffer for analysis
    const std::vector<Sample>& getBuffer() const { return buffer_; }

    // Check if buffer has enough samples for analysis
    bool hasEnoughSamples(size_t required_samples) const;

    // Consume samples from the front of the buffer after analysis
    void consumeSamples(size_t num_samples);

    // Clear the buffer
    void clear();
    
    // Get configuration
    const AudioConfig& getConfig() const { return config_; }
    
    // Get current fill level
    size_t getSize() const { return buffer_.size(); }

    // Configure latency compensation per preprocess target
    void setLatencyCompensation(PreprocessTarget target, size_t samples);

    // Retrieve configured latency compensation
    size_t getLatencyCompensation(PreprocessTarget target) const;

    // Clear all latency compensation data
    void clearLatencyCompensation();

private:
    AudioConfig config_;
    std::vector<Sample> buffer_;
    size_t write_pos_ = 0;
    struct TargetHash {
        size_t operator()(PreprocessTarget target) const {
            return static_cast<size_t>(toMask(target));
        }
    };
    std::unordered_map<PreprocessTarget, size_t, TargetHash> latency_compensation_;
};

} // namespace yvc
