#include <gtest/gtest.h>

#include <yvc_core/Logger.h>

#include <filesystem>
#include <fstream>
#include <thread>
#include <vector>
#include <atomic>

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
    config.minLevel = LogLevel::LOGLV_TRACE;
    config.enableConsole = false;
    config.enableFile = true;
    config.logFilePath = log_path.string();
    config.maxFileSize = 256;
    config.rotateOnMaxSize = true;

    logger.configure(config);

    for (int i = 0; i < 200; ++i) {
        logger.log(LogLevel::LOGLV_INFO, __FILE__, __LINE__, __FUNCTION__, "Filling log file with data");
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

TEST(LoggerTests, ThreadSafeLogging) {
    auto& logger = Logger::getInstance();
    
    LoggerConfig config;
    config.minLevel = LogLevel::LOGLV_DEBUG;
    config.enableConsole = false;
    config.enableFile = false;
    logger.configure(config);

    constexpr int kNumThreads = 10;
    constexpr int kLogsPerThread = 100;
    std::atomic<int> completedThreads{0};
    
    std::vector<std::thread> threads;
    threads.reserve(kNumThreads);
    
    for (int i = 0; i < kNumThreads; ++i) {
        threads.emplace_back([i, &logger, &completedThreads]() {
            for (int j = 0; j < kLogsPerThread; ++j) {
                LOG_DEBUGF("Thread %d message %d", i, j);
                LOG_INFO(std::string("Thread ") + std::to_string(i) + " info");
                LOG_WARN("Thread warning");
            }
            completedThreads.fetch_add(1, std::memory_order_relaxed);
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    EXPECT_EQ(completedThreads.load(), kNumThreads);
}

TEST(LoggerTests, MinLevelThreadSafe) {
    auto& logger = Logger::getInstance();
    
    LoggerConfig config;
    config.minLevel = LogLevel::LOGLV_INFO;
    config.enableConsole = false;
    config.enableFile = false;
    logger.configure(config);
    
    std::atomic<bool> stopFlag{false};
    std::atomic<int> logCount{0};
    
    // Thread that changes log level
    std::thread configThread([&]() {
        for (int i = 0; i < 50; ++i) {
            logger.setMinLevel(LogLevel::LOGLV_DEBUG);
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            logger.setMinLevel(LogLevel::LOGLV_WARN);
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        stopFlag.store(true);
    });
    
    // Thread that logs
    std::thread logThread([&]() {
        while (!stopFlag.load()) {
            if (logger.shouldLog(LogLevel::LOGLV_DEBUG)) {
                LOG_DEBUG("Debug message");
                logCount.fetch_add(1);
            }
            if (logger.shouldLog(LogLevel::LOGLV_WARN)) {
                LOG_WARN("Warning message");
                logCount.fetch_add(1);
            }
        }
    });
    
    configThread.join();
    logThread.join();
    
    // Just verify we didn't crash
    EXPECT_GT(logCount.load(), 0);
}

TEST(LoggerTests, FormattedLoggingNoArgs) {
    auto& logger = Logger::getInstance();
    
    LoggerConfig config;
    config.minLevel = LogLevel::LOGLV_INFO;
    config.enableConsole = false;
    config.enableFile = false;
    logger.configure(config);
    
    // Should not crash with no arguments
    LOG_INFOF("Simple message with no args");
    SUCCEED();
}

TEST(LoggerTests, FormattedLoggingWithArgs) {
    auto& logger = Logger::getInstance();
    
    LoggerConfig config;
    config.minLevel = LogLevel::LOGLV_INFO;
    config.enableConsole = false;
    config.enableFile = false;
    logger.configure(config);
    
    LOG_INFOF("Value: %d, String: %s", 42, "test");
    LOG_DEBUGF("Float: %.2f", 3.14159);
    SUCCEED();
}

TEST(LoggerTests, ScopedTimerBasic) {
    auto& logger = Logger::getInstance();
    
    LoggerConfig config;
    config.minLevel = LogLevel::LOGLV_DEBUG;
    config.enableConsole = false;
    config.enableFile = false;
    logger.configure(config);
    
    {
        LOG_SCOPE_TIMER("TestOperation");
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    SUCCEED();
}

TEST(LoggerTests, LogLevelOrdering) {
    EXPECT_LT(static_cast<int>(LogLevel::LOGLV_TRACE), static_cast<int>(LogLevel::LOGLV_DEBUG));
    EXPECT_LT(static_cast<int>(LogLevel::LOGLV_DEBUG), static_cast<int>(LogLevel::LOGLV_INFO));
    EXPECT_LT(static_cast<int>(LogLevel::LOGLV_INFO), static_cast<int>(LogLevel::LOGLV_WARN));
    EXPECT_LT(static_cast<int>(LogLevel::LOGLV_WARN), static_cast<int>(LogLevel::LOGLV_ERROR));
    EXPECT_LT(static_cast<int>(LogLevel::LOGLV_ERROR), static_cast<int>(LogLevel::LOGLV_FATAL));
}

} // namespace
} // namespace yvc::test
