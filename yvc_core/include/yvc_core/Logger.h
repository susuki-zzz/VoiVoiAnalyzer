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

// On Windows, other headers may define ERROR as a macro which breaks our enum.
#ifdef _WIN32
# ifdef ERROR
#  undef ERROR
# endif
#endif

namespace yvc {

// Log levels (ordered by severity)
enum class LogLevel {
    TRACE = 0,  // Detailed trace for deep debugging
    DEBUG = 1,  // Debug information
    INFO = 2,   // General information
    WARN = 3,   // Warning messages
    ERROR = 4,  // Error messages
    FATAL = 5,  // Fatal errors (application may crash)
    OFF = 6     // Logging disabled
};

// Convert log level to string
const char* logLevelToString(LogLevel level);

// Logger configuration
struct LoggerConfig {
    LogLevel minLevel = LogLevel::INFO;
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
    
    // Set minimum log level at runtime
    void setMinLevel(LogLevel level);
    
    // Check if a log level would be logged
    bool shouldLog(LogLevel level) const;
    
    // Log a message
    void log(LogLevel level, const char* file, int line, const char* function, 
             const std::string& message);
    
    // Formatted logging (printf-style)
    template<typename... Args>
    void logf(LogLevel level, const char* file, int line, const char* function,
              const char* format, Args... args) {
        if (!shouldLog(level)) return;
        
        char buffer[1024];
        std::snprintf(buffer, sizeof(buffer), format, args...);
        log(level, file, line, function, std::string(buffer));
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
    
    void writeToConsole(const std::string& message);
    void writeToFile(const std::string& message);
    void rotateLogFile();
    std::string formatMessage(LogLevel level, const char* file, int line, 
                             const char* function, const std::string& message);
    
    LoggerConfig config_;
    std::mutex mutex_;
    std::ofstream fileStream_;
    std::atomic<bool> isShutdown_;
};

// Convenience macros for logging
#define LOG_TRACE(msg) \
    yvc::Logger::getInstance().log(yvc::LogLevel::TRACE, __FILE__, __LINE__, __FUNCTION__, msg)

#define LOG_DEBUG(msg) \
    yvc::Logger::getInstance().log(yvc::LogLevel::DEBUG, __FILE__, __LINE__, __FUNCTION__, msg)

#define LOG_INFO(msg) \
    yvc::Logger::getInstance().log(yvc::LogLevel::INFO, __FILE__, __LINE__, __FUNCTION__, msg)

#define LOG_WARN(msg) \
    yvc::Logger::getInstance().log(yvc::LogLevel::WARN, __FILE__, __LINE__, __FUNCTION__, msg)

#define LOG_ERROR(msg) \
    yvc::Logger::getInstance().log(yvc::LogLevel::ERROR, __FILE__, __LINE__, __FUNCTION__, msg)

#define LOG_FATAL(msg) \
    yvc::Logger::getInstance().log(yvc::LogLevel::FATAL, __FILE__, __LINE__, __FUNCTION__, msg)

// Formatted logging macros
#define LOG_TRACEF(fmt, ...) \
    yvc::Logger::getInstance().logf(yvc::LogLevel::TRACE, __FILE__, __LINE__, __FUNCTION__, fmt, __VA_ARGS__)

#define LOG_DEBUGF(fmt, ...) \
    yvc::Logger::getInstance().logf(yvc::LogLevel::DEBUG, __FILE__, __LINE__, __FUNCTION__, fmt, __VA_ARGS__)

#define LOG_INFOF(fmt, ...) \
    yvc::Logger::getInstance().logf(yvc::LogLevel::INFO, __FILE__, __LINE__, __FUNCTION__, fmt, __VA_ARGS__)

#define LOG_WARNF(fmt, ...) \
    yvc::Logger::getInstance().logf(yvc::LogLevel::WARN, __FILE__, __LINE__, __FUNCTION__, fmt, __VA_ARGS__)

#define LOG_ERRORF(fmt, ...) \
    yvc::Logger::getInstance().logf(yvc::LogLevel::ERROR, __FILE__, __LINE__, __FUNCTION__, fmt, __VA_ARGS__)

#define LOG_FATALF(fmt, ...) \
    yvc::Logger::getInstance().logf(yvc::LogLevel::FATAL, __FILE__, __LINE__, __FUNCTION__, fmt, __VA_ARGS__)

// Scoped timer for performance profiling
class ScopedTimer {
public:
    ScopedTimer(const char* name, LogLevel level = LogLevel::DEBUG);
    ~ScopedTimer();
    
private:
    std::string name_;
    LogLevel level_;
    std::chrono::high_resolution_clock::time_point start_;
};

// Macro for easy scoped timing
#define LOG_SCOPE_TIMER(name) yvc::ScopedTimer _scoped_timer_##__LINE__(name)
#define LOG_SCOPE_TIMER_TRACE(name) yvc::ScopedTimer _scoped_timer_##__LINE__(name, yvc::LogLevel::TRACE)

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
