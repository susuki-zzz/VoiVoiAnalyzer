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
endif()
