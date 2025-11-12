#pragma once

#include <chrono>
#include <string>
#include <unordered_map>

namespace yvc {

class SessionPersistenceManager {
public:
    SessionPersistenceManager();

    void setAutoSave(bool enabled, std::chrono::milliseconds interval);
    bool isAutoSaveEnabled() const { return auto_save_enabled_; }
    std::chrono::milliseconds autoSaveInterval() const { return auto_save_interval_; }

    void updateSetting(const std::string& key, const std::string& value);
    const std::unordered_map<std::string, std::string>& settings() const { return settings_; }

    void loadFromDisk(const std::string& path);
    void manualSave(const std::string& path) const;
    bool autoSave(const std::string& path, std::chrono::system_clock::time_point now);

private:
    void persist(const std::string& path) const;

    bool auto_save_enabled_ = false;
    std::chrono::milliseconds auto_save_interval_{60000};
    std::unordered_map<std::string, std::string> settings_;
    std::chrono::system_clock::time_point last_save_{};
};

} // namespace yvc
