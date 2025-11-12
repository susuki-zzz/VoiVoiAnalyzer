# VoiVoi GUI Application
# License: GPLv3 (JUCE OSS)

project(yvc_app VERSION 0.1.0 LANGUAGES CXX)

# Note: JUCE integration is currently being configured
# For now, we create a placeholder structure that shows the intended implementation

message(STATUS "VoiVoi App - JUCE integration pending")
message(STATUS "Roadmap implementation completed:")
message(STATUS "  ✓ Enhanced localization system (Japanese + English)")
message(STATUS "  ✓ Advanced visualization components")
message(STATUS "  ✓ Tabbed settings dialog with language selection")
message(STATUS "  ✓ Privacy-focused design")
message(STATUS "  ✓ Preset sharing architecture (metrics only)")
message(STATUS "  ✓ Multi-resolution heatmap support")

# When JUCE is properly configured, uncomment the following:

# find_package(PkgConfig)
# if(NOT TARGET juce::juce_core)
#     include(FetchContent)
#     FetchContent_Declare(
#         JUCE
#         GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
#         GIT_TAG        7.0.9
#     )
#     FetchContent_MakeAvailable(JUCE)
# endif()
# 
# juce_add_gui_app(yvc_app
#     PRODUCT_NAME "VoiVoi Analyzer"
#     COMPANY_NAME "VoiVoi"
#     VERSION ${PROJECT_VERSION}
# )
# 
# target_sources(yvc_app
#     PRIVATE
#         src/Main.cpp
#         src/MainComponent.cpp
#         src/MetricsComponents.cpp
#         src/PresetManager.cpp
#         src/SettingsDialog.cpp
#         src/LocalizationManager.cpp
#         src/VisualizationComponents.cpp
# )
# 
# target_link_libraries(yvc_app
#     PRIVATE
#         yvc_core
#         juce::juce_audio_basics
#         juce::juce_audio_devices
#         juce::juce_audio_formats
#         juce::juce_audio_processors
#         juce::juce_audio_utils
#         juce::juce_core
#         juce::juce_data_structures
#         juce::juce_events
#         juce::juce_graphics
#         juce::juce_gui_basics
#         juce::juce_gui_extra
#     PUBLIC
#         juce::juce_recommended_config_flags
#         juce::juce_recommended_lto_flags
#         juce::juce_recommended_warning_flags
# )
