# Contributing to VoiVoiAnalyzer

Thank you for your interest in contributing to VoiVoiAnalyzer!

## Code of Conduct

- Be respectful and inclusive
- Focus on constructive feedback
- Help maintain code quality and privacy standards

## How to Contribute

### Reporting Issues

- Check existing issues first
- Provide clear reproduction steps
- Include system information (OS, compiler, CMake version)
- For privacy concerns, see [PRIVACY.md](docs/PRIVACY.md)

### Pull Requests

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request

### Code Guidelines

#### C++ Style

- Follow C++20 best practices
- Use modern C++ features appropriately
- Prefer `const` and `constexpr` where possible
- Use smart pointers for ownership
- Follow existing naming conventions:
  - Classes: `PascalCase`
  - Functions: `camelCase`
  - Variables: `snake_case_`
  - Constants: `UPPER_CASE`

#### Comments

- Document public APIs
- Explain complex algorithms
- Include references for DSP algorithms
- Keep comments up-to-date

#### Performance

- Real-time code must be deterministic
- Avoid allocations in audio callback
- Profile performance changes
- Maintain latency targets

### Licensing

- Core library (yvc_core): Must remain MIT licensed
- GUI app (yvc_app): GPLv3 due to JUCE
- Offline tool (yvc_offline): GPLv3
- By contributing, you agree to these licenses

### Privacy Requirements

**Critical**: VoiVoiAnalyzer is privacy-first software.

✅ **Allowed**:
- Local-only processing
- User-controlled data export
- Optional metrics-only transmission (with consent)

❌ **Not Allowed**:
- Automatic audio transmission
- Telemetry without explicit consent
- Required online services
- Analytics tracking

### Testing

- Add tests for new features
- Ensure existing tests pass
- Test on Windows (primary platform)
- Verify performance impact

### Documentation

- Update README.md for user-facing changes
- Update ARCHITECTURE.md for design changes
- Update BUILD.md for build changes
- Add inline documentation for APIs

## Development Setup

See [BUILD.md](docs/BUILD.md) for build instructions.

### Quick Setup

```bash
git clone https://github.com/susuki-zzz/VoiVoiAnalyzer.git
cd VoiVoiAnalyzer
mkdir build && cd build
cmake ..
cmake --build .
```

## Areas for Contribution

### High Priority

- JUCE integration for GUI
- Audio file I/O (WAV support)
- Performance optimization
- Unit tests for DSP algorithms
- Windows build testing

### Medium Priority

- Additional audio formats (MP3, FLAC)
- Advanced visualization
- Preset system
- Documentation improvements
- macOS/Linux support

### Nice to Have

- UI localization
- Export formats (PDF, JSON)
- Additional analysis metrics
- Plugin support

## Questions?

- Open an issue for questions
- Check existing documentation
- Review architecture document

Thank you for contributing!
