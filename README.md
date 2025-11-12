# VoiVoiAnalyzer

A local-only, low-latency voice training analyzer for Windows.

## Features

VoiVoiAnalyzer provides comprehensive real-time voice analysis for voice training and speech improvement:

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
- **Light Mode**: ≤40ms latency for real-time feedback
- **Standard Mode**: ≤60ms latency with balanced features
- **Diagnostic Mode**: ≤80ms latency with full analysis

### Privacy First
- **100% Local Processing**: No audio transmission over network
- **RAM-Only Live Mode**: Data stored in memory only during live analysis
- **Manual/Auto Save**: You control when data is saved
- **AI Coach (Optional)**: Sends metrics only, never raw audio

See [Privacy Policy](docs/PRIVACY.md) for details.

## Architecture

VoiVoiAnalyzer consists of three main components:

1. **yvc_core** (MIT License): Core audio analysis library
   - Portable C++20 library
   - Can be integrated into other projects
   - All DSP and analysis algorithms

2. **yvc_app** (GPLv3): GUI Application
   - Built with JUCE Framework
   - Real-time visualization
   - Audio I/O management
   - Settings and presets

3. **yvc_offline**: Offline Analysis Tool
   - Process audio files up to 3 hours
   - Batch analysis capabilities
   - CSV output for further analysis

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
- [ ] Audio file import/export
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
