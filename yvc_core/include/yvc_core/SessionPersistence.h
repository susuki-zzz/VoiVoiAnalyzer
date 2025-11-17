#pragma once

#include <chrono>
#include <string>
#include <unordered_map>

namespace yvc {

/// <summary>
/// Manages session persistence including auto-save and settings.
/// </summary>
class SessionPersistenceManager {
public:
    /// <summary>
    /// Constructs a session persistence manager.
    /// </summary>
    SessionPersistenceManager();

    /// <summary>
    /// Configures auto-save behavior.
    /// </summary>
    /// <param name="enabled">Enable or disable auto-save</param>
    /// <param name="interval">Auto-save interval</param>
    void setAutoSave(bool enabled, std::chrono::milliseconds interval);

    /// <summary>
    /// Checks if auto-save is enabled.
    /// </summary>
    /// <returns>True if auto-save is enabled</returns>
    bool isAutoSaveEnabled() const { return auto_save_enabled_; }

    /// <summary>
    /// Gets auto-save interval.
    /// </summary>
    /// <returns>Auto-save interval in milliseconds</returns>
    std::chrono::milliseconds autoSaveInterval() const { return auto_save_interval_; }

    /// <summary>
    /// Updates a setting value.
    /// </summary>
    /// <param name="key">Setting key</param>
    /// <param name="value">Setting value</param>
    void updateSetting(const std::string& key, const std::string& value);

    /// <summary>
    /// Gets all settings.
    /// </summary>
    /// <returns>Map of settings key-value pairs</returns>
    const std::unordered_map<std::string, std::string>& settings() const { return settings_; }

    /// <summary>
    /// Loads session data from disk.
    /// </summary>
    /// <param name="path">File path to load from</param>
    void loadFromDisk(const std::string& path);

    /// <summary>
    /// Manually saves session data.
    /// </summary>
    /// <param name="path">File path to save to</param>
    void manualSave(const std::string& path) const;

    /// <summary>
    /// Auto-saves if interval has elapsed.
    /// </summary>
    /// <param name="path">File path to save to</param>
    /// <param name="now">Current time point</param>
    /// <returns>True if save occurred, false otherwise</returns>
    bool autoSave(const std::string& path, std::chrono::system_clock::time_point now);

private:
    void persist(const std::string& path) const;

    bool auto_save_enabled_ = false;
    std::chrono::milliseconds auto_save_interval_{60000};
    std::unordered_map<std::string, std::string> settings_;
    std::chrono::system_clock::time_point last_save_{};
};

} // namespace yvc
