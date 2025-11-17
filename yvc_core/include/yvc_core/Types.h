// VoiVoi Core Library - Common Types
// License: MIT
// Purpose: Common type definitions for audio analysis

#pragma once

#include <cstdint>
#include <vector>
#include <string>

namespace yvc {

/// <summary>
/// Audio sample type.
/// </summary>
using Sample = float;

/// <summary>
/// Sample rate type.
/// </summary>
using SampleRate = uint32_t;

/// <summary>
/// Common sample rates.
/// </summary>
constexpr SampleRate SAMPLE_RATE_48K = 48000;
constexpr SampleRate SAMPLE_RATE_44_1K = 44100;

/// <summary>
/// Performance modes with latency targets.
/// </summary>
enum class PerformanceMode {
    Mode_Light,      // ≤ 40ms latency
    Mode_Standard,   // ≤ 60ms latency  
    Mode_Diagnostic  // ≤ 80ms latency
};

/// <summary>
/// Analysis results structure containing all metrics.
/// </summary>
struct AnalysisResults {
    // F0 (Fundamental Frequency) in Hz
    float f0 = 0.0f;
    bool f0_valid = false;
    float f0_confidence = 0.0f; // 0..1 confidence from detector
    
    // Level metrics
    float rms = 0.0f;           // RMS level
    float peak = 0.0f;          // Peak level
    float crest_factor = 0.0f;  // Crest factor (Peak/RMS)
    
    // CPP (Cepstral Peak Prominence) in dB
    float cpp = 0.0f;
    
    // HNR (Harmonics-to-Noise Ratio) in dB
    float hnr = 0.0f;
    
    // Spectral Tilt in dB/octave
    float spectral_tilt = 0.0f;
    
    // /s/ Centroid in Hz (for sibilant analysis)
    float s_centroid = 0.0f;
    bool s_detected = false;
    
    // VAD (Voice Activity Detection)
    bool voice_active = false;
    float speech_rate = 0.0f;   // syllables per second
    float pause_ratio = 0.0f;   // ratio of pauses to speech

    // Formants (basic 4-formant tracking now)
    float f1 = 0.0f;
    float f2 = 0.0f;
    float f3 = 0.0f;
    float f4 = 0.0f;
    bool formants_valid = false;

    // Timestamp
    double timestamp = 0.0;
};

/// <summary>
/// Audio buffer configuration.
/// </summary>
struct AudioConfig {
    SampleRate sample_rate = SAMPLE_RATE_48K;
    uint32_t buffer_size = 512;
    uint8_t num_channels = 1;  // Mono analysis
    PerformanceMode mode = PerformanceMode::Mode_Standard;
    uint32_t fft_size = 2048;
    uint32_t hop_size = 512;
};

} // namespace yvc
