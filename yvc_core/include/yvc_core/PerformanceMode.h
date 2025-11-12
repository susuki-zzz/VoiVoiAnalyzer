// VoiVoi Core Library - Performance Mode
// License: MIT
// Purpose: Performance mode configuration and latency management

#pragma once

#include "Types.h"

namespace yvc {

class PerformanceModeConfig {
public:
    explicit PerformanceModeConfig(PerformanceMode mode);
    
    // Get the current mode
    PerformanceMode getMode() const { return mode_; }
    
    // Get maximum allowed latency in milliseconds
    float getMaxLatencyMs() const { return max_latency_ms_; }
    
    // Get suggested FFT size for this mode
    uint32_t getFFTSize() const { return fft_size_; }
    
    // Get hop size for analysis
    uint32_t getHopSize() const { return hop_size_; }

    // Check if a feature is enabled in this mode
    bool isFeatureEnabled(const std::string& feature) const;

    // Apply this performance mode to an audio configuration
    static AudioConfig applyToConfig(const AudioConfig& base_config, PerformanceMode mode);

private:
    PerformanceMode mode_;
    float max_latency_ms_ = 0.0f;
    uint32_t fft_size_ = 0;
    uint32_t hop_size_ = 0;
};

// Convenience helper that applies the performance mode stored in the config
AudioConfig configureForPerformanceMode(const AudioConfig& config);

} // namespace yvc
