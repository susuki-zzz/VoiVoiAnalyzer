# VoiVoiAnalyzer

A local-first, real-time voice training application for conversation practice.

**Platform**: Windows first (JUCE 7), later Android/iOS  
**Stack**: C++20, CMake, JUCE 7, KissFFT (MIT), Eigen (optional)  
**Privacy**: No audio leaves the device. Optional AI coach shares **metrics only**.

## Overview

VoiVoiAnalyzer provides comprehensive real-time voice analysis for voice training and speech improvement with strict privacy guarantees and performance targets:

### Voice Metrics
- **F0 Detection**: Fundamental frequency (pitch) tracking
- **Level Analysis**: RMS, Peak, and Crest Factor
- **CPP**: Cepstral Peak Prominence for voice quality assessment
- **HNR**: Harmonics-to-Noise Ratio measurement
- **Spectral Tilt**: Spectral balance analysis
- **/s/ Centroid**: Sibilant sound analysis
- **VAD**: Voice Activity Detection with speech rate and pause analysis

### Visualization
- Real-time metrics display
- Heatmap visualization
- Preset management for common voice training scenarios

### Performance Modes

Three performance modes with guaranteed latency targets (UI round-trip):

| Mode | Latency | FFT Size | Hop Size | Use Case |
|------|---------|----------|----------|----------|
| **Light** | ≤40ms | 1024 | 512 | Real-time practice with minimal latency |
| **Normal** | ≤60ms | 2048 | 512 | Balanced analysis and responsiveness |
| **Diagnostic** | ≤80ms | 4096 | 1024 | Detailed analysis for assessment |

**Auto-degradation**: Drawing FPS only (60→45→30 Hz). Audio processing parameters never change automatically.

### Privacy Architecture

- **100% Local Processing**: No audio transmission over network
- **RAM-Only Live Mode**: All live analysis data stored in RAM only
- **Dry Signal Analysis**: Analysis always uses unprocessed (dry) signal
- **Preprocessing Separation**: Effects chain only affects monitoring/recording, not analysis meters
- **Manual Save Control**: Manual save button + optional auto-save on stop/limit
- **AI Coach (Optional)**: When enabled, sends **metrics only** (never raw audio)
  - Disabled by default
  - API key stored securely (DPAPI on Windows)
  - Rate-limited advice requests

See [Privacy Policy](docs/PRIVACY.md) for details.

## Hard Requirements (Acceptance Criteria)

Performance targets on reference hardware (Ryzen 5 3600XT + RTX 2060 or equivalent):

- **1 hour continuous run**: XRuns = 0, p95 latency ≤60ms (normal mode)
- **CPU usage**: <12% sustained
- **RAM usage**: ≤1.0 GB for 1 hour recording
- **Sample Rate**: Auto-init at 48kHz if supported, otherwise device SR (user-changeable in Settings)
- **Buffer Size**: Platform default (Win=256, Android=burst×2, iOS=128), user-fixed
- **RAM-only live path**: No persistent storage until manual/auto-save triggered

## Architecture

```
Audio I/O (WASAPI) → RAM RingBuffer (PCM float32 mono)
                                    ↓
                              Analyzer (dry signal)
                                    ↓
                              MetricsBus (double-buffer)
                                    ↓
                                   UI
                                    
                     ↘ PreprocessChain (monitor/record paths only)
```

VoiVoiAnalyzer consists of three main components:

1. **yvc_core** (MIT License): Core audio analysis library
   - Audio I/O (WASAPI on Windows)
   - Lock-free ring buffer (single-producer/consumer)
   - Analysis engine: F0, RMS/Peak/Crest, CPP, HNR, Spectral Tilt, /s/ Centroid, VAD
   - MetricsBus (double-buffered, thread-safe)
   - Preprocessing interfaces (IPreprocessor)
   - AI Coach interfaces (ICoachProvider - metrics only)
   - Configuration and preset management
   - Utility functions (FTZ/DAZ, windowing, NaN guards)
   - All DSP and analysis algorithms

2. **yvc_app** (GPLv3): GUI Application
   - Built with JUCE Framework
   - Real-time visualization:
     - F0 gauge with target band overlay
     - CPP, HNR, spectral tilt meters
     - Speech rate and pause analysis
     - Mini waveform display
     - Time-F0 and time-RMS heatmaps
   - Status bar: `FPS | CPU | RAM | Recording Time Left`
   - 4 fixed presets (MVP): Natural Conversation, Phone Training, Resonance Focus, Diagnostic
   - Settings: SR, buffer size, live max duration, autosave toggle, preprocess chain
   - Save dialog: WAV (32-bit float) + metrics (CSV/Parquet) + session.json
   - FPS limiter with auto-degradation (60→45→30 Hz, silent)

3. **yvc_offline**: Offline Analysis Tool
   - Process audio files up to 3 hours
   - 30-second chunks with 1-second overlap
   - Outputs: metrics CSV, summary JSON, anomalies JSON, heatmap CSV
   - Same analysis code path as real-time (reproducibility)
   - Heatmap generation and anomaly highlighting

## Key Interfaces

### IAnalyzer
```cpp
class IAnalyzer {
public:
    virtual void analyze(const float* mono, size_t n, double sr, 
                        double t0, AnalysisResults& out) = 0;
};
```

### IPreprocessor
```cpp
class IPreprocessor {
public:
    virtual void process(const float* in, float* out, size_t n) = 0;
    virtual int latency_samples() const { return 0; }
    virtual void setParams(const std::unordered_map<std::string, float>& kv) = 0;
};
```

### ICoachProvider
```cpp
struct SummarySnapshot { /* Aggregated metrics only - NO audio */ };

class ICoachProvider {
public:
    virtual void setApiKey(const std::string& key) = 0;
    virtual std::string advise(const SummarySnapshot& snapshot) = 0;
};
```

## Presets (MVP - Fixed)

Four built-in presets optimized for different use cases:

1. **Natural Conversation**: Balanced metrics for everyday speaking
   - Meters: F0 Gauge, CPP, HNR, Spectral Tilt, Speech Rate, Pause
   - Target F0: 170-230 Hz

2. **Phone Training**: Clarity for phone/VoIP calls
   - Meters: F0 Gauge, Timing, Intelligibility, Spectral Tilt
   - Emphasizes 300-3400 Hz bandwidth

3. **Resonance Focus**: Spectral balance and vocal resonance
   - Meters: Spectral Tilt, /s/ Centroid, F0 Gauge, CPP
   - Target Spectral Tilt: -6 to -3 dB/octave

4. **Diagnostic**: Complete analysis for assessment
   - All meters: F0, RMS, Peak, Crest, CPP, HNR, Tilt, VAD, etc.
   - Full spectral display with harmonics

*Note: User-defined presets and layout editor planned for future release.*

## Technology Stack

- **C++20**: Modern C++ with latest features
- **CMake**: Cross-platform build system
- **JUCE 7**: Professional audio application framework (GUI only)
- **KissFFT**: Fast and simple FFT library
- **Eigen** (Optional): Advanced matrix operations

## Building

### Requirements
- CMake 3.20 or later
- C++20 compatible compiler (MSVC 2019+, GCC 10+, Clang 12+)
- Windows 10/11 (primary target platform)

### Build Instructions

```bash
# Clone the repository
git clone https://github.com/susuki-zzz/VoiVoiAnalyzer.git
cd VoiVoiAnalyzer

# Create build directory
mkdir build
cd build

# Configure
cmake ..

# Build
cmake --build . --config Release

# Build only core library
cmake .. -DBUILD_YVC_APP=OFF -DBUILD_YVC_OFFLINE=OFF

# Build with Eigen support
cmake .. -DUSE_EIGEN=ON
```

### Build Outputs
- `yvc_core.lib`: Core analysis library
- `yvc_app.exe`: GUI application (when JUCE is available)
- `yvc_offline.exe`: Offline analysis tool

## Audio Configuration

- **Default Sample Rate**: 48 kHz (auto-configured on first run)
- **Buffer Size**: Uses OS default (fixed for optimal latency)
- **Channels**: Mono analysis
- **Sample Rate**: Configurable in settings (44.1 kHz, 48 kHz, etc.)

## Performance

The application automatically degrades rendering FPS when needed to maintain audio processing performance:
- Target: 60 FPS
- Degraded: 45 FPS
- Minimum: 30 FPS

Audio processing latency is always maintained within mode targets.

## License

- **yvc_core**: MIT License (permissive, use in any project)
- **yvc_app**: GPLv3 (due to JUCE framework OSS licensing)
- **yvc_offline**: GPLv3

See [LICENSE](LICENSE) for full license text.

## Contributing

Contributions are welcome! Please ensure:
- Code follows C++20 best practices
- Core library remains MIT licensed
- Privacy guarantees are maintained
- No audio transmission features are added

## Roadmap

- [ ] Complete JUCE GUI implementation
- [x] Audio file import/export (offline tooling)
- [ ] Advanced visualization options
- [ ] Preset sharing (metrics only)
- [ ] Multi-language support
- [ ] macOS and Linux support

## Support

For issues and questions:
- GitHub Issues: https://github.com/susuki-zzz/VoiVoiAnalyzer/issues
- Privacy concerns: See [Privacy Policy](docs/PRIVACY.md)

---

**Note**: This is voice training software. Always consult with a qualified speech-language pathologist or voice coach for professional voice health guidance.
