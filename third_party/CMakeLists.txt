# Third-party dependencies

# KissFFT - simple FFT library
add_subdirectory(kissfft)

# JUCE Framework - for GUI application
# Note: JUCE is used under GPLv3 for the yvc_app component
if(BUILD_YVC_APP)
    # JUCE will be added via FetchContent or git submodule
    # For now, we'll set up the structure to add it
    message(STATUS "JUCE integration will be configured")
    # add_subdirectory(JUCE) # Will be added when JUCE is included
endif()

# Optional: Eigen for advanced matrix operations
if(USE_EIGEN)
    message(STATUS "Eigen support enabled")
    # add_subdirectory(eigen) # Will be added if needed
endif()
