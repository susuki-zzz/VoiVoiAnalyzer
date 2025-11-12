// VoiVoi Offline Analysis Tool - File Processor Implementation
// License: GPLv3

#include "FileProcessor.h"
#include <fstream>
#include <iostream>
#include <iomanip>

namespace yvc {

FileProcessor::FileProcessor()
    : mode_(PerformanceMode::Diagnostic) {
}

void FileProcessor::initializeAnalyzers(const AudioConfig& config) {
    f0_detector_ = std::make_unique<F0Detector>(config);
    level_analyzer_ = std::make_unique<LevelAnalyzer>(config);
    cpp_analyzer_ = std::make_unique<CPPAnalyzer>(config);
    hnr_analyzer_ = std::make_unique<HNRAnalyzer>(config);
    spectral_analyzer_ = std::make_unique<SpectralAnalyzer>(config);
    vad_analyzer_ = std::make_unique<VADAnalyzer>(config);
}

bool FileProcessor::processFile(const std::string& input_path, const std::string& output_path) {
    std::cout << "Processing file: " << input_path << std::endl;
    
    // Configure audio settings
    AudioConfig config;
    config.sample_rate = SAMPLE_RATE_48K;
    config.mode = mode_;
    
    // Initialize analyzers
    initializeAnalyzers(config);
    
    // For now, create a simple placeholder that would load and process WAV files
    // In a real implementation, we would use a library like libsndfile or similar
    
    std::cout << "Note: Audio file loading not yet implemented" << std::endl;
    std::cout << "This tool is designed to process up to 3 hours of audio" << std::endl;
    
    // Placeholder: Generate some sample results
    for (int i = 0; i < 10; ++i) {
        AnalysisResults result;
        result.timestamp = i * 0.1;
        result.f0 = 150.0f;
        result.rms = 0.1f;
        results_.push_back(result);
    }
    
    // Write results
    return writeResults(output_path);
}

AnalysisResults FileProcessor::processChunk(const Sample* samples, size_t num_samples, double timestamp) {
    AnalysisResults results;
    results.timestamp = timestamp;
    
    // Level analysis
    auto level_results = level_analyzer_->analyze(samples, num_samples);
    results.rms = level_results.rms;
    results.peak = level_results.peak;
    results.crest_factor = level_results.crest_factor;
    
    // F0 detection
    results.f0 = f0_detector_->detect(samples, num_samples, results.f0_valid);
    
    // CPP analysis
    results.cpp = cpp_analyzer_->analyze(samples, num_samples);
    
    // HNR analysis
    if (results.f0_valid) {
        results.hnr = hnr_analyzer_->analyze(samples, num_samples, results.f0);
    }
    
    // Spectral analysis
    auto spectral_results = spectral_analyzer_->analyze(samples, num_samples);
    results.spectral_tilt = spectral_results.spectral_tilt;
    results.s_centroid = spectral_results.s_centroid;
    results.s_detected = spectral_results.s_detected;
    
    // VAD analysis
    auto vad_results = vad_analyzer_->analyze(samples, num_samples, results.rms);
    results.voice_active = vad_results.voice_active;
    results.speech_rate = vad_results.speech_rate;
    results.pause_ratio = vad_results.pause_ratio;
    
    return results;
}

bool FileProcessor::writeResults(const std::string& output_path) {
    std::ofstream out(output_path);
    if (!out.is_open()) {
        std::cerr << "Failed to open output file: " << output_path << std::endl;
        return false;
    }
    
    // Write CSV header
    out << "timestamp,f0,f0_valid,rms,peak,crest_factor,cpp,hnr,spectral_tilt,"
        << "s_centroid,s_detected,voice_active,speech_rate,pause_ratio" << std::endl;
    
    // Write results
    for (const auto& result : results_) {
        out << std::fixed << std::setprecision(6)
            << result.timestamp << ","
            << result.f0 << ","
            << result.f0_valid << ","
            << result.rms << ","
            << result.peak << ","
            << result.crest_factor << ","
            << result.cpp << ","
            << result.hnr << ","
            << result.spectral_tilt << ","
            << result.s_centroid << ","
            << result.s_detected << ","
            << result.voice_active << ","
            << result.speech_rate << ","
            << result.pause_ratio << std::endl;
    }
    
    out.close();
    std::cout << "Results written to: " << output_path << std::endl;
    return true;
}

} // namespace yvc
