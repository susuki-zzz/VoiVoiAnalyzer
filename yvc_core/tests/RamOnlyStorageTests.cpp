#include "yvc_core/RamOnlyStorage.h"

#include <cassert>
#include <filesystem>
#include <iostream>

int main() {
    yvc::RamOnlyStorage storage;
    float sample_block[4] = {0.1f, 0.2f, 0.3f, 0.4f};
    storage.append(sample_block, 4);

    std::filesystem::path tmp = std::filesystem::temp_directory_path() / "yvc_ram_only_storage_test.dat";
    if (std::filesystem::exists(tmp)) {
        std::filesystem::remove(tmp);
    }

    bool flushed = storage.flushToDisk(tmp.string());
    assert(!flushed && "Live data should not flush to disk");
    assert(!std::filesystem::exists(tmp) && "No file should be created on disk");

    std::cout << "RamOnlyStorage test passed" << std::endl;
    return 0;
}
