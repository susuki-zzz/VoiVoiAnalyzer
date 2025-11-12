#include "yvc_core/AnalyzerEngine.h"

#include <algorithm>
#include <cstring>

namespace yvc {

AnalyzerEngine::AnalyzerEngine(const AudioConfig& config)
    : analysis_buffer_(config), chain_(std::make_shared<PreprocessChain>()) {}

void AnalyzerEngine::addPreprocessor(const PreprocessChain::ProcessorPtr& processor, PreprocessTarget targets) {
    if (!chain_) {
        chain_ = std::make_shared<PreprocessChain>();
    }
    chain_->addProcessor(processor, targets);
    analysis_buffer_.setLatencyCompensation(PreprocessTarget::Monitor, chain_->totalLatency(PreprocessTarget::Monitor));
    analysis_buffer_.setLatencyCompensation(PreprocessTarget::Record, chain_->totalLatency(PreprocessTarget::Record));
}

void AnalyzerEngine::removePreprocessor(const std::string& name) {
    if (!chain_) {
        return;
    }
    chain_->removeProcessor(name);
    analysis_buffer_.setLatencyCompensation(PreprocessTarget::Monitor, chain_->totalLatency(PreprocessTarget::Monitor));
    analysis_buffer_.setLatencyCompensation(PreprocessTarget::Record, chain_->totalLatency(PreprocessTarget::Record));
}

void AnalyzerEngine::setPreprocessorParams(const std::string& name, const std::unordered_map<std::string, float>& params) {
    if (!chain_) {
        return;
    }
    chain_->setParams(name, params);
    analysis_buffer_.setLatencyCompensation(PreprocessTarget::Monitor, chain_->totalLatency(PreprocessTarget::Monitor));
    analysis_buffer_.setLatencyCompensation(PreprocessTarget::Record, chain_->totalLatency(PreprocessTarget::Record));
}

void AnalyzerEngine::pushInput(const Sample* samples, size_t num_samples) {
    if (!samples || num_samples == 0) {
        return;
    }
    analysis_buffer_.addSamples(samples, num_samples);
}

void AnalyzerEngine::renderMonitor(const Sample* in, Sample* out, size_t num_samples) const {
    if (!chain_) {
        if (in && out && in != out) {
            std::memcpy(out, in, num_samples * sizeof(Sample));
        }
        return;
    }
    chain_->process(in, out, num_samples, PreprocessTarget::Monitor);
}

void AnalyzerEngine::renderRecord(const Sample* in, Sample* out, size_t num_samples) const {
    if (!chain_) {
        if (in && out && in != out) {
            std::memcpy(out, in, num_samples * sizeof(Sample));
        }
        return;
    }
    chain_->process(in, out, num_samples, PreprocessTarget::Record);
}

size_t AnalyzerEngine::latencyForTarget(PreprocessTarget target) const {
    if (!chain_) {
        return 0;
    }
    return chain_->totalLatency(target);
}

void AnalyzerEngine::clearAnalysis() {
    analysis_buffer_.clear();
}

void AnalyzerEngine::setPreprocessChain(std::shared_ptr<PreprocessChain> chain) {
    chain_ = std::move(chain);
    if (!chain_) {
        analysis_buffer_.clearLatencyCompensation();
        return;
    }
    analysis_buffer_.setLatencyCompensation(PreprocessTarget::Monitor, chain_->totalLatency(PreprocessTarget::Monitor));
    analysis_buffer_.setLatencyCompensation(PreprocessTarget::Record, chain_->totalLatency(PreprocessTarget::Record));
}

} // namespace yvc
