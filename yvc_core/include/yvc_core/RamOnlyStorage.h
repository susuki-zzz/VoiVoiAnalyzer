#pragma once

#include "Types.h"
#include <string>
#include <vector>

namespace yvc {

/// <summary>
/// RAM-only storage for live audio data.
/// Ensures audio data remains in memory only and is never automatically written to disk.
/// </summary>
class RamOnlyStorage {
public:
    /// <summary>
    /// Appends audio samples to storage.
    /// </summary>
    /// <param name="samples">Pointer to audio samples</param>
    /// <param name="count">Number of samples to append</param>
    void append(const Sample* samples, size_t count);

    /// <summary>
    /// Appends audio samples from vector to storage.
    /// </summary>
    /// <param name="samples">Vector of audio samples</param>
    void append(const std::vector<Sample>& samples);

    /// <summary>
    /// Gets the stored audio data.
    /// </summary>
    /// <returns>Reference to internal storage vector</returns>
    const std::vector<Sample>& data() const { return storage_; }

    /// <summary>
    /// Clears all stored data.
    /// </summary>
    void clear();

    /// <summary>
    /// Attempting to flush live data to disk is disallowed.
    /// </summary>
    /// <param name="path">File path (ignored)</param>
    /// <returns>Always returns false. No file is created on disk.</returns>
    bool flushToDisk(const std::string& path) const;

private:
    std::vector<Sample> storage_;
};

} // namespace yvc
