#include <gtest/gtest.h>

#include <yvc_core/Logger.h>

#include <filesystem>
#include <fstream>

namespace yvc::test {
namespace {

TEST(LoggerTests, RotatesWhenFileReachesLimit) {
    auto& logger = Logger::getInstance();
    auto log_path = std::filesystem::temp_directory_path() / "yvc_logger_test.log";
    auto rotated_path = log_path;
    rotated_path += ".old";

    if (std::filesystem::exists(log_path)) {
        std::filesystem::remove(log_path);
    }
    if (std::filesystem::exists(rotated_path)) {
        std::filesystem::remove(rotated_path);
    }

    LoggerConfig config;
    config.minLevel = LogLevel::TRACE;
    config.enableConsole = false;
    config.enableFile = true;
    config.logFilePath = log_path.string();
    config.maxFileSize = 256;
    config.rotateOnMaxSize = true;

    logger.configure(config);

    for (int i = 0; i < 200; ++i) {
        logger.log(LogLevel::INFO, __FILE__, __LINE__, __FUNCTION__, "Filling log file with data");
    }
    logger.flush();

    EXPECT_TRUE(std::filesystem::exists(log_path));
    EXPECT_TRUE(std::filesystem::exists(rotated_path));

    LoggerConfig reset;
    reset.enableConsole = false;
    reset.enableFile = false;
    logger.configure(reset);

    std::filesystem::remove(log_path);
    std::filesystem::remove(rotated_path);
}

} // namespace
} // namespace yvc::test
