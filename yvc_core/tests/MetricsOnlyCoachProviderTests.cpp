#include <gtest/gtest.h>

#include <yvc_core/MetricsOnlyCoachProvider.h>
#include <yvc_core/ICoachProvider.h>

namespace yvc::test {
namespace {

SummarySnapshot createSummary(float f0_mean) {
    SummarySnapshot s{};
    s.f0.mean = f0_mean;
    s.rms.mean = -12.0f;
    s.cpp.mean = 20.0f;
    s.speech_rate = 4.0f;
    s.voice_active_ratio = 0.75f;
    return s;
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
