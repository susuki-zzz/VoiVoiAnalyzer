#pragma once

#include "ApiKeyVault.h"
#include "ICoachProvider.h"
#include <chrono>

namespace yvc {

class MetricsOnlyCoachProvider : public ICoachProvider {
public:
    MetricsOnlyCoachProvider();

    void setApiKey(const std::string& key) override;
    std::string advise(const SummarySnapshot& snapshot) override;
    bool isAvailable() const override;
    const char* name() const override;
    void setMinIntervalMs(int ms) override;

private:
    ApiKeyVault vault_;
    std::chrono::steady_clock::time_point last_request_{};
    std::chrono::milliseconds min_interval_{1000};
};

} // namespace yvc
