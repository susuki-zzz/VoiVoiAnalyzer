#include "yvc_core/RamOnlyStorage.h"

#include <filesystem>

namespace yvc {

/// <summary>
/// Appends raw sample block into in-memory store.
/// </summary>
void RamOnlyStorage::append(const Sample* samples,
                            size_t count) {
    if (!samples || count == 0) {
        return;
    }
    storage_.insert(storage_.end(), samples, samples + count);
}

/// <summary>
/// Appends vector of samples into storage.
/// </summary>
void RamOnlyStorage::append(const std::vector<Sample>& samples) {
    storage_.insert(storage_.end(), samples.begin(), samples.end());
}

/// <summary>
/// Clears all stored audio (privacy safeguard).
/// </summary>
void RamOnlyStorage::clear() {
    storage_.clear();
}

/// <summary>
/// Disk flush is intentionally disabled; returns false and removes any existing file path.
/// </summary>
bool RamOnlyStorage::flushToDisk(const std::string& path) const {
    if (path.empty()) {
        return false;
    }
    std::filesystem::path fsPath(path);
    if (std::filesystem::exists(fsPath)) {
        std::filesystem::remove(fsPath);
    }
    return false;
}

} // namespace yvc
