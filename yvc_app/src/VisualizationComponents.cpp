// VoiVoi GUI Application - Visualization Components (implementations)
// License: GPLv3

#include "VisualizationComponents.h"
#include <juce_graphics/juce_graphics.h>
#include <cmath>
#include <algorithm>

namespace yvc::app {

// AdvancedHeatmapComponent now fully inline in header (no out-of-line definitions here)
// ComparativeMetricsComponent also inline.

// ================= SpectrumAnalyzerComponent =================
SpectrumAnalyzerComponent::SpectrumAnalyzerComponent(){ currentSpectrum_.resize(fftSize_/2+1); peakSpectrum_.resize(fftSize_/2+1); startTimerHz(30); }
void SpectrumAnalyzerComponent::setFFTSize(int s){ if(s>32){ fftSize_=s; currentSpectrum_.assign(fftSize_/2+1,0.0f); peakSpectrum_.assign(fftSize_/2+1,0.0f); repaint(); }}
void SpectrumAnalyzerComponent::setSampleRate(float sr){ if(sr>0) sampleRate_=sr; }
void SpectrumAnalyzerComponent::setFrequencyRange(float mi,float ma){ if(mi>0 && ma>mi){ minFreq_=mi; maxFreq_=ma; repaint(); }}
void SpectrumAnalyzerComponent::setPeakHoldEnabled(bool e){ peakHoldEnabled_=e; if(!e) peakSpectrum_.assign(peakSpectrum_.size(),0.0f); }
void SpectrumAnalyzerComponent::setHarmonicsOverlayEnabled(bool e){ harmonicsOverlayEnabled_=e; repaint(); }
void SpectrumAnalyzerComponent::updateSpectrum(const float* mag,int size){ if(!mag||size<=0) return; int n=std::min(size,(int)currentSpectrum_.size()); for(int i=0;i<n;++i){ currentSpectrum_[i]=mag[i]; if(peakHoldEnabled_) peakSpectrum_[i]=std::max(peakSpectrum_[i],currentSpectrum_[i]); } repaint(); }
void SpectrumAnalyzerComponent::updateF0(float f0,bool valid){ currentF0_=f0; f0Valid_=valid; repaint(); }
void SpectrumAnalyzerComponent::paint(juce::Graphics& g){ auto area=getLocalBounds(); g.fillAll(juce::Colours::black); g.setColour(juce::Colours::white); g.drawRect(area); if(currentSpectrum_.empty()){ g.drawText("No spectrum", area, juce::Justification::centred); return; } auto plot=area.reduced(4); juce::Path p; bool started=false; for(int i=0;i<(int)currentSpectrum_.size();++i){ float freq=float(i)*sampleRate_/fftSize_; if(freq<minFreq_) continue; if(freq>maxFreq_) break; float x=frequencyToX(freq,plot); float y=magnitudeToY(currentSpectrum_[i],plot); if(!started){ p.startNewSubPath(x,y); started=true; } else p.lineTo(x,y); } g.setColour(juce::Colours::lightgreen); g.strokePath(p, juce::PathStrokeType(1.0f)); if(peakHoldEnabled_){ juce::Path pp; started=false; for(int i=0;i<(int)peakSpectrum_.size();++i){ float freq=float(i)*sampleRate_/fftSize_; if(freq<minFreq_) continue; if(freq>maxFreq_) break; float x=frequencyToX(freq,plot); float y=magnitudeToY(peakSpectrum_[i],plot); if(!started){ pp.startNewSubPath(x,y); started=true; } else pp.lineTo(x,y); } g.setColour(juce::Colours::orange.withAlpha(0.5f)); g.strokePath(pp, juce::PathStrokeType(1.0f)); }
    drawSpectralTilt(g, plot); if(harmonicsOverlayEnabled_ && f0Valid_ && currentF0_>0){ g.setColour(juce::Colours::yellow); for(int h=1; h<=10; ++h){ float hf=currentF0_*h; if(hf>maxFreq_) break; float x=frequencyToX(hf,plot); g.drawVerticalLine((int)std::round(x), plot.getY(), plot.getBottom()); }} if(f0Valid_ && currentF0_>=minFreq_ && currentF0_<=maxFreq_){ float x=frequencyToX(currentF0_,plot); g.setColour(juce::Colours::red); g.drawLine(x, plot.getY(), x, plot.getBottom(), 2.0f); }}
void SpectrumAnalyzerComponent::resized(){}
void SpectrumAnalyzerComponent::timerCallback(){ repaint(); }
float SpectrumAnalyzerComponent::frequencyToX(float freq, juce::Rectangle<int> area) const { float cl=juce::jlimit(minFreq_,maxFreq_,freq); float norm=std::log10(cl/minFreq_)/std::log10(maxFreq_/minFreq_); return area.getX()+norm*area.getWidth(); }
int SpectrumAnalyzerComponent::frequencyToBin(float freq) const { float cl=juce::jlimit(minFreq_,maxFreq_,freq); return juce::jlimit(0, fftSize_/2, (int)std::round(cl*(fftSize_/2)/(sampleRate_/2.0f))); }
float SpectrumAnalyzerComponent::magnitudeToY(float mag, juce::Rectangle<int> area) const { float db=20.0f*std::log10(std::max(mag,1e-9f)); float norm=juce::jlimit(0.0f,1.0f,(db+100.0f)/100.0f); return area.getBottom()-norm*area.getHeight(); }
void SpectrumAnalyzerComponent::drawSpectralTilt(juce::Graphics& g, juce::Rectangle<int> area){ if(currentSpectrum_.size()<4) return; std::vector<float> xs; std::vector<float> ys; for(int i=1;i<(int)currentSpectrum_.size();++i){ float f=float(i)*sampleRate_/fftSize_; if(f<minFreq_||f>maxFreq_) continue; xs.push_back(std::log10(f)); ys.push_back(20.0f*std::log10(std::max(currentSpectrum_[i],1e-9f))); } if(xs.size()<4) return; float mx=0,my=0; for(size_t i=0;i<xs.size();++i){ mx+=xs[i]; my+=ys[i]; } mx/=xs.size(); my/=xs.size(); float num=0,den=0; for(size_t i=0;i<xs.size();++i){ num+=(xs[i]-mx)*(ys[i]-my); den+=(xs[i]-mx)*(xs[i]-mx);} float slope= den>0? num/den:0; float y1=magnitudeToY(std::pow(10.0f,(my - slope*(mx - std::log10(minFreq_)))/20.0f), area); float y2=magnitudeToY(std::pow(10.0f,(my + slope*(std::log10(maxFreq_) - mx))/20.0f), area); g.setColour(juce::Colours::aqua.withAlpha(0.6f)); g.drawLine(area.getX(), y1, area.getRight(), y2, 1.2f); }

} // namespace yvc::app
