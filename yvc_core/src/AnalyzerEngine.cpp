#include "yvc_core/AnalyzerEngine.h"

namespace yvc {
namespace {
AudioConfig prepareConfig(const AudioConfig& config, PerformanceMode mode) {
    return PerformanceModeConfig::applyToConfig(config, mode);
}
} // namespace

AnalyzerEngine::AnalyzerEngine(const AudioConfig& config, MetricsBus& bus, PerformanceMode mode)
    : audio_config_(prepareConfig(config, mode)),
      audio_buffer_(audio_config_),
      metrics_bus_(bus),
      f0_detector_(audio_config_),
      level_analyzer_(audio_config_),
      cpp_analyzer_(audio_config_),
      hnr_analyzer_(audio_config_),
      spectral_analyzer_(audio_config_),
      vad_analyzer_(audio_config_),
      performance_config_(audio_config_.mode),
      fft_size_(performance_config_.getFFTSize()),
      hop_size_(performance_config_.getHopSize()) {
}

void AnalyzerEngine::process(const Sample* samples, size_t num_samples, double timestamp) {
    if (samples == nullptr || num_samples == 0) {
        return;
    }

    audio_buffer_.addSamples(samples, num_samples);

    if (!stream_timestamp_initialized_) {
        stream_start_timestamp_ = timestamp;
        stream_timestamp_initialized_ = true;
    }

    while (audio_buffer_.hasEnoughSamples(fft_size_)) {
        const auto& buffer = audio_buffer_.getBuffer();
        const Sample* analysis_ptr = buffer.data();
        const size_t analysis_samples = fft_size_;

        AnalysisResults results{};
        const double hop_offset_seconds = static_cast<double>(processed_samples_) /
                                          static_cast<double>(audio_config_.sample_rate);
        results.timestamp = stream_start_timestamp_ + hop_offset_seconds;

        results.f0 = f0_detector_.detect(analysis_ptr, analysis_samples, results.f0_valid);

        const auto level_results = level_analyzer_.analyze(analysis_ptr, analysis_samples);
        results.rms = level_results.rms;
        results.peak = level_results.peak;
        results.crest_factor = level_results.crest_factor;

        results.cpp = cpp_analyzer_.analyze(analysis_ptr, analysis_samples);

        if (results.f0_valid) {
            results.hnr = hnr_analyzer_.analyze(analysis_ptr, analysis_samples, results.f0);
        } else {
            results.hnr = 0.0f;
        }

        const auto spectral_results = spectral_analyzer_.analyze(analysis_ptr, analysis_samples);
        results.spectral_tilt = spectral_results.spectral_tilt;
        results.s_centroid = spectral_results.s_centroid;
        results.s_detected = spectral_results.s_detected;

        const auto vad_results = vad_analyzer_.analyze(analysis_ptr, analysis_samples, results.rms);
        results.voice_active = vad_results.voice_active;
        results.speech_rate = vad_results.speech_rate;
        results.pause_ratio = vad_results.pause_ratio;

        metrics_bus_.write(results);

        audio_buffer_.consumeSamples(hop_size_);
        processed_samples_ += hop_size_;
    }
}

void AnalyzerEngine::setPerformanceMode(PerformanceMode mode) {
    if (audio_config_.mode == mode) {
        return;
    }

    AudioConfig base_config = audio_config_;
    base_config.mode = mode;
    audio_config_ = PerformanceModeConfig::applyToConfig(base_config, mode);
    performance_config_ = PerformanceModeConfig(audio_config_.mode);
    fft_size_ = performance_config_.getFFTSize();
    hop_size_ = performance_config_.getHopSize();

    audio_buffer_ = AudioBuffer(audio_config_);
    processed_samples_ = 0;
    stream_timestamp_initialized_ = false;
    stream_start_timestamp_ = 0.0;
    rebuildAnalyzers();
}

void AnalyzerEngine::rebuildAnalyzers() {
    f0_detector_ = F0Detector(audio_config_);
    level_analyzer_ = LevelAnalyzer(audio_config_);
    cpp_analyzer_ = CPPAnalyzer(audio_config_);
    hnr_analyzer_ = HNRAnalyzer(audio_config_);
    spectral_analyzer_ = SpectralAnalyzer(audio_config_);
    vad_analyzer_ = VADAnalyzer(audio_config_);
    vad_analyzer_.reset();
}

} // namespace yvc
