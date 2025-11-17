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

/// <summary>
/// Adds processor to chain with routing targets; captures latency.
/// </summary>
void PreprocessChain::addProcessor(const ProcessorPtr& processor, PreprocessTarget target) {
    if (!processor || !any(target)) {
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    Node node{processor, target, static_cast<size_t>(std::max(0, processor->latency_samples()))};
    chain_.push_back(std::move(node));
}

/// <summary>
/// Removes processor by name; returns true if found.
/// </summary>
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

/// <summary>
/// Clears entire chain.
/// </summary>
void PreprocessChain::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    chain_.clear();
}

/// <summary>
/// Processes audio through processors matching provided target flags.
/// </summary>
void PreprocessChain::process(const float* in, float* out, size_t n, PreprocessTarget target) const {
    if (!in || !out || n == 0) {
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    copyBuffer(in, out, n);

    for (const auto& node : chain_) {
        if (!node.processor || !matches(node.target, target)) {
            continue;
        }
        node.processor->process(out, out, n);
    }
}

/// <summary>
/// Aggregates latency from processors matching target.
/// </summary>
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

/// <summary>
/// Sets parameters on named processor and refreshes cached latency.
/// </summary>
void PreprocessChain::setParams(const std::string& name, const std::unordered_map<std::string, float>& params) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (Node* node = findNode(name)) {
        node->processor->setParams(params);
        refreshLatency(*node);
    }
}

/// <summary>
/// Retrieves current parameters for named processor (empty if not found).
/// </summary>
std::unordered_map<std::string, float> PreprocessChain::getParams(const std::string& name) const {
    std::lock_guard<std::mutex> lock(mutex_);
    if (const Node* node = findNode(name)) {
        return node->processor->getParams();
    }
    return {};
}

/// <summary>
/// Returns routing targets for named processor (0 if not found).
/// </summary>
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

/// <summary>
/// Recalculate latency for a node after parameter changes.
/// </summary>
void PreprocessChain::refreshLatency(Node& node) {
    if (!node.processor) {
        node.latency = 0;
        return;
    }
    node.latency = static_cast<size_t>(std::max(0, node.processor->latency_samples()));
}

} // namespace yvc
