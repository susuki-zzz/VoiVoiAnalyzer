# KissFFT - Simple FFT library
# This is a minimal CMake wrapper for KissFFT

project(kissfft LANGUAGES C)

set(KISSFFT_SOURCES
    kiss_fft.c
    kiss_fftr.c
)

set(KISSFFT_HEADERS
    kiss_fft.h
    kiss_fftr.h
    _kiss_fft_guts.h
)

add_library(kissfft STATIC ${KISSFFT_SOURCES} ${KISSFFT_HEADERS})

target_include_directories(kissfft
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
)

# Ensure C99 standard
set_target_properties(kissfft PROPERTIES
    C_STANDARD 99
    C_STANDARD_REQUIRED ON
)

# Platform-specific settings
if(MSVC)
    target_compile_definitions(kissfft PRIVATE _CRT_SECURE_NO_WARNINGS)
    # Use MultiThreadedDebugDLL runtime for debug builds
    # Use MultiThreadedDLL runtime for release builds
    set_target_properties(kissfft PROPERTIES
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
    )
endif()
