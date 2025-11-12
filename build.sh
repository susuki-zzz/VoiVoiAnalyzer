#!/bin/bash
set -e

echo "=================================================="
echo "VoiVoi Analyzer Build Script (Linux/macOS)"
echo "=================================================="

# Default values
BUILD_TYPE="x64-debug"
CLEAN_BUILD=false
RUN_TESTS=false

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --release)
            BUILD_TYPE="x64-release"
            shift
            ;;
        --debug)
            BUILD_TYPE="x64-debug"
            shift
            ;;
        --relwithdebinfo)
            BUILD_TYPE="x64-relwithdebinfo"
            shift
            ;;
        --core-only)
            BUILD_TYPE="x64-core-only"
            shift
            ;;
        --clean)
            CLEAN_BUILD=true
            shift
            ;;
        --test)
            RUN_TESTS=true
            shift
            ;;
        --help)
            echo "Usage: ./build.sh [options]"
            echo ""
            echo "Options:"
            echo "  --debug          Build Debug configuration (default)"
            echo "  --release        Build Release configuration"
            echo "  --relwithdebinfo Build RelWithDebInfo configuration"
            echo "  --core-only      Build only yvc_core library"
            echo "  --clean          Clean build directory before building"
            echo "  --test           Run tests after building"
            echo "  --help           Show this help message"
            echo ""
            echo "Examples:"
            echo "  ./build.sh --release --test"
            echo "  ./build.sh --core-only --clean"
            echo "  ./build.sh --debug"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

# Check if CMake is available
if ! command -v cmake &> /dev/null; then
    echo "ERROR: CMake not found in PATH"
    echo "Please install CMake 3.20+ and ensure it's in your PATH"
    exit 1
fi

echo "Building with preset: $BUILD_TYPE"

# Clean build if requested
if [ "$CLEAN_BUILD" = true ]; then
    echo "Cleaning build directory..."
    rm -rf "out/build/$BUILD_TYPE"
fi

# Configure
echo "Configuring project..."
cmake --preset=$BUILD_TYPE

# Build
echo "Building project..."
cmake --build --preset=$BUILD_TYPE

# Run tests if requested
if [ "$RUN_TESTS" = true ]; then
    echo "Running tests..."
    ctest --preset=core-tests || echo "WARNING: Some tests failed"
fi

echo ""
echo "=================================================="
echo "Build completed successfully!"
echo "Build type: $BUILD_TYPE"
echo "Output directory: out/build/$BUILD_TYPE"
echo "=================================================="

if [ "$BUILD_TYPE" = "x64-release" ]; then
    echo ""
    echo "Built executables:"
    [ -f "out/build/$BUILD_TYPE/yvc_offline/yvc_offline" ] && echo "- yvc_offline (Offline analysis tool)"
    [ -f "out/build/$BUILD_TYPE/yvc_app/yvc_app" ] && echo "- yvc_app (GUI application)"
fi
