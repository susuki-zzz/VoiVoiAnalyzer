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

### Using Command Line (Windows)

```bash
# 1. Clone repository
git clone https://github.com/susuki-zzz/VoiVoiAnalyzer.git
cd VoiVoiAnalyzer

# 2. Create build directory
mkdir build
cd build

# 3. Configure and build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

## Build Options

### CMake Configuration Options

```bash
# Build only core library (no GUI, no offline tool)
cmake .. -DBUILD_YVC_APP=OFF -DBUILD_YVC_OFFLINE=OFF

# Build with Eigen support
cmake .. -DUSE_EIGEN=ON

# Specify build type (Debug or Release)
cmake .. -DCMAKE_BUILD_TYPE=Release

# Specify generator (for example, use Ninja)
cmake .. -G Ninja
```

### Build Configurations

- **Debug**: Includes debug symbols, no optimization, useful for development
- **Release**: Optimized build for production use
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

## Build Outputs

After successful build, you'll find:

- `build/yvc_core/Release/yvc_core.lib` - Core analysis library
- `build/yvc_offline/Release/yvc_offline.exe` - Offline analysis tool
- `build/yvc_app/Release/yvc_app.exe` - GUI application (when JUCE is available)

## Testing the Build

### Test Offline Tool

```bash
# Navigate to build output
cd build/yvc_offline/Release

# Run offline tool
yvc_offline.exe --help

# Test with sample file (when audio loading is implemented)
yvc_offline.exe input.wav output.csv
```

### Test Core Library

The core library can be tested by integrating it into your own C++ project:

```cpp
#include <yvc_core/Types.h>
#include <yvc_core/F0Detector.h>
#include <yvc_core/LevelAnalyzer.h>

// Your code here
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

### KissFFT Build Errors

**Error**: Issues building KissFFT

**Solution**:
- Ensure all KissFFT files were downloaded correctly
- Check that `third_party/kissfft/` contains all source files
- Try cleaning build directory and rebuilding

### Missing JUCE Framework

**Note**: GUI application build is currently configured as a placeholder

**Solution**:
- The GUI app requires JUCE framework
- For now, build with `-DBUILD_YVC_APP=OFF` to skip GUI
- JUCE integration will be completed in future updates

## Platform-Specific Notes

### Windows
- Primary development platform
- Best tested with Visual Studio 2019/2022
- MSVC compiler recommended for optimal Windows integration

### Future Platforms
- Linux support: Planned
- macOS support: Planned

## Performance Optimization

For optimal performance:

1. Always use Release build configuration
2. Enable compiler optimizations
3. Consider using Eigen for matrix operations: `-DUSE_EIGEN=ON`
4. Use native architecture optimization if available

## Getting Help

If you encounter build issues:

1. Check this document for common solutions
2. Verify all prerequisites are installed
3. Try cleaning build directory: `rm -rf build/`
4. Check CMake version: `cmake --version`
5. Open an issue on GitHub with:
   - CMake version
   - Compiler version
   - Full error message
   - CMake configuration command used

## Advanced Configuration

### Custom Installation Path

```bash
cmake .. -DCMAKE_INSTALL_PREFIX=/path/to/install
cmake --build . --config Release --target install
```

### Cross-Compilation

Cross-compilation setup will be documented when Linux/macOS support is added.

### Using vcpkg

VoiVoiAnalyzer can potentially use vcpkg for dependency management:

```bash
# If using vcpkg
cmake .. -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
```

## Contributing to Build System

When contributing changes to the build system:

1. Test on multiple CMake versions (3.20+)
2. Verify Windows builds work
3. Ensure both Debug and Release configurations work
4. Document any new build options
5. Update this document if adding new requirements
