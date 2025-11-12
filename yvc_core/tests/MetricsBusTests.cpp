#include <gtest/gtest.h>

#include <yvc_core/MetricsBus.h>

namespace yvc::test {
namespace {

TEST(MetricsBusTests, DoubleBufferingProvidesLatestSample) {
    MetricsBus bus;
    AnalysisResults first{};
    first.rms = -12.0f;
    first.timestamp = 0.5;

    bus.write(first);

    AnalysisResults out{};
    ASSERT_TRUE(bus.read(out));
    EXPECT_FLOAT_EQ(out.rms, first.rms);
    EXPECT_DOUBLE_EQ(out.timestamp, first.timestamp);
    EXPECT_FALSE(bus.read(out));
}

TEST(MetricsBusTests, HistoryMaintainsFixedWindow) {
    MetricsBus bus;
    for (int i = 0; i < 32; ++i) {
        AnalysisResults result{};
        result.timestamp = static_cast<double>(i);
        result.rms = static_cast<float>(i);
        bus.write(result);
    }

    const auto history = bus.getHistory(10);
    ASSERT_EQ(history.size(), 10u);
    EXPECT_DOUBLE_EQ(history.front().timestamp, 22.0);
    EXPECT_DOUBLE_EQ(history.back().timestamp, 31.0);
}

} // namespace
} // namespace yvc::test
