# HTML File Selector

A modern and elegant GUI application for managing and launching HTML files, developed with Win32 API and modern C++.

## Key Features

### User Interface
- Modern dark theme with high DPI support
- High-quality rounded buttons with hover effects
- Dynamic layout with automatic centering
- Smooth animations and transitions
- ClearType text rendering

### Core Functions
- Automatic HTML file discovery
- One-click file launching
- Custom directory configuration
- Window size/position memory
- Real-time file monitoring

### Technical Highlights
- Native Win32 API implementation
- GDI+ high-quality graphics rendering
- Double buffering for smooth display
- Registry-based configuration storage
- Resource compilation with icon support

## Installation

No installation required - simply download and run the executable.

## Quick Start

1. First Launch:
   - Program scans `E:\guide_html` by default
   - All HTML files are automatically listed
   - Window position is remembered

2. Basic Operations:
   - Click any file button to open the HTML file
   - Use "Settings" to change directory
   - Window size adjusts automatically

3. Configuration:
   - Click "Settings" to select folder
   - All settings auto-save
   - Instant UI update on changes

## Development

### Prerequisites
- Visual Studio Code
- MSYS2 (MinGW-w64)
- Windows SDK
- C++17 or later

### Build Steps
1. Install required tools:
   ```bash
   pacman -S mingw-w64-x86_64-toolchain
   ```

2. Clone and build:
   ```bash
   git clone [repository-url]
   cd HTML-File-Selector
   code .
   # Press F5 to build and run
   ```

### Project Structure
```
HTML-File-Selector/
├── main.cpp          # Main application code
├── resource.h        # Resource definitions
├── resource.rc      # Resource configuration
├── app.manifest     # Application manifest
└── app.ico          # Application icon
```

## Technical Details

### Dependencies
- Win32 API
- GDI+ Graphics Library
- Windows Registry API
- COM for folder dialogs

### Performance
- Optimized rendering engine
- Minimal memory footprint
- Fast startup time
- Efficient file handling

## Contributing

1. Fork the repository
2. Create your feature branch
3. Commit your changes
4. Push to the branch
5. Create a Pull Request

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Notes

- Windows 7 or later required
- High DPI display supported
- UTF-8 encoding for all files
- Registry used for settings