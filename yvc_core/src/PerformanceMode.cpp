// VoiVoi Core Library - Performance Mode Implementation
// License: MIT

#include "yvc_core/PerformanceMode.h"

namespace yvc {

PerformanceModeConfig::PerformanceModeConfig(PerformanceMode mode)
    : mode_(mode) {
    
    switch (mode) {
        case PerformanceMode::Mode_Light:
            max_latency_ms_ = 40.0f;
            fft_size_ = 1024;
            hop_size_ = 512;
            break;
            
        case PerformanceMode::Mode_Standard:
            max_latency_ms_ = 60.0f;
            fft_size_ = 2048;
            hop_size_ = 512;
            break;
            
        case PerformanceMode::Mode_Diagnostic:
            max_latency_ms_ = 80.0f;
            fft_size_ = 4096;
            hop_size_ = 1024;
            break;
    }
}

bool PerformanceModeConfig::isFeatureEnabled(const std::string& feature) const {
    // In Light mode, some advanced features may be disabled
    if (mode_ == PerformanceMode::Mode_Light) {
        if (feature == "cpp" || feature == "spectral_tilt") {
            return false;  // Disable expensive features in light mode
        }
    }
    return true;
}

AudioConfig PerformanceModeConfig::applyToConfig(const AudioConfig& base_config, PerformanceMode mode) {
    AudioConfig config = base_config;
    config.mode = mode;

    PerformanceModeConfig mode_config(mode);
    config.fft_size = mode_config.getFFTSize();
    config.hop_size = mode_config.getHopSize();

    // Ensure the streaming buffer can at least fit one hop
    if (config.buffer_size < config.hop_size) {
        config.buffer_size = config.hop_size;
    }

    return config;
}

AudioConfig configureForPerformanceMode(const AudioConfig& config) {
    return PerformanceModeConfig::applyToConfig(config, config.mode);
}

} // namespace yvc
