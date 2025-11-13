# VoiVoi GUI Application
# License: GPLv3 (JUCE OSS)

project(yvc_app VERSION 0.1.0 LANGUAGES CXX)

# Try to locate JUCE locally first (e.g. as a submodule in third_party/JUCE)
set(JUCE_FOUND FALSE)
set(_JUCE_LOCAL_PATH "${CMAKE_SOURCE_DIR}/third_party/JUCE")
if(EXISTS "${_JUCE_LOCAL_PATH}/modules/juce_core/juce_core.h")
    message(STATUS "Found JUCE submodule at: ${_JUCE_LOCAL_PATH}")
    add_subdirectory(${_JUCE_LOCAL_PATH} ${_JUCE_LOCAL_PATH}/build)
    set(JUCE_FOUND TRUE)
endif()

# If not found locally, fetch it if allowed.
if(NOT JUCE_FOUND AND YVC_FETCH_JUCE)
    message(STATUS "JUCE not found locally, fetching via FetchContent...")
    include(FetchContent)
    # Ensure JUCE options are set BEFORE making available
    set(JUCE_BUILD_EXAMPLES OFF CACHE BOOL "Build JUCE examples" FORCE)
    set(JUCE_BUILD_EXTRAS OFF CACHE BOOL "Build JUCE extras" FORCE)

    FetchContent_Declare(
        juce
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG        7.0.12
        GIT_SHALLOW    TRUE
        GIT_PROGRESS   TRUE
    )
    FetchContent_GetProperties(juce)
    if(NOT juce_POPULATED)
        FetchContent_MakeAvailable(juce)
    endif()
    set(JUCE_FOUND TRUE)
    message(STATUS "JUCE fetched and configured")
elseif(NOT JUCE_FOUND AND NOT YVC_FETCH_JUCE)
    message(STATUS "JUCE not found and YVC_FETCH_JUCE=OFF; will build stub app.")
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

    # Set C++ standard (JUCE handles this but explicit is better)
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
        # Ensure consistent runtime library
        set_target_properties(yvc_app PROPERTIES
            MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
        )
    elseif(APPLE)
        target_compile_definitions(yvc_app PRIVATE JUCE_MAC=1)
    elseif(UNIX)
        target_compile_definitions(yvc_app PRIVATE JUCE_LINUX=1)
    endif()

    if(BUILD_TESTING)
        yvc_add_gtest(yvc_app_tests
            tests/PresetManagerTests.cpp
        )

        # Compile implementation sources needed by tests
        target_sources(yvc_app_tests PRIVATE
            src/PresetManager.cpp
        )

        target_link_libraries(yvc_app_tests
            PRIVATE
                juce::juce_core
                juce::juce_gui_basics
                yvc_core
        )

        target_include_directories(yvc_app_tests
            PRIVATE
                ${CMAKE_CURRENT_SOURCE_DIR}/include
                ${CMAKE_SOURCE_DIR}/yvc_core/include
        )
    endif()

else()
    message(WARNING "JUCE not available - creating stub application")
    message(STATUS "Enable YVC_FETCH_JUCE=ON or add JUCE under third_party/JUCE to build full GUI.")
    add_executable(yvc_app src/MainStub.cpp)
    target_compile_features(yvc_app PRIVATE cxx_std_20)
    target_link_libraries(yvc_app PRIVATE yvc_core)
    target_include_directories(yvc_app PRIVATE include ${CMAKE_SOURCE_DIR}/yvc_core/include)
    message(STATUS "Built stub application: yvc_app")
endif()

# Installation targets
if(TARGET yvc_app)
    install(TARGETS yvc_app
        BUNDLE DESTINATION .
        RUNTIME DESTINATION bin
        COMPONENT applications
    )
endif()
