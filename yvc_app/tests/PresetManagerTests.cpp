#include <gtest/gtest.h>

#include "PresetManager.h"

#include <algorithm>
#include <limits>
#include <memory>

#include <juce_gui_basics/juce_gui_basics.h>

namespace yvc::app::test {
namespace {

class JuceFixture : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        juce_initialiser_ = std::make_unique<juce::ScopedJuceInitialiser_GUI>();
    }

    static void TearDownTestSuite() {
        juce_initialiser_.reset();
    }

    static inline std::unique_ptr<juce::ScopedJuceInitialiser_GUI> juce_initialiser_;
};

TEST_F(JuceFixture, PresetManagerDefaultsToFirstPreset) {
    PresetManager manager;
    EXPECT_EQ(manager.getActivePresetIndex(), 0);
    ASSERT_FALSE(manager.getPresets().empty());
    EXPECT_EQ(&manager.getActivePreset(), &manager.getPresets().front());
}

TEST_F(JuceFixture, PresetSwitchingUpdatesActivePreset) {
    PresetManager manager;
    ASSERT_GE(manager.getPresets().size(), 2u);

    const auto& target = manager.setActivePreset(1);
    EXPECT_EQ(manager.getActivePresetIndex(), 1);
    EXPECT_EQ(&manager.getActivePreset(), &target);

    manager.setActivePreset(99);  // Out of range should keep previous value
    EXPECT_EQ(manager.getActivePresetIndex(), 1);
    EXPECT_EQ(&manager.getActivePreset(), &target);
}

TEST_F(JuceFixture, PresetsProvideMetricLayouts) {
    PresetManager manager;
    for (const auto& preset : manager.getPresets()) {
        EXPECT_FALSE(preset.nameKey.isEmpty());
        EXPECT_FALSE(preset.descriptionKey.isEmpty());
        EXPECT_FALSE(preset.metrics.empty());
        EXPECT_TRUE(std::all_of(preset.metrics.begin(), preset.metrics.end(), [](auto metric) {
            return metric >= MetricDisplayType::F0Gauge && metric <= MetricDisplayType::SCentroid;
        }));
    }
}

TEST_F(JuceFixture, InvalidPresetSelectionsReturnCurrentPreset) {
    PresetManager manager;
    ASSERT_FALSE(manager.getPresets().empty());

    const auto& initial = manager.getActivePreset();
    const auto initialIndex = manager.getActivePresetIndex();

    const auto& sameFromSize = manager.setActivePreset(manager.getPresets().size());
    EXPECT_EQ(&sameFromSize, &initial);
    EXPECT_EQ(manager.getActivePresetIndex(), initialIndex);
    EXPECT_EQ(&manager.getActivePreset(), &initial);

    const auto& sameFromMax = manager.setActivePreset(std::numeric_limits<size_t>::max());
    EXPECT_EQ(&sameFromMax, &initial);
    EXPECT_EQ(manager.getActivePresetIndex(), initialIndex);
}

} // namespace
} // namespace yvc::app::test
