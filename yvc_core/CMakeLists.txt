# VoiVoi Core Library
# License: MIT

project(yvc_core VERSION 0.1.0 LANGUAGES CXX)

include(CTest)

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

# Unit tests
if(BUILD_TESTING)
    add_executable(yvc_core_tests
        tests/AnalyzerEngineTests.cpp
    )

    target_link_libraries(yvc_core_tests
        PRIVATE
            yvc_core
    )

    add_test(NAME yvc_core_tests COMMAND yvc_core_tests)

    add_executable(yvc_core_cpp_tests
        tests/CPPAnalyzerTests.cpp
    )

    target_link_libraries(yvc_core_cpp_tests
        PRIVATE
            yvc_core
    )

    add_test(NAME yvc_core_cpp_tests COMMAND yvc_core_cpp_tests)

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

enable_testing()

add_executable(yvc_core_ram_only_tests tests/RamOnlyStorageTests.cpp)
# Link Threads in tests transitively via yvc_core
target_link_libraries(yvc_core_ram_only_tests PRIVATE yvc_core)
add_test(NAME RamOnlyStorageNoDisk COMMAND yvc_core_ram_only_tests)
