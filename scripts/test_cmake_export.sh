#!/bin/bash
# Test script to verify CMake export/install works correctly

set -e

echo "=== Testing yvc_core CMake Export/Install ==="

# Create temporary directories
BUILD_DIR=$(mktemp -d)
INSTALL_DIR=$(mktemp -d)
TEST_PROJECT_DIR=$(mktemp -d)

echo "Build directory: ${BUILD_DIR}"
echo "Install directory: ${INSTALL_DIR}"
echo "Test project directory: ${TEST_PROJECT_DIR}"

# Configure and build
echo ""
echo "Step 1: Configure yvc_core..."
cmake -S . -B "${BUILD_DIR}" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}" \
    -DBUILD_TESTING=OFF \
    -DBUILD_YVC_APP=OFF \
    -DBUILD_YVC_OFFLINE=OFF \
    -GNinja

echo ""
echo "Step 2: Build yvc_core..."
cmake --build "${BUILD_DIR}"

echo ""
echo "Step 3: Install yvc_core..."
cmake --install "${BUILD_DIR}"

echo ""
echo "Step 4: Verify installation..."
if [ ! -f "${INSTALL_DIR}/lib/cmake/yvc_core/yvc_coreConfig.cmake" ]; then
    echo "ERROR: yvc_coreConfig.cmake not found!"
    exit 1
fi

if [ ! -f "${INSTALL_DIR}/lib/cmake/yvc_core/yvc_coreTargets.cmake" ]; then
    echo "ERROR: yvc_coreTargets.cmake not found!"
    exit 1
fi

echo "✓ Config files found"

# Create test project
echo ""
echo "Step 5: Create test project..."
cat > "${TEST_PROJECT_DIR}/CMakeLists.txt" << 'EOF'
cmake_minimum_required(VERSION 3.20)
project(yvc_core_test CXX)

find_package(yvc_core REQUIRED)

add_executable(test_app main.cpp)
target_link_libraries(test_app PRIVATE yvc::yvc_core)
target_compile_features(test_app PRIVATE cxx_std_20)
EOF

cat > "${TEST_PROJECT_DIR}/main.cpp" << 'EOF'
#include <yvc_core/Logger.h>
#include <iostream>

int main() {
    yvc::LoggerConfig config;
    config.minLevel = yvc::LogLevel::INFO;
    config.enableConsole = true;
    config.enableFile = false;
    
    yvc::Logger::getInstance().configure(config);
    LOG_INFO("Test application using yvc_core via find_package");
    
    std::cout << "Success: yvc_core imported correctly!" << std::endl;
    return 0;
}
EOF

echo ""
echo "Step 6: Configure test project..."
cmake -S "${TEST_PROJECT_DIR}" -B "${TEST_PROJECT_DIR}/build" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH="${INSTALL_DIR}" \
    -GNinja

echo ""
echo "Step 7: Build test project..."
cmake --build "${TEST_PROJECT_DIR}/build"

echo ""
echo "Step 8: Run test application..."
"${TEST_PROJECT_DIR}/build/test_app"

echo ""
echo "=== All tests passed! ==="
echo ""
echo "Installed files:"
find "${INSTALL_DIR}" -type f | head -20

# Cleanup
echo ""
echo "Cleaning up temporary directories..."
rm -rf "${BUILD_DIR}" "${INSTALL_DIR}" "${TEST_PROJECT_DIR}"

echo "✓ Export/Install test completed successfully"
