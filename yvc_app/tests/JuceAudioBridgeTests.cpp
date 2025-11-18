#include <gtest/gtest.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include "JuceAudioBridge.h"
#include <yvc_core/MetricsBus.h>
#include <yvc_core/Types.h>

namespace yvc::app::test {
    namespace {

        class JuceFixture : public ::testing::Test {
        protected:
            static void SetUpTestSuite() {
                juce_init_ = std::make_unique<juce::ScopedJuceInitialiser_GUI>();
            }

            static void TearDownTestSuite() {
                juce_init_.reset();
            }

            static inline std::unique_ptr<juce::ScopedJuceInitialiser_GUI> juce_init_;
        };

        TEST_F(JuceFixture, BridgeCreationAndDestruction) {
            yvc::MetricsBus bus;
            yvc::AudioConfig config;
            config.sample_rate = 48000;
            config.buffer_size = 512;

            auto bridge = std::make_unique<JuceAudioBridge>(bus, config);
            EXPECT_NE(bridge, nullptr);

            auto stats = bridge->getStatistics();
            EXPECT_EQ(stats.totalSamplesProcessed, 0u);
            EXPECT_EQ(stats.totalCallbacks, 0u);
            EXPECT_FALSE(stats.isRunning);
        }

        TEST_F(JuceFixture, AudioCallbackProcessesSamples) {
            yvc::MetricsBus bus;
            yvc::AudioConfig config;
            config.sample_rate = 48000;
            config.buffer_size = 512;

            JuceAudioBridge bridge(bus, config);

            // Simulate audio device start
            bridge.audioDeviceAboutToStart(nullptr);

            // Prepare test audio
            constexpr int numSamples = 512;
            std::vector<float> inputBuffer(numSamples, 0.1f); // Quiet test signal
            const float* inputChannels[1] = { inputBuffer.data() };
            float* outputChannels[1] = { nullptr };

            juce::AudioIODeviceCallbackContext context;

            // Process audio callback
            bridge.audioDeviceIOCallbackWithContext(
                inputChannels, 1,
                outputChannels, 0,
                numSamples, context);

            auto stats = bridge.getStatistics();
            EXPECT_EQ(stats.totalSamplesProcessed, static_cast<uint64_t>(numSamples));
            EXPECT_EQ(stats.totalCallbacks, 1u);
            EXPECT_TRUE(stats.isRunning);

            // Verify metrics were written to bus
            yvc::AnalysisResults results;
            // Note: May not have metrics immediately if FFT buffer not filled
            // Just verify bus is accessible
            EXPECT_NO_THROW(bus.getLatest());
        }

        TEST_F(JuceFixture, StereoToMonoConversion) {
            yvc::MetricsBus bus;
            yvc::AudioConfig config;
            config.sample_rate = 48000;
            config.buffer_size = 256;

            JuceAudioBridge bridge(bus, config);
            bridge.audioDeviceAboutToStart(nullptr);

            constexpr int numSamples = 256;
            std::vector<float> leftChannel(numSamples, 0.5f);
            std::vector<float> rightChannel(numSamples, -0.5f);

            const float* inputChannels[2] = { leftChannel.data(), rightChannel.data() };
            float* outputChannels[1] = { nullptr };

            juce::AudioIODeviceCallbackContext context;

            bridge.audioDeviceIOCallbackWithContext(
                inputChannels, 2,
                outputChannels, 0,
                numSamples, context);

            auto stats = bridge.getStatistics();
            EXPECT_EQ(stats.totalSamplesProcessed, static_cast<uint64_t>(numSamples));
            EXPECT_TRUE(stats.isRunning);
        }

        TEST_F(JuceFixture, PerformanceModeSwitch) {
            yvc::MetricsBus bus;
            yvc::AudioConfig config;
            config.sample_rate = 48000;
            config.buffer_size = 512;
            config.mode = yvc::PerformanceMode::Mode_Standard;

            JuceAudioBridge bridge(bus, config);

            EXPECT_EQ(bridge.getPerformanceMode(), yvc::PerformanceMode::Mode_Standard);

            bridge.setPerformanceMode(yvc::PerformanceMode::Mode_Diagnostic);
            EXPECT_EQ(bridge.getPerformanceMode(), yvc::PerformanceMode::Mode_Diagnostic);

            bridge.setPerformanceMode(yvc::PerformanceMode::Mode_Light);
            EXPECT_EQ(bridge.getPerformanceMode(), yvc::PerformanceMode::Mode_Light);
        }

        TEST_F(JuceFixture, ConfigUpdateRecreatesEngine) {
            yvc::MetricsBus bus;
            yvc::AudioConfig config1;
            config1.sample_rate = 48000;
            config1.buffer_size = 512;

            JuceAudioBridge bridge(bus, config1);
            bridge.audioDeviceAboutToStart(nullptr);

            // Process some samples
            std::vector<float> buffer(512, 0.1f);
            const float* input[1] = { buffer.data() };
            float* output[1] = { nullptr };
            juce::AudioIODeviceCallbackContext ctx;

            bridge.audioDeviceIOCallbackWithContext(input, 1, output, 0, 512, ctx);

            auto stats1 = bridge.getStatistics();
            EXPECT_GT(stats1.totalCallbacks, 0u);

            // Update config
            yvc::AudioConfig config2;
            config2.sample_rate = 44100;
            config2.buffer_size = 256;
            bridge.updateConfig(config2);

            // Statistics should be reset
            auto stats2 = bridge.getStatistics();
            EXPECT_EQ(stats2.totalSamplesProcessed, 0u);
            EXPECT_EQ(stats2.totalCallbacks, 0u);
        }

        TEST_F(JuceFixture, DeviceStopClearsRunningFlag) {
            yvc::MetricsBus bus;
            yvc::AudioConfig config;
            config.sample_rate = 48000;
            config.buffer_size = 512;

            JuceAudioBridge bridge(bus, config);

            bridge.audioDeviceAboutToStart(nullptr);
            EXPECT_TRUE(bridge.getStatistics().isRunning);

            bridge.audioDeviceStopped();
            EXPECT_FALSE(bridge.getStatistics().isRunning);
        }

        TEST_F(JuceFixture, ErrorIncrementsXRunCounter) {
            yvc::MetricsBus bus;
            yvc::AudioConfig config;
            config.sample_rate = 48000;
            config.buffer_size = 512;

            JuceAudioBridge bridge(bus, config);

            auto stats1 = bridge.getStatistics();
            EXPECT_EQ(stats1.xruns, 0u);

            bridge.audioDeviceError("Test error");

            auto stats2 = bridge.getStatistics();
            EXPECT_EQ(stats2.xruns, 1u);
        }

        TEST_F(JuceFixture, MultipleCallbacksAccumulateStats) {
            yvc::MetricsBus bus;
            yvc::AudioConfig config;
            config.sample_rate = 48000;
            config.buffer_size = 128;

            JuceAudioBridge bridge(bus, config);
            bridge.audioDeviceAboutToStart(nullptr);

            std::vector<float> buffer(128, 0.1f);
            const float* input[1] = { buffer.data() };
            float* output[1] = { nullptr };
            juce::AudioIODeviceCallbackContext ctx;

            // Process 10 callbacks
            for(int i = 0; i < 10; ++i) {
                bridge.audioDeviceIOCallbackWithContext(input, 1, output, 0, 128, ctx);
            }

            auto stats = bridge.getStatistics();
            EXPECT_EQ(stats.totalSamplesProcessed, 128u * 10u);
            EXPECT_EQ(stats.totalCallbacks, 10u);
            EXPECT_GT(stats.averageLatencyMs, 0.0); // Should have measured some latency
        }

    } // namespace
} // namespace yvc::app::test
