#pragma once

#include "IPreprocessor.h"
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace yvc {

/// <summary>
/// Chain of audio preprocessors with target-specific routing.
/// Preprocessors affect monitoring/recording paths only, not analysis.
/// </summary>
class PreprocessChain {
public:
    using ProcessorPtr = std::shared_ptr<IPreprocessor>;

    /// <summary>
    /// Adds a preprocessor to the chain with specified target.
    /// </summary>
    /// <param name="processor">Shared pointer to preprocessor</param>
    /// <param name="target">Target routing flags</param>
    void addProcessor(const ProcessorPtr& processor, PreprocessTarget target);

    /// <summary>
    /// Removes a preprocessor by name.
    /// </summary>
    /// <param name="name">Preprocessor name</param>
    /// <returns>True if removed, false if not found</returns>
    bool removeProcessor(const std::string& name);

    /// <summary>
    /// Clears all preprocessors from the chain.
    /// </summary>
    void clear();

    /// <summary>
    /// Processes audio through the chain for specified target.
    /// </summary>
    /// <param name="in">Input audio samples</param>
    /// <param name="out">Output audio samples</param>
    /// <param name="n">Number of samples</param>
    /// <param name="target">Target routing (Monitor/Record)</param>
    void process(const float* in, float* out, size_t n, PreprocessTarget target) const;

    /// <summary>
    /// Calculates total latency for specified target.
    /// </summary>
    /// <param name="target">Target routing</param>
    /// <returns>Total latency in samples</returns>
    size_t totalLatency(PreprocessTarget target) const;

    /// <summary>
    /// Sets parameters for a named preprocessor.
    /// </summary>
    /// <param name="name">Preprocessor name</param>
    /// <param name="params">Parameter key-value map</param>
    void setParams(const std::string& name, const std::unordered_map<std::string, float>& params);

    /// <summary>
    /// Gets parameters from a named preprocessor.
    /// </summary>
    /// <param name="name">Preprocessor name</param>
    /// <returns>Parameter key-value map</returns>
    std::unordered_map<std::string, float> getParams(const std::string& name) const;

    /// <summary>
    /// Gets routing targets for a named preprocessor.
    /// </summary>
    /// <param name="name">Preprocessor name</param>
    /// <returns>Target routing flags</returns>
    PreprocessTarget getTargets(const std::string& name) const;

private:
    struct Node {
        ProcessorPtr processor;
        PreprocessTarget target;
        size_t latency = 0;
    };

    Node* findNode(const std::string& name);
    const Node* findNode(const std::string& name) const;
    void refreshLatency(Node& node);

    mutable std::mutex mutex_;
    std::vector<Node> chain_;
};

} // namespace yvc
