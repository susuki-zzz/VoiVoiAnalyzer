#pragma once

#include "Types.h"
#include <string>
#include <vector>

namespace yvc {

class RamOnlyStorage {
public:
    void append(const Sample* samples, size_t count);
    void append(const std::vector<Sample>& samples);

    const std::vector<Sample>& data() const { return storage_; }
    void clear();

    // Attempting to flush live data to disk is disallowed. The function returns false and
    // guarantees that no file is created on disk.
    bool flushToDisk(const std::string& path) const;

private:
    std::vector<Sample> storage_;
};

} // namespace yvc
