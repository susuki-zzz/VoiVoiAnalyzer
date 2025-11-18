// VoiVoi GUI Application - Timeline Controller Implementation
// License: GPLv3

#include "TimelineController.h"

namespace yvc::app {

/// <summary>
/// Registers a listener.
/// </summary>
void TimelineController::addListener(ITimelineListener* l){
    listeners_.add(l);
}
/// <summary>
/// Unregisters a listener.
/// </summary>
void TimelineController::removeListener(ITimelineListener* l){
    listeners_.remove(l);
}

/// <summary>
/// Sets visible range explicitly and disables followLatest.
/// </summary>
void TimelineController::setVisibleRange(const juce::Range<double>& r){ {
        std::scoped_lock lk(mtx_);
        state_.visibleRange = r;
        state_.windowSeconds = r.getLength();
        state_.followLatest = false;
    }
    listeners_.call([&](ITimelineListener& x){
        x.timelineRangeChanged(r);
    });
}

/// <summary>
/// Sets the playhead position.
/// </summary>
void TimelineController::setPlayhead(double t){ {
        std::scoped_lock lk(mtx_);
        state_.playhead = t;
    }
    listeners_.call([&](ITimelineListener& x){ x.timelinePlayheadChanged(t); });
}

/// <summary>
/// Enables/disables auto-follow mode.
/// </summary>
void TimelineController::setFollowLatest(bool enabled){ {
        std::scoped_lock lk(mtx_);
        state_.followLatest = enabled;
    }
    listeners_.call([&](ITimelineListener& x){
        x.timelineFollowModeChanged(enabled);
    });
}

/// <summary>
/// Sets window duration used while following latest timestamp.
/// </summary>
void TimelineController::setWindowSeconds(double s){ if(s <= 0.1) s = 0.1; {
        std::scoped_lock lk(mtx_);
        state_.windowSeconds = s;
        state_.visibleRange = {
            state_.visibleRange.getEnd() - s,
            state_.visibleRange.getEnd()
        };
    }
    listeners_.call([&](ITimelineListener& x){
        x.timelineRangeChanged(getVisibleRange());
    });
}

/// <summary>
/// Advances range/playhead to include latest timestamp when followLatest is enabled.
/// </summary>
void TimelineController::advanceToLatest(double latest){
    juce::Range<double> r;
    bool notify = false;
    {
        std::scoped_lock lk(mtx_);
        if(state_.followLatest){
            r = { latest - state_.windowSeconds, latest };
            if(r != state_.visibleRange){
                state_.visibleRange = r;
                notify = true;
            }
            state_.playhead = latest;
        } else {
            return;
        }
    }
    if(notify) listeners_.call([&](ITimelineListener& x){
        x.timelineRangeChanged(r);
    });
    listeners_.call([&](ITimelineListener& x){
        x.timelinePlayheadChanged(latest);
    });
}

/// <summary>
/// Returns a copy of current timeline state.
/// </summary>
TimelineState TimelineController::getState() const {
    std::scoped_lock lk(mtx_);
    return state_;
}
/// <summary>
/// Returns current visible range.
/// </summary>
juce::Range<double> TimelineController::getVisibleRange() const {
    std::scoped_lock lk(mtx_);
    return state_.visibleRange;
}

} // namespace yvc::app
