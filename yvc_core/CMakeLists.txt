# VoiVoi Core Library
# License: MIT

project(yvc_core VERSION 0.1.0 LANGUAGES CXX)

# Core library sources
set(YVC_CORE_SOURCES
    src/AudioBuffer.cpp
    src/AnalyzerEngine.cpp
    src/F0Detector.cpp
    src/LevelAnalyzer.cpp
    src/CPPAnalyzer.cpp
    src/HNRAnalyzer.cpp
    src/SpectralAnalyzer.cpp
    src/VADAnalyzer.cpp
    src/PerformanceMode.cpp
    src/MetricsBus.cpp
    src/PreprocessChain.cpp
    src/RamOnlyStorage.cpp
    src/SessionPersistence.cpp
    src/MetricsOnlyCoachProvider.cpp
    src/ApiKeyVault.cpp
    src/Logger.cpp
    src/audio/AudioBackendFactory.cpp
    src/audio/CoreAudioBackend.cpp
    src/audio/AlsaAudioBackend.cpp
)

# Core library headers
set(YVC_CORE_HEADERS
    include/yvc_core/AudioBuffer.h
    include/yvc_core/F0Detector.h
    include/yvc_core/LevelAnalyzer.h
    include/yvc_core/CPPAnalyzer.h
    include/yvc_core/HNRAnalyzer.h
    include/yvc_core/SpectralAnalyzer.h
    include/yvc_core/VADAnalyzer.h
    include/yvc_core/PerformanceMode.h
    include/yvc_core/MetricsBus.h
    include/yvc_core/AnalyzerEngine.h
    include/yvc_core/IAnalyzer.h
    include/yvc_core/IPreprocessor.h
    include/yvc_core/ICoachProvider.h
    include/yvc_core/Types.h
    include/yvc_core/PreprocessChain.h
    include/yvc_core/RamOnlyStorage.h
    include/yvc_core/SessionPersistence.h
    include/yvc_core/MetricsOnlyCoachProvider.h
    include/yvc_core/ApiKeyVault.h
    include/yvc_core/Logger.h
    include/yvc_core/AudioBackend.h
    include/yvc_core/LockFreeRingBuffer.h
)

# Create library
add_library(yvc_core STATIC ${YVC_CORE_SOURCES} ${YVC_CORE_HEADERS})

# Set C++ standard per target (not globally)
target_compile_features(yvc_core PUBLIC cxx_std_20)

# Threads (needed for std::thread on non-Windows platforms)
find_package(Threads REQUIRED)

# Include directories
target_include_directories(yvc_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link dependencies
target_link_libraries(yvc_core
    PRIVATE
        $<BUILD_INTERFACE:kissfft>
        $<INSTALL_INTERFACE:yvc::kissfft>
    PUBLIC
        Threads::Threads
)

# Platform-specific audio backends
if(APPLE)
    target_compile_definitions(yvc_core PUBLIC YVC_CORE_HAVE_COREAUDIO=1)
    target_link_libraries(yvc_core PRIVATE "-framework AudioToolbox" "-framework AudioUnit" "-framework CoreAudio" "-framework CoreFoundation")
elseif(UNIX AND NOT APPLE)
    find_package(ALSA)
    if(ALSA_FOUND)
        target_compile_definitions(yvc_core PUBLIC YVC_CORE_HAVE_ALSA=1)
        target_link_libraries(yvc_core PRIVATE ALSA::ALSA)
    else()
        message(WARNING "ALSA development files not found; ALSA backend will be disabled.")
    endif()
endif()

# Windows-specific settings
if(WIN32)
    target_compile_definitions(yvc_core PRIVATE _CRT_SECURE_NO_WARNINGS)
    target_link_libraries(yvc_core PRIVATE Crypt32)
    list(APPEND YVC_CORE_SOURCES src/audio/WasapiAudioBackend.cpp)
    target_compile_definitions(yvc_core PUBLIC YVC_CORE_HAVE_WASAPI=1)
    # Ensure consistent runtime library
    set_target_properties(yvc_core PROPERTIES
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
    )
endif()

# Compiler warnings and MSVC conforming preprocessor
if(MSVC)
    target_compile_options(yvc_core PRIVATE /W4 /Zc:preprocessor)
else()
    target_compile_options(yvc_core PRIVATE 
        -Wall -Wextra -Wpedantic
        -Wconversion -Wshadow -Wnon-virtual-dtor
    )
endif()

# Enable LTO for Release builds
set_target_properties(yvc_core PROPERTIES
    INTERPROCEDURAL_OPTIMIZATION_RELEASE ON
    INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO ON
)

# Logging configuration
if(YVC_ENABLE_FILE_LOG)
    target_compile_definitions(yvc_core PUBLIC YVC_DEFAULT_FILE_LOG_ENABLED)
endif()

include(CTest)

# Unit tests
if(BUILD_TESTING)
    yvc_add_gtest(yvc_core_tests
        tests/AnalyzerEngineTests.cpp
        tests/AudioBufferTests.cpp
        tests/AudioBackendTests.cpp
        tests/CPPAnalyzerTests.cpp
        tests/F0DetectorTests.cpp
        tests/LevelAnalyzerTests.cpp
        tests/HNRAnalyzerTests.cpp
        tests/SpectralAnalyzerTests.cpp
        tests/VADAnalyzerTests.cpp
        tests/MetricsBusTests.cpp
        tests/PerformanceModeTests.cpp
        tests/PreprocessChainTests.cpp
        tests/RamOnlyStorageTests.cpp
        tests/SessionPersistenceTests.cpp
        tests/ApiKeyVaultTests.cpp
        tests/MetricsOnlyCoachProviderTests.cpp
        tests/LoggerTests.cpp
        tests/TestSignalHelpersDemoTests.cpp
        tests/LockFreeRingBufferTests.cpp
        tests/TestSignalHelpers.h
    )

    target_link_libraries(yvc_core_tests PRIVATE yvc_core)
    target_include_directories(yvc_core_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests)

    # Logger examples/tests
    add_executable(yvc_logger_examples tests/LoggerExamples.cpp)
    target_link_libraries(yvc_logger_examples PRIVATE yvc_core)
endif()

# Installation rules
include(GNUInstallDirs)

install(TARGETS yvc_core
    EXPORT yvc_coreTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/yvc_core
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h"
)

install(EXPORT yvc_coreTargets
    FILE yvc_coreTargets.cmake
    NAMESPACE yvc::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/yvc_core
)

# Generate package config file
include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/yvc_coreConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/yvc_coreConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/yvc_core
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/yvc_coreConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/yvc_coreConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/yvc_coreConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/yvc_core
)

