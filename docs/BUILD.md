# Building VoiVoiAnalyzer

This document provides detailed build instructions for VoiVoiAnalyzer on Windows.

## Prerequisites

### Required Tools

1. **CMake** (3.20 or later)
   - Download from: https://cmake.org/download/
   - Make sure to add CMake to system PATH during installation

2. **C++ Compiler** (one of the following):
   - **Visual Studio 2019 or later** (Recommended for Windows)
     - Download: https://visualstudio.microsoft.com/
     - Install "Desktop development with C++" workload
     - Make sure C++20 support is included
   
   - **MinGW-w64** with GCC 10+ (Alternative)
   - **Clang 12+** (Alternative)

3. **Git** (for cloning repository)
   - Download from: https://git-scm.com/

### Optional Dependencies

1. **JUCE Framework** (for GUI application)
   - Will be fetched automatically or can be added as git submodule
   - Not required for building core library

2. **Eigen** (for advanced matrix operations)
   - Only needed if building with `-DUSE_EIGEN=ON`
   - Can improve performance for certain operations

## Build Options

### CMake Configuration Options

The following CMake options are available:

| Option | Default | Description |
|--------|---------|-------------|
| `BUILD_TESTING` | `ON` | Build unit tests |
| `BUILD_YVC_APP` | `ON` | Build GUI application |
| `BUILD_YVC_OFFLINE` | `ON` | Build offline analysis tool |
| `USE_EIGEN` | `OFF` | Use Eigen for matrix operations |
| `YVC_ENABLE_FILE_LOG` | `OFF` | Enable file logging by default |
| `YVC_ENABLE_COVERAGE` | `OFF` | Enable code coverage reporting (GCC/Clang only) |
| `YVC_ENABLE_SANITIZERS` | `OFF` | Enable AddressSanitizer and UBSan (Debug only, GCC/Clang) |

### Quick Configuration Examples

```bash
# Build only core library (no GUI, no offline tool)
cmake .. -DBUILD_YVC_APP=OFF -DBUILD_YVC_OFFLINE=OFF

# Build with Eigen support
cmake .. -DUSE_EIGEN=ON

# Specify build type (Debug or Release)
cmake .. -DCMAKE_BUILD_TYPE=Release

# Specify generator (for example, use Ninja)
cmake .. -G Ninja

# Enable coverage reporting (Linux/macOS with GCC/Clang)
cmake .. -DCMAKE_BUILD_TYPE=Debug -DYVC_ENABLE_COVERAGE=ON

# Enable sanitizers for debugging (Linux/macOS)
cmake .. -DCMAKE_BUILD_TYPE=Debug -DYVC_ENABLE_SANITIZERS=ON
```

## Quick Start

### Using Visual Studio (Windows)

```bash
# 1. Clone repository
git clone https://github.com/susuki-zzz/VoiVoiAnalyzer.git
cd VoiVoiAnalyzer

# 2. Create build directory
mkdir build
cd build

# 3. Generate Visual Studio solution
cmake ..

# 4. Build using CMake
cmake --build . --config Release

# Or open VoiVoiAnalyzer.sln in Visual Studio and build from IDE
```

### Using Command Line (Windows with Ninja)

```bash
# 1. Clone repository
git clone https://github.com/susuki-zzz/VoiVoiAnalyzer.git
cd VoiVoiAnalyzer

# 2. Create build directory and configure
cmake -S . -B build -GNinja -DCMAKE_BUILD_TYPE=Release

# 3. Build
cmake --build build

# 4. Run tests
cd build
ctest --output-on-failure
```

### Debug Build with Tests

```bash
cmake -S . -B build/debug -GNinja -DCMAKE_BUILD_TYPE=Debug
cmake --build build/debug
cd build/debug
ctest --output-on-failure
```

## Advanced Builds

### Build with Coverage (Linux/macOS)

```bash
# Configure with coverage enabled
cmake -S . -B build/coverage -GNinja \
    -DCMAKE_BUILD_TYPE=Debug \
    -DYVC_ENABLE_COVERAGE=ON

# Build and run tests
cmake --build build/coverage
cd build/coverage
ctest

# Generate coverage report (requires lcov)
lcov --capture --directory . --output-file coverage.info
lcov --remove coverage.info '/usr/*' '*/third_party/*' '*/tests/*' --output-file coverage_filtered.info
genhtml coverage_filtered.info --output-directory coverage_report

# Open coverage_report/index.html in browser
```

### Build with Sanitizers (Debug, Linux/macOS)

```bash
cmake -S . -B build/sanitized -GNinja \
    -DCMAKE_BUILD_TYPE=Debug \
    -DYVC_ENABLE_SANITIZERS=ON

cmake --build build/sanitized
cd build/sanitized
ctest  # Will detect memory leaks and undefined behavior
```

### Build Only Core Library

```bash
cmake -S . -B build/core -GNinja \
    -DBUILD_YVC_APP=OFF \
    -DBUILD_YVC_OFFLINE=OFF

cmake --build build/core
```

## Build Configurations

- **Debug**: Includes debug symbols, no optimization, useful for development
- **Release**: Optimized build for production use (includes LTO)
- **RelWithDebInfo**: Optimized with debug info
- **MinSizeRel**: Optimized for minimal size

## Build Targets

After configuration, you can build specific targets:

```bash
# Build everything
cmake --build . --config Release

# Build only core library
cmake --build . --config Release --target yvc_core

# Build only offline tool
cmake --build . --config Release --target yvc_offline

# Build only GUI app (when JUCE is available)
cmake --build . --config Release --target yvc_app
```

## Installation and Using in Other Projects

### Installing yvc_core

```bash
# Install to custom prefix
cmake --install build/release --prefix /path/to/install

# Or use default system prefix (may require admin rights)
cmake --install build/release
```

### Using yvc_core in Your CMake Project

After installation, you can use yvc_core in your CMake project:

```cmake
find_package(yvc_core REQUIRED)

add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE yvc::yvc_core)
```

Example usage:

```cpp
#include <yvc_core/Types.h>
#include <yvc_core/F0Detector.h>
#include <yvc_core/Logger.h>

int main() {
    // Configure logger
    yvc::LoggerConfig config;
    config.minLevel = yvc::LogLevel::DEBUG;
    yvc::Logger::getInstance().configure(config);
    
    LOG_INFO("Starting application");
    
    // Use F0 detector, etc.
    return 0;
}
```

## Testing the Build

### Run All Tests

```bash
cd build
ctest --output-on-failure
```

### Run Specific Test

```bash
cd build
./yvc_core/yvc_core_tests
./yvc_offline/yvc_offline_tests  # if built
```

### Test Offline Tool

```bash
# Navigate to build output
cd build/yvc_offline

# Run offline tool
./yvc_offline --help  # Linux/macOS
yvc_offline.exe --help  # Windows
```

## Troubleshooting

### CMake Can't Find Compiler

**Error**: "No CMAKE_CXX_COMPILER could be found"

**Solution**: 
- Make sure Visual Studio is installed with C++ support
- Run CMake from "Developer Command Prompt for VS"
- Or specify compiler explicitly: `cmake .. -DCMAKE_CXX_COMPILER="path/to/cl.exe"`

### C++20 Features Not Available

**Error**: "This project requires a C++20 compatible compiler"

**Solution**:
- Update to Visual Studio 2019 16.11 or later
- Or use GCC 10+ / Clang 12+
- Check CMake output to verify C++20 is enabled

### Runtime Library Mismatch (Windows)

**Error**: "LNK2038: mismatch detected for 'RuntimeLibrary'"

**Solution**:
- All targets use `/MD` (release) or `/MDd` (debug) by default
- This is configured automatically via `CMAKE_MSVC_RUNTIME_LIBRARY`
- If integrating with external libraries, ensure they also use dynamic runtime

### Coverage Not Working

**Error**: Coverage reports are empty or not generated

**Solution**:
- Ensure you're using GCC or Clang compiler
- Install lcov: `sudo apt-get install lcov` (Ubuntu) or `brew install lcov` (macOS)
- Make sure tests were run: `ctest` before generating report
- Check that `YVC_ENABLE_COVERAGE=ON` was set during configuration

### KissFFT Build Errors

**Error**: Issues building KissFFT

**Solution**:
- Ensure all KissFFT files were downloaded correctly
- Check that `third_party/kissfft/` contains all source files
- Try cleaning build directory and rebuilding

### Missing JUCE Framework

**Note**: GUI application requires JUCE framework

**Solution**:
- JUCE will be fetched automatically if internet connection is available
- Alternatively, manually clone: `git clone --depth 1 --branch 7.0.12 https://github.com/juce-framework/JUCE.git third_party/JUCE`
- Or build without GUI: `-DBUILD_YVC_APP=OFF`

## Platform-Specific Notes

### Windows
- Primary development platform
- Best tested with Visual Studio 2019/2022 or Ninja
- MSVC compiler recommended for optimal Windows integration

### Linux (Planned)
- GCC 10+ or Clang 12+ required
- Install dependencies: `sudo apt-get install build-essential cmake ninja-build`

### macOS (Planned)
- Xcode Command Line Tools required
- Or use Homebrew: `brew install cmake ninja`

## Performance Optimization

For optimal performance:

1. Always use Release build configuration
2. Link-Time Optimization (LTO) is enabled automatically for Release builds
3. Consider using Eigen for matrix operations: `-DUSE_EIGEN=ON`
4. Profile with sanitizers in Debug: `-DYVC_ENABLE_SANITIZERS=ON`

## Continuous Integration

### GitHub Actions Example

```yaml
name: Build and Test

on: [push, pull_request]

jobs:
  build:
    runs-on: ${{ matrix.os }}
    strategy:
      matrix:
        os: [ubuntu-latest, windows-latest, macos-latest]
        build_type: [Debug, Release]
        
    steps:
    - uses: actions/checkout@v2
    
    - name: Configure
      run: cmake -S . -B build -DCMAKE_BUILD_TYPE=${{ matrix.build_type }} -GNinja
      
    - name: Build
      run: cmake --build build
      
    - name: Test
      run: cd build && ctest --output-on-failure
