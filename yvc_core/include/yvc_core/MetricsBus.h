// VoiVoi Core Library - Metrics Bus
// License: MIT
// Purpose: Thread-safe double-buffered metrics distribution

#pragma once

#include "Types.h"
#include <vector>
#include <mutex>

namespace yvc {

// Double-buffered metrics bus for thread-safe communication
// between audio thread (producer) and UI thread (consumer)
class MetricsBus {
public:
    MetricsBus();
    
    // Write metrics from audio thread (producer)
    void write(const AnalysisResults& metrics);
    
    // Read latest metrics from UI thread (consumer)
    // Returns false if no new data available
    bool read(AnalysisResults& metrics);
    
    // Get history of metrics (for visualization)
    std::vector<AnalysisResults> getHistory(size_t max_count = 1000) const;
    
    // Clear all metrics
    void clear();
    
private:
    mutable std::mutex mutex_;
    AnalysisResults buffer_[2];  // Double buffer
    int write_index_ = 0;
    int read_index_ = 1;
    bool has_new_data_ = false;
    
    std::vector<AnalysisResults> history_;
    size_t max_history_size_ = 10000;
};

} // namespace yvc
