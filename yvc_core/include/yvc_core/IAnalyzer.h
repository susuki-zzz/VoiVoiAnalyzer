// VoiVoi Core Library - Analyzer Interface
// License: MIT
// Purpose: Base interface for audio analysis components

#pragma once

#include "Types.h"

namespace yvc {

/// <summary>
/// Base analyzer interface following YunoVoiceCoach specification.
/// Defines the contract for all audio analysis components.
/// </summary>
class IAnalyzer {
public:
    virtual ~IAnalyzer() = default;
    
    /// <summary>
    /// Analyzes audio samples and populates metrics.
    /// </summary>
    /// <param name="mono">Input audio samples (mono)</param>
    /// <param name="n">Number of samples</param>
    /// <param name="sr">Sample rate in Hz</param>
    /// <param name="t0">Frame start time in seconds</param>
    /// <param name="out">Output metrics structure</param>
    virtual void analyze(const float* mono, size_t n, double sr, double t0, AnalysisResults& out) = 0;
    
    /// <summary>
    /// Gets the analyzer name.
    /// </summary>
    /// <returns>Analyzer name as C-string</returns>
    virtual const char* name() const = 0;
};

} // namespace yvc
