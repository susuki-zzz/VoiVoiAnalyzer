#include <gtest/gtest.h>

#include <yvc_core/MetricsOnlyCoachProvider.h>

namespace yvc::test {
namespace {

MetricsSummary createSummary(float f0_mean) {
    MetricsSummary summary{};
    summary.f0.mean = f0_mean;
    summary.rms.mean = -12.0f;
    summary.cpp.mean = 20.0f;
    summary.speech_rate = 4.0f;
    summary.voice_active_ratio = 0.75f;
    return summary;
}

TEST(MetricsOnlyCoachProviderTests, RequiresApiKeyBeforeResponding) {
    MetricsOnlyCoachProvider coach;
    auto response = coach.advise(createSummary(220.0f));
    EXPECT_EQ(response, "Coach unavailable: missing API key");

    coach.setApiKey("token");
    response = coach.advise(createSummary(220.0f));
    EXPECT_NE(response.find("Metrics summary"), std::string::npos);
}

TEST(MetricsOnlyCoachProviderTests, EnforcesRateLimiting) {
    MetricsOnlyCoachProvider coach;
    coach.setApiKey("token");
    coach.setMinIntervalMs(200);

    const auto first = coach.advise(createSummary(180.0f));
    EXPECT_NE(first.find("Metrics summary"), std::string::npos);

    const auto second = coach.advise(createSummary(180.0f));
    EXPECT_EQ(second, "Coach cooling down");
}

} // namespace
} // namespace yvc::test
