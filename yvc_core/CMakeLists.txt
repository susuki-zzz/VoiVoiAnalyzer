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
)

# Create library
add_library(yvc_core STATIC ${YVC_CORE_SOURCES} ${YVC_CORE_HEADERS})

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
    PUBLIC
        kissfft
        Threads::Threads
)

include(CTest)

# Unit tests
if(BUILD_TESTING)
    include(GoogleTest)

    add_executable(yvc_core_tests
        tests/AnalyzerEngineTests.cpp
        tests/AudioBufferTests.cpp
        tests/CPPAnalyzerTests.cpp
        tests/MetricsBusTests.cpp
        tests/PerformanceModeTests.cpp
        tests/PreprocessChainTests.cpp
        tests/RamOnlyStorageTests.cpp
        tests/SessionPersistenceTests.cpp
        tests/ApiKeyVaultTests.cpp
        tests/MetricsOnlyCoachProviderTests.cpp
        tests/LoggerTests.cpp
        tests/TestSignalHelpers.h
    )

    if(NOT YVC_GTEST_MAIN_TARGET)
        message(FATAL_ERROR "GoogleTest main target not available. Set BUILD_TESTING=OFF to skip tests or provide GTest.")
    endif()

    target_link_libraries(yvc_core_tests
        PRIVATE
            yvc_core
            ${YVC_GTEST_MAIN_TARGET}
    )

    target_include_directories(yvc_core_tests
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/tests
    )

    gtest_discover_tests(yvc_core_tests)

    # Logger examples/tests
    add_executable(yvc_logger_examples
        tests/LoggerExamples.cpp
    )

    target_link_libraries(yvc_logger_examples
        PRIVATE
            yvc_core
    )

    # Not added to CTest as it's more of an example program
endif()

# Windows-specific settings
if(WIN32)
    target_compile_definitions(yvc_core PRIVATE _CRT_SECURE_NO_WARNINGS)
    target_link_libraries(yvc_core PRIVATE Crypt32)
    # Ensure consistent runtime library
    set_target_properties(yvc_core PROPERTIES
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
    )
endif()

# Compiler warnings
if(MSVC)
    target_compile_options(yvc_core PRIVATE /W4)
else()
    target_compile_options(yvc_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

