// VoiVoi Offline Analysis Tool - File Processor
// License: GPLv3

#pragma once

#include <yvc_core/Types.h>
#include <yvc_core/AudioBuffer.h>
#include <yvc_core/F0Detector.h>
#include <yvc_core/LevelAnalyzer.h>
#include <yvc_core/CPPAnalyzer.h>
#include <yvc_core/HNRAnalyzer.h>
#include <yvc_core/SpectralAnalyzer.h>
#include <yvc_core/VADAnalyzer.h>
#include <string>
#include <vector>
#include <memory>

namespace yvc {

class FileProcessor {
public:
    FileProcessor();
    
    // Process an audio file and generate analysis
    bool processFile(const std::string& input_path, const std::string& output_path);
    
    // Set processing mode
    void setMode(PerformanceMode mode) { mode_ = mode; }
    
    // Get analysis results
    const std::vector<AnalysisResults>& getResults() const { return results_; }
    
private:
    PerformanceMode mode_;
    std::vector<AnalysisResults> results_;
    
    // Analyzers
    std::unique_ptr<F0Detector> f0_detector_;
    std::unique_ptr<LevelAnalyzer> level_analyzer_;
    std::unique_ptr<CPPAnalyzer> cpp_analyzer_;
    std::unique_ptr<HNRAnalyzer> hnr_analyzer_;
    std::unique_ptr<SpectralAnalyzer> spectral_analyzer_;
    std::unique_ptr<VADAnalyzer> vad_analyzer_;
    
    // Initialize analyzers
    void initializeAnalyzers(const AudioConfig& config);
    
    // Process a chunk of audio
    AnalysisResults processChunk(const Sample* samples, size_t num_samples, double timestamp);
    
    // Write results to file
    bool writeResults(const std::string& output_path);
};

} // namespace yvc
