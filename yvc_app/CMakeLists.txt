# VoiVoi GUI Application
# License: GPLv3 (JUCE OSS)

project(yvc_app VERSION 0.1.0 LANGUAGES CXX)

# Check if JUCE is available
find_package(PkgConfig QUIET)

# Try to find JUCE
set(JUCE_FOUND FALSE)

# Method 1: Try to find system-installed JUCE
find_path(JUCE_INCLUDE_DIR
    NAMES juce_core/juce_core.h
    PATHS
        /usr/include/JUCE
        /usr/local/include/JUCE
        C:/JUCE/modules
        ${CMAKE_SOURCE_DIR}/third_party/JUCE/modules
        ${CMAKE_SOURCE_DIR}/JUCE/modules
    DOC "JUCE include directory"
)

if(JUCE_INCLUDE_DIR)
    message(STATUS "Found JUCE at: ${JUCE_INCLUDE_DIR}")
    set(JUCE_FOUND TRUE)
endif()

# Method 2: Download JUCE if not found
if(NOT JUCE_FOUND)
    message(STATUS "JUCE not found locally, downloading...")
    include(FetchContent)
    
    FetchContent_Declare(
        JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG        7.0.12
        GIT_SHALLOW    TRUE
        GIT_PROGRESS   TRUE
    )
    
    FetchContent_GetProperties(JUCE)
    if(NOT juce_POPULATED)
        message(STATUS "Populating JUCE...")
        FetchContent_Populate(JUCE)
        
        # Set JUCE options before adding subdirectory
        set(JUCE_BUILD_EXAMPLES OFF CACHE BOOL "Build JUCE examples")
        set(JUCE_BUILD_EXTRAS OFF CACHE BOOL "Build JUCE extras")
        
        add_subdirectory(${juce_SOURCE_DIR} ${juce_BINARY_DIR})
        set(JUCE_FOUND TRUE)
        message(STATUS "JUCE downloaded and configured")
    endif()
endif()

if(JUCE_FOUND)
    message(STATUS "Building VoiVoi App with JUCE support")
    
    # Check for icon files
    set(ICON_BIG_PATH "${CMAKE_CURRENT_SOURCE_DIR}/assets/icon_512.png")
    set(ICON_SMALL_PATH "${CMAKE_CURRENT_SOURCE_DIR}/assets/icon_64.png")
    
    set(JUCE_APP_ARGS
        PRODUCT_NAME "VoiVoi Analyzer"
        COMPANY_NAME "VoiVoi Team"
        VERSION ${PROJECT_VERSION}
        BUNDLE_ID com.voivoi.analyzer
        MICROPHONE_PERMISSION_ENABLED TRUE
        MICROPHONE_PERMISSION_TEXT "VoiVoi Analyzer needs microphone access for real-time voice analysis"
    )
    
    # Add icon files only if they exist
    if(EXISTS "${ICON_BIG_PATH}")
        list(APPEND JUCE_APP_ARGS ICON_BIG "${ICON_BIG_PATH}")
        message(STATUS "Using large icon: ${ICON_BIG_PATH}")
    else()
        message(STATUS "Large icon not found at ${ICON_BIG_PATH}, using default")
    endif()
    
    if(EXISTS "${ICON_SMALL_PATH}")
        list(APPEND JUCE_APP_ARGS ICON_SMALL "${ICON_SMALL_PATH}")
        message(STATUS "Using small icon: ${ICON_SMALL_PATH}")
    else()
        message(STATUS "Small icon not found at ${ICON_SMALL_PATH}, using default")
    endif()
    
    # Create the JUCE GUI application
    juce_add_gui_app(yvc_app ${JUCE_APP_ARGS})
    
    # Add source files
    target_sources(yvc_app
        PRIVATE
            src/Main.cpp
            src/MainComponent.cpp
            src/MetricsComponents.cpp
            src/PresetManager.cpp
            src/SettingsDialog.cpp
            src/LocalizationManager.cpp
            src/VisualizationComponents.cpp
    )
    
    # Add include directories
    target_include_directories(yvc_app
        PRIVATE
            include
            ${CMAKE_SOURCE_DIR}/yvc_core/include
    )
    
    # Link libraries
    target_link_libraries(yvc_app
        PRIVATE
            yvc_core
            juce::juce_audio_basics
            juce::juce_audio_devices
            juce::juce_audio_formats
            juce::juce_audio_processors
            juce::juce_audio_utils
            juce::juce_core
            juce::juce_data_structures
            juce::juce_events
            juce::juce_graphics
            juce::juce_gui_basics
            juce::juce_gui_extra
        PUBLIC
            juce::juce_recommended_config_flags
            juce::juce_recommended_lto_flags
            juce::juce_recommended_warning_flags
    )
    
    # Set C++20 standard
    target_compile_features(yvc_app PRIVATE cxx_std_20)
    
    # JUCE specific definitions
    target_compile_definitions(yvc_app
        PRIVATE
            JUCE_WEB_BROWSER=0
            JUCE_USE_CURL=0
            JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:yvc_app,JUCE_PRODUCT_NAME>"
            JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:yvc_app,JUCE_VERSION>"
            JUCE_DISPLAY_SPLASH_SCREEN=0
            JUCE_USE_DARK_SPLASH_SCREEN=1
    )
    
    # Platform-specific settings
    if(WIN32)
        target_compile_definitions(yvc_app PRIVATE JUCE_WIN32=1)
    elseif(APPLE)
        target_compile_definitions(yvc_app PRIVATE JUCE_MAC=1)
    elseif(UNIX)
        target_compile_definitions(yvc_app PRIVATE JUCE_LINUX=1)
    endif()
    
else()
    message(WARNING "JUCE not available - creating stub application")
    message(STATUS "To build with JUCE, install JUCE 7.0+ or ensure internet connection for download")
    
    # Create stub executable for testing core integration  
    add_executable(yvc_app src/MainStub.cpp)
    target_link_libraries(yvc_app PRIVATE yvc_core)
    target_include_directories(yvc_app PRIVATE include ${CMAKE_SOURCE_DIR}/yvc_core/include)
    target_compile_features(yvc_app PRIVATE cxx_std_20)
    
    message(STATUS "Built stub application: yvc_app")
    message(STATUS "Run the stub to test core integration without GUI")
endif()

# Installation targets
if(TARGET yvc_app)
    install(TARGETS yvc_app
        BUNDLE DESTINATION .
        RUNTIME DESTINATION bin
        COMPONENT applications
    )
endif()
