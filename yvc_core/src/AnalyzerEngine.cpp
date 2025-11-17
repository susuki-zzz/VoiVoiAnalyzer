#include "yvc_core/AnalyzerEngine.h"
#include <algorithm>

namespace yvc {
namespace {
/// <summary>
/// Applies performance mode defaults to the provided audio config.
/// </summary>
AudioConfig prepareConfig(const AudioConfig& config, PerformanceMode mode) {
    return PerformanceModeConfig::applyToConfig(config, mode);
}
} // namespace

/// <summary>
/// Constructs the analyzer engine and initializes analyzers with the requested performance mode.
/// </summary>
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
      formant_analyzer_(audio_config_),
      performance_config_(audio_config_.mode),
      fft_size_(performance_config_.getFFTSize()),
      hop_size_(performance_config_.getHopSize()) {
}

/// <summary>
/// Pushes PCM into internal buffer and runs analyzers on hop-sized windows, writing results to MetricsBus.
/// </summary>
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

        // F0 + confidence
        {
            bool valid = false;
            float conf = 0.0f;
            results.f0 = f0_detector_.detect(analysis_ptr, analysis_samples, valid, conf);
            results.f0_valid = valid;
            results.f0_confidence = conf;
        }

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

        // Formant analysis: try even if current frame unvoiced; mark valid only if F1/F2 plausible
        auto formants = formant_analyzer_.analyze(analysis_ptr, analysis_samples);
        if (formants.valid) {
            results.f1 = formants.f1;
            results.f2 = formants.f2;
            results.f3 = formants.f3;
            results.f4 = formants.f4;
            results.formants_valid = true;
        } else {
            results.formants_valid = false;
        }

        // Post-hoc confidence refinement
        if (results.f0_valid) {
            float cppNorm = std::clamp(results.cpp / 30.0f, 0.0f, 1.0f);
            float hnrNorm = std::clamp(results.hnr / 30.0f, 0.0f, 1.0f);
            // Blend detector confidence with quality cues
            results.f0_confidence = std::clamp(0.5f * results.f0_confidence + 0.25f * cppNorm + 0.25f * hnrNorm, 0.0f, 1.0f);
        }

        metrics_bus_.write(results);

        audio_buffer_.consumeSamples(hop_size_);
        processed_samples_ += hop_size_;
    }
}

/// <summary>
/// Changes performance mode and rebuilds analyzers with the new configuration.
/// </summary>
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

/// <summary>
/// Recreates analyzer instances to match current audio configuration.
/// </summary>
void AnalyzerEngine::rebuildAnalyzers() {
    f0_detector_ = F0Detector(audio_config_);
    level_analyzer_ = LevelAnalyzer(audio_config_);
    cpp_analyzer_ = CPPAnalyzer(audio_config_);
    hnr_analyzer_ = HNRAnalyzer(audio_config_);
    spectral_analyzer_ = SpectralAnalyzer(audio_config_);
    vad_analyzer_ = VADAnalyzer(audio_config_);
    formant_analyzer_ = FormantAnalyzer(audio_config_);
    vad_analyzer_.reset();
}

} // namespace yvc
