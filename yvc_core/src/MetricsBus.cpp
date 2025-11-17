// VoiVoi Core Library - Metrics Bus Implementation
// License: MIT

#include "yvc_core/MetricsBus.h"
#include <algorithm>

namespace yvc {

MetricsBus::MetricsBus() {
    history_.reserve(max_history_size_);
}

/// <summary>
/// Writes a new metrics frame (producer). Uses buffer swap and appends to history.
/// </summary>
void MetricsBus::write(const AnalysisResults& metrics) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Write to current buffer
    buffer_[write_index_] = metrics;
    
    // Swap buffers
    std::swap(write_index_, read_index_);
    has_new_data_ = true;
    
    // Add to history
    history_.push_back(metrics);
    if (history_.size() > max_history_size_) {
        history_.erase(history_.begin());
    }
}

/// <summary>
/// Reads latest metrics (consumer). Returns false if no fresh data.
/// </summary>
bool MetricsBus::read(AnalysisResults& metrics) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!has_new_data_) {
        return false;
    }
    
    metrics = buffer_[read_index_];
    has_new_data_ = false;
    return true;
}

/// <summary>
/// Returns up to max_count recent metrics; copies out of internal history.
/// </summary>
std::vector<AnalysisResults> MetricsBus::getHistory(size_t max_count) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (history_.size() <= max_count) {
        return history_;
    }
    
    // Return last max_count items
    return std::vector<AnalysisResults>(
        history_.end() - max_count,
        history_.end()
    );
}

/// <summary>
/// Clears history and new-data flag.
/// </summary>
void MetricsBus::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    history_.clear();
    has_new_data_ = false;
}

/// <summary>
/// Non-consuming check for fresh metrics.
/// </summary>
bool MetricsBus::hasNewData() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return has_new_data_;
}

/// <summary>
/// Returns latest metrics snapshot (may not be new).
/// </summary>
AnalysisResults MetricsBus::getLatest() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return buffer_[read_index_];
}

} // namespace yvc
