// VoiVoi Core Library - Metrics Bus
// License: MIT
// Purpose: Thread-safe double-buffered metrics distribution

#pragma once

#include "Types.h"
#include <vector>
#include <mutex>

namespace yvc {

/// <summary>
/// Double-buffered metrics bus for thread-safe communication
/// between audio thread (producer) and UI thread (consumer).
/// </summary>
class MetricsBus {
public:
    /// <summary>
    /// Constructs a metrics bus.
    /// </summary>
    MetricsBus();
    
    /// <summary>
    /// Writes metrics from audio thread (producer).
    /// </summary>
    /// <param name="metrics">Analysis results to write</param>
    void write(const AnalysisResults& metrics);
    
    /// <summary>
    /// Reads latest metrics from UI thread (consumer).
    /// </summary>
    /// <param name="metrics">Output parameter for metrics</param>
    /// <returns>False if no new data available</returns>
    bool read(AnalysisResults& metrics);
    
    /// <summary>
    /// Gets history of metrics (for visualization).
    /// </summary>
    /// <param name="max_count">Maximum number of historical metrics to return</param>
    /// <returns>Vector of analysis results</returns>
    std::vector<AnalysisResults> getHistory(size_t max_count = 1000) const;
    
    /// <summary>
    /// Clears all metrics.
    /// </summary>
    void clear();

    /// <summary>
    /// Checks if new metrics are available without consuming them.
    /// </summary>
    /// <returns>True if new data is available</returns>
    bool hasNewData() const;

    /// <summary>
    /// Gets the most recently published metrics without consuming them.
    /// </summary>
    /// <returns>Latest analysis results</returns>
    AnalysisResults getLatest() const;

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
