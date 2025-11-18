// VoiVoi Core Library - Audio Buffer
// License: MIT
// Purpose: Audio buffer management for real-time processing

#pragma once

#include "Types.h"
#include "IPreprocessor.h"
#include <vector>
#include <memory>
#include <unordered_map>

namespace yvc {

    /// <summary>
    /// Manages audio buffering for real-time processing and analysis.
    /// Provides latency compensation for preprocessing targets.
    /// </summary>
    class AudioBuffer {
    public:
        /// <summary>
        /// Constructs an audio buffer with the specified configuration.
        /// </summary>
        /// <param name="config">Audio configuration parameters</param>
        explicit AudioBuffer(const AudioConfig& config);

        /// <summary>
        /// Adds samples to the buffer.
        /// </summary>
        /// <param name="samples">Pointer to audio samples</param>
        /// <param name="num_samples">Number of samples to add</param>
        void addSamples(const Sample* samples, size_t num_samples);

        /// <summary>
        /// Gets the current buffer for analysis.
        /// </summary>
        /// <returns>Reference to the internal buffer</returns>
        const std::vector<Sample>& getBuffer() const { return buffer_; }

        /// <summary>
        /// Checks if buffer has enough samples for analysis.
        /// </summary>
        /// <param name="required_samples">Minimum number of samples required</param>
        /// <returns>True if buffer contains at least required_samples</returns>
        bool hasEnoughSamples(size_t required_samples) const;

        /// <summary>
        /// Consumes samples from the front of the buffer after analysis.
        /// </summary>
        /// <param name="num_samples">Number of samples to consume</param>
        void consumeSamples(size_t num_samples);

        /// <summary>
        /// Clears the buffer.
        /// </summary>
        void clear();

        /// <summary>
        /// Gets the audio configuration.
        /// </summary>
        /// <returns>Reference to the audio configuration</returns>
        const AudioConfig& getConfig() const { return config_; }

        /// <summary>
        /// Gets the current fill level of the buffer.
        /// </summary>
        /// <returns>Number of samples currently in the buffer</returns>
        size_t getSize() const { return buffer_.size(); }

        /// <summary>
        /// Configures latency compensation per preprocess target.
        /// </summary>
        /// <param name="target">Preprocessing target</param>
        /// <param name="samples">Number of samples to compensate</param>
        void setLatencyCompensation(PreprocessTarget target, size_t samples);

        /// <summary>
        /// Retrieves configured latency compensation.
        /// </summary>
        /// <param name="target">Preprocessing target</param>
        /// <returns>Number of samples configured for latency compensation</returns>
        size_t getLatencyCompensation(PreprocessTarget target) const;

        /// <summary>
        /// Clears all latency compensation data.
        /// </summary>
        void clearLatencyCompensation();

    private:
        AudioConfig config_;
        std::vector<Sample> buffer_;
        size_t write_pos_ = 0;

        struct TargetHash {
            size_t operator()(PreprocessTarget target) const {
                return static_cast<size_t>(toMask(target));
            }
        };

        std::unordered_map<PreprocessTarget, size_t, TargetHash> latency_compensation_;
    };

} // namespace yvc
