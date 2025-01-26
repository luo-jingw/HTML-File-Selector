# HTML File Selector

A modern GUI application developed with Win32 API for managing and opening HTML files.

## Features

- Modern dark theme interface
- High-definition rounded button design
- Intelligent folder scanning
- Auto-save window size and position
- Customizable HTML file directory
- Real-time button hover effects
- Clear font rendering with ClearType

## Usage

1. By default, scans HTML files in `E:\guide_html` directory
2. Click corresponding buttons to open HTML files
3. Click "Settings" to change the HTML file directory
4. Window size and position are automatically remembered
5. Error messages displayed when files are not found

## Interface

- Main interface automatically lists all found HTML files
- Bottom toolbar:
  - Settings: Change HTML file directory
  - Exit: Close application

## Technical Features

- Native Win32 API development
- GDI+ high-quality graphics rendering
- ClearType text rendering technology
- Double buffering to prevent flickering
- Registry configuration persistence
- Resource compilation and icon support

## System Requirements

- Windows 7 or higher
- Unicode support
- No additional runtime required

## Development Environment

- Visual Studio Code
- MinGW-w64 (MSYS2)
- Windows SDK

## Build Instructions

Using VS Code task system:
1. Ensure MSYS2 and necessary development tools are installed
2. Install C/C++ and CMake Tools extensions
3. Press F5 to compile and run

## Notes

- Ensure target directory has read permissions
- Supports dynamic addition of new HTML files
- Window settings are stored in registry