# VoiVoiAnalyzer Architecture

## Overview

VoiVoiAnalyzer is a local-first, real-time voice training application designed for conversation practice with strict privacy guarantees and performance targets. The architecture follows the YunoVoiceCoach specification with modular design and clear separation of concerns.

## Design Principles

1. **Privacy First**: No audio data leaves the device
2. **Performance Guaranteed**: Strict latency targets (≤40/60/80ms)
3. **Dry Signal Analysis**: Preprocessing never affects measurement
4. **Modular Licensing**: MIT core + GPLv3 application
5. **Real-time Optimized**: Lock-free communication, zero allocations in hot path

## System Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                      VoiVoiAnalyzer                         │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  Audio I/O (WASAPI) ──→ RAM RingBuffer (float32 mono)      │
│                               │                             │
│                               ├──→ Analyzer (DRY signal)    │
│                               │         ↓                   │
│                               │    MetricsBus               │
│                               │    (double-buffer)          │
│                               │         ↓                   │
│                               │        UI                   │
│                               │    (60→45→30 FPS)          │
│                               │                             │
│                               └──→ PreprocessChain          │
│                                    (monitor/record only)    │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

## Components

### 1. yvc_core (Core Library)

**License**: MIT  
**Language**: C++20  
**Purpose**: Platform-independent voice analysis algorithms

#### Modules

- **AudioBuffer**: Ring buffer management for real-time audio
- **F0Detector**: Fundamental frequency detection using autocorrelation
- **LevelAnalyzer**: RMS, Peak, and Crest Factor calculation
- **CPPAnalyzer**: Cepstral Peak Prominence for voice quality
- **HNRAnalyzer**: Harmonics-to-Noise Ratio measurement
- **SpectralAnalyzer**: Spectral Tilt and /s/ Centroid analysis
- **VADAnalyzer**: Voice Activity Detection with speech rate estimation
- **PerformanceMode**: Latency and feature management

#### Key Features

- Header-only public API
- No dependencies except KissFFT
- Thread-safe design
- Zero audio data transmission
- Optimized for real-time processing

#### Performance Modes

| Mode       | Latency | FFT Size | Hop Size | Features      |
|------------|---------|----------|----------|---------------|
| Light      | ≤40ms   | 1024     | 512      | Essential     |
| Standard   | ≤60ms   | 2048     | 512      | All enabled   |
| Diagnostic | ≤80ms   | 4096     | 1024     | Full analysis |

### 2. yvc_app (GUI Application)

**License**: GPLv3 (due to JUCE)  
**Language**: C++20  
**Framework**: JUCE 7  
**Purpose**: Real-time visualization and user interface

#### Components (Planned)

- **Main Application**: JUCE application lifecycle
- **Audio I/O Manager**: System audio device management
- **Real-time Processor**: Audio callback handling
- **Visualization**:
  - Metrics display (F0, RMS, CPP, HNR, etc.)
  - Heatmap visualization
  - Timeline view
- **Settings Manager**: User preferences and presets
- **FPS Controller**: Adaptive rendering (60→45→30 FPS)

#### Features

- Windows-first design (macOS/Linux later)
- 48kHz default sample rate
- OS-default buffer size (fixed)
- RAM-only live mode
- Manual/auto-save options
- No network transmission

### 3. yvc_offline (Offline Analysis Tool)

**License**: GPLv3  
**Language**: C++20  
**Purpose**: Batch processing of audio files

#### Features

- Command-line interface
- Process files up to 3 hours
- CSV output format
- All analysis modes supported
- No GUI dependencies

#### Usage

```bash
yvc_offline [options] <input_file> <output_file>
  -m, --mode <light|standard|diagnostic>
  -h, --help
```

## Data Flow

### Real-time Analysis (yvc_app)

```
Audio Input Device
       │
       ▼
┌──────────────┐
│ Audio Buffer │ (512-2048 samples)
└──────┬───────┘
       │
       ▼
┌──────────────────────┐
│  Analysis Pipeline   │
│  ┌────────────────┐  │
│  │ Level Analysis │  │
│  ├────────────────┤  │
│  │ F0 Detection   │  │
│  ├────────────────┤  │
│  │ CPP Analysis   │  │
│  ├────────────────┤  │
│  │ HNR Analysis   │  │
│  ├────────────────┤  │
│  │ Spectral Tilt  │  │
│  ├────────────────┤  │
│  │ /s/ Centroid   │  │
│  ├────────────────┤  │
│  │ VAD + Speech   │  │
│  │ Rate           │  │
│  └────────────────┘  │
└──────┬───────────────┘
       │
       ▼
┌──────────────┐
│ Visualization│ (60/45/30 FPS)
└──────────────┘
       │
       ▼
   User Display
```

### Offline Analysis (yvc_offline)

```
Audio File (WAV/etc.)
       │
       ▼
┌──────────────┐
│ File Reader  │
└──────┬───────┘
       │
       ▼
┌──────────────────┐
│ Chunk Processing │ (overlapping windows)
└──────┬───────────┘
       │
       ▼
┌──────────────────┐
│ Analysis Results │ (timestamped)
└──────┬───────────┘
       │
       ▼
    CSV File
```

## Algorithm Details

### F0 Detection

- **Method**: Autocorrelation
- **Range**: 80-400 Hz (configurable)
- **Window**: 2-4 pitch periods minimum
- **Threshold**: Correlation > 0.3

### CPP (Cepstral Peak Prominence)

- **FFT**: Log-magnitude spectrum
- **IFFT**: Cepstrum calculation
- **Peak Finding**: Quefrency range for 80-400 Hz
- **Output**: dB prominence above baseline

### HNR (Harmonics-to-Noise Ratio)

- **Method**: Autocorrelation at pitch period
- **Formula**: `HNR = 10 * log10(harmonic_energy / noise_energy)`
- **Requires**: Valid F0 estimate

### Spectral Tilt

- **Method**: Linear regression on log-magnitude spectrum
- **Output**: dB/octave slope

### /s/ Centroid

- **Range**: 4-8 kHz
- **Detection**: Energy ratio > 0.3
- **Centroid**: Weighted frequency average in sibilant range

### VAD (Voice Activity Detection)

- **Threshold**: RMS-based (configurable)
- **Speech Rate**: Syllable transitions per second
- **Pause Ratio**: Non-speech time / total time

## Privacy Architecture

### Local-Only Guarantee

1. **No Network Stack**: Core library has no network code
2. **No Telemetry**: Zero analytics or usage tracking
3. **RAM-Only Live**: No persistent storage during live analysis
4. **User-Controlled Save**: Explicit save action required

### AI Coach Integration (Optional)

```
┌──────────────┐
│ yvc_app      │
└──────┬───────┘
       │
       ▼
┌──────────────────┐
│ Metrics Extract  │ (F0, RMS, CPP, HNR only)
└──────┬───────────┘
       │
       ▼
┌──────────────────┐
│ HTTPS API        │ (metrics only, no audio)
└──────┬───────────┘
       │
       ▼
    AI Service
```

**Transmitted**: Only analysis metrics (numbers)  
**Not Transmitted**: Raw audio, spectrograms, waveforms

## Threading Model

### yvc_core

- **Thread-Safe**: All analyzers are thread-safe
- **No Internal Threads**: Caller controls threading
- **Lock-Free**: Where possible for real-time safety

### yvc_app

```
┌─────────────────┐
│  Audio Thread   │ (High priority)
│  - Buffer fill  │
│  - Analysis     │
└────────┬────────┘
         │ (Lock-free queue)
         ▼
┌─────────────────┐
│   GUI Thread    │ (Normal priority)
│  - Rendering    │
│  - User Input   │
└─────────────────┘
```

## Build System

### CMake Structure

```
VoiVoiAnalyzer/
├── CMakeLists.txt          # Root configuration
├── yvc_core/
│   ├── CMakeLists.txt      # Core library
│   ├── include/yvc_core/   # Public headers
│   └── src/                # Implementation
├── yvc_app/
│   ├── CMakeLists.txt      # GUI app (JUCE)
│   ├── src/
│   └── resources/
├── yvc_offline/
│   ├── CMakeLists.txt      # Offline tool
│   └── src/
└── third_party/
    ├── CMakeLists.txt
    └── kissfft/            # KissFFT library
```

### Build Options

- `BUILD_YVC_APP`: Enable/disable GUI (default: ON)
- `BUILD_YVC_OFFLINE`: Enable/disable offline tool (default: ON)
- `USE_EIGEN`: Enable Eigen support (default: OFF)

## Dependencies

### Core Library (yvc_core)

- **KissFFT**: BSD-3-Clause, FFT operations
- **Standard Library**: C++20 STL only

### GUI App (yvc_app)

- **JUCE 7**: GPLv3/Commercial, GUI framework
- **yvc_core**: MIT, analysis engine

### Offline Tool (yvc_offline)

- **yvc_core**: MIT, analysis engine
- **Future**: libsndfile or similar for audio I/O

## Performance Targets

| Component | Target FPS | Latency | CPU Usage |
|-----------|-----------|---------|-----------|
| Light     | 60        | ≤40ms   | <10%      |
| Standard  | 60→45     | ≤60ms   | <20%      |
| Diagnostic| 45→30     | ≤80ms   | <30%      |

CPU usage targets are for modern quad-core processors (2020+).

## Future Extensions

### Planned Features

1. **Audio File I/O**: Full WAV/MP3 support
2. **JUCE Integration**: Complete GUI implementation
3. **Advanced Visualization**: Spectrograms, formant tracking
4. **Preset System**: Save/load analysis configurations
5. **Export Options**: PDF reports, JSON data
6. **Multi-language**: UI localization

### Platform Support

- **Current**: Windows 10/11
- **Next**: macOS 10.15+
- **Future**: Linux (Ubuntu 20.04+)

## Testing Strategy

### Unit Tests (Planned)

- Core algorithms accuracy
- Performance benchmarks
- Thread safety validation

### Integration Tests (Planned)

- Full analysis pipeline
- File I/O operations
- GUI interaction

### Performance Tests

- Latency measurements
- CPU usage profiling
- Memory usage tracking

## Licensing Compliance

### MIT License (yvc_core)

- Permissive use
- Commercial-friendly
- Attribution required

### GPLv3 (yvc_app, yvc_offline)

- JUCE requires GPL for open source
- Source distribution required
- Viral copyleft
- Patent protection

### Third-Party

- **KissFFT**: BSD-3-Clause (compatible)
- **JUCE**: GPLv3 (forces GPLv3 for app)
- **Eigen** (optional): MPL-2.0 (compatible)

## Security Considerations

1. **No Buffer Overflows**: Modern C++ with bounds checking
2. **No Arbitrary Code Execution**: No dynamic loading
3. **No Network Attack Surface**: No network code in core
4. **Privacy by Design**: Local-only architecture
5. **Input Validation**: Audio buffer size limits

## Documentation

- **README.md**: Project overview
- **docs/BUILD.md**: Build instructions
- **docs/PRIVACY.md**: Privacy policy
- **docs/ARCHITECTURE.md**: This file
- **Code Comments**: Inline API documentation
