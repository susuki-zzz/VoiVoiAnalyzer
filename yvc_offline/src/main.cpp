// VoiVoi Offline Analysis Tool - Main
// License: GPLv3

#include "FileProcessor.h"
#include <yvc_core/Logger.h>
#include <iostream>
#include <string>

/// <summary>
/// Prints CLI usage information for the offline analyzer.
/// </summary>
void printUsage() {
    std::cout << "VoiVoi Offline Analyzer v0.1.0" << std::endl;
    std::cout << "Usage: yvc_offline [options] <input_file> <output_file>" << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -m, --mode <mode>    Analysis mode: light, standard, diagnostic (default: diagnostic)" << std::endl;
    std::cout << "  -h, --help           Show this help message" << std::endl;
    std::cout << std::endl;
    std::cout << "Description:" << std::endl;
    std::cout << "  Processes audio files (up to 3 hours) and generates detailed voice analysis." << std::endl;
    std::cout << "  Output is a CSV file with timestamped analysis results." << std::endl;
    std::cout << std::endl;
    std::cout << "License: GPLv3" << std::endl;
}

/// <summary>
/// Program entry point for the offline analyzer CLI.
/// </summary>
int main(int argc, char* argv[]) {
    // Initialize logger for offline tool
    auto& logger = yvc::Logger::getInstance();
    yvc::LoggerConfig logConfig;
    
    #ifdef NDEBUG
    // Release build - minimal logging to console
    logConfig.minLevel = yvc::LogLevel::LOGLV_INFO;
    logConfig.enableFile = false;
    logConfig.enableConsole = true;
    logConfig.includeTimestamp = false;
    logConfig.includeThreadId = false;
    logConfig.includeSourceLocation = false;
    #else
    // Debug build - verbose logging
    logConfig.minLevel = yvc::LogLevel::LOGLV_DEBUG;
    logConfig.enableFile = true;
    logConfig.enableConsole = true;
    logConfig.logFilePath = "voivoi_offline_debug.log";
    logConfig.maxFileSize = 5 * 1024 * 1024;  // 5MB
    logConfig.includeTimestamp = true;
    logConfig.includeThreadId = false;
    logConfig.includeSourceLocation = true;
    #endif
    
    logger.configure(logConfig);
    
    if (argc < 3) {
        printUsage();
        return 1;
    }
    
    std::string input_file;
    std::string output_file;
    yvc::PerformanceMode mode = yvc::PerformanceMode::Mode_Diagnostic;
    
    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "-h" || arg == "--help") {
            printUsage();
            return 0;
        } else if (arg == "-m" || arg == "--mode") {
            if (i + 1 < argc) {
                std::string mode_str = argv[++i];
                if (mode_str == "light") {
                    mode = yvc::PerformanceMode::Mode_Light;
                } else if (mode_str == "standard") {
                    mode = yvc::PerformanceMode::Mode_Standard;
                } else if (mode_str == "diagnostic") {
                    mode = yvc::PerformanceMode::Mode_Diagnostic;
                } else {
                    LOG_ERRORF("Unknown mode: %s", mode_str.c_str());
                    return 1;
                }
            }
        } else if (input_file.empty()) {
            input_file = arg;
        } else if (output_file.empty()) {
            output_file = arg;
        }
    }
    
    if (input_file.empty() || output_file.empty()) {
        LOG_ERROR("Error: Input and output files must be specified");
        printUsage();
        return 1;
    }
    
    // Process the file
    yvc::FileProcessor processor;
    processor.setMode(mode);
    
    if (!processor.processFile(input_file, output_file)) {
        LOG_ERROR("Error: Failed to process file");
        logger.shutdown();
        return 1;
    }
    
    LOG_INFO("Analysis complete!");
    logger.shutdown();
    return 0;
}
