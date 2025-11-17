// VoiVoi Core Library - Logger Implementation
// License: MIT

#include "yvc_core/Logger.h"
#include <iostream>
#include <iomanip>
#include <ctime>
#include <cstring>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#undef ERROR  // Undefine Windows ERROR macro to avoid conflicts
#endif

namespace yvc {

/// <summary>
/// Converts a LogLevel to a fixed-width uppercase string label.
/// </summary>
const char* logLevelToString(LogLevel level) {
    switch (level) {
        case LogLevel::LOGLV_TRACE: return "TRACE";
        case LogLevel::LOGLV_DEBUG: return "DEBUG";
        case LogLevel::LOGLV_INFO:  return "INFO ";
        case LogLevel::LOGLV_WARN:  return "WARN ";
        case LogLevel::LOGLV_ERROR: return "ERROR";
        case LogLevel::LOGLV_FATAL: return "FATAL";
        case LogLevel::LOGLV_OFF:   return "OFF  ";
        default:                    return "UNKNW";
    }
}

// Logger implementation

/// <summary>
/// Returns process-wide singleton logger instance.
/// </summary>
Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

/// <summary>
/// Constructs logger with safe defaults (INFO level, console on, file off).
/// </summary>
Logger::Logger() : minLevel_(LogLevel::LOGLV_INFO), isShutdown_(false) {
    config_.minLevel = LogLevel::LOGLV_INFO;
    config_.enableConsole = true;
    config_.enableFile = false;
}

/// <summary>
/// Ensures graceful shutdown (flush + file close) on destruction.
/// </summary>
Logger::~Logger() {
    shutdown();
}

/// <summary>
/// Applies configuration: sets min level, opens file if requested, writes headers, enforces privacy.
/// Thread-safe.
/// </summary>
void Logger::configure(const LoggerConfig& config) {
    std::lock_guard<std::mutex> lock(mutex_);

    // Close existing file stream if open
    if (fileStream_.is_open()) {
        fileStream_.close();
    }

    config_ = config;
    minLevel_.store(config.minLevel, std::memory_order_release);

    // Privacy: never allow audio payload logs in production builds
    #if defined(NDEBUG) || defined(YVC_PRODUCTION)
    config_.allowAudioDataLogging = false;
    #endif

    // Open log file if enabled
    if (config_.enableFile && !config_.logFilePath.empty()) {
        fileStream_.open(config_.logFilePath, std::ios::out | std::ios::app);
        if (!fileStream_.is_open()) {
            std::cerr << "Logger: Failed to open log file: " << config_.logFilePath << std::endl;
            config_.enableFile = false;
        } else {
            // Write header
            auto now = std::chrono::system_clock::now();
            auto time = std::chrono::system_clock::to_time_t(now);
            fileStream_ << "\n========================================\n";
            fileStream_ << "VoiVoi Analyzer Log - " << std::ctime(&time);
            fileStream_ << "========================================\n";
            fileStream_.flush();
        }
    }
}

/// <summary>
/// Atomically updates minimum log level and caches it into config.
/// </summary>
void Logger::setMinLevel(LogLevel level) {
    minLevel_.store(level, std::memory_order_release);
    std::lock_guard<std::mutex> lock(mutex_);
    config_.minLevel = level;
}

/// <summary>
/// Fast-path check whether a given level would be emitted.
/// </summary>
bool Logger::shouldLog(LogLevel level) const {
    return !isShutdown_.load(std::memory_order_acquire) && 
           level >= minLevel_.load(std::memory_order_acquire);
}

/// <summary>
/// Formats and emits a log record to console and/or file depending on configuration.
/// Thread-safe; returns immediately if level is filtered.
/// </summary>
void Logger::log(LogLevel level, const char* file, int line, const char* function,
                const std::string& message) {
    if (!shouldLog(level)) {
        return;
    }

    std::string formattedMessage = formatMessage(level, file, line, function, message);

    std::lock_guard<std::mutex> lock(mutex_);

    if (config_.enableConsole) {
        writeToConsole(level, formattedMessage);
    }

    if (config_.enableFile && fileStream_.is_open()) {
        writeToFile(formattedMessage);
    }
}

/// <summary>
/// Flushes file and console streams.
/// </summary>
void Logger::flush() {
    std::lock_guard<std::mutex> lock(mutex_);

    if (fileStream_.is_open()) {
        fileStream_.flush();
    }

    std::cout.flush();
    std::cerr.flush();
}

/// <summary>
/// Idempotent shutdown: flushes output, writes trailer and closes file.
/// </summary>
void Logger::shutdown() {
    if (isShutdown_.exchange(true, std::memory_order_acq_rel)) {
        return;  // Already shut down
    }

    flush();  // Flush before shutdown

    std::lock_guard<std::mutex> lock(mutex_);

    if (fileStream_.is_open()) {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        fileStream_ << "\n========================================\n";
        fileStream_ << "Logger shutdown - " << std::ctime(&time);
        fileStream_ << "========================================\n";
        fileStream_.flush();
        fileStream_.close();
    }
}

/// <summary>
/// Writes a line to console. WARN+ to stderr, INFO- to stdout. Mirrors to OutputDebugString on Windows.
/// </summary>
void Logger::writeToConsole(LogLevel level, const std::string& message) {
    auto& stream = (level >= LogLevel::LOGLV_WARN) ? std::cerr : std::cout;
    stream << message << std::endl;

    #ifdef _WIN32
    OutputDebugStringA(message.c_str());
    OutputDebugStringA("\n");
    #endif
}

/// <summary>
/// Writes a line to file and rotates if size exceeded.
/// </summary>
void Logger::writeToFile(const std::string& message) {
    if (!fileStream_.is_open()) {
        return;
    }

    fileStream_ << message << std::endl;

    if (config_.rotateOnMaxSize) {
        auto currentPos = fileStream_.tellp();
        if (currentPos > 0 && static_cast<size_t>(currentPos) >= config_.maxFileSize) {
            rotateLogFile();
        }
    }
}

/// <summary>
/// Rotates the log file to `<path>.old` (or timestamped backup if rename fails) and opens a fresh file.
/// </summary>
void Logger::rotateLogFile() {
    if (!fileStream_.is_open()) {
        return;
    }

    fileStream_.close();

    std::string oldPath = config_.logFilePath + ".old";
    std::remove(oldPath.c_str());

    if (std::rename(config_.logFilePath.c_str(), oldPath.c_str()) != 0) {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::ostringstream backupPath;
        backupPath << config_.logFilePath << ".backup." << time;
        std::rename(config_.logFilePath.c_str(), backupPath.str().c_str());
    }

    fileStream_.open(config_.logFilePath, std::ios::out | std::ios::trunc);
    if (fileStream_.is_open()) {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        fileStream_ << "========================================\n";
        fileStream_ << "VoiVoi Analyzer Log (Rotated) - " << std::ctime(&time);
        fileStream_ << "========================================\n";
    } else {
        std::cerr << "Logger: Failed to reopen log file after rotation: " 
                  << config_.logFilePath << std::endl;
    }
}

/// <summary>
/// Assembles a log line using timestamp/thread id/level and optional source location.
/// </summary>
std::string Logger::formatMessage(LogLevel level, const char* file, int line,
                                 const char* function, const std::string& message) {
    std::ostringstream oss;

    // Timestamp
    if (config_.includeTimestamp) {
        auto now = std::chrono::system_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;
        auto time = std::chrono::system_clock::to_time_t(now);

        struct tm timeinfo;
        #ifdef _WIN32
        localtime_s(&timeinfo, &time);
        #else
        localtime_r(&time, &timeinfo);
        #endif

        oss << std::put_time(&timeinfo, "%Y-%m-%d %H:%M:%S");
        oss << '.' << std::setfill('0') << std::setw(3) << ms.count();
        oss << " ";
    }

    // Thread ID
    if (config_.includeThreadId) {
        oss << "[T:" << std::this_thread::get_id() << "] ";
    }

    // Log level
    oss << "[" << logLevelToString(level) << "] ";

    // Source location (only if provided)
    if (config_.includeSourceLocation && file && function && file[0] != '\0') {
        const char* filename = file;
        const char* slash = strrchr(file, '/');
        const char* backslash = strrchr(file, '\\');
        if (slash && (!backslash || slash > backslash)) filename = slash + 1; else if (backslash) filename = backslash + 1;
        oss << filename << ":" << line << " (" << function << ") - ";
    }

    // Message
    oss << message;

    return oss.str();
}

// ScopedTimer implementation

/// <summary>
/// Starts a scoped performance timer; logs elapsed time on destruction.
/// </summary>
ScopedTimer::ScopedTimer(const char* name, LogLevel level)
    : name_(name), level_(level), start_(std::chrono::high_resolution_clock::now()) {}

/// <summary>
/// Logs the elapsed microseconds since construction using the configured level.
/// </summary>
ScopedTimer::~ScopedTimer() {
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start_);
    std::ostringstream oss;
    oss << "[TIMER] " << name_ << " took " << duration.count() << " µs";
    Logger::getInstance().log(level_, nullptr, 0, nullptr, oss.str());
}

// LogStream implementation

/// <summary>
/// Captures stream insertion until destruction, then emits as a single log record.
/// </summary>
LogStream::LogStream(LogLevel level, const char* file, int line, const char* function)
    : level_(level), file_(file), line_(line), function_(function) {}

/// <summary>
/// Flushes accumulated message to the logger.
/// </summary>
LogStream::~LogStream() {
    Logger::getInstance().log(level_, file_, line_, function_, stream_.str());
}

} // namespace yvc
