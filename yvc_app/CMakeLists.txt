# VoiVoi GUI Application
# License: GPLv3 (JUCE OSS)

project(yvc_app VERSION 0.1.0 LANGUAGES CXX)

# Note: JUCE framework would be added here
# For now, we create a minimal structure that can be built when JUCE is added

# This is a placeholder - in a real build, JUCE would be fetched or added as submodule
# include(FetchContent)
# FetchContent_Declare(
#     JUCE
#     GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
#     GIT_TAG        7.0.9
# )
# FetchContent_MakeAvailable(JUCE)

# For now, we'll just create a minimal app stub
message(STATUS "VoiVoi App configured - JUCE integration pending")

# When JUCE is available, uncomment and configure:
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
#         src/AudioProcessor.cpp
#         src/VisualizationComponent.cpp
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
