// VoiVoi GUI Application - Timeline Controller
// License: GPLv3
// Purpose: Provide a shared time axis (visible range + playhead) across UI components.
// Notes:
//  - Thread-safety: setters/getters are thread-safe; listeners are notified on the caller thread.
//  - Typical usage: MainComponent owns TimelineController and advances it with latest timestamps.

#pragma once

#include <juce_core/juce_core.h>
#include <mutex>

namespace yvc::app {

/// Aggregated timeline state shared between time-axis components.
struct TimelineState {
    /// Visible time range in seconds (absolute timeline since stream start).
    juce::Range<double> visibleRange{0.0, 10.0};
    /// Current playhead position in seconds.
    double playhead = 0.0;
    /// When true, controller auto-follows latest timestamp using windowSeconds.
    bool followLatest = true;
    /// Window length (seconds) used while followLatest is enabled.
    double windowSeconds = 10.0;
};

/// Listener interface for timeline state changes.
class ITimelineListener {
public:
    virtual ~ITimelineListener() = default;
    /// Called when visible range changes.
    virtual void timelineRangeChanged(const juce::Range<double>& range) = 0;
    /// Called when playhead changes.
    virtual void timelinePlayheadChanged(double playhead) = 0;
    /// Called when follow mode toggles. Default no-op.
    virtual void timelineFollowModeChanged(bool follow) {}
};

/// Controller that owns and synchronizes a shared timeline.
/// All setters are thread-safe; notifications are synchronous on the caller thread.
class TimelineController {
public:
    TimelineController() = default;
    ~TimelineController() = default;

    /// Register a listener. No ownership is taken; caller must ensure lifetime.
    void addListener(ITimelineListener* l);
    /// Unregister a listener.
    void removeListener(ITimelineListener* l);

    /// Set the visible time range (disables followLatest).
    void setVisibleRange(const juce::Range<double>& r);
    /// Set current playhead time.
    void setPlayhead(double t);
    /// Enable or disable auto-follow to the latest timestamp.
    void setFollowLatest(bool enabled);
    /// Set window length used by auto-follow.
    void setWindowSeconds(double seconds);

    /// Advance the timeline to include latestTimestamp if followLatest is enabled.
    void advanceToLatest(double latestTimestamp);

    /// Thread-safe snapshot of current state.
    TimelineState getState() const;
    /// Convenience accessor for the current visible range.
    juce::Range<double> getVisibleRange() const;

private:
    mutable std::mutex mtx_;
    TimelineState state_;
    juce::ListenerList<ITimelineListener> listeners_;
};

} // namespace yvc::app
