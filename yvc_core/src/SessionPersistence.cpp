#include "yvc_core/SessionPersistence.h"

#include <filesystem>
#include <fstream>

namespace yvc {

SessionPersistenceManager::SessionPersistenceManager() = default;

void SessionPersistenceManager::setAutoSave(bool enabled, std::chrono::milliseconds interval) {
    auto_save_enabled_ = enabled;
    if (interval.count() > 0) {
        auto_save_interval_ = interval;
    }
}

void SessionPersistenceManager::updateSetting(const std::string& key, const std::string& value) {
    settings_[key] = value;
}

void SessionPersistenceManager::loadFromDisk(const std::string& path) {
    std::ifstream in(path);
    if (!in.is_open()) {
        return;
    }
    settings_.clear();
    std::string line;
    while (std::getline(in, line)) {
        auto pos = line.find('=');
        if (pos == std::string::npos) {
            continue;
        }
        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);
        settings_[key] = value;
    }
}

void SessionPersistenceManager::manualSave(const std::string& path) const {
    persist(path);
}

bool SessionPersistenceManager::autoSave(const std::string& path, std::chrono::system_clock::time_point now) {
    if (!auto_save_enabled_) {
        return false;
    }
    if (last_save_.time_since_epoch().count() != 0) {
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_save_);
        if (elapsed < auto_save_interval_) {
            return false;
        }
    }
    persist(path);
    last_save_ = now;
    return true;
}

void SessionPersistenceManager::persist(const std::string& path) const {
    std::filesystem::path fsPath(path);
    if (fsPath.has_parent_path()) {
        std::filesystem::create_directories(fsPath.parent_path());
    }
    std::ofstream out(fsPath, std::ios::trunc);
    for (const auto& [key, value] : settings_) {
        out << key << '=' << value << '\n';
    }
}

} // namespace yvc
