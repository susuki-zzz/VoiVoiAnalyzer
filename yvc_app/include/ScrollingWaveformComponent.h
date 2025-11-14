// VoiVoi GUI Application - Scrolling Waveform Component
// License: GPLv3
// Purpose: Display a time vs amplitude scrolling view of recent input audio.

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <vector>
#include <mutex>
#include "TimelineController.h"

namespace yvc::app {

class ScrollingWaveformComponent : public juce::Component, public ITimelineListener {
public:
    ScrollingWaveformComponent();
    ~ScrollingWaveformComponent() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override {}

    // Push a block of mono samples (float32) at block end timestamp (seconds).
    void pushSamples(const float* samples, size_t numSamples, float sampleRate, double blockEndTimestamp);

    // Backward compatibility; if timestamp is unavailable, just append and free-run.
    void pushSamples(const float* samples, size_t numSamples, float sampleRate) { pushSamples(samples, numSamples, sampleRate, -1.0); }

    // Set how many seconds of audio to retain & display (reallocates buffer).
    void setDisplayDurationSeconds(double seconds);

    // Set vertical range manually (otherwise fixed to [-1,1]).
    void setVerticalRange(float minAmp, float maxAmp) { std::lock_guard<std::mutex> lk(mutex_); minY_ = minAmp; maxY_ = maxAmp; autoRange_ = false; repaint(); }

    // Enable / disable automatic vertical range adaptation based on current buffer.
    void setAutoRange(bool enabled) { std::lock_guard<std::mutex> lk(mutex_); autoRange_ = enabled; repaint(); }

    // Timeline
    void setTimelineController(TimelineController* ctl) { std::lock_guard<std::mutex> lk(mutex_); timeline_ = ctl; if(timeline_) timeline_->addListener(this); }
    void timelineRangeChanged(const juce::Range<double>&) override { repaint(); }
    void timelinePlayheadChanged(double) override { repaint(); }

private:
    void ensureCapacityLocked();
    void computeAutoRangeLocked();

    std::vector<float> ring_;
    size_t writePos_ = 0;
    size_t filled_ = 0;       // number of valid samples stored (<= ring_.size())
    double displaySeconds_ = 5.0; // default window length
    float sampleRate_ = 48000.0f;

    bool autoRange_ = true;
    float minY_ = -1.0f;
    float maxY_ = 1.0f;

    // Time mapping
    double firstTimestamp_ = 0.0; // timestamp at ring_[0] for current logical order when full
    double lastTimestamp_ = 0.0;  // block end timestamp of latest write

    std::mutex mutex_;
    TimelineController* timeline_ = nullptr;
};

} // namespace yvc::app
