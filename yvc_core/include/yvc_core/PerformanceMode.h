// VoiVoi Core Library - Performance Mode
// License: MIT
// Purpose: Performance mode configuration and latency management

#pragma once

#include "Types.h"

namespace yvc {

/// <summary>
/// Performance mode configuration for managing latency and features.
/// </summary>
class PerformanceModeConfig {
public:
    /// <summary>
    /// Constructs performance mode configuration.
    /// </summary>
    /// <param name="mode">Performance mode to configure</param>
    explicit PerformanceModeConfig(PerformanceMode mode);
    
    /// <summary>
    /// Gets the current mode.
    /// </summary>
    /// <returns>Performance mode</returns>
    PerformanceMode getMode() const { return mode_; }
    
    /// <summary>
    /// Gets maximum allowed latency in milliseconds.
    /// </summary>
    /// <returns>Maximum latency in ms</returns>
    float getMaxLatencyMs() const { return max_latency_ms_; }
    
    /// <summary>
    /// Gets suggested FFT size for this mode.
    /// </summary>
    /// <returns>FFT size in samples</returns>
    uint32_t getFFTSize() const { return fft_size_; }
    
    /// <summary>
    /// Gets hop size for analysis.
    /// </summary>
    /// <returns>Hop size in samples</returns>
    uint32_t getHopSize() const { return hop_size_; }

    /// <summary>
    /// Checks if a feature is enabled in this mode.
    /// </summary>
    /// <param name="feature">Feature name</param>
    /// <returns>True if feature is enabled</returns>
    bool isFeatureEnabled(const std::string& feature) const;

    /// <summary>
    /// Applies this performance mode to an audio configuration.
    /// </summary>
    /// <param name="base_config">Base audio configuration</param>
    /// <param name="mode">Performance mode to apply</param>
    /// <returns>Modified audio configuration</returns>
    static AudioConfig applyToConfig(const AudioConfig& base_config, PerformanceMode mode);

private:
    PerformanceMode mode_;
    float max_latency_ms_ = 0.0f;
    uint32_t fft_size_ = 0;
    uint32_t hop_size_ = 0;
};

/// <summary>
/// Convenience helper that applies the performance mode stored in the config.
/// </summary>
/// <param name="config">Audio configuration with performance mode</param>
/// <returns>Configured audio configuration</returns>
AudioConfig configureForPerformanceMode(const AudioConfig& config);

} // namespace yvc
