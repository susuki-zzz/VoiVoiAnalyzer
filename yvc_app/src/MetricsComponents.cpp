// VoiVoi GUI Application - Metrics Visualization Components Implementation
// License: GPLv3
// Purpose: Render real-time analysis values (gauges/meters/heatmaps) with minimal logic.

#include "MetricsComponents.h"
#include <juce_graphics/juce_graphics.h>
#include <cmath>
#include <algorithm>

namespace yvc::app {

// ================= F0GaugeComponent =================
F0GaugeComponent::F0GaugeComponent() { setSize(140, 140); }

void F0GaugeComponent::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();
    g.setColour(juce::Colours::darkgrey.withAlpha(0.4f));
    g.fillRoundedRectangle(bounds, 8.0f);
    g.setColour(juce::Colours::white);
    g.setFont(14.0f);
    auto header = bounds.removeFromTop(22.0f);
    g.drawText("Pitch / Formants", header, juce::Justification::centredLeft);

    auto content = bounds.reduced(6.0f);
    g.setFont(20.0f);
    if (valid_) {
        g.setColour(juce::Colours::lightgreen);
        auto pitchArea = content.removeFromTop(30.0f);
        g.drawText(juce::String(currentF0_, 1) + " Hz", pitchArea, juce::Justification::centredLeft);
    } else {
        g.setColour(juce::Colours::red);
        auto pitchArea = content.removeFromTop(30.0f);
        g.drawText("No Pitch", pitchArea, juce::Justification::centredLeft);
    }

    g.setFont(12.0f);
    if (formantsValid_) {
        g.setColour(juce::Colours::cyan);
        auto a = content.removeFromTop(18.0f); g.drawText("F1: " + juce::String(f1_,0) + " Hz", a, juce::Justification::centredLeft);
        a = content.removeFromTop(18.0f); g.drawText("F2: " + juce::String(f2_,0) + " Hz", a, juce::Justification::centredLeft);
        a = content.removeFromTop(18.0f); g.drawText("F3: " + juce::String(f3_,0) + " Hz", a, juce::Justification::centredLeft);
        a = content.removeFromTop(18.0f); g.drawText("F4: " + juce::String(f4_,0) + " Hz", a, juce::Justification::centredLeft);
    } else {
        g.setColour(juce::Colours::grey);
        auto a = content.removeFromTop(18.0f);
        g.drawText("Formants unavailable", a, juce::Justification::centredLeft);
    }

    g.setColour(juce::Colours::yellow); g.setFont(11.0f);
    g.drawText("Target: " + juce::String(targetMin_,0) + "-" + juce::String(targetMax_,0) + " Hz", content.removeFromBottom(16.0f), juce::Justification::centredLeft);
}

void F0GaugeComponent::resized() {}

void F0GaugeComponent::update(const yvc::AnalysisResults& r) {
    currentF0_ = r.f0; valid_ = r.f0_valid;
    if (r.formants_valid) { f1_=r.f1; f2_=r.f2; f3_=r.f3; f4_=r.f4; formantsValid_=true; } else formantsValid_=false;
    repaint();
}

// ================= ScalarMeterComponent =================
ScalarMeterComponent::ScalarMeterComponent(Options opts, std::function<float(const yvc::AnalysisResults&)> getter, std::function<bool(const yvc::AnalysisResults&)> validityGetter)
    : options_(std::move(opts)), getter_(std::move(getter)), validityGetter_(std::move(validityGetter)) { setSize(80,120); }

void ScalarMeterComponent::paint(juce::Graphics& g) {
    auto b = getLocalBounds().toFloat();
    g.setColour(juce::Colours::darkgrey.withAlpha(0.3f)); g.fillRect(b);
    g.setColour(juce::Colours::white); g.setFont(12.0f); auto label = b.removeFromTop(20.0f); g.drawText(options_.label, label, juce::Justification::centred);
    auto meter = b.reduced(8.0f,4.0f); g.setColour(juce::Colours::black); g.fillRect(meter);
    if (isValid_) {
        float norm = (currentValue_ - options_.minimum) / (options_.maximum - options_.minimum); norm = juce::jlimit(0.0f,1.0f,norm);
        auto valueHeight = meter.getHeight()*norm; auto valueRect = meter.removeFromBottom(valueHeight);
        juce::Colour colour = juce::Colours::green; if(norm < 0.3f) colour=juce::Colours::blue; else if(norm>0.7f) colour=juce::Colours::orange;
        g.setColour(colour); g.fillRect(valueRect);
        if (options_.showBaseline) {
            float baseline = (options_.defaultValue - options_.minimum)/(options_.maximum - options_.minimum);
            if (baseline >=0 && baseline <=1) { float y = meter.getBottom() - baseline * meter.getHeight(); g.setColour(juce::Colours::yellow); g.fillRect(meter.getX(), y-1.0f, meter.getWidth(),2.0f);} }
        g.setColour(juce::Colours::white); g.setFont(10.0f); auto textArea = meter.removeFromBottom(15.0f);
        g.drawText(juce::String(currentValue_,1) + " " + options_.unit, textArea, juce::Justification::centred);
    } else { g.setColour(juce::Colours::red.withAlpha(0.5f)); g.drawText("N/A", meter, juce::Justification::centred); }
}

void ScalarMeterComponent::update(const yvc::AnalysisResults& r) { currentValue_ = getter_(r); isValid_ = validityGetter_ ? validityGetter_(r) : true; repaint(); }

// ================= VadMeterComponent =================
VadMeterComponent::VadMeterComponent(){ setSize(100,80); }

void VadMeterComponent::paint(juce::Graphics& g){ auto b=getLocalBounds().toFloat(); auto vad=b.removeFromTop(30.0f); g.setColour(voiceActive_?juce::Colours::green:juce::Colours::darkgrey); g.fillEllipse(vad.reduced(10.0f)); g.setColour(juce::Colours::white); g.setFont(12.0f); auto rate=b.removeFromTop(20.0f); g.drawText(juce::String(speechRate_,1)+" syl/s", rate, juce::Justification::centred); auto pause=b.removeFromTop(20.0f); g.drawText("Pause: "+juce::String((int)(pauseRatio_*100))+"%", pause, juce::Justification::centred); }

void VadMeterComponent::update(const yvc::AnalysisResults& r){ voiceActive_=r.voice_active; speechRate_=r.speech_rate; pauseRatio_=r.pause_ratio; repaint(); }

// ================= MetricsDisplayComponent =================
MetricsDisplayComponent::MetricsDisplayComponent(){ setSize(400,300); }

void MetricsDisplayComponent::setDisplayedMetrics(const std::vector<MetricDisplayType>& types){ activeTypes_=types; components_.clear(); for(auto t:activeTypes_){ auto c=createComponentFor(t); if(c){ components_.push_back(std::move(c)); addAndMakeVisible(components_.back().get()); } } resized(); }

void MetricsDisplayComponent::updateMetrics(const yvc::AnalysisResults& r){ for(auto& c:components_) if(c) c->update(r); }

void MetricsDisplayComponent::resized(){ auto area=getLocalBounds(); int perRow=4; int rows=(int(components_.size())+perRow-1)/perRow; if(rows>0){ int h=area.getHeight()/rows; int w=area.getWidth()/perRow; for(size_t i=0;i<components_.size(); ++i){ if(components_[i]){ int row=int(i)/perRow; int col=int(i)%perRow; components_[i]->setBounds(juce::Rectangle<int>(col*w,row*h,w,h).reduced(4)); } } } }

std::unique_ptr<MetricComponent> MetricsDisplayComponent::createComponentFor(MetricDisplayType t) { switch(t) { case MetricDisplayType::F0Gauge: return std::make_unique<F0GaugeComponent>(); case MetricDisplayType::CPP: { ScalarMeterComponent::Options o{ "CPP","dB",0,30,10,true }; return std::make_unique<ScalarMeterComponent>(o, [](auto& r) {return r.cpp; }); } case MetricDisplayType::HNR: { ScalarMeterComponent::Options o{ "HNR","dB",0,30,15,true }; return std::make_unique<ScalarMeterComponent>(o, [](auto& r) {return r.hnr; }); } case MetricDisplayType::SpectralTilt: { ScalarMeterComponent::Options o{ "Tilt","dB/oct",-20,5,-6,true }; return std::make_unique<ScalarMeterComponent>(o, [](auto& r) {return r.spectral_tilt; }); } case MetricDisplayType::SpeechRate: { ScalarMeterComponent::Options o{ "Speech","syl/s",0,10,4,false }; return std::make_unique<ScalarMeterComponent>(o, [](auto& r) {return r.speech_rate; }); } case MetricDisplayType::PauseRatio: { ScalarMeterComponent::Options o{ "Pause","%",0,1,0.3f,false }; return std::make_unique<ScalarMeterComponent>(o, [](auto& r) {return r.pause_ratio; }); } case MetricDisplayType::VoiceActivity: return std::make_unique<VadMeterComponent>(); case MetricDisplayType::RMS: { ScalarMeterComponent::Options o{ "RMS","",0,1,0.2f,false }; return std::make_unique<ScalarMeterComponent>(o, [](auto& r) {return r.rms; }); } case MetricDisplayType::Peak: { ScalarMeterComponent::Options o{ "Peak","",0,1,0.5f,false }; return std::make_unique<ScalarMeterComponent>(o, [](auto& r) {return r.peak; }); } case MetricDisplayType::CrestFactor: { ScalarMeterComponent::Options o{ "Crest","",1,5,1.4f,false }; return std::make_unique<ScalarMeterComponent>(o, [](auto& r) {return r.crest_factor; }); } case MetricDisplayType::SCentroid: { ScalarMeterComponent::Options o{ "S-Cent","Hz",2000,8000,4000,false }; return std::make_unique<ScalarMeterComponent>(o, [](auto& r) {return r.s_centroid; }, [](auto& r) {return r.s_detected; }); } default: return nullptr; } }

// ================= HeatmapComponent =================
HeatmapComponent::HeatmapComponent(){ setSize(200,300); }

void HeatmapComponent::appendSample(const yvc::AnalysisResults& results) {
    timeHistory_.push_back(results.timestamp);
    f0History_.push_back(results.f0_valid ? results.f0 : 0.0f);
    f0ConfHistory_.push_back(results.f0_valid ? juce::jlimit(0.0f,1.0f, results.f0_confidence) : 0.0f);
    rmsHistory_.push_back(results.rms);
    if (results.formants_valid) {
        f1History_.push_back(results.f1);
        f2History_.push_back(results.f2);
        f3History_.push_back(results.f3);
        f4History_.push_back(results.f4);
    } else {
        f1History_.push_back(0.0f);
        f2History_.push_back(0.0f);
        f3History_.push_back(0.0f);
        f4History_.push_back(0.0f);
    }
    auto trim = [this](auto& dq){ while ((int)dq.size() > maxSamples_) dq.pop_front(); };
    trim(timeHistory_); trim(f0History_); trim(f0ConfHistory_); trim(rmsHistory_); trim(f1History_); trim(f2History_); trim(f3History_); trim(f4History_);
    repaint();
}

float HeatmapComponent::mapFrequencyToY(float f,float minF,float maxF,float h) const { if(f<=0) return h; switch(scaleMode_){ case ScaleMode::LinearHz:{ float n=(f-minF)/(maxF-minF); return juce::jlimit(0.0f,1.0f,n)*h; } case ScaleMode::LogHz:{ float lf=std::log10(std::max(1.0f,f)); float lmin=std::log10(std::max(1.0f,minF)); float lmax=std::log10(std::max(1.0f,maxF)); float n=(lf-lmin)/(lmax-lmin); return juce::jlimit(0.0f,1.0f,n)*h; } case ScaleMode::MidiNote:{ float midi=69.0f+12.0f*std::log2(f/440.0f); float minMidi=69.0f+12.0f*std::log2(minF/440.0f); float maxMidi=69.0f+12.0f*std::log2(maxF/440.0f); float n=(midi-minMidi)/(maxMidi-minMidi); return juce::jlimit(0.0f,1.0f,n)*h; } } return h; }

juce::String HeatmapComponent::formatAxisLabel(float f) const { switch(scaleMode_){ case ScaleMode::LinearHz: case ScaleMode::LogHz: return juce::String((int)f)+" Hz"; case ScaleMode::MidiNote:{ float midi=69.0f+12.0f*std::log2(f/440.0f); return juce::String((int)midi)+" MIDI"; } } return {}; }

void HeatmapComponent::paint(juce::Graphics& g){ auto bounds=getLocalBounds().toFloat(); auto f0Area=bounds.removeFromTop(bounds.getHeight()*0.7f); auto rmsArea=bounds; g.setColour(juce::Colours::black); g.fillRect(f0Area); g.setColour(juce::Colours::darkgrey); g.drawRect(f0Area,1.0f); const float minF=60.0f; const float maxF=4000.0f; 
    // Axis lines with overlap avoidance
    g.setColour(juce::Colours::grey);
    float lastLabelY = -1e9f; const float labelH = 12.0f;
    auto emitLabel = [&](float fMarker){ float y=f0Area.getBottom()-mapFrequencyToY(fMarker,minF,maxF,f0Area.getHeight()); g.drawHorizontalLine((int)std::round(y), f0Area.getX(), f0Area.getRight()); if (std::abs(y-lastLabelY) < (labelH+4.0f)) return; g.setColour(juce::Colours::white); g.setFont(10.0f); g.drawText(formatAxisLabel(fMarker), f0Area.withHeight(16).withY(y-8).removeFromLeft(60), juce::Justification::left); g.setColour(juce::Colours::grey); lastLabelY=y; };
    if (scaleMode_ == ScaleMode::LogHz) {
        std::vector<float> major = { 100,200,300,500,1000,2000,3000,4000 };
        for (auto f: major) if (f>=minF && f<=maxF) emitLabel(f);
    } else {
        for(int i=0;i<=6;++i){ float fMarker=minF + (maxF-minF) * (i/6.0f); emitLabel(fMarker);} }
    // F0 with intensity colour using confidence and timeline mapping
    juce::Range<double> range{0.0, 0.0}; if(timeline_) range = timeline_->getVisibleRange();
    if(!f0History_.empty() && !timeHistory_.empty()){
        int N = (int)std::min(f0History_.size(), timeHistory_.size());
        for(int i=0;i<N;++i){ double ts = timeHistory_[i]; if(range.getLength()>0 && (ts < range.getStart() || ts > range.getEnd())) continue; float f=f0History_[i]; float y=f0Area.getBottom()-mapFrequencyToY(f,minF,maxF,f0Area.getHeight()); float tnorm = range.getLength()>0 ? float((ts - range.getStart())/range.getLength()) : float(i)/float(N-1); float x = f0Area.getX() + tnorm * f0Area.getWidth(); float conf = (i < (int)f0ConfHistory_.size()) ? f0ConfHistory_[i] : 0.0f; juce::Colour c = juce::Colour::fromHSV(juce::jlimit(0.0f,1.0f,0.66f - 0.66f*conf), 0.9f, 0.9f, 0.6f); g.setColour(c); g.fillRect(juce::Rectangle<float>(x, y, 2.0f, 2.0f)); }
    }
    // Formants overlay
    auto drawFormant=[&](const std::deque<float>& hist, juce::Colour col){ if(hist.empty()) return; juce::Path p; bool started=false; int N=(int)std::min(hist.size(), timeHistory_.size()); for(int i=0;i<N; ++i){ float f=hist[i]; if(f<=0) continue; double ts=timeHistory_[i]; if(range.getLength()>0 && (ts<range.getStart()||ts>range.getEnd())) continue; float y=f0Area.getBottom()-mapFrequencyToY(f,minF,maxF,f0Area.getHeight()); float tnorm = range.getLength()>0 ? float((ts - range.getStart())/range.getLength()) : float(i)/float(N-1); float x=f0Area.getX()+ tnorm*f0Area.getWidth(); if(!started){ p.startNewSubPath(x,y); started=true;} else p.lineTo(x,y);} g.setColour(col); g.strokePath(p, juce::PathStrokeType(1.4f)); };
    drawFormant(f1History_, juce::Colours::cyan); drawFormant(f2History_, juce::Colours::green); drawFormant(f3History_, juce::Colours::orange); drawFormant(f4History_, juce::Colours::magenta); 
    // RMS heatmap
    drawHeatmap(g, rmsArea, rmsHistory_, timeHistory_, -60.0f, 0.0f, "RMS", "dBFS"); }

void HeatmapComponent::resized(){ setMaxSamples(getWidth()/2); }

void HeatmapComponent::drawHeatmap(juce::Graphics& g, juce::Rectangle<float> area, const std::deque<float>& samples, const std::deque<double>& times, float minValue, float maxValue, const juce::String& label, const juce::String& unit){ g.setColour(juce::Colours::black); g.fillRect(area); g.setColour(juce::Colours::darkgrey); g.drawRect(area,1.0f); g.setColour(juce::Colours::white); g.setFont(12.0f); auto labelArea=area.removeFromTop(20.0f); g.drawText(label+" ("+unit+")", labelArea.reduced(4.0f), juce::Justification::centredLeft); if(samples.empty()||times.empty()) return; juce::Range<double> range{0.0,0.0}; if(timeline_) range = timeline_->getVisibleRange(); int N=(int)std::min(samples.size(), times.size()); for(int i=0;i<N; ++i){ double ts=times[i]; if(range.getLength()>0 && (ts<range.getStart()||ts>range.getEnd())) continue; float v=samples[i]; float norm=(v-minValue)/(maxValue-minValue); norm=juce::jlimit(0.0f,1.0f,norm); juce::Colour c; if(norm<0.33f) c=juce::Colours::blue.interpolatedWith(juce::Colours::green,norm*3.0f); else if(norm<0.66f) c=juce::Colours::green.interpolatedWith(juce::Colours::yellow,(norm-0.33f)*3.0f); else c=juce::Colours::yellow.interpolatedWith(juce::Colours::red,(norm-0.66f)*3.0f); float tnorm = range.getLength()>0 ? float((ts - range.getStart())/range.getLength()) : float(i)/float(N-1); float x=area.getX()+ tnorm * area.getWidth(); g.setColour(c); g.fillRect(juce::Rectangle<float>(x, area.getY(), 2.0f, area.getHeight())); } }

} // namespace yvc::app
