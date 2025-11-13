// VoiVoi Core Library - Logger
// License: MIT
// Purpose: Thread-safe, low-overhead logging system for debugging

#pragma once

#include <string>
#include <memory>
#include <mutex>
#include <fstream>
#include <sstream>
#include <chrono>
#include <thread>
#include <vector>
#include <atomic>
#include <cstdio>

namespace yvc {

// Log levels (ordered by severity)
enum class LogLevel {
    LOGLV_TRACE = 0,  // Detailed trace for deep debugging
    LOGLV_DEBUG = 1,  // Debug information
    LOGLV_INFO = 2,   // General information
    LOGLV_WARN = 3,   // Warning messages
    LOGLV_ERROR = 4,  // Error messages
    LOGLV_FATAL = 5,  // Fatal errors (application may crash)
    LOGLV_OFF = 6     // Logging disabled
};

// Convert log level to string
const char* logLevelToString(LogLevel level);

// Logger configuration
struct LoggerConfig {
    LogLevel minLevel = LogLevel::LOGLV_INFO;
    bool enableConsole = true;
    bool enableFile = false;
    std::string logFilePath = "voivoi_debug.log";
    bool includeTimestamp = true;
    bool includeThreadId = true;
    bool includeSourceLocation = true;
    size_t maxFileSize = 10 * 1024 * 1024;  // 10MB default
    bool rotateOnMaxSize = true;
    
    // Privacy settings
    bool allowAudioDataLogging = false;  // MUST be false in production
};

// Logger class (Singleton pattern)
class Logger {
public:
    // Get singleton instance
    static Logger& getInstance();
    
    // Configure logger
    void configure(const LoggerConfig& config);
    
    // Get current configuration
    const LoggerConfig& getConfig() const { return config_; }
    
    // Set minimum log level at runtime (thread-safe)
    void setMinLevel(LogLevel level);
    
    // Check if a log level would be logged (thread-safe)
    bool shouldLog(LogLevel level) const;
    
    // Log a message
    void log(LogLevel level, const char* file, int line, const char* function, 
             const std::string& message);
    
    // Formatted logging (variadic template version for C++20)
    template<typename... Args>
    void logf(LogLevel level, const char* file, int line, const char* function,
              const char* format, Args&&... args) {
        if (!shouldLog(level)) return;
        
        // Calculate required buffer size
        int size = std::snprintf(nullptr, 0, format, std::forward<Args>(args)...);
        if (size <= 0) {
            log(level, file, line, function, "[Format error]");
            return;
        }
        
        // Allocate and format
        std::vector<char> buffer(size + 1);
        std::snprintf(buffer.data(), buffer.size(), format, std::forward<Args>(args)...);
        log(level, file, line, function, std::string(buffer.data()));
    }
    
    // Overload for no-argument case
    void logf(LogLevel level, const char* file, int line, const char* function,
              const char* message) {
        log(level, file, line, function, std::string(message));
    }
    
    // Flush all pending log messages
    void flush();
    
    // Close logger (call before application exit)
    void shutdown();
    
    // Delete copy constructor and assignment operator
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

private:
    Logger();
    ~Logger();
    
    void writeToConsole(LogLevel level, const std::string& message);
    void writeToFile(const std::string& message);
    void rotateLogFile();
    std::string formatMessage(LogLevel level, const char* file, int line, 
                             const char* function, const std::string& message);
    
    LoggerConfig config_;
    std::atomic<LogLevel> minLevel_;  // Thread-safe access
    std::mutex mutex_;
    std::ofstream fileStream_;
    std::atomic<bool> isShutdown_;
};

// Convenience macros for logging (wrapped in do-while for safety)
#define LOG_TRACE(msg) \
    do { \
        yvc::Logger::getInstance().log(yvc::LogLevel::LOGLV_TRACE, __FILE__, __LINE__, __FUNCTION__, msg); \
    } while(0)

#define LOG_DEBUG(msg) \
    do { \
        yvc::Logger::getInstance().log(yvc::LogLevel::LOGLV_DEBUG, __FILE__, __LINE__, __FUNCTION__, msg); \
    } while(0)

#define LOG_INFO(msg) \
    do { \
        yvc::Logger::getInstance().log(yvc::LogLevel::LOGLV_INFO, __FILE__, __LINE__, __FUNCTION__, msg); \
    } while(0)

#define LOG_WARN(msg) \
    do { \
        yvc::Logger::getInstance().log(yvc::LogLevel::LOGLV_WARN, __FILE__, __LINE__, __FUNCTION__, msg); \
    } while(0)

#define LOG_ERROR(msg) \
    do { \
        yvc::Logger::getInstance().log(yvc::LogLevel::LOGLV_ERROR, __FILE__, __LINE__, __FUNCTION__, msg); \
    } while(0)

#define LOG_FATAL(msg) \
    do { \
        yvc::Logger::getInstance().log(yvc::LogLevel::LOGLV_FATAL, __FILE__, __LINE__, __FUNCTION__, msg); \
    } while(0)

// Formatted logging macros without __VA_OPT__ (works for both with/without args)
#define LOG_TRACEF(...) \
    do { \
        yvc::Logger::getInstance().logf(yvc::LogLevel::LOGLV_TRACE, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__); \
    } while(0)

#define LOG_DEBUGF(...) \
    do { \
        yvc::Logger::getInstance().logf(yvc::LogLevel::LOGLV_DEBUG, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__); \
    } while(0)

#define LOG_INFOF(...) \
    do { \
        yvc::Logger::getInstance().logf(yvc::LogLevel::LOGLV_INFO, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__); \
    } while(0)

#define LOG_WARNF(...) \
    do { \
        yvc::Logger::getInstance().logf(yvc::LogLevel::LOGLV_WARN, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__); \
    } while(0)

#define LOG_ERRORF(...) \
    do { \
        yvc::Logger::getInstance().logf(yvc::LogLevel::LOGLV_ERROR, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__); \
    } while(0)

#define LOG_FATALF(...) \
    do { \
        yvc::Logger::getInstance().logf(yvc::LogLevel::LOGLV_FATAL, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__); \
    } while(0)

// Scoped timer for performance profiling
class ScopedTimer {
public:
    ScopedTimer(const char* name, LogLevel level = LogLevel::LOGLV_DEBUG);
    ~ScopedTimer();
    
private:
    std::string name_;
    LogLevel level_;
    std::chrono::high_resolution_clock::time_point start_;
};

// Macro for easy scoped timing
#define LOG_SCOPE_TIMER(name) yvc::ScopedTimer _scoped_timer_##__LINE__(name)
#define LOG_SCOPE_TIMER_TRACE(name) yvc::ScopedTimer _scoped_timer_##__LINE__(name, yvc::LogLevel::LOGLV_TRACE)

// Stream-style logging helper
class LogStream {
public:
    LogStream(LogLevel level, const char* file, int line, const char* function);
    ~LogStream();
    
    template<typename T>
    LogStream& operator<<(const T& value) {
        stream_ << value;
        return *this;
    }
    
private:
    LogLevel level_;
    const char* file_;
    int line_;
    const char* function_;
    std::ostringstream stream_;
};

// Stream-style logging macros
#define LOG_STREAM(level) \
    yvc::LogStream(level, __FILE__, __LINE__, __FUNCTION__)

} // namespace yvc
