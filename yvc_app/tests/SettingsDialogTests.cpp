#include <gtest/gtest.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>

#include "SettingsDialog.h"
#include "LocalizationManager.h"

namespace yvc::app::test {
    namespace {

        class JuceGuiFixture : public ::testing::Test {
        protected:
            static void SetUpTestSuite() {
                juce_init_ = std::make_unique<juce::ScopedJuceInitialiser_GUI>();
            }
            static void TearDownTestSuite() {
                juce_init_.reset();
            }
            static inline std::unique_ptr<juce::ScopedJuceInitialiser_GUI> juce_init_;
        };

        class DummyAudioDeviceManager : public juce::AudioDeviceManager {
        public:
            DummyAudioDeviceManager() {
                // Initialise with default device to enable queries in SettingsDialog.
                // Using 0 inputs/0 outputs can still create a device in JUCE (e.g. dummy) depending on platform.
                juce::AudioDeviceManager::AudioDeviceSetup setup;
                // Leave default; dialog uses fallbacks when no device info available.
            }
        };

        TEST_F(JuceGuiFixture, DefaultsReflectCurrentSettings) {
            AppSettings s;
            s.sampleRate = 48000;
            s.bufferSize = 256;
            s.maxRecordingTimeSeconds = 1200.0;
            s.language = LocalizationManager::Language::English;
            s.heatmapResolution = 2;

            DummyAudioDeviceManager adm;

            bool called = false;
            auto dlg = SettingsDialog::createForTest(s, adm, [&](bool accepted, const AppSettings& out) {
                called = true;
                // on Cancel/implicit close we expect working copy
                EXPECT_FALSE(accepted);
                EXPECT_EQ(out.sampleRate, s.sampleRate);
            });

            // Verify UI reflects the provided settings
            EXPECT_EQ(dlg->getSelectedSampleRateId(), s.sampleRate);
            EXPECT_EQ(dlg->getSelectedBufferSizeId(), s.bufferSize);
            EXPECT_EQ(dlg->getHeatmapResolutionId(), s.heatmapResolution);
            EXPECT_EQ(dlg->getSelectedLanguage(), s.language);
            EXPECT_NEAR(dlg->getMaxRecordingTimeForTest(), s.maxRecordingTimeSeconds, 1e-3);

            // Destroy without clicking OK/Cancel -> destructor should invoke callback with accepted=false
            dlg.reset();
            EXPECT_TRUE(called);
        }

        TEST_F(JuceGuiFixture, ApplyCollectsChangesOnOk) {
            AppSettings s;
            s.sampleRate = 44100;
            s.bufferSize = 512;
            s.maxRecordingTimeSeconds = 600.0;
            s.language = LocalizationManager::Language::English;
            s.heatmapResolution = 1;

            DummyAudioDeviceManager adm;

            bool called = false;
            AppSettings captured{};
            auto dlg = SettingsDialog::createForTest(s, adm, [&](bool accepted, const AppSettings& out) {
                called = true;
                EXPECT_TRUE(accepted);
                captured = out;
            });

            // Simulate user changes
            dlg->setSelectedSampleRateForTest(48000);
            dlg->setSelectedBufferSizeForTest(256);
            dlg->setHeatmapResolutionForTest(3);
            dlg->setLanguageForTest(LocalizationManager::Language::Japanese);
            dlg->setAutoSaveForTest(false);
            dlg->setPreprocessingForTest(false);
            dlg->setAdvancedVisualizationForTest(false);
            dlg->setSpectralAnalysisForTest(true);
            dlg->setMaxRecordingTimeForTest(1800.0);

            // OK
            dlg->simulateOkForTest();

            EXPECT_TRUE(called);
            EXPECT_EQ(captured.sampleRate, 48000);
            EXPECT_EQ(captured.bufferSize, 256);
            EXPECT_EQ(captured.heatmapResolution, 3);
            EXPECT_EQ(captured.language, LocalizationManager::Language::Japanese);
            EXPECT_FALSE(captured.autoSave);
            EXPECT_FALSE(captured.enablePreprocessing);
            EXPECT_FALSE(captured.enableAdvancedVisualization);
            EXPECT_TRUE(captured.showSpectralAnalysis);
            EXPECT_NEAR(captured.maxRecordingTimeSeconds, 1800.0, 1e-6);
        }

        TEST_F(JuceGuiFixture, CancelKeepsOriginalSettings) {
            AppSettings s;
            s.sampleRate = 48000;
            s.bufferSize = 512;
            s.maxRecordingTimeSeconds = 900.0;

            DummyAudioDeviceManager adm;

            bool called = false;
            AppSettings captured{};
            auto dlg = SettingsDialog::createForTest(s, adm, [&](bool accepted, const AppSettings& out) {
                called = true;
                EXPECT_FALSE(accepted);
                captured = out;
            });

            // Change some fields
            dlg->setSelectedSampleRateForTest(44100);
            dlg->setSelectedBufferSizeForTest(128);
            dlg->setMaxRecordingTimeForTest(300.0);

            // Cancel
            dlg->simulateCancelForTest();

            EXPECT_TRUE(called);
            // For cancel we expect the working copy (original), not the changed values
            EXPECT_EQ(captured.sampleRate, s.sampleRate);
            EXPECT_EQ(captured.bufferSize, s.bufferSize);
            EXPECT_NEAR(captured.maxRecordingTimeSeconds, s.maxRecordingTimeSeconds, 1e-6);
        }

    } // namespace
} // namespace yvc::app::test
