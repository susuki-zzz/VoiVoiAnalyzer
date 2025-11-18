// VoiVoi GUI Application - Preset Sharing System (Metrics Only)
// License: GPLv3

#pragma once

#include <juce_core/juce_core.h>
#include "yvc_core/Types.h"
#include <vector>
#include <string>
#include <mutex>

namespace yvc::app {

    /// <summary>
    /// Privacy-compliant preset sharing system.
    /// Shares only metrics metadata and layout configuration. No raw audio or personal data.
    /// </summary>
    class PresetSharingManager {
    public:
        /// <summary>Metric target configuration for a shared preset.</summary>
        struct MetricTarget {
            std::string metricName;
            float minValue = 0.0f;
            float maxValue = 100.0f;
            float targetValue = 50.0f;
            bool showBaseline = false;
            std::string unit;
        };
        /// <summary>Layout configuration for one UI component within a preset.</summary>
        struct LayoutConfig {
            std::string componentType;
            int gridColumn = 0;
            int gridRow = 0;
            int spanColumns = 1;
            int spanRows = 1;
            bool visible = true;
        };
        /// <summary>Shareable preset container (metrics only).</summary>
        struct ShareablePreset {
            std::string name;
            std::string description;
            std::string category;
            std::string language;
            std::string createdBy;
            std::string version = "1.0";
            int64_t timestamp = 0;
            std::vector<MetricTarget> metricTargets;
            std::vector<LayoutConfig> layoutConfig;
            std::string checksum;
            bool isValidated = false;
        };

        /// <summary>Returns singleton instance.</summary>
        static PresetSharingManager& getInstance();
        /// <summary>Exports current preset (by name) into a shareable structure.</summary>
        ShareablePreset exportPreset(const std::string& presetName) const;
        /// <summary>Imports a shareable preset into local library (validation performed).</summary>
        bool importPreset(const ShareablePreset& sharedPreset);
        /// <summary>Returns all locally stored shareable presets.</summary>
        std::vector<ShareablePreset> getLocalPresets() const;
        /// <summary>Saves preset to library persistence location.</summary>
        bool savePresetToLibrary(const ShareablePreset& preset);
        /// <summary>Deletes preset by generated id (name+version).</summary>
        bool deletePresetFromLibrary(const std::string& presetId);
        /// <summary>Exports preset as JSON file.</summary>
        bool exportToFile(const ShareablePreset& preset, const juce::File& file) const;
        /// <summary>Loads preset from JSON file.</summary>
        bool importFromFile(const juce::File& file, ShareablePreset& preset) const;
        /// <summary>Serializes preset to juce::var (JSON-compatible).</summary>
        juce::var presetToJson(const ShareablePreset& preset) const;
        /// <summary>Deserializes preset from juce::var.</summary>
        ShareablePreset presetFromJson(const juce::var& json) const;
        /// <summary>Validates structural integrity of preset.</summary>
        bool validatePreset(const ShareablePreset& preset) const;
        /// <summary>Computes checksum over preset content for integrity.</summary>
        std::string computeChecksum(const ShareablePreset& preset) const;
        /// <summary>Checks if preset contains personal data (should always be false).</summary>
        bool containsPersonalData(const ShareablePreset& preset) const;
        /// <summary>Sanitizes preset by removing any sensitive fields (defense-in-depth).</summary>
        void sanitizePreset(ShareablePreset& preset) const;

    private:
        PresetSharingManager();
        juce::File getPresetsDirectory() const;
        std::string generatePresetId(const ShareablePreset& preset) const;
        bool isValidPresetName(const std::string& name) const;
        mutable std::mutex mutex_;
        std::vector<ShareablePreset> localPresets_;
    };

    /// <summary>
    /// Preset library UI component for browsing and managing shared presets.
    /// </summary>
    class PresetLibraryComponent : public juce::Component,
        public juce::TableListBoxModel,
        private juce::Button::Listener,
        private juce::FilenameComponentListener {
    public:
        PresetLibraryComponent();
        ~PresetLibraryComponent() override;
        void resized() override; void paint(juce::Graphics& g) override;
        // TableListBoxModel
        int getNumRows() override;
        void paintRowBackground(juce::Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override;
        void paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;
        void cellClicked(int rowNumber, int columnId, const juce::MouseEvent& e) override;
        juce::Component* refreshComponentForCell(int rowNumber, int columnId, bool isRowSelected, juce::Component* existingComponentToUpdate) override;
        // Button::Listener
        void buttonClicked(juce::Button* button) override;
        // FilenameComponentListener
        void filenameComponentChanged(juce::FilenameComponent*) override;
        // Callbacks
        std::function<void(const PresetSharingManager::ShareablePreset&)> onPresetSelected;
        std::function<void(const std::string&)> onPresetDeleted;
    private:
        void refreshPresetList(); void updateUI();
        juce::TableListBox presetTable_;
        juce::TextButton importButton_{ "Import Preset" };
        juce::TextButton exportButton_{ "Export Selected" };
        juce::TextButton deleteButton_{ "Delete Selected" };
        juce::FilenameComponent fileChooser_{ "Choose preset file", {}, true, false, false, "*.json", {}, "Select preset file to import" };
        std::vector<PresetSharingManager::ShareablePreset> displayedPresets_;
        int selectedRow_ = -1;
    };

} // namespace yvc::app
