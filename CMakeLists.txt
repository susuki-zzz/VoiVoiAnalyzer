cmake_minimum_required(VERSION 3.20)

project(VoiVoiAnalyzer VERSION 0.1.0 LANGUAGES CXX C)

# Options (defined early for visibility)
option(BUILD_TESTING "Build tests" ON)
option(BUILD_YVC_APP "Build GUI application (yvc_app)" ON)
option(BUILD_YVC_OFFLINE "Build offline analysis tool (yvc_offline)" ON)
option(USE_EIGEN "Use Eigen for matrix operations (optional)" OFF)
option(YVC_ENABLE_FILE_LOG "Enable file logging by default" OFF)
option(YVC_ENABLE_COVERAGE "Enable code coverage reporting" OFF)
option(YVC_ENABLE_SANITIZERS "Enable address and undefined behavior sanitizers (Debug only)" OFF)
option(YVC_FETCH_JUCE "Automatically fetch JUCE if not present as submodule" OFF)

# Display configuration
message(STATUS "=== VoiVoi Analyzer Configuration ===")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "BUILD_TESTING: ${BUILD_TESTING}")
message(STATUS "BUILD_YVC_APP: ${BUILD_YVC_APP}")
message(STATUS "BUILD_YVC_OFFLINE: ${BUILD_YVC_OFFLINE}")
message(STATUS "USE_EIGEN: ${USE_EIGEN}")
message(STATUS "YVC_ENABLE_FILE_LOG: ${YVC_ENABLE_FILE_LOG}")
message(STATUS "YVC_ENABLE_COVERAGE: ${YVC_ENABLE_COVERAGE}")
message(STATUS "YVC_ENABLE_SANITIZERS: ${YVC_ENABLE_SANITIZERS}")
message(STATUS "YVC_FETCH_JUCE: ${YVC_FETCH_JUCE}")
message(STATUS "=====================================")

# Build type (only for single-config generators)
if(NOT CMAKE_CONFIGURATION_TYPES)
    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
        set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
    endif()
    message(STATUS "Single-config generator: CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")
else()
    message(STATUS "Multi-config generator: ${CMAKE_CONFIGURATION_TYPES}")
endif()

# Use dynamic runtime library for MSVC (set via target properties, not global)
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

# JUCE settings for OSS GPL
set(JUCE_BUILD_EXTRAS OFF CACHE BOOL "Build JUCE extras" FORCE)
set(JUCE_BUILD_EXAMPLES OFF CACHE BOOL "Build JUCE examples" FORCE)

# Add cmake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Enable testing infrastructure
if(BUILD_TESTING)
    include(CTest)
    find_package(GTest QUIET)

    if(GTest_FOUND)
        set(YVC_GTEST_MAIN_TARGET GTest::gtest_main CACHE INTERNAL "GTest main target")
        set(YVC_GTEST_TARGET GTest::gtest CACHE INTERNAL "GTest target")
        message(STATUS "Using system-provided GTest")
    else()
        message(STATUS "GTest not found, fetching from GitHub")
        include(FetchContent)

        FetchContent_Declare(
            googletest
            URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
        )

        # Prevent overriding the parent project's compiler/linker settings on Windows
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

        FetchContent_MakeAvailable(googletest)

        set(YVC_GTEST_MAIN_TARGET gtest_main CACHE INTERNAL "GTest main target")
        set(YVC_GTEST_TARGET gtest CACHE INTERNAL "GTest target")
    endif()

    enable_testing()
endif()

# Helper function for adding tests with GTest
function(yvc_add_gtest TARGET_NAME)
    if(NOT BUILD_TESTING)
        return()
    endif()
    
    if(NOT YVC_GTEST_MAIN_TARGET)
        message(FATAL_ERROR "GoogleTest main target not available. Set BUILD_TESTING=OFF to skip tests or provide GTest.")
    endif()
    
    include(GoogleTest)
    add_executable(${TARGET_NAME} ${ARGN})
    target_link_libraries(${TARGET_NAME} PRIVATE ${YVC_GTEST_MAIN_TARGET})
    gtest_discover_tests(${TARGET_NAME})
    
    # Apply coverage flags if enabled
    if(YVC_ENABLE_COVERAGE AND (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang"))
        target_compile_options(${TARGET_NAME} PRIVATE --coverage)
        target_link_options(${TARGET_NAME} PRIVATE --coverage)
    endif()
    
    # Apply sanitizers if enabled (Debug only)
    if(YVC_ENABLE_SANITIZERS AND (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang"))
        target_compile_options(${TARGET_NAME} PRIVATE 
            $<$<CONFIG:Debug>:-fsanitize=address,undefined>
            $<$<CONFIG:Debug>:-fno-omit-frame-pointer>
        )
        target_link_options(${TARGET_NAME} PRIVATE 
            $<$<CONFIG:Debug>:-fsanitize=address,undefined>
        )
    endif()
endfunction()

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

# CPack configuration for packaging
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
