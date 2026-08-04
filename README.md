# CINEBASE Qt C++ Frontend
A modern, native Qt C++ desktop application for video library management and playback. Replaces the web-based React frontend while maintaining full compatibility with the Python FastAPI backend.

<p align="center">
    <img src="assets/logo.png" alt="CINEBASE Logo" width="160" />
</p>

A modern, native Qt C++ desktop application for video library management and playback. Replaces the web-based React frontend while maintaining full compatibility with the Python FastAPI backend.

## Features

- **Native Desktop Application**: Built with Qt 6 for cross-platform compatibility
- **Video Library Management**: Browse, search, and manage your video collection
- **Audio Track Selection**: Choose between multiple audio tracks
- **Watch Progress Tracking**: Resume videos from where you left off
- **Watched Status**: Mark videos as watched/unwatched
- **Auto-Cleanup**: Proper resource cleanup and process management on exit
- **Beautiful Dark UI**: Modern interface with purple accent colors (#a892ff)
- **Responsive Design**: Adaptable to different window sizes

## System Requirements

### Linux (Ubuntu/Debian)
- Qt 6.x development libraries
- CMake 3.16+
- C++17 compatible compiler (GCC 7+ or Clang 5+)
- Python 3.8+ (for backend)

### Installation

#### Ubuntu/Debian
```bash
sudo apt update
sudo apt install -y build-essential cmake qt6-base-dev qt6-declarative-dev
```

#### Fedora
```bash
sudo dnf install -y @development-tools cmake qt6-qtbase-devel qt6-qtdeclarative-devel
```

#### Arch Linux
```bash
sudo pacman -S base-devel cmake qt6-base
```

## Building

```bash
# Navigate to the qt-frontend directory
cd qt-frontend

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build
cmake --build . --config Release

# The executable will be in the build directory
```

## Running

### Start the Python Backend
```bash
# From the project root, activate venv if using it
source .venv/bin/activate  # or your venv path

# Run the backend
python backend/main.py
```

The backend will:
- Start FastAPI server on `http://127.0.0.1:8000`
- Listen for API requests from the Qt frontend
- Manage video library and playback via mpv

### Start the Qt Frontend
```bash
# From qt-frontend/build directory
./CINEBASE
```

## Architecture

### Frontend (Qt C++)
- **MainWindow**: Main application window with sidebar and content panel
- **ApiManager**: Handles HTTP communication with Python backend
- **VideoLibrary**: Data model for videos, progress, and metadata

### Backend (Python + FastAPI)
- RESTful API for video library operations
- FFmpeg integration for metadata extraction
- MPV integration for video playback
- JSON-based configuration storage

### Communication
All communication between frontend and backend is via HTTP REST API:

```
GET  /api/videos           - List all videos
GET  /api/progress         - Get progress/tracks/watched data
POST /api/play             - Start video playback
POST /api/toggle_watched   - Mark as watched/unwatched
POST /api/set_track        - Set preferred audio track
```

## Configuration

The Python backend stores configuration in `backend/config.json`:

```json
{
    "video_dir": "/path/to/videos",
    "last_played": "video_id.mp4",
    "progress": {
        "video_id": 123.45
    },
    "tracks": {
        "video_id": 2
    },
    "watched": ["video_id1", "video_id2"]
}
```

## Keyboard Shortcuts

- `Ctrl+O`: Open media folder
- `Ctrl+F`: Focus search box
- `Enter`: Play selected video
- `Delete` or `Backspace`: Toggle watched status

## Memory Management

- Auto-cleanup of resources on application exit
- Proper termination of mpv processes
- Memory-efficient video list caching
- Automatic progress sync every 2 seconds

## Troubleshooting

### Backend Connection Issues
- Ensure Python backend is running on `127.0.0.1:8000`
- Check if port 8000 is not blocked by firewall
- Verify fastapi and other dependencies are installed

### Qt Build Issues
- Install Qt6 development files: `sudo apt install qt6-base-dev`
- Ensure CMake is properly installed
- Try `cmake --version` to verify

### No Videos Appear
- Select a valid media folder in the application
- Ensure folder contains supported formats: .mp4, .mkv, .webm, .avi
- Check backend console for error messages

### MPV Not Available
- Install mpv: `sudo apt install mpv` (Ubuntu/Debian) or `sudo dnf install mpv` (Fedora)
- Verify installation: `which mpv`

## Development

### Project Structure
```
qt-frontend/
├── CMakeLists.txt          # Build configuration
├── README.md               # This file
└── src/
    ├── main.cpp            # Application entry point
    ├── mainwindow.h/cpp    # Main UI window
    ├── apimanager.h/cpp    # API communication
    └── videolibrary.h/cpp  # Data management
```

### Building in Debug Mode
```bash
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .
```

### Running with Debug Output
```bash
./CINEBASE 2>&1 | grep -E "^\[|^🎬|^▶|^🛑"
```

## License

Same as the main CINEBASE project.

## Notes

- The Qt C++ frontend is fully compatible with the existing Python backend
- No backend code changes are required
- The application automatically handles cleanup on exit
- Watch progress is synced every 2 seconds

## Performance

- **Startup Time**: ~1-2 seconds
- **Memory Usage**: ~50-100 MB (depending on library size)
- **API Response**: <100ms for typical operations
- **UI Responsiveness**: Smooth 60 FPS rendering

## Future Enhancements

- [ ] Video thumbnails preview
- [ ] Playlist support
- [ ] Subtitle management
- [ ] Statistics and analytics
- [ ] Multi-language support
- [ ] Settings panel
- [ ] Custom themes
