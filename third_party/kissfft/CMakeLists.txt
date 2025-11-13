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
        $<INSTALL_INTERFACE:include/kissfft>
)

# Ensure C99 standard
set_target_properties(kissfft PROPERTIES
    C_STANDARD 99
    C_STANDARD_REQUIRED ON
    C_EXTENSIONS OFF
)

# Platform-specific settings
if(MSVC)
    target_compile_definitions(kissfft PRIVATE _CRT_SECURE_NO_WARNINGS)
    # Ensure consistent runtime library
    set_target_properties(kissfft PROPERTIES
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
    )
endif()

# Make kissfft position independent (good practice for static libs)
set_target_properties(kissfft PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

# Add alias for consistent namespacing
add_library(yvc::kissfft ALIAS kissfft)

# Installation (if yvc_core is installed)
include(GNUInstallDirs)

install(TARGETS kissfft
    EXPORT yvc_coreTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/kissfft
)

install(FILES ${KISSFFT_HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/kissfft
)
