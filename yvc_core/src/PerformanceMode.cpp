// VoiVoi Core Library - Performance Mode Implementation
// License: MIT

#include "yvc_core/PerformanceMode.h"

namespace yvc {

PerformanceModeConfig::PerformanceModeConfig(PerformanceMode mode)
    : mode_(mode) {
    
    switch (mode) {
        case PerformanceMode::Light:
            max_latency_ms_ = 40.0f;
            fft_size_ = 1024;
            hop_size_ = 512;
            break;
            
        case PerformanceMode::Standard:
            max_latency_ms_ = 60.0f;
            fft_size_ = 2048;
            hop_size_ = 512;
            break;
            
        case PerformanceMode::Diagnostic:
            max_latency_ms_ = 80.0f;
            fft_size_ = 4096;
            hop_size_ = 1024;
            break;
    }
}

bool PerformanceModeConfig::isFeatureEnabled(const std::string& feature) const {
    // In Light mode, some advanced features may be disabled
    if (mode_ == PerformanceMode::Light) {
        if (feature == "cpp" || feature == "spectral_tilt") {
            return false;  // Disable expensive features in light mode
        }
    }
    return true;
}

} // namespace yvc
