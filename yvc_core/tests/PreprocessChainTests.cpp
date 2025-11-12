#include <gtest/gtest.h>

#include <yvc_core/PreprocessChain.h>

#include <atomic>
#include <unordered_map>

namespace yvc::test {
namespace {

class GainPreprocessor : public IPreprocessor {
public:
    explicit GainPreprocessor(float gain) : gain_(gain) {}

    void process(const float* in, float* out, size_t n) override {
        last_call_count_.fetch_add(1, std::memory_order_relaxed);
        for (size_t i = 0; i < n; ++i) {
            out[i] = in[i] * gain_;
        }
    }

    int latency_samples() const override { return latency_; }
    const char* name() const override { return "gain"; }

    void setParams(const std::unordered_map<std::string, float>& kv) override {
        auto it = kv.find("gain");
        if (it != kv.end()) {
            gain_ = it->second;
        }
        auto latency = kv.find("latency");
        if (latency != kv.end()) {
            latency_ = static_cast<int>(latency->second);
        }
    }

    std::unordered_map<std::string, float> getParams() const override {
        return {{"gain", gain_}, {"latency", static_cast<float>(latency_)}};
    }

    int callCount() const { return last_call_count_.load(std::memory_order_relaxed); }

private:
    float gain_;
    int latency_ = 0;
    mutable std::atomic<int> last_call_count_{0};
};

TEST(PreprocessChainTests, AppliesProcessorsForMatchingTargets) {
    PreprocessChain chain;
    auto processor = std::make_shared<GainPreprocessor>(2.0f);
    chain.addProcessor(processor, PreprocessTarget::Monitor);

    float samples[4] = {1.0f, 2.0f, 3.0f, 4.0f};
    float out[4] = {0.0f};
    chain.process(samples, out, 4, PreprocessTarget::Monitor);

    EXPECT_FLOAT_EQ(out[0], 2.0f);
    EXPECT_EQ(processor->callCount(), 1);

    chain.process(samples, out, 4, PreprocessTarget::Record);
    EXPECT_EQ(processor->callCount(), 1);
}

TEST(PreprocessChainTests, TracksLatencyAndParameters) {
    PreprocessChain chain;
    auto processor = std::make_shared<GainPreprocessor>(1.0f);
    chain.addProcessor(processor, PreprocessTarget::Monitor | PreprocessTarget::Record);

    EXPECT_EQ(chain.totalLatency(PreprocessTarget::Monitor), 0u);
    chain.setParams("gain", {{"gain", 0.5f}, {"latency", 128.0f}});
    EXPECT_EQ(chain.totalLatency(PreprocessTarget::Monitor), 128u);

    const auto params = chain.getParams("gain");
    ASSERT_EQ(params.size(), 2u);
    EXPECT_FLOAT_EQ(params.at("gain"), 0.5f);
    EXPECT_FLOAT_EQ(params.at("latency"), 128.0f);

    EXPECT_TRUE(matches(chain.getTargets("gain"), PreprocessTarget::Record));
}

} // namespace
} // namespace yvc::test
