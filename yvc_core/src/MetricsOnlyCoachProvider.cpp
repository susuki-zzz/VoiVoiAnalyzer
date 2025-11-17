#include "yvc_core/MetricsOnlyCoachProvider.h"

#include <sstream>

namespace yvc {

/// <summary>
/// Constructs a metrics-only coach provider (no external state).
/// </summary>
MetricsOnlyCoachProvider::MetricsOnlyCoachProvider() = default;

/// <summary>
/// Stores API key in the secure vault (DPAPI on Windows).
/// </summary>
void MetricsOnlyCoachProvider::setApiKey(const std::string& key) {
    vault_.store(key);
}

/// <summary>
/// Returns a short textual advice assembled purely from provided metrics snapshot.
/// Applies simple cooldown throttling to avoid hammering provider.
/// </summary>
std::string MetricsOnlyCoachProvider::advise(const SummarySnapshot& snapshot) {
    if (!isAvailable()) {
        return "Coach unavailable: missing API key";
    }
    auto now = std::chrono::steady_clock::now();
    if (last_request_.time_since_epoch().count() != 0) {
        auto elapsed = now - last_request_;
        if (elapsed < min_interval_) {
            return "Coach cooling down";
        }
    }
    last_request_ = now;

    std::ostringstream oss;
    oss << "Metrics summary:\n";
    oss << "F0 mean: " << snapshot.f0.mean << " Hz\n";
    oss << "RMS mean: " << snapshot.rms.mean << " dBFS\n";
    oss << "CPP mean: " << snapshot.cpp.mean << " dB\n";
    oss << "Speech rate: " << snapshot.speech_rate << " syllables/s\n";
    oss << "Voice activity: " << snapshot.voice_active_ratio * 100.0f << "%";
    return oss.str();
}

/// <summary>
/// Provider considered available iff an API key exists in the vault.
/// </summary>
bool MetricsOnlyCoachProvider::isAvailable() const {
    return vault_.hasKey();
}

/// <summary>
/// Display name for this provider instance.
/// </summary>
const char* MetricsOnlyCoachProvider::name() const {
    return "MetricsOnly";
}

/// <summary>
/// Sets minimal duration between subsequent advise() calls.
/// </summary>
void MetricsOnlyCoachProvider::setMinIntervalMs(int ms) {
    if (ms <= 0) {
        min_interval_ = std::chrono::milliseconds(0);
    } else {
        min_interval_ = std::chrono::milliseconds(ms);
    }
}

} // namespace yvc
