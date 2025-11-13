#include <gtest/gtest.h>

#include <FileProcessor.h>

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <limits>
#include <numbers>
#include <random>
#include <string>
#include <vector>

using namespace yvc;

namespace {

std::filesystem::path uniqueTempFile(const std::string& prefix, const std::string& extension) {
    auto base = std::filesystem::temp_directory_path();
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<int> dist(0, std::numeric_limits<int>::max());

    for (int attempt = 0; attempt < 32; ++attempt) {
        auto candidate = base / (prefix + std::to_string(dist(rng)) + extension);
        if (!std::filesystem::exists(candidate)) {
            return candidate;
        }
    }
    return base / (prefix + "_fallback" + extension);
}

void writeTestWav(const std::filesystem::path& path, SampleRate sample_rate, double duration_seconds) {
    const uint16_t audio_format = 1;
    const uint16_t num_channels = 1;
    const uint16_t bits_per_sample = 16;
    const uint16_t block_align = num_channels * bits_per_sample / 8;
    const uint32_t byte_rate = sample_rate * block_align;
    const uint32_t data_samples = static_cast<uint32_t>(duration_seconds * static_cast<double>(sample_rate));
    const uint32_t data_size = data_samples * block_align;
    const uint32_t riff_chunk_size = 36 + data_size;

    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    out.write("RIFF", 4);
    out.write(reinterpret_cast<const char*>(&riff_chunk_size), sizeof(riff_chunk_size));
    out.write("WAVE", 4);

    out.write("fmt ", 4);
    const uint32_t fmt_chunk_size = 16;
    out.write(reinterpret_cast<const char*>(&fmt_chunk_size), sizeof(fmt_chunk_size));
    out.write(reinterpret_cast<const char*>(&audio_format), sizeof(audio_format));
    out.write(reinterpret_cast<const char*>(&num_channels), sizeof(num_channels));
    out.write(reinterpret_cast<const char*>(&sample_rate), sizeof(sample_rate));
    out.write(reinterpret_cast<const char*>(&byte_rate), sizeof(byte_rate));
    out.write(reinterpret_cast<const char*>(&block_align), sizeof(block_align));
    out.write(reinterpret_cast<const char*>(&bits_per_sample), sizeof(bits_per_sample));

    out.write("data", 4);
    out.write(reinterpret_cast<const char*>(&data_size), sizeof(data_size));

    const double frequency = 220.0;
    for (uint32_t i = 0; i < data_samples; ++i) {
        const double t = static_cast<double>(i) / static_cast<double>(sample_rate);
        const double value = std::sin(2.0 * std::numbers::pi_v<double> * frequency * t);
        const int16_t sample = static_cast<int16_t>(std::clamp(value, -1.0, 1.0) * 32767.0);
        out.write(reinterpret_cast<const char*>(&sample), sizeof(sample));
    }
}

bool fileContains(const std::filesystem::path& path, const std::string& token) {
    std::ifstream in(path);
    if (!in.is_open()) {
        return false;
    }
    std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    return content.find(token) != std::string::npos;
}

TEST(FileProcessorTests, GeneratesExpectedOutputsForStandardMode) {
    const SampleRate sample_rate = SAMPLE_RATE_48K;
    const double duration_seconds = 2.0;

    const auto wav_path = uniqueTempFile("yvc_offline_test", ".wav");
    const auto csv_path = uniqueTempFile("yvc_offline_test", ".csv");
    auto summary_path = csv_path;
    summary_path.replace_extension(".summary.json");
    auto anomalies_path = csv_path;
    anomalies_path.replace_extension(".anomalies.json");
    auto heatmap_path = csv_path;
    heatmap_path.replace_extension(".heatmap.csv");

    writeTestWav(wav_path, sample_rate, duration_seconds);

    FileProcessor processor;
    processor.setMode(PerformanceMode::Mode_Standard);

    ASSERT_TRUE(processor.processFile(wav_path.string(), csv_path.string()));
    const auto& results = processor.getResults();
    EXPECT_FALSE(results.empty());

    EXPECT_TRUE(std::filesystem::exists(csv_path));
    EXPECT_TRUE(std::filesystem::exists(summary_path));
    EXPECT_TRUE(fileContains(summary_path, "\"duration_seconds\""));
    EXPECT_TRUE(std::filesystem::exists(anomalies_path));
    EXPECT_TRUE(fileContains(anomalies_path, "\"anomalies\""));
    EXPECT_TRUE(std::filesystem::exists(heatmap_path));
    EXPECT_TRUE(fileContains(heatmap_path, "chunk_index"));

    std::filesystem::remove(wav_path);
    std::filesystem::remove(csv_path);
    std::filesystem::remove(summary_path);
    std::filesystem::remove(anomalies_path);
    std::filesystem::remove(heatmap_path);
}

TEST(FileProcessorTests, FailsGracefullyWhenFileMissing) {
    FileProcessor processor;
    processor.setMode(PerformanceMode::Mode_Light);
    EXPECT_FALSE(processor.processFile("/nonexistent/input.wav", "/tmp/output.csv"));
}

} // namespace
