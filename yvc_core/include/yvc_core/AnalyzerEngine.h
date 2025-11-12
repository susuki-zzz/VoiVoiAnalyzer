#pragma once

#include "AudioBuffer.h"
#include "PreprocessChain.h"
#include <memory>
#include <unordered_map>

namespace yvc {

class AnalyzerEngine {
public:
    explicit AnalyzerEngine(const AudioConfig& config);

    void addPreprocessor(const PreprocessChain::ProcessorPtr& processor, PreprocessTarget targets);
    void removePreprocessor(const std::string& name);
    void setPreprocessorParams(const std::string& name, const std::unordered_map<std::string, float>& params);

    void pushInput(const Sample* samples, size_t num_samples);

    void renderMonitor(const Sample* in, Sample* out, size_t num_samples) const;
    void renderRecord(const Sample* in, Sample* out, size_t num_samples) const;

    size_t latencyForTarget(PreprocessTarget target) const;

    const AudioBuffer& analysisBuffer() const { return analysis_buffer_; }
    void clearAnalysis();

    void setPreprocessChain(std::shared_ptr<PreprocessChain> chain);

private:
    AudioBuffer analysis_buffer_;
    std::shared_ptr<PreprocessChain> chain_;
};

} // namespace yvc
