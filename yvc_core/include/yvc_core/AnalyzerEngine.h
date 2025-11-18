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
#include "FormantAnalyzer.h"

namespace yvc {

    /// <summary>
    /// Coordinates the audio buffer with the individual analyzers and publishes
    /// aggregated metrics via the MetricsBus.
    /// </summary>
    class AnalyzerEngine {
    public:
        /// <summary>
        /// Constructs a new AnalyzerEngine with the specified configuration.
        /// </summary>
        /// <param name="config">Audio configuration parameters</param>
        /// <param name="bus">Reference to the metrics bus for publishing results</param>
        /// <param name="mode">Performance mode (default: Standard)</param>
        AnalyzerEngine(const AudioConfig& config, MetricsBus& bus, PerformanceMode mode = PerformanceMode::Mode_Standard);

        /// <summary>
        /// Process a block of audio samples. When enough samples are accumulated for
        /// the configured FFT size a new set of AnalysisResults is produced and
        /// written to the MetricsBus.
        /// </summary>
        /// <param name="samples">Pointer to audio samples</param>
        /// <param name="num_samples">Number of samples to process</param>
        /// <param name="timestamp">Timestamp of the audio block</param>
        void process(const Sample* samples, size_t num_samples, double timestamp);

        /// <summary>
        /// Change performance mode at runtime.
        /// </summary>
        /// <param name="mode">New performance mode to apply</param>
        void setPerformanceMode(PerformanceMode mode);

        /// <summary>
        /// Gets the audio configuration.
        /// </summary>
        /// <returns>Reference to the current audio configuration</returns>
        const AudioConfig& getConfig() const { return audio_config_; }

        /// <summary>
        /// Gets the FFT size used for analysis.
        /// </summary>
        /// <returns>FFT size in samples</returns>
        uint32_t getFFTSize() const { return fft_size_; }

        /// <summary>
        /// Gets the hop size used for analysis.
        /// </summary>
        /// <returns>Hop size in samples</returns>
        uint32_t getHopSize() const { return hop_size_; }

        /// <summary>
        /// Gets the current performance mode.
        /// </summary>
        /// <returns>Current performance mode</returns>
        PerformanceMode getPerformanceMode() const { return audio_config_.mode; }

    private:
        /// <summary>
        /// Rebuilds all analyzers based on the current configuration.
        /// </summary>
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
        FormantAnalyzer formant_analyzer_;

        PerformanceModeConfig performance_config_;
        uint32_t fft_size_;
        uint32_t hop_size_;

        uint64_t processed_samples_ = 0;
        double stream_start_timestamp_ = 0.0;
        bool stream_timestamp_initialized_ = false;
    };

} // namespace yvc
