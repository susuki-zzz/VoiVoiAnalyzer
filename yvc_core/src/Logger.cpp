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

// Convert log level to string
const char* logLevelToString(LogLevel level) {
    switch (level) {
        case LogLevel::TRACE: return "TRACE";
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO:  return "INFO ";
        case LogLevel::WARN:  return "WARN ";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::FATAL: return "FATAL";
        case LogLevel::OFF:   return "OFF  ";
        default:              return "UNKNW";
    }
}

// Logger implementation

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

Logger::Logger() : isShutdown_(false) {
    // Default configuration
    config_.minLevel = LogLevel::INFO;
    config_.enableConsole = true;
    config_.enableFile = false;
}

Logger::~Logger() {
    shutdown();
}

void Logger::configure(const LoggerConfig& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Close existing file stream if open
    if (fileStream_.is_open()) {
        fileStream_.close();
    }
    
    config_ = config;
    
    // Privacy check: Never allow audio data logging in production
    #ifdef NDEBUG
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

void Logger::setMinLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(mutex_);
    config_.minLevel = level;
}

bool Logger::shouldLog(LogLevel level) const {
    return !isShutdown_.load() && level >= config_.minLevel;
}

void Logger::log(LogLevel level, const char* file, int line, const char* function,
                const std::string& message) {
    if (!shouldLog(level)) {
        return;
    }
    
    std::string formattedMessage = formatMessage(level, file, line, function, message);
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (config_.enableConsole) {
        writeToConsole(formattedMessage);
    }
    
    if (config_.enableFile && fileStream_.is_open()) {
        writeToFile(formattedMessage);
    }
}

void Logger::flush() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (fileStream_.is_open()) {
        fileStream_.flush();
    }
    
    std::cout.flush();
    std::cerr.flush();
}

void Logger::shutdown() {
    if (isShutdown_.exchange(true)) {
        return;  // Already shut down
    }
    
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

void Logger::writeToConsole(const std::string& message) {
    // Output to appropriate stream based on log level
    std::cout << message << std::endl;
    
    // Also output to debugger on Windows
    #ifdef _WIN32
    OutputDebugStringA(message.c_str());
    OutputDebugStringA("\n");
    #endif
}

void Logger::writeToFile(const std::string& message) {
    if (!fileStream_.is_open()) {
        return;
    }
    
    fileStream_ << message << std::endl;
    
    // Check file size and rotate if necessary
    if (config_.rotateOnMaxSize) {
        auto currentPos = fileStream_.tellp();
        if (currentPos > 0 && static_cast<size_t>(currentPos) >= config_.maxFileSize) {
            rotateLogFile();
        }
    }
}

void Logger::rotateLogFile() {
    if (!fileStream_.is_open()) {
        return;
    }
    
    fileStream_.close();
    
    // Rename old log file
    std::string oldPath = config_.logFilePath + ".old";
    std::remove(oldPath.c_str());  // Remove previous .old file
    std::rename(config_.logFilePath.c_str(), oldPath.c_str());
    
    // Open new log file
    fileStream_.open(config_.logFilePath, std::ios::out | std::ios::trunc);
    if (fileStream_.is_open()) {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        fileStream_ << "========================================\n";
        fileStream_ << "VoiVoi Analyzer Log (Rotated) - " << std::ctime(&time);
        fileStream_ << "========================================\n";
    }
}

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
    
    // Source location
    if (config_.includeSourceLocation && file && function) {
        // Extract filename from full path
        const char* filename = file;
        const char* slash = strrchr(file, '/');
        const char* backslash = strrchr(file, '\\');
        
        if (slash && (!backslash || slash > backslash)) {
            filename = slash + 1;
        } else if (backslash) {
            filename = backslash + 1;
        }
        
        oss << filename << ":" << line << " (" << function << ") - ";
    }
    
    // Message
    oss << message;
    
    return oss.str();
}

// ScopedTimer implementation

ScopedTimer::ScopedTimer(const char* name, LogLevel level)
    : name_(name), level_(level), start_(std::chrono::high_resolution_clock::now()) {
}

ScopedTimer::~ScopedTimer() {
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start_);
    
    std::ostringstream oss;
    oss << "[TIMER] " << name_ << " took " << duration.count() << " µs";
    
    Logger::getInstance().log(level_, "", 0, "", oss.str());
}

// LogStream implementation

LogStream::LogStream(LogLevel level, const char* file, int line, const char* function)
    : level_(level), file_(file), line_(line), function_(function) {
}

LogStream::~LogStream() {
    Logger::getInstance().log(level_, file_, line_, function_, stream_.str());
}

} // namespace yvc
