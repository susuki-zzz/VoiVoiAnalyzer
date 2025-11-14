// VoiVoi GUI Application - Waveform Component Implementation
// License: GPLv3

#include "WaveformComponent.h"
#include <cmath>

namespace yvc::app {

WaveformComponent::WaveformComponent(){ setSize(400,120); }

void WaveformComponent::setSamples(const std::vector<float>& samples, float sampleRate){
    samples_ = samples; sampleRate_ = sampleRate;
    if (samples_.empty()) { minY_=-1.0f; maxY_=1.0f; repaint(); return; }
    float mn = samples_[0]; float mx = samples_[0];
    for(size_t i=1;i<samples_.size(); ++i){ float v=samples_[i]; if(v<mn) mn=v; if(v>mx) mx=v; }
    minY_ = mn; maxY_ = mx; if (std::fabs(maxY_-minY_)<1e-6f) maxY_ = minY_ + 1.0f; repaint();
}

void WaveformComponent::paint(juce::Graphics& g){
    auto b = getLocalBounds().toFloat();
    g.setColour(juce::Colours::black); g.fillRect(b);
    g.setColour(juce::Colours::darkgrey); g.drawRect(b);
    if(samples_.empty()) return; size_t N=samples_.size(); if(N<2) return;
    juce::Path p; float w=b.getWidth(); float h=b.getHeight(); float range=(maxY_-minY_)+1e-12f;
    for(size_t i=0;i<N;++i){ float x=b.getX()+ (float)i*w/(float)(N-1); float norm=(samples_[i]-minY_)/range; norm=juce::jlimit(0.0f,1.0f,norm); float y=b.getBottom()-norm*h; if(i==0) p.startNewSubPath(x,y); else p.lineTo(x,y);} g.setColour(juce::Colours::lightgreen); g.strokePath(p, juce::PathStrokeType(1.2f));
}

} // namespace yvc::app
