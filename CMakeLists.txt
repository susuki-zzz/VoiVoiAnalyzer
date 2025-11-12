cmake_minimum_required(VERSION 3.20)

project(VoiVoiAnalyzer VERSION 0.1.0 LANGUAGES CXX)

# C++20 requirement
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Options
option(BUILD_YVC_APP "Build GUI application (yvc_app)" ON)
option(BUILD_YVC_OFFLINE "Build offline analysis tool (yvc_offline)" ON)
option(USE_EIGEN "Use Eigen for matrix operations (optional)" OFF)

# JUCE settings for OSS GPL
set(JUCE_BUILD_EXTRAS OFF CACHE BOOL "Build JUCE extras" FORCE)
set(JUCE_BUILD_EXAMPLES OFF CACHE BOOL "Build JUCE examples" FORCE)

# Add cmake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Third-party dependencies
add_subdirectory(third_party)

# Core library (MIT License)
add_subdirectory(yvc_core)

# GUI Application (GPLv3 License - uses JUCE)
if(BUILD_YVC_APP)
    add_subdirectory(yvc_app)
endif()

# Offline analysis tool
if(BUILD_YVC_OFFLINE)
    add_subdirectory(yvc_offline)
endif()

# Enable testing
enable_testing()
