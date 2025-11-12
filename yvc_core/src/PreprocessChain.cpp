#include "yvc_core/PreprocessChain.h"

#include <algorithm>
#include <cstring>

namespace yvc {
namespace {
inline void copyBuffer(const float* in, float* out, size_t n) {
    if (in == out) {
        return;
    }
    std::memcpy(out, in, n * sizeof(float));
}
} // namespace

void PreprocessChain::addProcessor(const ProcessorPtr& processor, PreprocessTarget target) {
    if (!processor || !any(target)) {
        return;
    }

    const size_t latency = static_cast<size_t>(std::max(0, processor->latency_samples()));
    std::lock_guard<std::mutex> lock(mutex_);
    Node node{processor, target, latency};
    chain_.push_back(std::move(node));
}

bool PreprocessChain::removeProcessor(const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = std::remove_if(chain_.begin(), chain_.end(), [&](const Node& node) {
        return node.processor && node.processor->name() == name;
    });
    if (it == chain_.end()) {
        return false;
    }
    chain_.erase(it, chain_.end());
    return true;
}

void PreprocessChain::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    chain_.clear();
}

void PreprocessChain::process(const float* in, float* out, size_t n, PreprocessTarget target) const {
    if (!in || !out || n == 0) {
        return;
    }

    std::vector<ProcessorPtr> processors;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        processors.reserve(chain_.size());
        for (const auto& node : chain_) {
            if (!node.processor || !matches(node.target, target)) {
                continue;
            }
            processors.push_back(node.processor);
        }
    }

    copyBuffer(in, out, n);

    for (const auto& processor : processors) {
        processor->process(out, out, n);
    }
}

size_t PreprocessChain::totalLatency(PreprocessTarget target) const {
    std::lock_guard<std::mutex> lock(mutex_);
    size_t total = 0;
    for (const auto& node : chain_) {
        if (!node.processor || !matches(node.target, target)) {
            continue;
        }
        total += node.latency;
    }
    return total;
}

void PreprocessChain::setParams(const std::string& name, const std::unordered_map<std::string, float>& params) {
    ProcessorPtr processor;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (Node* node = findNode(name)) {
            processor = node->processor;
        }
    }

    if (!processor) {
        return;
    }

    processor->setParams(params);
    const size_t latency = static_cast<size_t>(std::max(0, processor->latency_samples()));

    std::lock_guard<std::mutex> lock(mutex_);
    if (Node* node = findNode(name)) {
        node->latency = latency;
    }
}

std::unordered_map<std::string, float> PreprocessChain::getParams(const std::string& name) const {
    ProcessorPtr processor;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (const Node* node = findNode(name)) {
            processor = node->processor;
        }
    }

    if (!processor) {
        return {};
    }

    return processor->getParams();
}

PreprocessTarget PreprocessChain::getTargets(const std::string& name) const {
    std::lock_guard<std::mutex> lock(mutex_);
    if (const Node* node = findNode(name)) {
        return node->target;
    }
    return static_cast<PreprocessTarget>(0);
}

PreprocessChain::Node* PreprocessChain::findNode(const std::string& name) {
    for (auto& node : chain_) {
        if (node.processor && node.processor->name() == name) {
            return &node;
        }
    }
    return nullptr;
}

const PreprocessChain::Node* PreprocessChain::findNode(const std::string& name) const {
    for (const auto& node : chain_) {
        if (node.processor && node.processor->name() == name) {
            return &node;
        }
    }
    return nullptr;
}

} // namespace yvc
