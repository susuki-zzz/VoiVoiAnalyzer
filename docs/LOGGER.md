# VoiVoi Analyzer - Logger Documentation

## Overview

The VoiVoi Logger is a thread-safe, low-overhead logging system designed for real-time audio applications. It provides comprehensive debugging capabilities while respecting privacy constraints and performance requirements.

## Key Features

### 1. **Thread-Safe**
- Multiple threads can log simultaneously without corruption
- Lock-based synchronization with minimal contention
- Async-safe for real-time audio threads (with proper configuration)

### 2. **Performance Optimized**
- Minimal overhead when logging is disabled
- Compile-time log level filtering
- Optional buffering for file output
- Automatic file rotation

### 3. **Privacy-First**
- **NEVER logs audio data** (enforced in production builds)
- Only logs metrics and metadata
- Configurable privacy settings
- Compliance with VoiVoi's privacy architecture

### 4. **Flexible Configuration**
- Multiple log levels (TRACE, DEBUG, INFO, WARN, ERROR, FATAL)
- Console and file output
- Customizable format (timestamp, thread ID, source location)
- Runtime configuration changes

### 5. **Developer-Friendly**
- Macro-based API for easy usage
- Stream-style logging support
- Scoped timers for performance profiling
- Formatted string support (printf-style)

## Quick Start

### Basic Usage

```cpp
#include "yvc_core/Logger.h"

// Simple logging
LOG_INFO("Application started");
LOG_DEBUG("Debug information");
LOG_WARN("Warning message");
LOG_ERROR("Error occurred");

// Formatted logging
int value = 42;
LOG_INFOF("The answer is %d", value);

// Stream-style logging
LOG_STREAM(LogLevel::INFO) << "Count: " << count << ", Name: " << name;
```

### Configuration

```cpp
#include "yvc_core/Logger.h"

auto& logger = Logger::getInstance();

LoggerConfig config;
config.minLevel = LogLevel::DEBUG;
config.enableConsole = true;
config.enableFile = true;
config.logFilePath = "voivoi_debug.log";
config.maxFileSize = 10 * 1024 * 1024;  // 10MB
config.rotateOnMaxSize = true;

logger.configure(config);
```

## Log Levels

| Level | Severity | Use Case |
|-------|----------|----------|
| TRACE | 0 | Very detailed debugging (e.g., per-sample processing) |
| DEBUG | 1 | Development debugging information |
| INFO | 2 | General runtime information |
| WARN | 3 | Warning conditions that don't prevent operation |
| ERROR | 4 | Error conditions that affect functionality |
| FATAL | 5 | Critical errors that may cause application crash |
| OFF | 6 | Disable all logging |

### Log Level Guidelines

- **TRACE**: Use for very detailed debugging (disabled in Release builds)
  ```cpp
  LOG_TRACEF("Processing sample %d: value=%.6f", i, sample);
  ```

- **DEBUG**: Use for development debugging
  ```cpp
  LOG_DEBUGF("F0 detected: %.2f Hz (confidence: %.3f)", f0, confidence);
  ```

- **INFO**: Use for important runtime information
  ```cpp
  LOG_INFO("Audio device initialized successfully");
  ```

- **WARN**: Use for unexpected but recoverable situations
  ```cpp
  LOG_WARN("Buffer underrun detected, recovering...");
  ```

- **ERROR**: Use for errors that affect functionality
  ```cpp
  LOG_ERROR("Failed to open audio file");
  ```

- **FATAL**: Use for critical failures
  ```cpp
  LOG_FATAL("Memory allocation failed - cannot continue");
  ```

## Performance Profiling

### Scoped Timers

Measure execution time of code blocks:

```cpp
void processAudio() {
    LOG_SCOPE_TIMER("processAudio");
    
    // Your code here
    // Timer automatically logs elapsed time when scope exits
}

// For very detailed timing (TRACE level)
void innerLoop() {
    LOG_SCOPE_TIMER_TRACE("innerLoop");
    // ...
}
```

Output example:
```
[TIMER] processAudio took 1234 µs
```

### Manual Timing

```cpp
auto start = std::chrono::high_resolution_clock::now();
// ... code to measure ...
auto end = std::chrono::high_resolution_clock::now();
auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
LOG_DEBUGF("Operation took %lld µs", duration.count());
```

## Privacy Guidelines

### ❌ NEVER Log Audio Data

```cpp
// DON'T DO THIS!
float audioBuffer[512];
LOG_DEBUG(audioBuffer);  // WRONG! Never log raw audio!
```

### ✅ Log Metrics and Metadata

```cpp
// CORRECT - Log metrics only
float f0 = 220.5f;
float rms = -12.3f;
LOG_INFOF("Metrics - F0: %.1f Hz, RMS: %.1f dB", f0, rms);

// CORRECT - Log metadata
LOG_DEBUGF("Processing buffer: %zu samples at %.1f kHz", 
           bufferSize, sampleRate / 1000.0);
```

### Configuration Check

The logger has built-in privacy protection:

```cpp
#ifdef NDEBUG  // Production/Release build
config.allowAudioDataLogging = false;  // Forced to false
#endif
```

## Advanced Usage

### Multi-threaded Logging

The logger is thread-safe by default:

```cpp
void workerThread(int id) {
    LOG_INFOF("Worker %d started", id);
    // Thread-safe logging
}

std::thread t1(workerThread, 1);
std::thread t2(workerThread, 2);
```

### Dynamic Log Level Changes

```cpp
// Start with INFO level
logger.setMinLevel(LogLevel::INFO);

// Switch to DEBUG during problem investigation
logger.setMinLevel(LogLevel::DEBUG);

// Disable logging for performance-critical sections
logger.setMinLevel(LogLevel::OFF);
```

### File Rotation

```cpp
LoggerConfig config;
config.maxFileSize = 5 * 1024 * 1024;  // 5MB
config.rotateOnMaxSize = true;

// When file reaches 5MB, it's renamed to .old and new file is created
```

### Flushing

```cpp
// Ensure all logs are written (useful before crash or exit)
logger.flush();
```

### Shutdown

```cpp
// Proper shutdown (writes footer and closes file)
logger.shutdown();
```

## Integration with VoiVoi Components

### In Core Library (yvc_core)

```cpp
#include "yvc_core/Logger.h"

class MyAnalyzer {
public:
    MyAnalyzer() {
        LOG_INFO("MyAnalyzer initialized");
    }
    
    void analyze(const float* samples, size_t n) {
        LOG_SCOPE_TIMER("MyAnalyzer::analyze");
        
        // Analysis code
        LOG_DEBUGF("Processed %zu samples", n);
    }
};
```

### In GUI Application (yvc_app)

```cpp
#include "yvc_core/Logger.h"

class MainComponent {
public:
    void buttonClicked() {
        LOG_INFO("Start button clicked");
        
        try {
            startRecording();
        } catch (const std::exception& e) {
            LOG_ERRORF("Recording failed: %s", e.what());
        }
    }
};
```

### In Offline Tool (yvc_offline)

```cpp
#include "yvc_core/Logger.h"

int main(int argc, char* argv[]) {
    // Configure for file output
    LoggerConfig config;
    config.enableFile = true;
    config.logFilePath = "offline_analysis.log";
    Logger::getInstance().configure(config);
    
    LOG_INFO("Offline analysis started");
    
    // ... processing ...
    
    Logger::getInstance().shutdown();
    return 0;
}
```

## Performance Considerations

### Real-Time Audio Threads

For real-time audio threads, minimize logging:

```cpp
void audioCallback(float* buffer, size_t frames) {
    // DON'T log in audio callback!
    // LOG_DEBUG("Processing audio");  // Too slow!
    
    // Instead, use lock-free flags or counters
    static std::atomic<int> xrunCount{0};
    if (detectXRun()) {
        xrunCount++;
    }
}

void statusThread() {
    // Log from non-realtime thread
    if (xrunCount > 0) {
        LOG_WARNF("Audio xruns detected: %d", xrunCount.load());
        xrunCount = 0;
    }
}
```

### Conditional Compilation

For maximum performance, disable logging in Release builds:

```cpp
#ifdef NDEBUG
    // Release build - minimal logging
    logger.setMinLevel(LogLevel::WARN);
#else
    // Debug build - verbose logging
    logger.setMinLevel(LogLevel::DEBUG);
#endif
```

## Example Output

### Console Output

```
2024-11-12 22:30:15.123 [T:12345] [INFO ] Main.cpp:42 (main) - Application started
2024-11-12 22:30:15.234 [T:12345] [DEBUG] F0Detector.cpp:15 (F0Detector) - F0Detector initialized: SR=48000, buffer_size=1200
2024-11-12 22:30:15.345 [T:12346] [INFO ] AudioEngine.cpp:78 (start) - Audio engine started
2024-11-12 22:30:15.456 [T:12346] [TIMER] processAudio took 1234 µs
2024-11-12 22:30:15.567 [T:12345] [WARN ] AudioEngine.cpp:123 (check) - Buffer underrun detected
```

### File Output (voivoi_debug.log)

```
========================================
VoiVoi Analyzer Log - Tue Nov 12 22:30:15 2024
========================================
2024-11-12 22:30:15.123 [T:12345] [INFO ] Main.cpp:42 (main) - Application started
2024-11-12 22:30:15.234 [T:12345] [DEBUG] F0Detector.cpp:15 (F0Detector) - F0Detector initialized: SR=48000, buffer_size=1200
...
========================================
Logger shutdown - Tue Nov 12 22:35:20 2024
========================================
```

## Troubleshooting

### Log File Not Created

```cpp
// Check if file path is writable
LoggerConfig config;
config.logFilePath = "C:/Temp/voivoi.log";  // Use absolute path
logger.configure(config);
```

### Too Much Output

```cpp
// Increase minimum log level
logger.setMinLevel(LogLevel::WARN);  // Only warnings and errors
```

### Missing Logs

```cpp
// Ensure logger is flushed before exit
logger.flush();
logger.shutdown();
```

### Performance Impact

```cpp
// Disable file logging in performance-critical situations
config.enableFile = false;
config.enableConsole = true;  // Console is faster
```

## Best Practices

1. **Use appropriate log levels**
   - Don't use DEBUG for production
   - Reserve TRACE for deep debugging only

2. **Log meaningful information**
   - Include context (function name, values)
   - Avoid cryptic messages

3. **Respect privacy**
   - NEVER log audio data
   - Only log aggregated metrics

4. **Performance awareness**
   - Don't log in real-time threads
   - Use scoped timers for profiling

5. **Clean shutdown**
   - Always call `logger.shutdown()` before exit
   - Flush logs after important operations

## Future Enhancements

- [ ] Asynchronous logging queue for zero-blocking
- [ ] Structured logging (JSON format)
- [ ] Remote logging support (optional, metrics only)
- [ ] Log level per-component configuration
- [ ] Colored console output
- [ ] Log filtering by component/tag

---

For questions or issues, please refer to the main VoiVoi Analyzer documentation or open an issue on GitHub.
