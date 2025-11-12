#pragma once

#include "IPreprocessor.h"
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace yvc {

class PreprocessChain {
public:
    using ProcessorPtr = std::shared_ptr<IPreprocessor>;

    void addProcessor(const ProcessorPtr& processor, PreprocessTarget target);
    bool removeProcessor(const std::string& name);
    void clear();

    void process(const float* in, float* out, size_t n, PreprocessTarget target) const;
    size_t totalLatency(PreprocessTarget target) const;

    void setParams(const std::string& name, const std::unordered_map<std::string, float>& params);
    std::unordered_map<std::string, float> getParams(const std::string& name) const;
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
