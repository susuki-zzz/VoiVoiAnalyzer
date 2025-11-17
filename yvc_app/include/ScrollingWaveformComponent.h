// VoiVoi GUI Application - Scrolling Waveform Component
// License: GPLv3
// Purpose: Display a time vs amplitude scrolling view of recent input audio synchronised by a shared timeline.

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <vector>
#include <mutex>
#include "TimelineController.h"

namespace yvc::app {

/// <summary>
/// A simple time-domain viewer that renders the latest N seconds of mono audio.
/// - Backed by a fixed-size ring buffer (seconds × sampleRate).
/// - Uses TimelineController's visibleRange when available to map X coordinates.
/// </summary>
class ScrollingWaveformComponent : public juce::Component, public ITimelineListener
{
public:
    /// <summary>
    /// Constructs a scrolling waveform component.
    /// </summary>
    ScrollingWaveformComponent();

    /// <summary>
    /// Destructor.
    /// </summary>
    ~ScrollingWaveformComponent() override = default;

    /// <summary>
    /// Paints the component.
    /// </summary>
    /// <param name="g">Graphics context</param>
    void paint(juce::Graphics& g) override;

    /// <summary>
    /// Called when component is resized.
    /// </summary>
    void resized() override {}

    /// <summary>
    /// Pushes a block of mono samples with its end timestamp (seconds).
    /// </summary>
    /// <param name="samples">Pointer to audio samples</param>
    /// <param name="numSamples">Number of samples</param>
    /// <param name="sampleRate">Sample rate in Hz</param>
    /// <param name="blockEndTimestamp">Block end timestamp in seconds</param>
    void pushSamples(const float* samples, size_t numSamples, float sampleRate, double blockEndTimestamp);

    /// <summary>
    /// Pushes samples - backward compatibility when timestamp is unavailable.
    /// </summary>
    /// <param name="samples">Pointer to audio samples</param>
    /// <param name="numSamples">Number of samples</param>
    /// <param name="sampleRate">Sample rate in Hz</param>
    void pushSamples(const float* samples, size_t numSamples, float sampleRate)
    {
        pushSamples(samples, numSamples, sampleRate, -1.0);
    }

    /// <summary>
    /// Sets how many seconds of audio to retain and display.
    /// </summary>
    /// <param name="seconds">Display duration in seconds</param>
    void setDisplayDurationSeconds(double seconds);

    /// <summary>
    /// Sets vertical range manually; disables auto-range.
    /// </summary>
    /// <param name="minAmp">Minimum amplitude</param>
    /// <param name="maxAmp">Maximum amplitude</param>
    void setVerticalRange(float minAmp, float maxAmp)
    {
        std::lock_guard<std::mutex> lk(mutex_);
        minY_ = minAmp;
        maxY_ = maxAmp;
        autoRange_ = false;
        repaint();
    }

    /// <summary>
    /// Enables or disables automatic vertical range based on current buffer.
    /// </summary>
    /// <param name="enabled">True to enable auto-range</param>
    void setAutoRange(bool enabled)
    {
        std::lock_guard<std::mutex> lk(mutex_);
        autoRange_ = enabled;
        repaint();
    }

    /// <summary>
    /// Connects to a shared timeline. Component registers/unregisters as listener.
    /// </summary>
    /// <param name="ctl">Pointer to timeline controller</param>
    void setTimelineController(TimelineController* ctl)
    {
        std::lock_guard<std::mutex> lk(mutex_);
        timeline_ = ctl;
        if (timeline_)
            timeline_->addListener(this);
    }

    /// <summary>
    /// Called when timeline range changes.
    /// </summary>
    void timelineRangeChanged(const juce::Range<double>&) override { repaint(); }

    /// <summary>
    /// Called when timeline playhead changes.
    /// </summary>
    void timelinePlayheadChanged(double) override { repaint(); }

private:
    /// <summary>
    /// Ensures ring buffer has correct capacity for display duration.
    /// </summary>
    void ensureCapacityLocked();

    /// <summary>
    /// Computes automatic vertical range from current buffer content.
    /// </summary>
    void computeAutoRangeLocked();

    std::vector<float> ring_;
    size_t writePos_ = 0;
    size_t filled_ = 0;           // number of valid samples stored (<= ring_.size())
    double displaySeconds_ = 5.0; // default window length
    float sampleRate_ = 48000.0f;

    bool autoRange_ = true;
    float minY_ = -1.0f;
    float maxY_ = 1.0f;

    // Time mapping
    double firstTimestamp_ = 0.0; // timestamp at oldest sample retained
    double lastTimestamp_ = 0.0;  // block end timestamp of latest write

    std::mutex mutex_;
    TimelineController* timeline_ = nullptr;
};

} // namespace yvc::app
