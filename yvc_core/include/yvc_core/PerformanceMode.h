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
    
private:
    PerformanceMode mode_;
    float max_latency_ms_;
    uint32_t fft_size_;
    uint32_t hop_size_;
};

} // namespace yvc
