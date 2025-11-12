// VoiVoi Core Library - Analyzer Interface
// License: MIT
// Purpose: Base interface for audio analysis components

#pragma once

#include "Types.h"

namespace yvc {

// Base analyzer interface following YunoVoiceCoach specification
class IAnalyzer {
public:
    virtual ~IAnalyzer() = default;
    
    // Analyze audio samples and populate metrics
    // @param mono: Input audio samples (mono)
    // @param n: Number of samples
    // @param sr: Sample rate in Hz
    // @param t0: Frame start time in seconds
    // @param out: Output metrics structure
    virtual void analyze(const float* mono, size_t n, double sr, double t0, AnalysisResults& out) = 0;
    
    // Get analyzer name
    virtual const char* name() const = 0;
};

} // namespace yvc
