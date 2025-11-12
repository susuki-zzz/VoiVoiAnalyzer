#pragma once

#include <vector>
#include <cmath>
#include <numbers>

#include "yvc_core/Types.h"

namespace yvc::test {

inline std::vector<Sample> generateSineWave(float frequency, size_t samples, SampleRate sample_rate,
                                            float amplitude = 1.0f) {
    std::vector<Sample> data(samples, 0.0f);
    const float sr = static_cast<float>(sample_rate);
    for (size_t i = 0; i < samples; ++i) {
        const float phase = 2.0f * std::numbers::pi_v<float> * frequency * static_cast<float>(i) / sr;
        data[i] = amplitude * std::sin(phase);
    }
    return data;
}

inline std::vector<Sample> generateAlternatingFrames(size_t samples_per_frame, int cycles,
                                                     float voiced_level = 0.1f, float silent_level = 0.0f) {
    std::vector<Sample> data;
    data.reserve(static_cast<size_t>(samples_per_frame) * static_cast<size_t>(cycles) * 2);
    auto voiced = std::vector<Sample>(samples_per_frame, voiced_level);
    auto silent = std::vector<Sample>(samples_per_frame, silent_level);
    for (int i = 0; i < cycles; ++i) {
        data.insert(data.end(), voiced.begin(), voiced.end());
        data.insert(data.end(), silent.begin(), silent.end());
    }
    return data;
}

inline bool approximatelyEqual(float lhs, float rhs, float tolerance) {
    return std::fabs(lhs - rhs) <= tolerance;
}

inline bool approximatelyEqual(double lhs, double rhs, double tolerance) {
    return std::fabs(lhs - rhs) <= tolerance;
}

} // namespace yvc::test
