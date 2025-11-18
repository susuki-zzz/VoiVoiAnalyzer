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

#include <cstddef>

namespace yvc {

    /// <summary>
    ///     Processes audio files offline and emits metrics CSV + summary/anomaly JSON.
    /// </summary>
    class FileProcessor {
    public:
        FileProcessor();

        /// <summary>
        ///     Process an audio file and write analysis artifacts near output_path.
        /// </summary>
        bool processFile(const std::string& input_path,
                         const std::string& output_path);

        /// <summary>
        ///     Set processing performance mode (light/standard/diagnostic).
        /// </summary>
        void setMode(PerformanceMode mode) { mode_ = mode; }

        /// <summary>
        ///     Access processed per-chunk results.
        /// </summary>
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
        AnalysisResults processChunk(const Sample* samples,
                                     size_t num_samples,
                                     double timestamp);

        // Write results to file
        bool writeResults(const std::string& output_path);

        struct SummaryStats {
            size_t chunk_count = 0;
            size_t f0_measurements = 0;
            size_t anomaly_count = 0;
            SampleRate sample_rate = 0;
            double duration_seconds = 0.0;
            double average_f0 = 0.0;
            double average_rms = 0.0;
            double max_peak = 0.0;
            double average_cpp = 0.0;
            double average_hnr = 0.0;
            double average_spectral_tilt = 0.0;
            double average_s_centroid = 0.0;
            double average_speech_rate = 0.0;
            double average_pause_ratio = 0.0;
            double voice_activity_ratio = 0.0;
        };

        struct Anomaly {
            std::string type;
            double timestamp = 0.0;
            std::string description;
            double score = 0.0;
        };
        struct HeatmapPoint {
            size_t index = 0;
            double timestamp = 0.0;
            float f0 = 0.0f;
            float rms = 0.0f;
            float speech_rate = 0.0f;
            float cpp = 0.0f;
        };

        // Load WAV audio data into a mono floating point buffer
        bool loadWavFile(const std::string& input_path,
                         std::vector<Sample>& samples,
                         SampleRate& sample_rate);

        // Compute summary statistics for the processed results
        SummaryStats computeSummary(SampleRate sample_rate,
                                    size_t processed_samples) const;

        // Write summary statistics to disk
        bool writeSummary(const std::string& output_path,
                          const SummaryStats& summary) const;

        // Generate anomaly list from processed results
        std::vector<Anomaly> detectAnomalies() const;

        // Write anomalies to disk
        bool writeAnomalies(const std::string& output_path,
                            const std::vector<Anomaly>& anomalies) const;

        // Generate heatmap points for visualization
        std::vector<HeatmapPoint> buildHeatmap() const;

        // Write heatmap representation to disk
        bool writeHeatmap(const std::string& output_path,
                          const std::vector<HeatmapPoint>& heatmap) const;
    };

} // namespace yvc
