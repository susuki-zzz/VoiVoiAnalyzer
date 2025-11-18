// VoiVoi Offline Analysis Tool - File Processor Implementation
// License: GPLv3

#include "FileProcessor.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <limits>
#include <sstream>
#include <vector>

#include <yvc_core/PerformanceMode.h>
#include <yvc_core/Logger.h>

namespace {

    std::string escapeJsonString(const std::string& input) {
        std::ostringstream escaped;
        for(char c : input) {
            switch(c) {
            case '\\':
            escaped << "\\\\";
            break;
            case '"':
            escaped << "\\\"";
            break;
            case '\n':
            escaped << "\\n";
            break;
            case '\r':
            escaped << "\\r";
            break;
            case '\t':
            escaped << "\\t";
            break;
            default:
            escaped << c;
            break;
            }
        }
        return escaped.str();
    }

} // namespace

namespace yvc {

    FileProcessor::FileProcessor()
        : mode_(PerformanceMode::Mode_Diagnostic) { }

    /// <summary>
    /// Initializes analyzer instances based on supplied audio configuration.
    /// </summary>
    /// <param name="config">The audio configuration used to initialize the analyzers.</param>
    void FileProcessor::initializeAnalyzers(const AudioConfig& config) {
        f0_detector_ = std::make_unique<F0Detector>(config);
        level_analyzer_ = std::make_unique<LevelAnalyzer>(config);
        cpp_analyzer_ = std::make_unique<CPPAnalyzer>(config);
        hnr_analyzer_ = std::make_unique<HNRAnalyzer>(config);
        spectral_analyzer_ = std::make_unique<SpectralAnalyzer>(config);
        vad_analyzer_ = std::make_unique<VADAnalyzer>(config);
        vad_analyzer_->reset();
    }

    /// <summary>
    /// Processes the input file in 30s chunks with 1s overlap and writes outputs.
    /// </summary>
    /// <param name="input_path">The path to the input WAV file.</param>
    /// <param name="output_path">The base path for output files (results, summary, anomalies, heatmap).</param>
    /// <returns>True if processing is successful, false otherwise.</returns>
    bool FileProcessor::processFile(const std::string& input_path, const std::string& output_path) {
        LOG_INFOF("Processing file: %s", input_path.c_str());

        results_.clear();

        std::vector<Sample> audio_samples;
        SampleRate sample_rate = 0;
        if(!loadWavFile(input_path, audio_samples, sample_rate)) {
            return false;
        }

        if(audio_samples.empty() || sample_rate == 0) {
            LOG_ERRORF("No audio samples decoded from: %s", input_path.c_str());
            return false;
        }

        constexpr double kMaxDurationSeconds = 3.0 * 60.0 * 60.0;
        const size_t max_samples = static_cast<size_t>(kMaxDurationSeconds * static_cast<double>(sample_rate));
        if(audio_samples.size() > max_samples) {
            LOG_WARNF("Input exceeds 3 hour limit. Truncating to %.1f seconds.", kMaxDurationSeconds);
            audio_samples.resize(max_samples);
        }

        AudioConfig config;
        config.sample_rate = sample_rate;
        config.num_channels = 1;
        config.mode = mode_;
        config = PerformanceModeConfig::applyToConfig(config, mode_);

        initializeAnalyzers(config);

        const double chunk_seconds = 30.0;
        const double overlap_seconds = 1.0;
        const size_t chunk_samples = static_cast<size_t>(chunk_seconds * static_cast<double>(sample_rate));
        const size_t overlap_samples = static_cast<size_t>(overlap_seconds * static_cast<double>(sample_rate));
        const size_t step = chunk_samples > overlap_samples ? (chunk_samples - overlap_samples) : chunk_samples;

        if(chunk_samples == 0 || step == 0) {
            LOG_ERRORF("Invalid chunk configuration for sample rate: %d", sample_rate);
            return false;
        }

        results_.reserve((audio_samples.size() / step) + 1);

        for(size_t start = 0; start < audio_samples.size(); start += step) {
            const size_t end = std::min(start + chunk_samples, audio_samples.size());
            const size_t count = end - start;
            if(count == 0) {
                break;
            }

            const Sample* chunk_ptr = audio_samples.data() + start;
            const double timestamp = static_cast<double>(start) / static_cast<double>(sample_rate);
            results_.push_back(processChunk(chunk_ptr, count, timestamp));

            if(end == audio_samples.size()) {
                break;
            }
        }

        const auto anomalies = detectAnomalies();
        const SummaryStats summary = computeSummary(sample_rate, audio_samples.size());
        const auto heatmap = buildHeatmap();

        const bool csv_written = writeResults(output_path);
        const bool summary_written = writeSummary(output_path, summary);
        const bool anomaly_written = writeAnomalies(output_path, anomalies);
        const bool heatmap_written = writeHeatmap(output_path, heatmap);
        return csv_written && summary_written && anomaly_written && heatmap_written;
    }

    /// <summary>
    /// Processes a single chunk and returns analysis results.
    /// </summary>
    /// <param name="samples">Pointer to the audio sample data.</param>
    /// <param name="num_samples">Number of samples in the chunk.</param>
    /// <param name="timestamp">Timestamp of the chunk in seconds.</param>
    /// <returns>Analysis results for the chunk.</returns>
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
        if(results.f0_valid) {
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

    /// <summary>
    /// Writes per-chunk results as CSV.
    /// </summary>
    /// <param name="output_path">The path to the output CSV file.</param>
    /// <returns>True if writing is successful, false otherwise.</returns>
    bool FileProcessor::writeResults(const std::string& output_path) {
        std::filesystem::path base_path(output_path);
        if(base_path.has_parent_path()) {
            std::filesystem::create_directories(base_path.parent_path());
        }

        std::ofstream out(base_path);
        if(!out.is_open()) {
            LOG_ERRORF("Failed to open output file: %s", output_path.c_str());
            return false;
        }

        // Write CSV header
        out << "timestamp,f0,f0_valid,rms,peak,crest_factor,cpp,hnr,spectral_tilt,"
            << "s_centroid,s_detected,voice_active,speech_rate,pause_ratio" << std::endl;

        // Write results
        for(const auto& result : results_) {
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
        LOG_INFOF("Results written to: %s", base_path.string().c_str());
        return true;
    }

    /// <summary>
    /// Loads a mono WAV file into float samples, mixing down channels.
    /// </summary>
    /// <param name="input_path">The path to the input WAV file.</param>
    /// <param name="samples">Vector to store the decoded audio samples.</param>
    /// <param name="sample_rate">Sample rate of the audio file.</param>
    /// <returns>True if loading is successful, false otherwise.</returns>
    bool FileProcessor::loadWavFile(const std::string& input_path, std::vector<Sample>& samples, SampleRate& sample_rate) {
        std::ifstream in(input_path, std::ios::binary);
        if(!in.is_open()) {
            LOG_ERRORF("Failed to open input file: %s", input_path.c_str());
            return false;
        }

        auto readFourCC = [](const std::array<char, 4>& tag) {
            return std::string(tag.begin(), tag.end());
        };

        std::array<char, 4> chunk_id{};
        std::array<char, 4> format{};
        uint32_t chunk_size = 0;

        in.read(chunk_id.data(), 4);
        in.read(reinterpret_cast<char*>(&chunk_size), sizeof(chunk_size));
        in.read(format.data(), 4);
        if(!in || readFourCC(chunk_id) != "RIFF" || readFourCC(format) != "WAVE") {
            LOG_ERRORF("Unsupported or corrupt WAV file: %s", input_path.c_str());
            return false;
        }

        bool fmt_found = false;
        bool data_found = false;
        uint16_t audio_format = 0;
        uint16_t num_channels = 0;
        uint16_t bits_per_sample = 0;
        uint32_t byte_rate = 0;
        uint16_t block_align = 0;
        std::vector<char> data_chunk;

        while(in && !(fmt_found && data_found)) {
            if(!in.read(chunk_id.data(), 4)) {
                break;
            }
            if(!in.read(reinterpret_cast<char*>(&chunk_size), sizeof(chunk_size))) {
                break;
            }

            if(readFourCC(chunk_id) == "fmt ") {
                std::vector<char> fmt_data(chunk_size);
                if(!in.read(fmt_data.data(), fmt_data.size())) {
                    LOG_ERROR("Failed to read fmt chunk");
                    return false;
                }

                std::memcpy(&audio_format, fmt_data.data(), sizeof(audio_format));
                std::memcpy(&num_channels, fmt_data.data() + 2, sizeof(num_channels));
                std::memcpy(&sample_rate, fmt_data.data() + 4, sizeof(sample_rate));
                std::memcpy(&byte_rate, fmt_data.data() + 8, sizeof(byte_rate));
                std::memcpy(&block_align, fmt_data.data() + 12, sizeof(block_align));
                std::memcpy(&bits_per_sample, fmt_data.data() + 14, sizeof(bits_per_sample));

                fmt_found = true;

                const size_t remaining = chunk_size > fmt_data.size() ? chunk_size - fmt_data.size() : 0;
                if(remaining > 0) {
                    in.seekg(static_cast<std::streamoff>(remaining), std::ios::cur);
                }
            }
            else if(readFourCC(chunk_id) == "data") {
                data_chunk.resize(chunk_size);
                if(!in.read(data_chunk.data(), data_chunk.size())) {
                    LOG_ERROR("Failed to read data chunk");
                    return false;
                }
                data_found = true;
            }
            else {
                in.seekg(static_cast<std::streamoff>(chunk_size), std::ios::cur);
            }

            if(chunk_size % 2 == 1) {
                in.seekg(1, std::ios::cur);  // Padding byte for word alignment
            }
        }

        if(!fmt_found || !data_found) {
            LOG_ERRORF("Incomplete WAV file: %s", input_path.c_str());
            return false;
        }

        if(num_channels == 0 || block_align == 0) {
            LOG_ERROR("Invalid WAV channel configuration");
            return false;
        }

        if(audio_format != 1 && audio_format != 3) {
            LOG_ERROR("Unsupported WAV encoding (only PCM and IEEE float supported)");
            return false;
        }

        const size_t frame_count = data_chunk.size() / block_align;
        if(frame_count == 0) {
            return true;
        }

        samples.resize(frame_count);
        const char* data_ptr = data_chunk.data();
        const double int16_scale = 1.0 / 32768.0;
        const double int24_scale = 1.0 / 8388608.0;
        const double int32_scale = 1.0 / 2147483648.0;

        for(size_t frame = 0; frame < frame_count; ++frame) {
            double accumulator = 0.0;
            for(uint16_t channel = 0; channel < num_channels; ++channel) {
                double sample_value = 0.0;
                if(audio_format == 3 && bits_per_sample == 32) {
                    float value = 0.0f;
                    std::memcpy(&value, data_ptr, sizeof(value));
                    data_ptr += sizeof(value);
                    sample_value = static_cast<double>(value);
                }
                else if(bits_per_sample == 8) {
                    const uint8_t value = static_cast<uint8_t>(*data_ptr++);
                    sample_value = (static_cast<double>(value) - 128.0) / 128.0;
                }
                else if(bits_per_sample == 16) {
                    int16_t value = 0;
                    std::memcpy(&value, data_ptr, sizeof(value));
                    data_ptr += sizeof(value);
                    sample_value = static_cast<double>(value) * int16_scale;
                }
                else if(bits_per_sample == 24) {
                    int32_t value = static_cast<uint8_t>(data_ptr[0]) |
                        (static_cast<uint8_t>(data_ptr[1]) << 8) |
                        (static_cast<uint8_t>(data_ptr[2]) << 16);
                    if(value & 0x800000) {
                        value |= ~0xFFFFFF;
                    }
                    data_ptr += 3;
                    sample_value = static_cast<double>(value) * int24_scale;
                }
                else if(bits_per_sample == 32) {
                    int32_t value = 0;
                    std::memcpy(&value, data_ptr, sizeof(value));
                    data_ptr += sizeof(value);
                    sample_value = static_cast<double>(value) * int32_scale;
                }
                else {
                    LOG_ERRORF("Unsupported bits per sample: %d", bits_per_sample);
                    return false;
                }

                accumulator += sample_value;
            }

            const double averaged = accumulator / static_cast<double>(num_channels);
            samples[frame] = static_cast<Sample>(std::clamp(averaged, -1.0, 1.0));
        }

        return true;
    }

    /// <summary>
    /// Computes summary metrics across processed chunks.
    /// </summary>
    /// <param name="sample_rate">The sample rate of the audio.</param>
    /// <param name="processed_samples">The number of samples processed.</param>
    /// <returns>A structure containing the summary statistics.</returns>
    FileProcessor::SummaryStats FileProcessor::computeSummary(SampleRate sample_rate, size_t processed_samples) const {
        SummaryStats summary;
        summary.sample_rate = sample_rate;
        summary.chunk_count = results_.size();
        summary.duration_seconds = sample_rate > 0 ? static_cast<double>(processed_samples) / static_cast<double>(sample_rate) : 0.0;

        double f0_sum = 0.0;
        double rms_sum = 0.0;
        double cpp_sum = 0.0;
        double hnr_sum = 0.0;
        double tilt_sum = 0.0;
        double centroid_sum = 0.0;
        double speech_rate_sum = 0.0;
        double pause_ratio_sum = 0.0;
        double voice_active_sum = 0.0;
        double peak_max = std::numeric_limits<double>::lowest();

        for(const auto& result : results_) {
            if(result.f0_valid) {
                f0_sum += result.f0;
                summary.f0_measurements++;
            }
            rms_sum += result.rms;
            cpp_sum += result.cpp;
            hnr_sum += result.hnr;
            tilt_sum += result.spectral_tilt;
            centroid_sum += result.s_centroid;
            speech_rate_sum += result.speech_rate;
            pause_ratio_sum += result.pause_ratio;
            voice_active_sum += result.voice_active ? 1.0 : 0.0;
            peak_max = std::max(peak_max, static_cast<double>(result.peak));
        }

        if(summary.chunk_count > 0) {
            const double denom = static_cast<double>(summary.chunk_count);
            summary.average_rms = rms_sum / denom;
            summary.average_cpp = cpp_sum / denom;
            summary.average_hnr = hnr_sum / denom;
            summary.average_spectral_tilt = tilt_sum / denom;
            summary.average_s_centroid = centroid_sum / denom;
            summary.average_speech_rate = speech_rate_sum / denom;
            summary.average_pause_ratio = pause_ratio_sum / denom;
            summary.voice_activity_ratio = voice_active_sum / denom;
        }

        if(summary.f0_measurements > 0) {
            summary.average_f0 = f0_sum / static_cast<double>(summary.f0_measurements);
        }

        if(summary.chunk_count > 0) {
            summary.max_peak = peak_max;
        }
        else {
            summary.max_peak = 0.0;
        }

        return summary;
    }

    /// <summary>
    /// Writes summary JSON next to output CSV.
    /// </summary>
    /// <param name="output_path">The path to the output CSV file.</param>
    /// <param name="summary">The summary statistics to write.</param>
    /// <returns>True if writing is successful, false otherwise.</returns>
    bool FileProcessor::writeSummary(const std::string& output_path, const SummaryStats& summary) const {
        std::filesystem::path base_path(output_path);
        base_path.replace_extension(".summary.json");

        if(base_path.has_parent_path()) {
            std::filesystem::create_directories(base_path.parent_path());
        }

        std::ofstream out(base_path);
        if(!out.is_open()) {
            LOG_ERRORF("Failed to open summary file: %s", base_path.string().c_str());
            return false;
        }

        out << std::fixed << std::setprecision(6);
        out << "{\n";
        out << "  \"sample_rate\": " << summary.sample_rate << ",\n";
        out << "  \"duration_seconds\": " << summary.duration_seconds << ",\n";
        out << "  \"chunks\": " << summary.chunk_count << ",\n";
        out << "  \"f0_measurements\": " << summary.f0_measurements << ",\n";
        out << "  \"average_f0\": " << summary.average_f0 << ",\n";
        out << "  \"average_rms\": " << summary.average_rms << ",\n";
        out << "  \"max_peak\": " << summary.max_peak << ",\n";
        out << "  \"average_cpp\": " << summary.average_cpp << ",\n";
        out << "  \"average_hnr\": " << summary.average_hnr << ",\n";
        out << "  \"average_spectral_tilt\": " << summary.average_spectral_tilt << ",\n";
        out << "  \"average_s_centroid\": " << summary.average_s_centroid << ",\n";
        out << "  \"average_speech_rate\": " << summary.average_speech_rate << ",\n";
        out << "  \"average_pause_ratio\": " << summary.average_pause_ratio << ",\n";
        out << "  \"voice_activity_ratio\": " << summary.voice_activity_ratio << "\n";
        out << "}\n";

        LOG_INFOF("Summary written to: %s", base_path.string().c_str());
        return true;
    }

    /// <summary>
    /// Scans chunk results to produce a simple anomaly list.
    /// </summary>
    /// <returns>A vector of detected anomalies.</returns>
    std::vector<FileProcessor::Anomaly> FileProcessor::detectAnomalies() const {
        std::vector<Anomaly> anomalies;
        anomalies.reserve(results_.size());

        const float kSilenceRmsThreshold = 0.01f;
        const float kLowCppThreshold = 5.0f;
        const float kLowHnrThreshold = 0.0f;
        const float kHighPauseRatio = 0.6f;

        for(const auto& result : results_) {
            if(!result.voice_active && result.rms < kSilenceRmsThreshold) {
                anomalies.push_back({ "Silence", result.timestamp,
                                    "Voice activity was not detected in this window.",
                                    static_cast<double>(result.rms) });
            }

            if(result.f0_valid && (result.f0 < 50.0f || result.f0 > 500.0f)) {
                anomalies.push_back({ "PitchOutOfRange", result.timestamp,
                                    "Fundamental frequency fell outside the expected range (50-500 Hz).",
                                    static_cast<double>(result.f0) });
            }

            if(result.cpp < kLowCppThreshold) {
                anomalies.push_back({ "LowCPP", result.timestamp,
                                    "Cepstral peak prominence indicates reduced voice clarity.",
                                    static_cast<double>(result.cpp) });
            }

            if(result.hnr < kLowHnrThreshold) {
                anomalies.push_back({ "LowHNR", result.timestamp,
                                    "Harmonics-to-noise ratio suggests noisy phonation.",
                                    static_cast<double>(result.hnr) });
            }

            if(result.pause_ratio > kHighPauseRatio) {
                anomalies.push_back({ "HighPauseRatio", result.timestamp,
                                    "Detected extended pauses relative to speech activity.",
                                    static_cast<double>(result.pause_ratio) });
            }
        }

        return anomalies;
    }

    /// <summary>
    /// Writes anomaly list as JSON next to output CSV.
    /// </summary>
    /// <param name="output_path">The path to the output CSV file.</param>
    /// <param name="anomalies">The vector of anomalies to write.</param>
    /// <returns>True if writing is successful, false otherwise.</returns>
    bool FileProcessor::writeAnomalies(const std::string& output_path,
                                       const std::vector<Anomaly>& anomalies) const {
        std::filesystem::path base_path(output_path);
        base_path.replace_extension(".anomalies.json");

        if(base_path.has_parent_path()) {
            std::filesystem::create_directories(base_path.parent_path());
        }

        std::ofstream out(base_path);
        if(!out.is_open()) {
            LOG_ERRORF("Failed to open anomalies file: %s", base_path.string().c_str());
            return false;
        }

        out << std::fixed << std::setprecision(6);
        out << "{\n";
        out << "  \"anomalies\": [\n";
        for(size_t i = 0; i < anomalies.size(); ++i) {
            const auto& anomaly = anomalies[i];
            out << "    {\n";
            out << "      \"type\": \"" << escapeJsonString(anomaly.type) << "\",\n";
            out << "      \"timestamp\": " << anomaly.timestamp << ",\n";
            out << "      \"description\": \"" << escapeJsonString(anomaly.description) << "\",\n";
            out << "      \"score\": " << anomaly.score << "\n";
            out << "    }";
            if(i + 1 < anomalies.size()) {
                out << ",";
            }
            out << "\n";
        }
        out << "  ]\n";
        out << "}\n";

        LOG_INFOF("Anomalies written to: %s", base_path.string().c_str());
        return true;
    }

    /// <summary>
    /// Builds heatmap points (F0/RMS/SpeechRate/CPP) per chunk.
    /// </summary>
    /// <returns>A vector of heatmap points.</returns>
    std::vector<FileProcessor::HeatmapPoint> FileProcessor::buildHeatmap() const {
        std::vector<HeatmapPoint> heatmap;
        heatmap.reserve(results_.size());

        for(size_t i = 0; i < results_.size(); ++i)
        {
            const auto& result = results_[i];
            heatmap.push_back({ i,
                               result.timestamp,
                               result.f0,
                               result.rms,
                               result.speech_rate,
                               result.cpp });
        }

        return heatmap;
    }

    /// <summary>
    /// Writes heatmap CSV next to output CSV.
    /// </summary>
    /// <param name="output_path">The path to the output CSV file.</param>
    /// <param name="heatmap">The vector of heatmap points to write.</param>
    /// <returns>True if writing is successful, false otherwise.</returns>
    bool FileProcessor::writeHeatmap(const std::string& output_path,
                                     const std::vector<HeatmapPoint>& heatmap) const {
        std::filesystem::path base_path(output_path);
        base_path.replace_extension(".heatmap.csv");

        if(base_path.has_parent_path()) {
            std::filesystem::create_directories(base_path.parent_path());
        }

        std::ofstream out(base_path);
        if(!out.is_open()) {
            LOG_ERRORF("Failed to open heatmap file: %s", base_path.string().c_str());
            return false;
        }

        out << "chunk_index,timestamp,f0,rms,speech_rate,cpp\n";
        out << std::fixed << std::setprecision(6);
        for(const auto& point : heatmap) {
            out << point.index << ","
                << point.timestamp << ","
                << point.f0 << ","
                << point.rms << ","
                << point.speech_rate << ","
                << point.cpp << "\n";
        }

        LOG_INFOF("Heatmap written to: %s", base_path.string().c_str());
        return true;
    }

} // namespace yvc
