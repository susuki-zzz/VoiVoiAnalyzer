// VoiVoi Core Library - Preprocessor Interface
// License: MIT
// Purpose: Base interface for audio preprocessing plugins

#pragma once

#include <string>
#include <unordered_map>
#include "Types.h"

namespace yvc {

// Preprocessor interface for audio effects chain
// Preprocessing affects monitoring/recording paths only, NOT analysis
// Analysis always uses dry (unprocessed) signal
class IPreprocessor {
public:
    virtual ~IPreprocessor() = default;
    
    // Process audio samples in-place or to output buffer
    // @param in: Input audio samples
    // @param out: Output audio samples (can be same as in for in-place)
    // @param n: Number of samples to process
    virtual void process(const float* in, float* out, size_t n) = 0;
    
    // Get the latency this preprocessor introduces (in samples)
    // Used for latency compensation in future implementations
    virtual int latency_samples() const { return 0; }
    
    // Get preprocessor name
    virtual const char* name() const = 0;
    
    // Set preprocessor parameters
    // @param kv: Key-value pairs of parameter name to value
    virtual void setParams(const std::unordered_map<std::string, float>& kv) = 0;
    
    // Get current parameter values
    virtual std::unordered_map<std::string, float> getParams() const = 0;
};

// Preprocessor flags for apply targets
enum class PreprocessTarget {
    Monitor = 0x01,  // Apply to monitoring output
    Record = 0x02    // Apply to recording output
};

constexpr int toMask(PreprocessTarget target) {
    return static_cast<int>(target);
}

constexpr PreprocessTarget operator|(PreprocessTarget lhs, PreprocessTarget rhs) {
    return static_cast<PreprocessTarget>(toMask(lhs) | toMask(rhs));
}

constexpr PreprocessTarget operator&(PreprocessTarget lhs, PreprocessTarget rhs) {
    return static_cast<PreprocessTarget>(toMask(lhs) & toMask(rhs));
}

constexpr bool any(PreprocessTarget value) {
    return toMask(value) != 0;
}

constexpr bool matches(PreprocessTarget configured, PreprocessTarget query) {
    return any(configured & query);
}

} // namespace yvc
