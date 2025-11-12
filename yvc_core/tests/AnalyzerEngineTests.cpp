#include <yvc_core/AnalyzerEngine.h>

#include <cmath>
#include <numbers>
#include <vector>

namespace {
using namespace yvc;

bool approximatelyEqual(float lhs, float rhs, float tolerance) {
    return std::fabs(lhs - rhs) <= tolerance;
}

bool approximatelyEqual(double lhs, double rhs, double tolerance) {
    return std::fabs(lhs - rhs) <= tolerance;
}

int runAnalyzerEngineSmokeTest() {
    MetricsBus bus;

    AudioConfig config;
    config.sample_rate = SAMPLE_RATE_48K;

    AnalyzerEngine engine(config, bus, PerformanceMode::Standard);
    const auto engine_config = engine.getConfig();

    const size_t frame_samples = engine_config.fft_size;
    std::vector<Sample> sine(frame_samples);
    const float frequency = 440.0f;

    for (size_t i = 0; i < frame_samples; ++i) {
        sine[i] = std::sin(2.0f * std::numbers::pi_v<float> * frequency * static_cast<float>(i) /
                           static_cast<float>(engine_config.sample_rate));
    }

    engine.process(sine.data(), sine.size(), 0.0);

    AnalysisResults results{};
    if (!bus.read(results)) {
        return 1;
    }

    F0Detector direct_f0(engine_config);
    bool expected_f0_valid = false;
    const float expected_f0 = direct_f0.detect(sine.data(), frame_samples, expected_f0_valid);

    if (results.f0_valid != expected_f0_valid) {
        return 2;
    }

    if (!approximatelyEqual(results.f0, expected_f0, 1e-3f)) {
        return 3;
    }

    LevelAnalyzer direct_level(engine_config);
    const auto expected_level = direct_level.analyze(sine.data(), frame_samples);
    if (!approximatelyEqual(results.rms, expected_level.rms, 1e-4f)) {
        return 4;
    }
    if (!approximatelyEqual(results.peak, expected_level.peak, 1e-4f)) {
        return 5;
    }
    if (!approximatelyEqual(results.crest_factor, expected_level.crest_factor, 1e-3f)) {
        return 6;
    }

    CPPAnalyzer direct_cpp(engine_config);
    const float expected_cpp = direct_cpp.analyze(sine.data(), frame_samples);
    if (!approximatelyEqual(results.cpp, expected_cpp, 1e-3f)) {
        return 7;
    }

    if (results.f0_valid) {
        HNRAnalyzer direct_hnr(engine_config);
        const float expected_hnr = direct_hnr.analyze(sine.data(), frame_samples, expected_f0);
        if (!approximatelyEqual(results.hnr, expected_hnr, 1e-3f)) {
            return 8;
        }
    }

    SpectralAnalyzer direct_spectral(engine_config);
    const auto expected_spectral = direct_spectral.analyze(sine.data(), frame_samples);
    if (!approximatelyEqual(results.spectral_tilt, expected_spectral.spectral_tilt, 1e-3f)) {
        return 9;
    }
    if (!approximatelyEqual(results.s_centroid, expected_spectral.s_centroid, 1e-3f)) {
        return 10;
    }
    if (results.s_detected != expected_spectral.s_detected) {
        return 11;
    }

    VADAnalyzer direct_vad(engine_config);
    const auto expected_vad = direct_vad.analyze(sine.data(), frame_samples, expected_level.rms);
    if (results.voice_active != expected_vad.voice_active) {
        return 12;
    }
    if (!approximatelyEqual(results.speech_rate, expected_vad.speech_rate, 1e-3f)) {
        return 13;
    }
    if (!approximatelyEqual(results.pause_ratio, expected_vad.pause_ratio, 1e-3f)) {
        return 14;
    }

    if (bus.hasNewData()) {
        return 15;
    }

    const auto history = bus.getHistory();
    if (history.empty()) {
        return 16;
    }

    const auto latest = bus.getLatest();
    if (!approximatelyEqual(latest.rms, results.rms, 1e-6f)) {
        return 17;
    }

    return 0;
}

int runAnalyzerEngineMultipleBlocksTest() {
    MetricsBus bus;

    AudioConfig config;
    config.sample_rate = SAMPLE_RATE_48K;

    AnalyzerEngine engine(config, bus, PerformanceMode::Standard);

    const auto fft_size = static_cast<size_t>(engine.getFFTSize());
    const auto hop_size = static_cast<size_t>(engine.getHopSize());

    std::vector<Sample> block(fft_size + hop_size, 0.0f);
    const double start_timestamp = 0.25;

    engine.process(block.data(), block.size(), start_timestamp);

    const auto history = bus.getHistory();
    if (history.size() != 2) {
        return 101;
    }

    const double sample_rate = static_cast<double>(engine.getConfig().sample_rate);
    const double hop_duration = static_cast<double>(hop_size) / sample_rate;

    if (!approximatelyEqual(history[0].timestamp, start_timestamp, 1e-6)) {
        return 102;
    }

    if (!approximatelyEqual(history[1].timestamp, start_timestamp + hop_duration, 1e-6)) {
        return 103;
    }

    return 0;
}

int runVADAnalyzerSustainedSpeechRateTest() {
    AudioConfig config;
    config.sample_rate = SAMPLE_RATE_48K;

    VADAnalyzer analyzer(config);

    const size_t frame_samples = static_cast<size_t>(0.1 * static_cast<double>(config.sample_rate));
    std::vector<Sample> voiced_frame(frame_samples, 0.1f);
    std::vector<Sample> silent_frame(frame_samples, 0.0f);

    // Prime with initial silence to establish baseline
    for (int i = 0; i < 5; ++i) {
        analyzer.analyze(silent_frame.data(), frame_samples, 0.0f);
    }

    VADAnalyzer::VADResults results{};

    // Alternate voiced and silent frames to simulate syllable-like transitions
    const int cycles = 40;
    for (int i = 0; i < cycles; ++i) {
        results = analyzer.analyze(voiced_frame.data(), frame_samples, 0.05f);
        results = analyzer.analyze(silent_frame.data(), frame_samples, 0.0f);
    }

    if (results.speech_rate < 3.0f || results.speech_rate > 6.0f) {
        return 201;
    }

    return 0;
}

} // namespace

int main() {
    if (int result = runAnalyzerEngineSmokeTest(); result != 0) {
        return result;
    }

    if (int result = runAnalyzerEngineMultipleBlocksTest(); result != 0) {
        return result;
    }

    if (int result = runVADAnalyzerSustainedSpeechRateTest(); result != 0) {
        return result;
    }

    return 0;
}
