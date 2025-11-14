// VoiVoi Core Library - Formant Analyzer
// License: MIT
// Purpose: Extract first three formant frequencies (F1,F2,F3) using LPC

#pragma once

#define _USE_MATH_DEFINES
#include "Types.h"
#include <vector>
#include <optional>

namespace yvc {

class FormantAnalyzer {
public:
    explicit FormantAnalyzer(const AudioConfig& config);

    struct FormantResults {
        float f1 = 0.0f;
        float f2 = 0.0f;
        float f3 = 0.0f;
        float f4 = 0.0f;
        bool valid = false;
    };

    FormantResults analyze(const Sample* samples, size_t num_samples);

private:
    AudioConfig config_;
    std::vector<float> windowed_;
    std::vector<float> autocorr_;
    std::vector<float> lpc_coeffs_;

    void applyPreEmphasis(const Sample* in, size_t n, float* out);
    void applyWindow(float* data, size_t n);
    void computeAutocorrelation(const float* data, size_t n, size_t order);
    bool levinsonDurbin(size_t order);
    std::vector<float> rootsToFormants(size_t order);
};

} // namespace yvc
