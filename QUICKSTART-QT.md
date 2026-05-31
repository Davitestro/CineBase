# CINEBASE Qt C++ - Quick Start Guide

## Overview

You now have a **native Qt C++ desktop application** replacing your React web frontend. The Python backend remains completely unchanged.

## File Structure

```
Movie for/
├── backend/                    (Python FastAPI - UNCHANGED)
│   ├── main.py
│   ├── config.json
│   └── requirements.txt
│
├── frontend/                   (Old React frontend - can remove)
│   └── src/
│
└── qt-frontend/               (NEW - Qt C++ Frontend)
    ├── CMakeLists.txt
    ├── src/
    │   ├── main.cpp
    │   ├── mainwindow.h/cpp
    │   ├── apimanager.h/cpp
    │   └── videolibrary.h/cpp
    ├── build.sh              (Linux/Mac build script)
    ├── build.bat             (Windows build script)
    └── README.md
```

## Installation & Setup

### Step 1: Install Dependencies

**Ubuntu/Debian:**
```bash
sudo apt update
sudo apt install -y build-essential cmake qt6-base-dev qt6-declarative-dev git
```

**Fedora:**
```bash
sudo dnf install -y @development-tools cmake qt6-qtbase-devel git
```

**macOS:**
```bash
brew install cmake qt@6
export PATH="/usr/local/opt/qt@6/bin:$PATH"
```

**Windows:**
- Download and install CMake: https://cmake.org/download/
- Download and install Qt6: https://www.qt.io/download-open-source
- Install Visual Studio or MinGW for build tools

### Step 2: Build the Qt Frontend

**Linux/macOS:**
```bash
cd "Movie for/qt-frontend"
chmod +x build.sh
./build.sh
```

**Windows:**
```cmd
cd "Movie for\qt-frontend"
build.bat
```

Or manually:
```bash
cd qt-frontend
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

### Step 3: Ensure Backend is Ready

```bash
# Make sure Python requirements are installed
cd "Movie for"
pip install -r requirements.txt
```

### Step 4: Run the Application

**Terminal 1 - Start the Python Backend:**
```bash
cd "Movie for"
source .venv/bin/activate  # if using venv
python backend/main.py
```

You should see:
```
INFO:     Uvicorn running on http://127.0.0.1:8000 (Press CTRL+C to quit)
```

**Terminal 2 - Start the Qt Frontend:**
```bash
cd "Movie for/qt-frontend/build"
./CINEBASE
```

The application window will appear! 🎬

## Using CINEBASE

### Main Features

1. **Select Media Folder**: Click "📂 Media Root" button to choose your video directory
2. **Search**: Type in the search box to filter videos
3. **Play Video**: 
   - Click on a video in the list to select it
   - Click "▶ Play" to start playback
4. **Audio Tracks**: Select preferred audio track before playing
5. **Mark Watched**: Click "☐ Mark as Watched" to track progress
6. **Progress Tracking**: Automatically saves play position from mpv

### Keyboard Shortcuts (Coming Soon)
- `Ctrl+O`: Open folder dialog
- `Ctrl+F`: Focus search
- `Enter`: Play selected video

## Features

✅ **Same Functionality as React Version:**
- Video library browsing
- Audio track selection
- Watch progress tracking
- Watched status management
- Last played video memory

✅ **Improvements:**
- Native desktop application (no browser needed)
- Better memory management
- Automatic cleanup on exit
- Modern dark UI (#0B0B0F background, #a892ff accents)
- Responsive design

✅ **Auto-Cleanup:**
- Kills mpv process when app closes
- Saves all configuration automatically
- No manual cleanup needed

## Troubleshooting

### "Connection refused" error
- Make sure Python backend is running on `127.0.0.1:8000`
- Check if another app is using port 8000

### No videos appear
- Click "📂 Media Root" to select your video folder
- Wait 2-3 seconds for videos to load
- Check backend console for errors

### Video won't play
- Ensure mpv is installed: `sudo apt install mpv` (Ubuntu)
- Check if video file is in a supported format (.mp4, .mkv, .webm, .avi)

### Build fails
- Verify Qt6 is installed: `pkg-config --exists Qt6Core && echo "OK" || echo "Qt6 not found"`
- Try clearing build: `rm -rf qt-frontend/build` then rebuild

### Memory/Performance Issues
- The Qt app automatically cleans up resources
- Close app properly (click X button) to trigger cleanup
- Check CPU usage in system monitor if slow

## Configuration

All settings are stored in `backend/config.json`:
- Video directory
- Last played video
- Play progress for each video
- Audio track preferences
- List of watched videos

This file is managed by the backend and synced with the Qt frontend.

## Development Notes

### Backend Communication
The Qt app communicates with Python via REST API:
- Endpoint: `http://127.0.0.1:8000/api`
- All requests use JSON format
- Updates auto-sync every 2 seconds

### Architecture
- **MainWindow**: UI container with sidebar + content panel
- **ApiManager**: HTTP client for backend communication
- **VideoLibrary**: Data model/cache

### Building for Release
```bash
cd qt-frontend/build
cmake --build . --config Release --parallel
```

## System Requirements

| Component | Minimum | Recommended |
|-----------|---------|-------------|
| OS | Windows 10, Ubuntu 20.04, macOS 10.15 | Latest LTS |
| CPU | Dual-core 2.0 GHz | Quad-core 2.5 GHz |
| RAM | 2 GB | 4 GB |
| Storage | 500 MB | 1 GB |
| Display | 800x600 | 1920x1080 |

## Removing Old Frontend

If you no longer need the React web frontend:
```bash
rm -rf "Movie for/frontend"
```

## Next Steps

1. ✅ Build the Qt C++ app (you just did this!)
2. ✅ Run both backend and frontend
3. Select your media folder
4. Browse and play videos
5. Enjoy the native desktop experience!

## Support

For issues:
1. Check the `README.md` in `qt-frontend/` for detailed documentation
2. Review console output for error messages
3. Ensure all dependencies are properly installed
4. Try rebuilding with a clean build directory

## Version Info

- **CINEBASE Qt C++**: Built with Qt 6.x
- **Backend**: Python 3.8+ with FastAPI
- **Build System**: CMake 3.16+
- **C++ Standard**: C++17

---

**Happy watching!** 🎬
