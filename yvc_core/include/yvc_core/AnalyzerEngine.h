#pragma once

#include <cstdint>

#include "AudioBuffer.h"
#include "CPPAnalyzer.h"
#include "F0Detector.h"
#include "HNRAnalyzer.h"
#include "LevelAnalyzer.h"
#include "MetricsBus.h"
#include "PerformanceMode.h"
#include "SpectralAnalyzer.h"
#include "VADAnalyzer.h"

namespace yvc {

// Coordinates the audio buffer with the individual analyzers and publishes
// aggregated metrics via the MetricsBus.
class AnalyzerEngine {
public:
    AnalyzerEngine(const AudioConfig& config, MetricsBus& bus, PerformanceMode mode = PerformanceMode::Standard);

    // Process a block of audio samples. When enough samples are accumulated for
    // the configured FFT size a new set of AnalysisResults is produced and
    // written to the MetricsBus.
    void process(const Sample* samples, size_t num_samples, double timestamp);

    // Change performance mode at runtime.
    void setPerformanceMode(PerformanceMode mode);

    const AudioConfig& getConfig() const { return audio_config_; }
    uint32_t getFFTSize() const { return fft_size_; }
    uint32_t getHopSize() const { return hop_size_; }
    PerformanceMode getPerformanceMode() const { return audio_config_.mode; }

private:
    void rebuildAnalyzers();

    AudioConfig audio_config_;
    AudioBuffer audio_buffer_;
    MetricsBus& metrics_bus_;

    F0Detector f0_detector_;
    LevelAnalyzer level_analyzer_;
    CPPAnalyzer cpp_analyzer_;
    HNRAnalyzer hnr_analyzer_;
    SpectralAnalyzer spectral_analyzer_;
    VADAnalyzer vad_analyzer_;

    PerformanceModeConfig performance_config_;
    uint32_t fft_size_;
    uint32_t hop_size_;

    uint64_t processed_samples_ = 0;
    double stream_start_timestamp_ = 0.0;
    bool stream_timestamp_initialized_ = false;
};

} // namespace yvc
