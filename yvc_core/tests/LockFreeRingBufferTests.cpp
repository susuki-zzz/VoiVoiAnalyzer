// VoiVoi Core Library - Lock-Free Ring Buffer Tests
// License: MIT

#include <gtest/gtest.h>
#include <yvc_core/LockFreeRingBuffer.h>

#include <thread>
#include <vector>
#include <numeric>

namespace yvc::test {

TEST(LockFreeRingBufferTest, BasicWriteRead) {
    LockFreeRingBuffer<float> buffer(1024);
    
    std::vector<float> write_data(100);
    std::iota(write_data.begin(), write_data.end(), 0.0f);
    
    size_t written = buffer.write(write_data.data(), write_data.size());
    EXPECT_EQ(written, write_data.size());
    EXPECT_EQ(buffer.getAvailableRead(), write_data.size());
    
    std::vector<float> read_data(100);
    size_t read = buffer.read(read_data.data(), read_data.size());
    EXPECT_EQ(read, write_data.size());
    EXPECT_EQ(buffer.getAvailableRead(), 0u);
    
    for (size_t i = 0; i < write_data.size(); ++i) {
        EXPECT_FLOAT_EQ(read_data[i], write_data[i]);
    }
}

TEST(LockFreeRingBufferTest, OverflowHandling) {
    LockFreeRingBuffer<float> buffer(100);
    
    std::vector<float> write_data(200, 1.0f);
    size_t written = buffer.write(write_data.data(), write_data.size());
    
    // Should only write up to capacity
    EXPECT_LE(written, 100u);
    EXPECT_EQ(buffer.getAvailableRead(), written);
}

TEST(LockFreeRingBufferTest, WrapAround) {
    LockFreeRingBuffer<float> buffer(100);
    
    std::vector<float> data1(60, 1.0f);
    std::vector<float> data2(60, 2.0f);
    
    buffer.write(data1.data(), data1.size());
    
    std::vector<float> read1(60);
    buffer.read(read1.data(), 60);
    
    // Now write more, should wrap around
    buffer.write(data2.data(), data2.size());
    
    std::vector<float> read2(60);
    buffer.read(read2.data(), 60);
    
    for (size_t i = 0; i < 60; ++i) {
        EXPECT_FLOAT_EQ(read2[i], 2.0f);
    }
}

TEST(LockFreeRingBufferTest, PeekOperation) {
    LockFreeRingBuffer<float> buffer(100);
    
    std::vector<float> write_data(50);
    std::iota(write_data.begin(), write_data.end(), 0.0f);
    buffer.write(write_data.data(), write_data.size());
    
    std::vector<float> peek_data(50);
    size_t peeked = buffer.peek(peek_data.data(), peek_data.size());
    EXPECT_EQ(peeked, 50u);
    EXPECT_EQ(buffer.getAvailableRead(), 50u);  // Should not consume
    
    for (size_t i = 0; i < 50; ++i) {
        EXPECT_FLOAT_EQ(peek_data[i], static_cast<float>(i));
    }
}

TEST(LockFreeRingBufferTest, SkipOperation) {
    LockFreeRingBuffer<float> buffer(100);
    
    std::vector<float> write_data(50);
    std::iota(write_data.begin(), write_data.end(), 0.0f);
    buffer.write(write_data.data(), write_data.size());
    
    buffer.skip(25);
    EXPECT_EQ(buffer.getAvailableRead(), 25u);
    
    std::vector<float> read_data(25);
    buffer.read(read_data.data(), 25);
    
    for (size_t i = 0; i < 25; ++i) {
        EXPECT_FLOAT_EQ(read_data[i], static_cast<float>(i + 25));
    }
}

TEST(LockFreeRingBufferTest, ConcurrentProducerConsumer) {
    LockFreeRingBuffer<float> buffer(10000);
    const size_t iterations = 10000;
    std::atomic<bool> done{false};
    
    // Producer thread
    std::thread producer([&buffer, &done, iterations]() {
        for (size_t i = 0; i < iterations; ++i) {
            float value = static_cast<float>(i);
            while (buffer.write(&value, 1) == 0) {
                std::this_thread::yield();
            }
        }
        done.store(true);
    });
    
    // Consumer thread
    std::thread consumer([&buffer, &done, iterations]() {
        size_t received = 0;
        float last_value = -1.0f;
        
        while (received < iterations) {
            float value;
            if (buffer.read(&value, 1) == 1) {
                EXPECT_FLOAT_EQ(value, static_cast<float>(received));
                last_value = value;
                ++received;
            } else {
                std::this_thread::yield();
            }
        }
    });
    
    producer.join();
    consumer.join();
}

TEST(LockFreeRingBufferTest, Clear) {
    LockFreeRingBuffer<float> buffer(100);
    
    std::vector<float> data(50, 1.0f);
    buffer.write(data.data(), data.size());
    EXPECT_EQ(buffer.getAvailableRead(), 50u);
    
    buffer.clear();
    EXPECT_EQ(buffer.getAvailableRead(), 0u);
    EXPECT_EQ(buffer.getAvailableWrite(), buffer.getCapacity());
}

} // namespace yvc::test
