// VoiVoi Core Library - Audio Buffer
// License: MIT
// Purpose: Audio buffer management for real-time processing

#pragma once

#include "Types.h"
#include <vector>
#include <memory>

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
    
    // Clear the buffer
    void clear();
    
    // Get configuration
    const AudioConfig& getConfig() const { return config_; }
    
    // Get current fill level
    size_t getSize() const { return buffer_.size(); }
    
private:
    AudioConfig config_;
    std::vector<Sample> buffer_;
    size_t write_pos_ = 0;
};

} // namespace yvc
