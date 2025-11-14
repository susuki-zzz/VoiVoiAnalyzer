// VoiVoi GUI Application - Timeline Controller
// License: GPLv3

#pragma once

#include <juce_core/juce_core.h>
#include <mutex>

namespace yvc::app {

struct TimelineState {
    juce::Range<double> visibleRange{0.0, 10.0};
    double playhead = 0.0; // seconds
    bool followLatest = true;
    double windowSeconds = 10.0; // used when followLatest
};

class ITimelineListener {
public:
    virtual ~ITimelineListener() = default;
    virtual void timelineRangeChanged(const juce::Range<double>& range) = 0;
    virtual void timelinePlayheadChanged(double playhead) = 0;
    virtual void timelineFollowModeChanged(bool follow) {}
};

class TimelineController {
public:
    TimelineController() = default;
    ~TimelineController() = default;

    void addListener(ITimelineListener* l);
    void removeListener(ITimelineListener* l);

    void setVisibleRange(const juce::Range<double>& r);
    void setPlayhead(double t);
    void setFollowLatest(bool enabled);
    void setWindowSeconds(double seconds);

    void advanceToLatest(double latestTimestamp);

    TimelineState getState() const;
    juce::Range<double> getVisibleRange() const;

private:
    mutable std::mutex mtx_;
    TimelineState state_;
    juce::ListenerList<ITimelineListener> listeners_;
};

} // namespace yvc::app
