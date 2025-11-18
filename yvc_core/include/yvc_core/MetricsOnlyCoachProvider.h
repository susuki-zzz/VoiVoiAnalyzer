#pragma once

#include "ApiKeyVault.h"
#include "ICoachProvider.h"
#include <chrono>

namespace yvc {

    /// <summary>
    /// Metrics-only AI coach provider implementation.
    /// Ensures no raw audio data is transmitted to AI services.
    /// </summary>
    class MetricsOnlyCoachProvider : public ICoachProvider {
    public:
        /// <summary>
        /// Constructs a metrics-only coach provider.
        /// </summary>
        MetricsOnlyCoachProvider();

        /// <summary>
        /// Sets API key for the coach service.
        /// </summary>
        /// <param name="key">API key string</param>
        void setApiKey(const std::string& key) override;

        /// <summary>
        /// Gets advice based on metrics summary.
        /// </summary>
        /// <param name="snapshot">Aggregated metrics (NO audio data)</param>
        /// <returns>Advice text from coach</returns>
        std::string advise(const SummarySnapshot& snapshot) override;

        /// <summary>
        /// Checks if coach is available/configured.
        /// </summary>
        /// <returns>True if available, false otherwise</returns>
        bool isAvailable() const override;

        /// <summary>
        /// Gets provider name.
        /// </summary>
        /// <returns>Provider name as C-string</returns>
        const char* name() const override;

        /// <summary>
        /// Sets minimum interval between advice requests (rate limiting).
        /// </summary>
        /// <param name="ms">Minimum interval in milliseconds</param>
        void setMinIntervalMs(int ms) override;

    private:
        ApiKeyVault vault_;
        std::chrono::steady_clock::time_point last_request_{};
        std::chrono::milliseconds min_interval_{ 1000 };
    };

} // namespace yvc
