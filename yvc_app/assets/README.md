# VoiVoi Analyzer Assets

This directory contains application assets:

- **icon_64.png**: Small application icon (64x64)
- **icon_512.png**: Large application icon (512x512)

## Icon Guidelines

Icons should follow these specifications:

### Small Icon (64x64)
- Format: PNG with transparency
- Size: 64x64 pixels
- Use: Taskbar, window titles, small UI elements

### Large Icon (512x512)  
- Format: PNG with transparency
- Size: 512x512 pixels
- Use: Application launcher, about dialog, high-DPI displays

### Design Notes
- Use VoiVoi brand colors
- Include waveform or audio visualization elements
- Maintain readability at small sizes
- Follow platform-specific design guidelines

## Placeholder Icons

Currently using system default icons. To add custom icons:

1. Create icon_64.png (64x64 pixels)
2. Create icon_512.png (512x512 pixels)
3. Place in this assets/ directory
4. Rebuild application

The CMake configuration will automatically detect and include these icons.
