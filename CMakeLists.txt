cmake_minimum_required(VERSION 3.20)

project(VoiVoiAnalyzer VERSION 0.1.0 LANGUAGES CXX C)

# C++20 requirement
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# C99 for KissFFT
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Use dynamic runtime library for MSVC
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    # Ensure we use /MD or /MDd
    add_compile_options(
        $<$<CONFIG:Debug>:/MDd>
        $<$<CONFIG:Release>:/MD>
        $<$<CONFIG:RelWithDebInfo>:/MD>
        $<$<CONFIG:MinSizeRel>:/MD>
    )
endif()

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

# CPack configuration for packaging (alternative to packagePresets)
include(CPack)
set(CPACK_PACKAGE_NAME "VoiVoiAnalyzer")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "VoiVoi Analyzer - Real-time voice training application")
set(CPACK_PACKAGE_VENDOR "VoiVoi Team")
set(CPACK_PACKAGE_CONTACT "contact@voivoi.dev")

# Set package file name
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-win64")

# Generator-specific settings
set(CPACK_GENERATOR "ZIP;NSIS")
set(CPACK_SOURCE_GENERATOR "ZIP")

# Windows-specific settings
if(WIN32)
    set(CPACK_NSIS_DISPLAY_NAME "VoiVoi Analyzer")
    set(CPACK_NSIS_PACKAGE_NAME "VoiVoi Analyzer")
    set(CPACK_NSIS_HELP_LINK "https://github.com/susuki-zzz/VoiVoiAnalyzer")
    set(CPACK_NSIS_URL_INFO_ABOUT "https://github.com/susuki-zzz/VoiVoiAnalyzer")
    set(CPACK_NSIS_CONTACT "contact@voivoi.dev")
    set(CPACK_NSIS_MODIFY_PATH ON)
endif()

# Include documentation and license files
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")

# Exclude unwanted files from source package
set(CPACK_SOURCE_IGNORE_FILES
    "/\\.git/"
    "/\\.gitignore"
    "/build/"
    "/out/"
    "\\.user$"
    "\\.tmp$"
    "\\.swp$"
)

# Component installation
set(CPACK_COMPONENTS_ALL applications libraries documentation)
set(CPACK_COMPONENT_APPLICATIONS_DISPLAY_NAME "VoiVoi Applications")
set(CPACK_COMPONENT_LIBRARIES_DISPLAY_NAME "VoiVoi Core Library")
set(CPACK_COMPONENT_DOCUMENTATION_DISPLAY_NAME "Documentation")

set(CPACK_COMPONENT_APPLICATIONS_DESCRIPTION "GUI application and offline analysis tool")
set(CPACK_COMPONENT_LIBRARIES_DESCRIPTION "Core voice analysis library")
set(CPACK_COMPONENT_DOCUMENTATION_DESCRIPTION "User and developer documentation")
