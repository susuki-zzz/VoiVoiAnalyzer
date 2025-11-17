// VoiVoi GUI Application - Enhanced Visualization Components
// License: GPLv3

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "yvc_core/Types.h"
#include <deque>
#include <vector>
#include <functional>
#include <cmath>
#include <algorithm>
#include "TimelineController.h"

namespace yvc::app {

namespace test { class AdvancedHeatmapComponentTestPeer; class ComparativeMetricsComponentTestPeer; class SpectrumAnalyzerComponentTestPeer; }

// ================= AdvancedHeatmapComponent =================
/// <summary>
/// High-performance heatmap component with zoom/scrub and export support.
/// Samples are appended and displayed in a fixed-size deque (oldest evicted).
/// Shift+Drag performs zoom range selection; mouse wheel zooms around cursor.
/// </summary>
class AdvancedHeatmapComponent : public juce::Component {
public:
    explicit AdvancedHeatmapComponent(const juce::String& title);
    /// <summary>Runtime configuration for value scaling and colours.</summary>
    struct HeatmapConfig { float minValue = 0.0f; float maxValue = 100.0f; juce::String unit = ""; juce::Colour lowColour = juce::Colours::blue; juce::Colour highColour = juce::Colours::red; bool showGrid = true; bool showTimestamps = true; int maxSamples = 300; };
    /// <summary>Applies a new configuration (repaints).</summary>
    void setConfig(const HeatmapConfig& config); 
    /// <summary>Appends a single sample (evicts oldest if beyond maxSamples).</summary>
    void appendSample(float value, double timestamp); 
    /// <summary>Clears all samples.</summary>
    void clear();
    /// <summary>Sets explicit zoom range in seconds.</summary>
    void setZoomRange(double startTime, double endTime); 
    /// <summary>Sets playhead marker position.</summary>
    void setPlayheadPosition(double timestamp); 
    /// <summary>Switch resolution mode (affects visual granularity).</summary>
    void setResolutionMode(bool highResolution);
    /// <summary>Creates an image export of the heatmap area.</summary>
    juce::Image createExportImage(int width, int height, bool includeAnnotations = true); 
    /// <summary>Exports heatmap snapshot to PNG.</summary>
    bool exportToPNG(const juce::File& file, int width = 1920, int height = 1080);
    /// <summary>Callback fired when zoom range changes.</summary>
    std::function<void(double,double)> onZoomRangeChanged; 
    /// <summary>Callback fired on scrub (click/drag) position changes.</summary>
    std::function<void(double)> onScrubPositionChanged;
    void paint(juce::Graphics& g) override; void resized() override; void mouseDown(const juce::MouseEvent& e) override; void mouseDrag(const juce::MouseEvent& e) override; void mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override;
protected:
    struct Sample { float value = 0.0f; double timestamp = 0.0; bool valid = true; };
    juce::String title_; HeatmapConfig config_; std::deque<Sample> samples_;
    double zoomStart_ = 0.0; double zoomEnd_ = 60.0; double playheadPosition_ = -1.0; bool highResolution_ = true; bool isZooming_ = false; juce::Point<int> zoomStartPoint_;
    juce::Rectangle<int> heatmapArea_; juce::Rectangle<int> timeAxis_; juce::Rectangle<int> valueAxis_;
    void drawHeatmapData(juce::Graphics& g, juce::Rectangle<int> area); void drawTimeAxis(juce::Graphics& g, juce::Rectangle<int> area); void drawValueAxis(juce::Graphics& g, juce::Rectangle<int> area); void drawPlayhead(juce::Graphics& g, juce::Rectangle<int> area); void drawZoomOverlay(juce::Graphics& g);
    juce::Colour getValueColour(float value) const; double timestampToX(double ts, juce::Rectangle<int> area) const; double xToTimestamp(int x, juce::Rectangle<int> area) const;
    friend class test::AdvancedHeatmapComponentTestPeer;
};

inline AdvancedHeatmapComponent::AdvancedHeatmapComponent(const juce::String& t):title_(t){ setConfig(HeatmapConfig{});} 
inline void AdvancedHeatmapComponent::setConfig(const HeatmapConfig& c){ config_=c; repaint(); }
inline void AdvancedHeatmapComponent::appendSample(float v,double ts){ samples_.push_back(Sample{v,ts,std::isfinite(v)}); while((int)samples_.size()>config_.maxSamples) samples_.pop_front(); if(!samples_.empty()){ double latest=samples_.back().timestamp; if(latest>zoomEnd_){ double span=zoomEnd_-zoomStart_; if(span<=0) span=1.0; zoomEnd_=latest; zoomStart_=zoomEnd_-span; if(onZoomRangeChanged) onZoomRangeChanged(zoomStart_,zoomEnd_);} } repaint(); }
inline void AdvancedHeatmapComponent::clear(){ samples_.clear(); repaint(); }
inline void AdvancedHeatmapComponent::setZoomRange(double s,double e){ zoomStart_=s; zoomEnd_=e; repaint(); }
inline void AdvancedHeatmapComponent::setPlayheadPosition(double ts){ playheadPosition_=ts; repaint(); }
inline void AdvancedHeatmapComponent::setResolutionMode(bool hr){ highResolution_=hr; repaint(); }
inline juce::Image AdvancedHeatmapComponent::createExportImage(int w,int h,bool ann){ juce::Image img(juce::Image::RGB,w,h,true); juce::Graphics g(img); g.fillAll(juce::Colours::black); g.setColour(juce::Colours::white); if(ann) g.drawText(title_,0,0,w,24,juce::Justification::centred); drawHeatmapData(g,juce::Rectangle<int>(0,ann?24:0,w,h-(ann?24:0))); return img; }
inline bool AdvancedHeatmapComponent::exportToPNG(const juce::File& f,int w,int h){ auto img=createExportImage(w,h,true); juce::PNGImageFormat fmt; juce::FileOutputStream out(f); return out.openedOk() && fmt.writeImageToStream(img,out); }
inline void AdvancedHeatmapComponent::paint(juce::Graphics& g){ auto area=getLocalBounds(); g.fillAll(juce::Colours::darkgrey.darker()); g.setColour(juce::Colours::white); g.drawText(title_, area.removeFromTop(24), juce::Justification::centred); heatmapArea_=area; drawHeatmapData(g,heatmapArea_); drawPlayhead(g,heatmapArea_); if(isZooming_) drawZoomOverlay(g); }
inline void AdvancedHeatmapComponent::resized(){ repaint(); }
inline void AdvancedHeatmapComponent::mouseDown(const juce::MouseEvent& e){ if(e.mods.isShiftDown()){ isZooming_=true; zoomStartPoint_=e.getPosition(); } else { double ts=xToTimestamp(e.x, heatmapArea_); if(onScrubPositionChanged) onScrubPositionChanged(ts);} }
inline void AdvancedHeatmapComponent::mouseDrag(const juce::MouseEvent& e){ if(isZooming_ && e.mods.isShiftDown()) repaint(); else if(e.mods.isLeftButtonDown()){ double ts=xToTimestamp(e.x, heatmapArea_); if(onScrubPositionChanged) onScrubPositionChanged(ts);} }
inline void AdvancedHeatmapComponent::mouseWheelMove(const juce::MouseEvent& e,const juce::MouseWheelDetails& wheel){ if(!heatmapArea_.contains(e.getPosition())) return; double span=zoomEnd_-zoomStart_; double factor=1.0-wheel.deltaY*0.2; span*=factor; double center=(zoomStart_+zoomEnd_)/2.0; zoomStart_=center-span/2.0; zoomEnd_=center+span/2.0; if(onZoomRangeChanged) onZoomRangeChanged(zoomStart_,zoomEnd_); repaint(); }
inline void AdvancedHeatmapComponent::drawHeatmapData(juce::Graphics& g, juce::Rectangle<int> area){ if(samples_.empty()){ g.setColour(juce::Colours::grey); g.drawText("(no data)", area, juce::Justification::centred); return;} g.setColour(juce::Colours::black); g.fillRect(area); double minT=zoomStart_, maxT=zoomEnd_; if(maxT<=minT) maxT=minT+1.0; for(auto&s: samples_){ if(s.timestamp<minT||s.timestamp>maxT) continue; float normT=float((s.timestamp-minT)/(maxT-minT)); float normV=juce::jlimit(0.0f,1.0f,(s.value-config_.minValue)/(config_.maxValue-config_.minValue)); int x=area.getX()+int(normT*area.getWidth()); int y=area.getBottom()-int(normV*area.getHeight()); g.setColour(getValueColour(s.value)); g.fillRect(x,y,2,2);} }
inline void AdvancedHeatmapComponent::drawTimeAxis(juce::Graphics&, juce::Rectangle<int>){ }
inline void AdvancedHeatmapComponent::drawValueAxis(juce::Graphics&, juce::Rectangle<int>){ }
inline void AdvancedHeatmapComponent::drawPlayhead(juce::Graphics& g, juce::Rectangle<int> area){ if(playheadPosition_>=zoomStart_ && playheadPosition_<=zoomEnd_){ float norm=float((playheadPosition_-zoomStart_)/(zoomEnd_-zoomStart_)); int x=area.getX()+int(norm*area.getWidth()); g.setColour(juce::Colours::yellow); g.drawLine((float)x,(float)area.getY(),(float)x,(float)area.getBottom(),1.5f);} }
inline void AdvancedHeatmapComponent::drawZoomOverlay(juce::Graphics& g){ g.setColour(juce::Colours::white.withAlpha(0.15f)); g.fillRect(heatmapArea_); }
inline juce::Colour AdvancedHeatmapComponent::getValueColour(float v) const{ float nv=juce::jlimit(0.0f,1.0f,(v-config_.minValue)/(config_.maxValue-config_.minValue)); return config_.lowColour.interpolatedWith(config_.highColour,nv);} inline double AdvancedHeatmapComponent::timestampToX(double ts, juce::Rectangle<int> area) const { if(zoomEnd_<=zoomStart_) return area.getX(); return area.getX()+(ts-zoomStart_)*(area.getWidth()/(zoomEnd_-zoomStart_)); } inline double AdvancedHeatmapComponent::xToTimestamp(int x, juce::Rectangle<int> area) const { return zoomStart_ + (double(x-area.getX())/double(area.getWidth()))*(zoomEnd_-zoomStart_); }

// ================= ComparativeMetricsComponent =================
/// <summary>
/// Plots selected metric over time for multiple recorded sessions for comparison.
/// Each session draws as a polyline over the chosen time range.
/// </summary>
class ComparativeMetricsComponent : public juce::Component, public ITimelineListener {
public:
    ComparativeMetricsComponent();
    /// <summary>Session container for comparison plotting.</summary>
    struct SessionData { juce::String name; juce::Colour colour; std::vector<yvc::AnalysisResults> data; float alpha = 1.0f; bool visible = true; };
    /// <summary>Adds a new session.</summary>
    void addSession(const juce::String& name, const std::vector<yvc::AnalysisResults>& data, juce::Colour colour = juce::Colours::white);
    /// <summary>Removes a session by name.</summary>
    void removeSession(const juce::String& name); 
    /// <summary>Sets visibility for a named session.</summary>
    void setSessionVisibility(const juce::String& name,bool visible); 
    /// <summary>Sets alpha blending value for a named session.</summary>
    void setSessionAlpha(const juce::String& name,float alpha); 
    /// <summary>Clears all sessions.</summary>
    void clearAllSessions();
    /// <summary>Changes the displayed metric key (e.g. "f0", "rms").</summary>
    void setDisplayedMetric(const juce::String& metricName); 
    /// <summary>Sets explicit time range for rendering.</summary>
    void setTimeRange(double startTime,double endTime);
    /// <summary>Assigns a timeline controller; auto-updates range.</summary>
    void setTimelineController(TimelineController* ctl) { timeline_ = ctl; if(timeline_) timeline_->addListener(this); }
    void timelineRangeChanged(const juce::Range<double>& r) override { setTimeRange(r.getStart(), r.getEnd()); }
    void timelinePlayheadChanged(double) override {}
    void paint(juce::Graphics& g) override; void resized() override;
protected:
    std::vector<SessionData> sessions_; juce::String currentMetric_="f0"; double timeStart_=0.0; double timeEnd_=60.0; TimelineController* timeline_ = nullptr;
    void drawSession(juce::Graphics& g,const SessionData& session, juce::Rectangle<int> area); float getMetricValue(const yvc::AnalysisResults& r,const juce::String& metric); juce::String getMetricUnit(const juce::String& metric); std::pair<float,float> getMetricRange(const juce::String& metric);
    friend class test::ComparativeMetricsComponentTestPeer;
};
inline ComparativeMetricsComponent::ComparativeMetricsComponent(){}
inline void ComparativeMetricsComponent::addSession(const juce::String& n,const std::vector<yvc::AnalysisResults>& d,juce::Colour c){ sessions_.push_back(SessionData{n,c,d,1.0f,true}); repaint(); }
inline void ComparativeMetricsComponent::removeSession(const juce::String& n){ sessions_.erase(std::remove_if(sessions_.begin(),sessions_.end(),[&](auto&s){return s.name==n;}),sessions_.end()); repaint(); }
inline void ComparativeMetricsComponent::setSessionVisibility(const juce::String& n,bool v){ for(auto& s: sessions_) if(s.name==n) s.visible=v; repaint(); }
inline void ComparativeMetricsComponent::setSessionAlpha(const juce::String& n,float a){ for(auto& s: sessions_) if(s.name==n) s.alpha=juce::jlimit(0.0f,1.0f,a); repaint(); }
inline void ComparativeMetricsComponent::clearAllSessions(){ sessions_.clear(); repaint(); }
inline void ComparativeMetricsComponent::setDisplayedMetric(const juce::String& m){ currentMetric_=m; repaint(); }
inline void ComparativeMetricsComponent::setTimeRange(double s,double e){ timeStart_=s; timeEnd_=e; repaint(); }
inline void ComparativeMetricsComponent::paint(juce::Graphics& g){ auto area=getLocalBounds(); g.fillAll(juce::Colours::black); g.setColour(juce::Colours::white); g.drawText("Comparative", area.removeFromTop(20), juce::Justification::centred); for(auto& ses: sessions_) if(ses.visible) drawSession(g,ses,area); }
inline void ComparativeMetricsComponent::resized(){ repaint(); }
inline void ComparativeMetricsComponent::drawSession(juce::Graphics& g,const SessionData& s,juce::Rectangle<int> area){ if(s.data.empty()) return; auto range=getMetricRange(currentMetric_); float minV=range.first, maxV=range.second; g.setColour(s.colour.withAlpha(s.alpha)); juce::Path p; bool started=false; for(auto& r: s.data){ if(r.timestamp<timeStart_||r.timestamp>timeEnd_) continue; float tNorm=float((r.timestamp-timeStart_)/(timeEnd_-timeStart_+1e-9)); float v=getMetricValue(r,currentMetric_); float vNorm=(v-minV)/(maxV-minV+1e-9f); float x=area.getX()+tNorm*area.getWidth(); float y=area.getBottom()-vNorm*area.getHeight(); if(!started){ p.startNewSubPath(x,y); started=true;} else p.lineTo(x,y);} g.strokePath(p, juce::PathStrokeType(1.2f)); }
inline float ComparativeMetricsComponent::getMetricValue(const yvc::AnalysisResults& r,const juce::String& m){ if(m=="f0") return r.f0; if(m=="rms") return r.rms; if(m=="cpp") return r.cpp; if(m=="hnr") return r.hnr; if(m=="speech_rate") return r.speech_rate; if(m=="pause_ratio") return r.pause_ratio; if(m=="spectral_tilt") return r.spectral_tilt; return r.rms; }
inline juce::String ComparativeMetricsComponent::getMetricUnit(const juce::String& m){ if(m=="f0") return "Hz"; if(m=="rms"||m=="cpp"||m=="hnr") return "dB"; if(m=="speech_rate") return "syll/s"; if(m=="pause_ratio") return "%"; if(m=="spectral_tilt") return "dB/oct"; return ""; }
inline std::pair<float,float> ComparativeMetricsComponent::getMetricRange(const juce::String& m){ if(m=="f0") return {50.0f,500.0f}; if(m=="rms") return {0.0f,1.0f}; if(m=="cpp") return {-10.0f,30.0f}; if(m=="hnr") return {0.0f,40.0f}; if(m=="speech_rate") return {0.0f,8.0f}; if(m=="pause_ratio") return {0.0f,1.0f}; if(m=="spectral_tilt") return {-24.0f,12.0f}; return {0.0f,100.0f}; }

// ================= SpectrumAnalyzerComponent =================
/// <summary>
/// Spectrum analyzer widget with F0 overlay and simple peak hold.
/// Displays log-frequency axis, magnitude (dB scaled) and optional harmonics.
/// </summary>
class SpectrumAnalyzerComponent : public juce::Component, public juce::Timer {
public:
    SpectrumAnalyzerComponent(); 
    /// <summary>Sets FFT size; reallocates buffers.</summary>
    void setFFTSize(int fftSize); 
    /// <summary>Sets sample rate for frequency mapping.</summary>
    void setSampleRate(float sampleRate); 
    /// <summary>Sets frequency display range.</summary>
    void setFrequencyRange(float minFreq,float maxFreq); 
    /// <summary>Enables peak hold curve overlay.</summary>
    void setPeakHoldEnabled(bool enabled); 
    /// <summary>Enables harmonic marker overlay lines.</summary>
    void setHarmonicsOverlayEnabled(bool enabled); 
    /// <summary>Updates magnitude spectrum (expects size >= fftSize_/2+1).</summary>
    void updateSpectrum(const float* magnitudeSpectrum,int spectrumSize); 
    /// <summary>Updates current F0 marker and validity.</summary>
    void updateF0(float f0,bool valid);
    int getFFTSize() const { return fftSize_; } float getCurrentF0() const { return currentF0_; } bool isF0Valid() const { return f0Valid_; } bool isHarmonicsOverlayEnabled() const { return harmonicsOverlayEnabled_; }
    float mapFrequencyToXPublic(float freq, juce::Rectangle<int> area) const { return frequencyToX(freq,area);} int mapFrequencyToBinPublic(float freq) const { return frequencyToBin(freq);} float mapMagnitudeToYPublic(float m, juce::Rectangle<int> area) const { return magnitudeToY(m,area);} void paint(juce::Graphics& g) override; void resized() override; void timerCallback() override;
protected:
    float frequencyToX(float freq, juce::Rectangle<int> area) const; int frequencyToBin(float freq) const; float magnitudeToY(float magnitude, juce::Rectangle<int> area) const; void drawSpectralTilt(juce::Graphics& g, juce::Rectangle<int> area);
    int fftSize_ = 2048; float sampleRate_ = 48000.0f; float minFreq_ = 20.0f; float maxFreq_ = 20000.0f; bool peakHoldEnabled_ = true; bool harmonicsOverlayEnabled_ = true; std::vector<float> currentSpectrum_; std::vector<float> peakSpectrum_; float currentF0_ = 0.0f; bool f0Valid_ = false; friend class test::SpectrumAnalyzerComponentTestPeer; };

} // namespace yvc::app
