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

    /// <summary>
    /// Log levels (ordered by severity).
    /// </summary>
    enum class LogLevel {
        LOGLV_TRACE = 0,  // Detailed trace for deep debugging
        LOGLV_DEBUG = 1,  // Debug information
        LOGLV_INFO = 2,   // General information
        LOGLV_WARN = 3,   // Warning messages
        LOGLV_ERROR = 4,  // Error messages
        LOGLV_FATAL = 5,  // Fatal errors (application may crash)
        LOGLV_OFF = 6     // Logging disabled
    };

    /// <summary>
    /// Converts log level to string.
    /// </summary>
    /// <param name="level">Log level</param>
    /// <returns>String representation of log level</returns>
    const char* logLevelToString(LogLevel level);

    /// <summary>
    /// Logger configuration structure.
    /// </summary>
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

    /// <summary>
    /// Thread-safe logger singleton for debugging and diagnostics.
    /// </summary>
    class Logger {
    public:
        /// <summary>
        /// Gets singleton instance.
        /// </summary>
        /// <returns>Reference to logger instance</returns>
        static Logger& getInstance();

        /// <summary>
        /// Configures logger with specified settings.
        /// </summary>
        /// <param name="config">Logger configuration</param>
        void configure(const LoggerConfig& config);

        /// <summary>
        /// Gets current configuration.
        /// </summary>
        /// <returns>Reference to current logger configuration</returns>
        const LoggerConfig& getConfig() const { return config_; }

        /// <summary>
        /// Sets minimum log level at runtime (thread-safe).
        /// </summary>
        /// <param name="level">Minimum log level</param>
        void setMinLevel(LogLevel level);

        /// <summary>
        /// Checks if a log level would be logged (thread-safe).
        /// </summary>
        /// <param name="level">Log level to check</param>
        /// <returns>True if level would be logged</returns>
        bool shouldLog(LogLevel level) const;

        /// <summary>
        /// Logs a message.
        /// </summary>
        /// <param name="level">Log level</param>
        /// <param name="file">Source file name</param>
        /// <param name="line">Line number</param>
        /// <param name="function">Function name</param>
        /// <param name="message">Log message</param>
        void log(LogLevel level, const char* file, int line, const char* function,
                 const std::string& message);

        /// <summary>
        /// Formatted logging (variadic template version for C++20).
        /// </summary>
        template<typename... Args>
        void logf(LogLevel level, const char* file, int line, const char* function,
                  const char* format, Args&&... args) {
            if(!shouldLog(level)) return;

            // Calculate required buffer size
            int size = std::snprintf(nullptr, 0, format, std::forward<Args>(args)...);
            if(size <= 0) {
                log(level, file, line, function, "[Format error]");
                return;
            }

            // Allocate and format
            std::vector<char> buffer(size + 1);
            std::snprintf(buffer.data(), buffer.size(), format, std::forward<Args>(args)...);
            log(level, file, line, function, std::string(buffer.data()));
        }

        /// <summary>
        /// Formatted logging overload for no-argument case.
        /// </summary>
        void logf(LogLevel level, const char* file, int line, const char* function,
                  const char* message) {
            log(level, file, line, function, std::string(message));
        }

        /// <summary>
        /// Flushes all pending log messages.
        /// </summary>
        void flush();

        /// <summary>
        /// Closes logger (call before application exit).
        /// </summary>
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

/// <summary>
/// Scoped timer for performance profiling.
/// </summary>
    class ScopedTimer {
    public:
        /// <summary>
        /// Constructs a scoped timer.
        /// </summary>
        /// <param name="name">Timer name</param>
        /// <param name="level">Log level (default: DEBUG)</param>
        ScopedTimer(const char* name, LogLevel level = LogLevel::LOGLV_DEBUG);

        /// <summary>
        /// Destructor logs elapsed time.
        /// </summary>
        ~ScopedTimer();

    private:
        std::string name_;
        LogLevel level_;
        std::chrono::high_resolution_clock::time_point start_;
    };

    // Macro for easy scoped timing
#define LOG_SCOPE_TIMER(name) yvc::ScopedTimer _scoped_timer_##__LINE__(name)
#define LOG_SCOPE_TIMER_TRACE(name) yvc::ScopedTimer _scoped_timer_##__LINE__(name, yvc::LogLevel::LOGLV_TRACE)

/// <summary>
/// Stream-style logging helper.
/// </summary>
    class LogStream {
    public:
        /// <summary>
        /// Constructs a log stream.
        /// </summary>
        /// <param name="level">Log level</param>
        /// <param name="file">Source file</param>
        /// <param name="line">Line number</param>
        /// <param name="function">Function name</param>
        LogStream(LogLevel level, const char* file, int line, const char* function);

        /// <summary>
        /// Destructor writes accumulated message to log.
        /// </summary>
        ~LogStream();

        /// <summary>
        /// Stream insertion operator.
        /// </summary>
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
