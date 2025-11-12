// VoiVoi GUI Application - Preset Sharing System (Metrics Only)
// License: GPLv3

#pragma once

#include <juce_core/juce_core.h>
#include "yvc_core/Types.h"
#include <vector>
#include <string>

namespace yvc::app {

/**
 * @brief Privacy-compliant preset sharing system
 * 
 * Implements preset sharing that only includes metrics metadata, targets,
 * and layout configuration. NO raw audio or identifiable data is ever included.
 * This ensures user privacy while enabling preset collaboration.
 */
class PresetSharingManager {
public:
    // Shareable preset structure - only metrics metadata
    struct ShareablePreset {
        struct MetricTarget {
            std::string metricName;
            float minValue = 0.0f;
            float maxValue = 100.0f;
            float targetValue = 50.0f;
            bool showBaseline = false;
            std::string unit;
        };
        
        struct LayoutConfig {
            std::string componentType;  // "gauge", "meter", "heatmap"
            int gridColumn = 0;
            int gridRow = 0;
            int spanColumns = 1;
            int spanRows = 1;
            bool visible = true;
        };
        
        // Preset metadata
        std::string name;
        std::string description;
        std::string category;        // "conversation", "training", "assessment"
        std::string language;        // "en", "ja"
        std::string createdBy;       // Anonymous or username (no personal data)
        std::string version = "1.0";
        int64_t timestamp = 0;       // Creation time
        
        // Metrics configuration
        std::vector<MetricTarget> metricTargets;
        std::vector<LayoutConfig> layoutConfig;
        
        // Validation metadata
        std::string checksum;        // For integrity validation
        bool isValidated = false;    // Community validation status
    };
    
    static PresetSharingManager& getInstance();
    
    // Export preset for sharing
    ShareablePreset exportPreset(const std::string& presetName) const;
    
    // Import preset from shared data
    bool importPreset(const ShareablePreset& sharedPreset);
    
    // Local preset library management
    std::vector<ShareablePreset> getLocalPresets() const;
    bool savePresetToLibrary(const ShareablePreset& preset);
    bool deletePresetFromLibrary(const std::string& presetId);
    
    // File I/O for sharing
    bool exportToFile(const ShareablePreset& preset, const juce::File& file) const;
    bool importFromFile(const juce::File& file, ShareablePreset& preset) const;
    
    // JSON serialization
    juce::var presetToJson(const ShareablePreset& preset) const;
    ShareablePreset presetFromJson(const juce::var& json) const;
    
    // Validation and security
    bool validatePreset(const ShareablePreset& preset) const;
    std::string computeChecksum(const ShareablePreset& preset) const;
    bool containsPersonalData(const ShareablePreset& preset) const; // Security check
    
    // Privacy compliance
    void sanitizePreset(ShareablePreset& preset) const;  // Remove any sensitive data
    
private:
    PresetSharingManager();
    
    juce::File getPresetsDirectory() const;
    std::string generatePresetId(const ShareablePreset& preset) const;
    bool isValidPresetName(const std::string& name) const;
    
    mutable std::mutex mutex_;
    std::vector<ShareablePreset> localPresets_;
};

/**
 * @brief Preset library component for browsing and managing shared presets
 */
class PresetLibraryComponent : public juce::Component,
                               public juce::TableListBoxModel,
                               private juce::Button::Listener,
                               private juce::FilenameComponentListener {
public:
    PresetLibraryComponent();
    ~PresetLibraryComponent() override;
    
    // Component overrides
    void resized() override;
    void paint(juce::Graphics& g) override;
    
    // TableListBoxModel overrides
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
    void refreshPresetList();
    void updateUI();
    
    juce::TableListBox presetTable_;
    juce::TextButton importButton_{ "Import Preset" };
    juce::TextButton exportButton_{ "Export Selected" };
    juce::TextButton deleteButton_{ "Delete Selected" };
    juce::FilenameComponent fileChooser_{ "Choose preset file", {}, true, false, false, "*.json", {}, "Select preset file to import" };
    
    std::vector<PresetSharingManager::ShareablePreset> displayedPresets_;
    int selectedRow_ = -1;
};

} // namespace yvc::app
