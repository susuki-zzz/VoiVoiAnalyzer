// VoiVoi Core Library - Preprocessor Interface
// License: MIT
// Purpose: Base interface for audio preprocessing plugins

#pragma once

#include <string>
#include <unordered_map>
#include "Types.h"

namespace yvc {

    /// <summary>
    /// Preprocessor interface for audio effects chain.
    /// Preprocessing affects monitoring/recording paths only, NOT analysis.
    /// Analysis always uses dry (unprocessed) signal.
    /// </summary>
    class IPreprocessor {
    public:
        virtual ~IPreprocessor() = default;

        /// <summary>
        /// Processes audio samples in-place or to output buffer.
        /// </summary>
        /// <param name="in">Input audio samples</param>
        /// <param name="out">Output audio samples (can be same as in for in-place)</param>
        /// <param name="n">Number of samples to process</param>
        virtual void process(const float* in, float* out, size_t n) = 0;

        /// <summary>
        /// Gets the latency this preprocessor introduces (in samples).
        /// Used for latency compensation in future implementations.
        /// </summary>
        /// <returns>Latency in samples</returns>
        virtual int latency_samples() const { return 0; }

        /// <summary>
        /// Gets preprocessor name.
        /// </summary>
        /// <returns>Preprocessor name as C-string</returns>
        virtual const char* name() const = 0;

        /// <summary>
        /// Sets preprocessor parameters.
        /// </summary>
        /// <param name="kv">Key-value pairs of parameter name to value</param>
        virtual void setParams(const std::unordered_map<std::string, float>& kv) = 0;

        /// <summary>
        /// Gets current parameter values.
        /// </summary>
        /// <returns>Map of parameter name to value</returns>
        virtual std::unordered_map<std::string, float> getParams() const = 0;
    };

    /// <summary>
    /// Preprocessor flags for apply targets.
    /// </summary>
    enum class PreprocessTarget {
        Monitor = 0x01,  // Apply to monitoring output
        Record = 0x02    // Apply to recording output
    };

    /// <summary>
    /// Converts PreprocessTarget to integer mask.
    /// </summary>
    /// <param name="target">Preprocess target</param>
    /// <returns>Integer mask value</returns>
    constexpr int toMask(PreprocessTarget target) {
        return static_cast<int>(target);
    }

    /// <summary>
    /// Bitwise OR operator for PreprocessTarget.
    /// </summary>
    constexpr PreprocessTarget operator|(PreprocessTarget lhs, PreprocessTarget rhs) {
        return static_cast<PreprocessTarget>(toMask(lhs) | toMask(rhs));
    }

    /// <summary>
    /// Bitwise AND operator for PreprocessTarget.
    /// </summary>
    constexpr PreprocessTarget operator&(PreprocessTarget lhs, PreprocessTarget rhs) {
        return static_cast<PreprocessTarget>(toMask(lhs) & toMask(rhs));
    }

    /// <summary>
    /// Checks if any target flags are set.
    /// </summary>
    /// <param name="value">Target value to check</param>
    /// <returns>True if any flags are set</returns>
    constexpr bool any(PreprocessTarget value) {
        return toMask(value) != 0;
    }

    /// <summary>
    /// Checks if configured targets match query.
    /// </summary>
    /// <param name="configured">Configured targets</param>
    /// <param name="query">Query targets</param>
    /// <returns>True if any configured targets match query</returns>
    constexpr bool matches(PreprocessTarget configured, PreprocessTarget query) {
        return any(configured & query);
    }

} // namespace yvc
