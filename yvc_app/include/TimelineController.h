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

    /// <summary>
    /// Aggregated timeline state shared between time-axis components.
    /// </summary>
    struct TimelineState {
        /// <summary>
        /// Visible time range in seconds (absolute timeline since stream start).
        /// </summary>
        juce::Range<double> visibleRange{ 0.0, 10.0 };
        /// <summary>
        /// Current playhead position in seconds.
        /// </summary>
        double playhead = 0.0;
        /// <summary>
        /// When true, controller auto-follows latest timestamp using windowSeconds.
        /// </summary>
        bool followLatest = true;
        /// <summary>
        /// Window length (seconds) used while followLatest is enabled.
        /// </summary>
        double windowSeconds = 10.0;
    };

    /// <summary>
    /// Listener interface for timeline state changes.
    /// </summary>
    class ITimelineListener {
    public:
        virtual ~ITimelineListener() = default;
        /// <summary>
        /// Called when visible range changes.
        /// </summary>
        virtual void timelineRangeChanged(const juce::Range<double>& range) = 0;
        /// <summary>
        /// Called when playhead changes.
        /// </summary>
        virtual void timelinePlayheadChanged(double playhead) = 0;
        /// <summary>
        /// Called when follow mode toggles. Default no-op.
        /// </summary>
        virtual void timelineFollowModeChanged(bool follow) { }
    };

    /// <summary>
    /// Controller that owns and synchronizes a shared timeline.
    /// All setters are thread-safe; notifications are synchronous on the caller thread.
    /// </summary>
    class TimelineController {
    public:
        TimelineController() = default;
        ~TimelineController() = default;

        /// <summary>
        /// Registers a listener. No ownership is taken; caller must ensure lifetime.
        /// </summary>
        void addListener(ITimelineListener* l);
        /// <summary>
        /// Unregisters a listener.
        /// </summary>
        void removeListener(ITimelineListener* l);

        /// <summary>
        /// Sets the visible time range (disables followLatest).
        /// </summary>
        void setVisibleRange(const juce::Range<double>& r);
        /// <summary>
        /// Sets current playhead time.
        /// </summary>
        void setPlayhead(double t);
        /// <summary>
        /// Enables or disables auto-follow to the latest timestamp.
        /// </summary>
        void setFollowLatest(bool enabled);
        /// <summary>
        /// Sets window length used by auto-follow.
        /// </summary>
        void setWindowSeconds(double seconds);

        /// <summary>
        /// Advances the timeline to include latestTimestamp if followLatest is enabled.
        /// </summary>
        void advanceToLatest(double latestTimestamp);

        /// <summary>
        /// Thread-safe snapshot of current state.
        /// </summary>
        TimelineState getState() const;
        /// <summary>
        /// Convenience accessor for the current visible range.
        /// </summary>
        juce::Range<double> getVisibleRange() const;

    private:
        mutable std::mutex mtx_;
        TimelineState state_;
        juce::ListenerList<ITimelineListener> listeners_;
    };

} // namespace yvc::app
