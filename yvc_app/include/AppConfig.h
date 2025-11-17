// VoiVoi GUI Application - Application Configuration
// License: GPLv3

#pragma once

// JUCE Application Configuration
#define JUCE_APPLICATION_NAME_STRING       "VoiVoi Analyzer"
#define JUCE_APPLICATION_VERSION_STRING    "0.1.0"
#define JUCE_COMPANY_NAME                  "VoiVoi Team"

// Features
#define JUCE_WEB_BROWSER                   0
#define JUCE_USE_CURL                      0
#define JUCE_DISPLAY_SPLASH_SCREEN         0
#define JUCE_USE_DARK_SPLASH_SCREEN        1

// Audio Configuration
#define JUCE_PLUGINHOST_VST3               0
#define JUCE_PLUGINHOST_AU                 0
#define JUCE_PLUGINHOST_VST                0
#define JUCE_PLUGINHOST_LADSPA             0

// Privacy Settings
#define JUCE_STRICT_REFCOUNTING           1
#define JUCE_ENABLE_LIVE_CONSTANT_EDITOR  0

// Performance
#define JUCE_ENABLE_ALLOCATION_HOOKS      1
#define JUCE_CATCHALL_IN_COMCALLS         1

namespace yvc::app {

/// <summary>
/// Application-wide constants for window sizing.
/// </summary>
constexpr int kDefaultWindowWidth = 1200;
constexpr int kDefaultWindowHeight = 720;
constexpr int kMinWindowWidth = 800;
constexpr int kMinWindowHeight = 600;

/// <summary>
/// Performance targets for FPS throttling.
/// </summary>
constexpr double kTargetFpsStandard = 60.0;
constexpr double kTargetFpsDegraded = 45.0;
constexpr double kTargetFpsMinimum = 30.0;

/// <summary>
/// Audio configuration constraints.
/// </summary>
constexpr double kMaxRecordingTimeDefaultSeconds = 3600.0; // 1 hour
constexpr int kDefaultBufferSize = 256;
constexpr double kDefaultSampleRate = 48000.0;

} // namespace yvc::app
