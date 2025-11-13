# Third-party dependencies

# KissFFT - simple FFT library
add_subdirectory(kissfft)

# JUCE Framework - for GUI application
# Note: JUCE is used under GPLv3 for the yvc_app component
if(BUILD_YVC_APP)
    set(_YVC_JUCE_SUBMODULE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/JUCE")
    if(EXISTS "${_YVC_JUCE_SUBMODULE_DIR}/CMakeLists.txt")
        if(NOT DEFINED YVC_JUCE_PATH OR "${YVC_JUCE_PATH}" STREQUAL "")
            set(YVC_JUCE_PATH "${_YVC_JUCE_SUBMODULE_DIR}" CACHE PATH "Override path to the JUCE source tree" FORCE)
            message(STATUS "Defaulting YVC_JUCE_PATH to ${YVC_JUCE_PATH}")
        else()
            message(STATUS "Using configured JUCE path: ${YVC_JUCE_PATH}")
        endif()
    else()
        message(STATUS "JUCE source tree not found in third_party/JUCE. Provide JUCE via YVC_JUCE_PATH, JUCE_DIR, or enable YVC_FETCH_JUCE.")
    endif()
endif()

# Optional: Eigen for advanced matrix operations
if(USE_EIGEN)
    message(STATUS "Eigen support enabled")
    # add_subdirectory(eigen) # Will be added if needed
endif()
