// VoiVoi Core Library - Lock-Free Ring Buffer
// License: MIT
// Purpose: Single-producer single-consumer lock-free ring buffer for real-time audio

#pragma once

#include "Types.h"
#include <atomic>
#include <memory>
#include <algorithm>

namespace yvc {

    /// <summary>
    /// Lock-free ring buffer for single producer (audio thread) and single consumer (analysis thread).
    /// Thread-safe without locks, suitable for real-time audio processing.
    /// </summary>
    /// <typeparam name="T">Element type</typeparam>
    template<typename T>
    class LockFreeRingBuffer {
    public:
        /// <summary>
        /// Constructs a lock-free ring buffer with the specified capacity.
        /// </summary>
        /// <param name="capacity">Buffer capacity in elements</param>
        explicit LockFreeRingBuffer(size_t capacity)
            : capacity_(capacity + 1)  // +1 to distinguish full from empty
            , buffer_(new T[capacity_])
            , write_index_(0)
            , read_index_(0) { }

        ~LockFreeRingBuffer() = default;

        // Non-copyable, non-movable for safety
        LockFreeRingBuffer(const LockFreeRingBuffer&) = delete;
        LockFreeRingBuffer& operator=(const LockFreeRingBuffer&) = delete;
        LockFreeRingBuffer(LockFreeRingBuffer&&) = delete;
        LockFreeRingBuffer& operator=(LockFreeRingBuffer&&) = delete;

        /// <summary>
        /// Writes samples to the buffer (called by producer/audio thread).
        /// </summary>
        /// <param name="samples">Pointer to samples to write</param>
        /// <param name="count">Number of samples to write</param>
        /// <returns>Number of samples actually written</returns>
        size_t write(const T* samples, size_t count) {
            const size_t write_idx = write_index_.load(std::memory_order_relaxed);
            const size_t read_idx = read_index_.load(std::memory_order_acquire);

            const size_t available = getAvailableWrite(write_idx, read_idx);
            const size_t to_write = std::min(count, available);

            if(to_write == 0) {
                return 0;
            }

            const size_t first_chunk = std::min(to_write, capacity_ - write_idx);
            std::copy(samples, samples + first_chunk, buffer_.get() + write_idx);

            if(to_write > first_chunk) {
                const size_t second_chunk = to_write - first_chunk;
                std::copy(samples + first_chunk, samples + to_write, buffer_.get());
            }

            write_index_.store((write_idx + to_write) % capacity_, std::memory_order_release);
            return to_write;
        }

        /// <summary>
        /// Reads samples from the buffer (called by consumer/analysis thread).
        /// </summary>
        /// <param name="dest">Destination buffer for read samples</param>
        /// <param name="count">Number of samples to read</param>
        /// <returns>Number of samples actually read</returns>
        size_t read(T* dest, size_t count) {
            const size_t read_idx = read_index_.load(std::memory_order_relaxed);
            const size_t write_idx = write_index_.load(std::memory_order_acquire);

            const size_t available = getAvailableRead(write_idx, read_idx);
            const size_t to_read = std::min(count, available);

            if(to_read == 0) {
                return 0;
            }

            const size_t first_chunk = std::min(to_read, capacity_ - read_idx);
            std::copy(buffer_.get() + read_idx, buffer_.get() + read_idx + first_chunk, dest);

            if(to_read > first_chunk) {
                const size_t second_chunk = to_read - first_chunk;
                std::copy(buffer_.get(), buffer_.get() + second_chunk, dest + first_chunk);
            }

            read_index_.store((read_idx + to_read) % capacity_, std::memory_order_release);
            return to_read;
        }

        /// <summary>
        /// Peeks samples without consuming (called by consumer).
        /// </summary>
        /// <param name="dest">Destination buffer for peeked samples</param>
        /// <param name="count">Number of samples to peek</param>
        /// <returns>Number of samples actually peeked</returns>
        size_t peek(T* dest, size_t count) const {
            const size_t read_idx = read_index_.load(std::memory_order_relaxed);
            const size_t write_idx = write_index_.load(std::memory_order_acquire);

            const size_t available = getAvailableRead(write_idx, read_idx);
            const size_t to_peek = std::min(count, available);

            if(to_peek == 0) {
                return 0;
            }

            const size_t first_chunk = std::min(to_peek, capacity_ - read_idx);
            std::copy(buffer_.get() + read_idx, buffer_.get() + read_idx + first_chunk, dest);

            if(to_peek > first_chunk) {
                const size_t second_chunk = to_peek - first_chunk;
                std::copy(buffer_.get(), buffer_.get() + second_chunk, dest + first_chunk);
            }

            return to_peek;
        }

        /// <summary>
        /// Advances read position without copying data.
        /// </summary>
        /// <param name="count">Number of samples to skip</param>
        void skip(size_t count) {
            const size_t read_idx = read_index_.load(std::memory_order_relaxed);
            const size_t write_idx = write_index_.load(std::memory_order_acquire);

            const size_t available = getAvailableRead(write_idx, read_idx);
            const size_t to_skip = std::min(count, available);

            read_index_.store((read_idx + to_skip) % capacity_, std::memory_order_release);
        }

        /// <summary>
        /// Gets number of samples available for reading.
        /// </summary>
        /// <returns>Number of available samples</returns>
        size_t getAvailableRead() const {
            const size_t write_idx = write_index_.load(std::memory_order_acquire);
            const size_t read_idx = read_index_.load(std::memory_order_relaxed);
            return getAvailableRead(write_idx, read_idx);
        }

        /// <summary>
        /// Gets number of samples available for writing.
        /// </summary>
        /// <returns>Number of available slots</returns>
        size_t getAvailableWrite() const {
            const size_t write_idx = write_index_.load(std::memory_order_relaxed);
            const size_t read_idx = read_index_.load(std::memory_order_acquire);
            return getAvailableWrite(write_idx, read_idx);
        }

        /// <summary>
        /// Gets total capacity.
        /// </summary>
        /// <returns>Buffer capacity in elements</returns>
        size_t getCapacity() const {
            return capacity_ - 1;  // -1 because we reserve one slot
        }

        /// <summary>
        /// Clears the buffer (should only be called when no concurrent access).
        /// </summary>
        void clear() {
            read_index_.store(0, std::memory_order_relaxed);
            write_index_.store(0, std::memory_order_relaxed);
        }

    private:
        size_t getAvailableRead(size_t write_idx, size_t read_idx) const {
            if(write_idx >= read_idx) {
                return write_idx - read_idx;
            }
            return capacity_ - read_idx + write_idx;
        }

        size_t getAvailableWrite(size_t write_idx, size_t read_idx) const {
            if(write_idx >= read_idx) {
                return capacity_ - 1 - (write_idx - read_idx);
            }
            return read_idx - write_idx - 1;
        }

        const size_t capacity_;
        std::unique_ptr<T[]> buffer_;
        alignas(64) std::atomic<size_t> write_index_;  // Cache line alignment
        alignas(64) std::atomic<size_t> read_index_;   // Cache line alignment
    };

    /// <summary>
    /// Type alias for audio sample ring buffer.
    /// </summary>
    using AudioRingBuffer = LockFreeRingBuffer<Sample>;

} // namespace yvc
