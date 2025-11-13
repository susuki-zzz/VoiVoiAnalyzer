// VoiVoi Core Library - CPP Analyzer
// License: MIT
// Purpose: Cepstral Peak Prominence analysis for voice quality

#pragma once

#include "Types.h"
#include <memory>

namespace yvc {

class CPPAnalyzer {
public:
    explicit CPPAnalyzer(const AudioConfig& config);
    ~CPPAnalyzer();
    CPPAnalyzer(CPPAnalyzer&&) noexcept;
    CPPAnalyzer& operator=(CPPAnalyzer&&) noexcept;
    CPPAnalyzer(const CPPAnalyzer&) = delete;
    CPPAnalyzer& operator=(const CPPAnalyzer&) = delete;
    
    // Compute CPP (Cepstral Peak Prominence) in dB
    float analyze(const Sample* samples, size_t num_samples);
    
private:
    struct Impl;                  // Pimpl to hide kissfft dependency
    AudioConfig config_;
    std::unique_ptr<Impl> impl_;  // Implementation storage
    
    // Compute cepstrum and find peak prominence
    float computeCPP(const Sample* samples, size_t num_samples);
};

} // namespace yvc
