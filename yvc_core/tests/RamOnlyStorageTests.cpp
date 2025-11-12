#include <gtest/gtest.h>

#include <yvc_core/RamOnlyStorage.h>

#include <filesystem>

namespace yvc::test {
namespace {

TEST(RamOnlyStorageTests, DoesNotFlushLiveDataToDisk) {
    RamOnlyStorage storage;
    const float sample_block[4] = {0.1f, 0.2f, 0.3f, 0.4f};
    storage.append(sample_block, 4);

    auto tmp = std::filesystem::temp_directory_path() / "yvc_ram_only_storage_test.dat";
    if (std::filesystem::exists(tmp)) {
        std::filesystem::remove(tmp);
    }

    EXPECT_FALSE(storage.flushToDisk(tmp.string()));
    EXPECT_FALSE(std::filesystem::exists(tmp));
}

} // namespace
} // namespace yvc::test
