// VoiVoi Core Library - Common Types
// License: MIT
// Purpose: Common type definitions for audio analysis

#pragma once

#include <cstdint>
#include <vector>
#include <string>

namespace yvc {

// Audio sample type
using Sample = float;

// Sample rate type
using SampleRate = uint32_t;

// Common sample rates
constexpr SampleRate SAMPLE_RATE_48K = 48000;
constexpr SampleRate SAMPLE_RATE_44_1K = 44100;

// Performance modes with latency targets
enum class PerformanceMode {
    Light,      // ≤ 40ms latency
    Standard,   // ≤ 60ms latency  
    Diagnostic  // ≤ 80ms latency
};

// Analysis results structure
struct AnalysisResults {
    // F0 (Fundamental Frequency) in Hz
    float f0 = 0.0f;
    bool f0_valid = false;
    
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
    
    // Timestamp
    double timestamp = 0.0;
};

// Audio buffer configuration
struct AudioConfig {
    SampleRate sample_rate = SAMPLE_RATE_48K;
    uint32_t buffer_size = 512;
    uint8_t num_channels = 1;  // Mono analysis
    PerformanceMode mode = PerformanceMode::Standard;
};

} // namespace yvc
