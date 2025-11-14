// VoiVoi GUI Application - Scrolling Waveform Component Implementation
// License: GPLv3

#include "ScrollingWaveformComponent.h"
#include <algorithm>
#include <cmath>

namespace yvc::app {

ScrollingWaveformComponent::ScrollingWaveformComponent(){ setSize(400,120); }

void ScrollingWaveformComponent::setDisplayDurationSeconds(double seconds){ std::lock_guard<std::mutex> lk(mutex_); if(seconds > 0.25 && seconds < 120.0){ displaySeconds_ = seconds; ensureCapacityLocked(); } }

void ScrollingWaveformComponent::ensureCapacityLocked(){ size_t needed = static_cast<size_t>(std::ceil(displaySeconds_ * sampleRate_)); if(needed == 0) needed = 1; if(ring_.size() != needed){ ring_.assign(needed, 0.0f); writePos_ = 0; filled_ = 0; } }

void ScrollingWaveformComponent::computeAutoRangeLocked(){ if(!autoRange_){ return; } if(filled_ == 0){ minY_ = -1.0f; maxY_ = 1.0f; return; } float mn = ring_[0]; float mx = ring_[0]; size_t count = filled_; size_t start = (writePos_ + ring_.size() - count) % ring_.size(); for(size_t i=0;i<count;++i){ float v = ring_[(start + i) % ring_.size()]; if(v < mn) mn = v; if(v > mx) mx = v; } if(std::fabs(mx - mn) < 1e-6f){ mx = mn + 1.0f; } minY_ = mn; maxY_ = mx; }

void ScrollingWaveformComponent::pushSamples(const float* samples, size_t numSamples, float sr){ if(!samples || numSamples==0) return; std::lock_guard<std::mutex> lk(mutex_); if(sr > 0 && std::fabs(sr - sampleRate_) > 1.0f){ sampleRate_ = sr; ensureCapacityLocked(); }
    ensureCapacityLocked(); for(size_t i=0;i<numSamples;++i){ ring_[writePos_] = samples[i]; writePos_ = (writePos_ + 1) % ring_.size(); if(filled_ < ring_.size()) ++filled_; }
    computeAutoRangeLocked(); repaint(); }

void ScrollingWaveformComponent::paint(juce::Graphics& g){ auto b = getLocalBounds().toFloat(); g.setColour(juce::Colours::black); g.fillRect(b); g.setColour(juce::Colours::darkgrey); g.drawRect(b);
    std::vector<float> snapshot; float minY, maxY; {
        std::lock_guard<std::mutex> lk(mutex_); if(filled_ == 0){ return; } size_t count = filled_; snapshot.resize(count); size_t start = (writePos_ + ring_.size() - count) % ring_.size(); for(size_t i=0;i<count;++i){ snapshot[i] = ring_[(start + i) % ring_.size()]; } minY = minY_; maxY = maxY_; }
    juce::Path p; float w = b.getWidth(); float h = b.getHeight(); float range = (maxY - minY) + 1e-12f; size_t N = snapshot.size(); for(size_t i=0;i<N;++i){ float x = b.getX() + (float)i * w / (float)(N - 1); float norm = (snapshot[i] - minY) / range; norm = juce::jlimit(0.0f, 1.0f, norm); float y = b.getBottom() - norm * h; if(i==0) p.startNewSubPath(x,y); else p.lineTo(x,y); }
    g.setColour(juce::Colours::lightblue); g.strokePath(p, juce::PathStrokeType(1.1f));
    // draw center line (zero)
    float zeroNorm = (0.0f - minY) / range; float zeroY = b.getBottom() - juce::jlimit(0.0f,1.0f, zeroNorm) * h; g.setColour(juce::Colours::grey); g.drawLine(b.getX(), zeroY, b.getRight(), zeroY, 1.0f);
    // labels
    g.setColour(juce::Colours::white.withAlpha(0.7f)); g.setFont(10.0f); g.drawText(juce::String(minY,2), b.withHeight(14).withY(b.getBottom()-14), juce::Justification::left); g.drawText(juce::String(maxY,2), b.withHeight(14).withY(b.getY()), juce::Justification::left);
}

} // namespace yvc::app
