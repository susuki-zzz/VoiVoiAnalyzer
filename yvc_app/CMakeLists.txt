# VoiVoi GUI Application
# License: GPLv3 (JUCE OSS)

project(yvc_app VERSION 0.1.0 LANGUAGES CXX)

# Allow callers to pass a custom JUCE source tree location.
set(YVC_JUCE_PATH "" CACHE PATH "Override path to the JUCE source tree")

# Try to locate JUCE from a list of well-known locations. The paths are evaluated in
# priority order: cache override, environment variable, repository submodule.
set(JUCE_FOUND FALSE)
set(_YVC_JUCE_SEARCH_PATHS)
if(YVC_JUCE_PATH)
    list(APPEND _YVC_JUCE_SEARCH_PATHS "${YVC_JUCE_PATH}")
endif()
if(DEFINED ENV{JUCE_DIR})
    list(APPEND _YVC_JUCE_SEARCH_PATHS "$ENV{JUCE_DIR}")
endif()
list(APPEND _YVC_JUCE_SEARCH_PATHS "${CMAKE_SOURCE_DIR}/third_party/JUCE")
list(REMOVE_DUPLICATES _YVC_JUCE_SEARCH_PATHS)

foreach(_YVC_JUCE_PATH IN LISTS _YVC_JUCE_SEARCH_PATHS)
    if(NOT JUCE_FOUND AND EXISTS "${_YVC_JUCE_PATH}/modules/juce_core/juce_core.h")
        message(STATUS "Found JUCE source tree at: ${_YVC_JUCE_PATH}")
        add_subdirectory(${_YVC_JUCE_PATH} ${CMAKE_BINARY_DIR}/juce-build)
        set(JUCE_FOUND TRUE)
    endif()
endforeach()

# Fall back to a JUCE package installation if available.
if(NOT JUCE_FOUND)
    find_package(JUCE CONFIG QUIET)
    if(JUCE_FOUND)
        message(STATUS "Found JUCE via find_package()")
    endif()
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
            src/JuceAudioBridge.cpp
            src/MetricsComponents.cpp
            src/PresetManager.cpp
            src/SettingsDialog.cpp
            src/SettingsComponent.cpp
            src/AudioInputManager.cpp
            src/LocalizationManager.cpp
            src/VisualizationComponents.cpp
            src/WaveformComponent.cpp
            src/ScrollingWaveformComponent.cpp
            src/TimelineController.cpp
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
            tests/SettingsDialogTests.cpp
            tests/JuceAudioBridgeTests.cpp
        )

        # Compile implementation sources needed by tests
        target_sources(yvc_app_tests PRIVATE
            src/PresetManager.cpp
            src/JuceAudioBridge.cpp
            src/LocalizationManager.cpp
            src/SettingsDialog.cpp
            src/SettingsComponent.cpp
        )

        target_link_libraries(yvc_app_tests
            PRIVATE
                juce::juce_core
                juce::juce_gui_basics
                juce::juce_audio_devices
                yvc_core
        )

        target_include_directories(yvc_app_tests
            PRIVATE
                ${CMAKE_CURRENT_SOURCE_DIR}/include
                ${CMAKE_SOURCE_DIR}/yvc_core/include
        )

        add_executable(yvc_app_gui_tests
            tests/VisualizationComponentScenarioTests.cpp
        )

        target_sources(yvc_app_gui_tests PRIVATE
            src/VisualizationComponents.cpp
            src/LocalizationManager.cpp
            src/MetricsComponents.cpp
            src/TimelineController.cpp
        )

        target_link_libraries(yvc_app_gui_tests
            PRIVATE
                juce::juce_core
                juce::juce_data_structures
                juce::juce_events
                juce::juce_graphics
                juce::juce_gui_basics
                yvc_core
        )

        target_include_directories(yvc_app_gui_tests
            PRIVATE
                ${CMAKE_CURRENT_SOURCE_DIR}/include
                ${CMAKE_SOURCE_DIR}/yvc_core/include
        )

        add_test(NAME yvc_app_gui_scenarios COMMAND yvc_app_gui_tests)
    endif()

else()
    message(WARNING "JUCE not available - creating stub application")
    message(STATUS "Provide JUCE via third_party/JUCE, -DYVC_JUCE_PATH=..., JUCE_DIR, or enable YVC_FETCH_JUCE=ON to build the full GUI.")
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
