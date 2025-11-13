# VoiVoi Offline Analysis Tool
# License: GPLv3

project(yvc_offline VERSION 0.1.0 LANGUAGES CXX)

# Offline analysis tool sources
set(YVC_OFFLINE_SOURCES
    src/main.cpp
    src/FileProcessor.cpp
)

# Offline analysis tool headers
set(YVC_OFFLINE_HEADERS
    include/FileProcessor.h
)

# Create executable
add_executable(yvc_offline ${YVC_OFFLINE_SOURCES} ${YVC_OFFLINE_HEADERS})

# Set C++ standard per target
target_compile_features(yvc_offline PRIVATE cxx_std_20)

# Include directories
target_include_directories(yvc_offline
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link dependencies
target_link_libraries(yvc_offline
    PRIVATE
        yvc_core
)

# Windows-specific settings
if(WIN32)
    target_compile_definitions(yvc_offline PRIVATE _CRT_SECURE_NO_WARNINGS)
    # Ensure consistent runtime library
    set_target_properties(yvc_offline PROPERTIES
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
    )
endif()

# Compiler warnings
if(MSVC)
    target_compile_options(yvc_offline PRIVATE /W4)
else()
    target_compile_options(yvc_offline PRIVATE 
        -Wall -Wextra -Wpedantic
        -Wconversion -Wshadow
    )
endif()

include(CTest)

# Unit tests
if(BUILD_TESTING)
    yvc_add_gtest(yvc_offline_tests
        tests/FileProcessorTests.cpp
        src/FileProcessor.cpp
    )

    target_include_directories(yvc_offline_tests
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

    target_link_libraries(yvc_offline_tests PRIVATE yvc_core)
endif()
