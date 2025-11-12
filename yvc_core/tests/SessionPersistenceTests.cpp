#include <gtest/gtest.h>

#include <yvc_core/SessionPersistence.h>

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>

namespace yvc::test {
namespace {

std::filesystem::path uniqueTempFile(const std::string& prefix) {
    auto base = std::filesystem::temp_directory_path();
    auto path = base / (prefix + std::to_string(std::rand()) + ".cfg");
    return path;
}

TEST(SessionPersistenceTests, ManualSavePersistsUpdatedValues) {
    SessionPersistenceManager manager;
    manager.updateSetting("mode", "standard");
    manager.updateSetting("fft", "2048");

    const auto path = uniqueTempFile("yvc_session_manual");
    manager.manualSave(path.string());

    SessionPersistenceManager loader;
    loader.loadFromDisk(path.string());

    const auto reloaded = uniqueTempFile("yvc_session_verify");
    loader.manualSave(reloaded.string());

    std::ifstream in(reloaded);
    std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    EXPECT_NE(content.find("mode=standard"), std::string::npos);
    EXPECT_NE(content.find("fft=2048"), std::string::npos);

    std::filesystem::remove(path);
    std::filesystem::remove(reloaded);
}

TEST(SessionPersistenceTests, AutoSaveHonorsInterval) {
    SessionPersistenceManager manager;
    manager.updateSetting("mode", "light");
    manager.setAutoSave(true, std::chrono::milliseconds(100));

    const auto path = uniqueTempFile("yvc_session_auto");
    const auto now = std::chrono::system_clock::now();

    EXPECT_TRUE(manager.autoSave(path.string(), now));
    EXPECT_FALSE(manager.autoSave(path.string(), now + std::chrono::milliseconds(50)));
    EXPECT_TRUE(manager.autoSave(path.string(), now + std::chrono::milliseconds(150)));

    std::filesystem::remove(path);
}

} // namespace
} // namespace yvc::test
