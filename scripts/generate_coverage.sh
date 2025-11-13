#!/bin/bash
# Coverage report generation script for VoiVoi Analyzer
# Usage: ./scripts/generate_coverage.sh

set -e

BUILD_DIR="${1:-build/coverage}"
COVERAGE_DIR="${BUILD_DIR}/coverage_report"

echo "=== VoiVoi Analyzer Coverage Report Generator ==="
echo "Build directory: ${BUILD_DIR}"
echo "Coverage report: ${COVERAGE_DIR}"

# Check if lcov is installed
if ! command -v lcov &> /dev/null; then
    echo "Error: lcov is not installed"
    echo "Install with: sudo apt-get install lcov (Ubuntu/Debian)"
    exit 1
fi

# Configure with coverage enabled
echo "Configuring project with coverage..."
cmake -S . -B "${BUILD_DIR}" \
    -DCMAKE_BUILD_TYPE=Debug \
    -DYVC_ENABLE_COVERAGE=ON \
    -DBUILD_TESTING=ON \
    -GNinja

# Build
echo "Building project..."
cmake --build "${BUILD_DIR}"

# Run tests
echo "Running tests..."
cd "${BUILD_DIR}"
ctest --output-on-failure

# Generate coverage data
echo "Generating coverage data..."
lcov --capture --directory . --output-file coverage.info

# Filter out external dependencies and tests
lcov --remove coverage.info \
    '/usr/*' \
    '*/third_party/*' \
    '*/tests/*' \
    '*/build/*' \
    --output-file coverage_filtered.info

# Generate HTML report
echo "Generating HTML report..."
genhtml coverage_filtered.info --output-directory "${COVERAGE_DIR}"

echo "=== Coverage report generated ==="
echo "Open ${COVERAGE_DIR}/index.html in your browser"
