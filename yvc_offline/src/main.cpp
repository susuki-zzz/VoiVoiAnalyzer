// VoiVoi Offline Analysis Tool - Main
// License: GPLv3

#include "FileProcessor.h"
#include <iostream>
#include <string>

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

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printUsage();
        return 1;
    }
    
    std::string input_file;
    std::string output_file;
    yvc::PerformanceMode mode = yvc::PerformanceMode::Diagnostic;
    
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
                    mode = yvc::PerformanceMode::Light;
                } else if (mode_str == "standard") {
                    mode = yvc::PerformanceMode::Standard;
                } else if (mode_str == "diagnostic") {
                    mode = yvc::PerformanceMode::Diagnostic;
                } else {
                    std::cerr << "Unknown mode: " << mode_str << std::endl;
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
        std::cerr << "Error: Input and output files must be specified" << std::endl;
        printUsage();
        return 1;
    }
    
    // Process the file
    yvc::FileProcessor processor;
    processor.setMode(mode);
    
    if (!processor.processFile(input_file, output_file)) {
        std::cerr << "Error: Failed to process file" << std::endl;
        return 1;
    }
    
    std::cout << "Analysis complete!" << std::endl;
    return 0;
}
