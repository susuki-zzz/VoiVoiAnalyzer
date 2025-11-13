// VoiVoi Core Library - Logger Usage Examples and Tests
// License: MIT

#include "yvc_core/Logger.h"
#include <iostream>
#include <thread>
#include <chrono>

using namespace yvc;

// Example 1: Basic logging
void example_basic_logging() {
    std::cout << "\n=== Example 1: Basic Logging ===\n";
    
    LOG_INFO("Application started");
    LOG_DEBUG("Debug information");
    LOG_WARN("This is a warning");
    LOG_ERROR("An error occurred");
    
    // Formatted logging
    int value = 42;
    LOG_INFOF("The answer is %d", value);
    LOG_DEBUGF("Processing %d items in %s mode", 100, "fast");
}

// Example 2: Stream-style logging
void example_stream_logging() {
    std::cout << "\n=== Example 2: Stream-Style Logging ===\n";
    
    int count = 10;
    std::string name = "test";
    
    LOG_STREAM(LogLevel::LOGLV_INFO) << "Count: " << count << ", Name: " << name;
    LOG_STREAM(LogLevel::LOGLV_DEBUG) << "Complex object: " << "{ id: 123, value: 456 }";
}

// Example 3: Scoped timing
void slow_function() {
    LOG_SCOPE_TIMER("slow_function");
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

void fast_function() {
    LOG_SCOPE_TIMER_TRACE("fast_function");
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
}

void example_scoped_timing() {
    std::cout << "\n=== Example 3: Scoped Timing ===\n";
    
    slow_function();
    fast_function();
    
    {
        LOG_SCOPE_TIMER("critical_section");
        // Some expensive operation
        for (int i = 0; i < 1000000; ++i) {
            volatile int x = i * i;
            (void)x;
        }
    }
}

// Example 4: Multi-threaded logging
void worker_thread(int id) {
    for (int i = 0; i < 3; ++i) {
        LOG_INFOF("Worker %d: Processing item %d", id, i);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void example_multithreaded() {
    std::cout << "\n=== Example 4: Multi-threaded Logging ===\n";
    
    std::thread t1(worker_thread, 1);
    std::thread t2(worker_thread, 2);
    std::thread t3(worker_thread, 3);
    
    t1.join();
    t2.join();
    t3.join();
}

// Example 5: Different log levels
void example_log_levels() {
    std::cout << "\n=== Example 5: Different Log Levels ===\n";
    
    auto& logger = Logger::getInstance();
    
    // Configure to show all levels
    LoggerConfig config = logger.getConfig();
    config.minLevel = LogLevel::LOGLV_TRACE;
    logger.configure(config);
    
    LOG_TRACE("Trace message - very detailed");
    LOG_DEBUG("Debug message - for development");
    LOG_INFO("Info message - general information");
    LOG_WARN("Warning message - something unexpected");
    LOG_ERROR("Error message - something failed");
    LOG_FATAL("Fatal message - critical failure");
    
    // Change to INFO level
    logger.setMinLevel(LogLevel::LOGLV_INFO);
    LOG_DEBUG("This debug message won't be shown");
    LOG_INFO("This info message will be shown");
}

// Example 6: File logging
void example_file_logging() {
    std::cout << "\n=== Example 6: File Logging ===\n";
    
    auto& logger = Logger::getInstance();
    
    LoggerConfig config;
    config.minLevel = LogLevel::LOGLV_DEBUG;
    config.enableConsole = true;
    config.enableFile = true;
    config.logFilePath = "voivoi_test.log";
    config.maxFileSize = 1024 * 1024;  // 1MB
    config.rotateOnMaxSize = true;
    
    logger.configure(config);
    
    LOG_INFO("Logging to file enabled");
    LOG_DEBUG("This message is written to both console and file");
    
    // Generate some logs
    for (int i = 0; i < 10; ++i) {
        LOG_INFOF("Log entry #%d with some data", i);
    }
    
    logger.flush();
    std::cout << "Check voivoi_test.log for output\n";
}

// Example 7: Performance monitoring
class AudioProcessor {
public:
    void processBuffer(float* buffer, size_t samples) {
        LOG_SCOPE_TIMER("AudioProcessor::processBuffer");
        
        // Simulate processing
        for (size_t i = 0; i < samples; ++i) {
            buffer[i] *= 0.5f;
        }
        
        LOG_DEBUGF("Processed %zu samples", samples);
    }
};

void example_performance_monitoring() {
    std::cout << "\n=== Example 7: Performance Monitoring ===\n";
    
    AudioProcessor processor;
    float buffer[512];
    
    for (int i = 0; i < 5; ++i) {
        processor.processBuffer(buffer, 512);
    }
}

// Privacy-safe logging example
void example_privacy_safe_logging() {
    std::cout << "\n=== Example 8: Privacy-Safe Logging ===\n";
    
    // Simulate audio metrics (OK to log)
    float f0 = 220.5f;
    float rms = -12.3f;
    
    LOG_INFOF("Metrics - F0: %.1f Hz, RMS: %.1f dB", f0, rms);
    
    // Audio data should NEVER be logged
    // float audioSamples[1024]; // DON'T LOG THIS!
    // Instead, log metadata:
    LOG_DEBUG("Processing audio buffer (512 samples, 48kHz)");
}

int main() {
    std::cout << "VoiVoi Logger - Usage Examples\n";
    std::cout << "================================\n";
    
    // Initialize logger with default config
    auto& logger = Logger::getInstance();
    LoggerConfig config;
    config.minLevel = LogLevel::LOGLV_DEBUG;
    config.enableConsole = true;
    config.includeTimestamp = true;
    config.includeThreadId = true;
    config.includeSourceLocation = true;
    logger.configure(config);
    
    // Run examples
    example_basic_logging();
    example_stream_logging();
    example_scoped_timing();
    example_multithreaded();
    example_log_levels();
    example_file_logging();
    example_performance_monitoring();
    example_privacy_safe_logging();
    
    std::cout << "\n=== All examples completed ===\n";
    
    logger.shutdown();
    return 0;
}
