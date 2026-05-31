#!/bin/bash

# CINEBASE Complete Setup Script
# Installs all dependencies for both Qt C++ frontend and Python backend

set -e

echo "╔════════════════════════════════════════════════════╗"
echo "║     CINEBASE Complete Installation Script         ║"
echo "║    (Qt C++ Frontend + Python Backend Setup)       ║"
echo "╚════════════════════════════════════════════════════╝"
echo ""

# Detect Linux distribution
if [ -f /etc/os-release ]; then
    . /etc/os-release
    OS=$ID
else
    echo "⚠️  Unable to detect OS. Please install dependencies manually."
    exit 1
fi

echo "📋 Detected OS: $OS"
echo ""

# Update package manager
echo "🔄 Updating package manager..."
case "$OS" in
    ubuntu|debian)
        sudo apt update
        ;;
    fedora)
        sudo dnf check-update
        ;;
    arch)
        sudo pacman -Sy
        ;;
    *)
        echo "⚠️  Unsupported OS. Manual installation required."
        ;;
esac

echo "✓ Package manager updated"
echo ""

# Install dependencies based on OS
case "$OS" in
    ubuntu|debian)
        echo "📦 Installing dependencies for Ubuntu/Debian..."
        
        # Build tools
        echo "  → Build essentials..."
        sudo apt install -y build-essential gcc g++ make cmake
        
        # Qt6
        echo "  → Qt6 development libraries..."
        sudo apt install -y qt6-base-dev qt6-declarative-dev
        
        # Python
        echo "  → Python development..."
        sudo apt install -y python3-dev python3-pip python3-venv
        
        # Media tools
        echo "  → FFmpeg and mpv..."
        sudo apt install -y ffmpeg mpv
        
        # Additional tools
        echo "  → Additional tools..."
        sudo apt install -y git pkg-config
        ;;
        
    fedora)
        echo "📦 Installing dependencies for Fedora..."
        
        # Build tools
        echo "  → Build essentials..."
        sudo dnf install -y @development-tools cmake gcc-c++
        
        # Qt6
        echo "  → Qt6 development libraries..."
        sudo dnf install -y qt6-qtbase-devel qt6-qtdeclarative-devel
        
        # Python
        echo "  → Python development..."
        sudo dnf install -y python3-devel python3-pip
        
        # Media tools
        echo "  → FFmpeg and mpv..."
        sudo dnf install -y ffmpeg mpv
        
        # Additional tools
        echo "  → Additional tools..."
        sudo dnf install -y git pkgconfig
        ;;
        
    arch)
        echo "📦 Installing dependencies for Arch Linux..."
        
        # Build tools
        echo "  → Build essentials..."
        sudo pacman -S --noconfirm base-devel cmake
        
        # Qt6
        echo "  → Qt6 development libraries..."
        sudo pacman -S --noconfirm qt6-base
        
        # Python
        echo "  → Python development..."
        sudo pacman -S --noconfirm python python-pip
        
        # Media tools
        echo "  → FFmpeg and mpv..."
        sudo pacman -S --noconfirm ffmpeg mpv
        
        # Additional tools
        echo "  → Additional tools..."
        sudo pacman -S --noconfirm git pkg-config
        ;;
        
    *)
        echo "❌ Unsupported OS: $OS"
        exit 1
        ;;
esac

echo "✓ System dependencies installed"
echo ""

# Python virtual environment setup
echo "🐍 Setting up Python virtual environment..."
cd "$(dirname "$0")"

if [ ! -d ".venv" ]; then
    python3 -m venv .venv
    echo "✓ Virtual environment created"
else
    echo "✓ Virtual environment already exists"
fi

source .venv/bin/activate
echo "✓ Virtual environment activated"
echo ""

# Install Python requirements
echo "📚 Installing Python requirements..."
if [ -f "requirements.txt" ]; then
    pip install --upgrade pip
    pip install -r requirements.txt
    echo "✓ Python packages installed"
else
    echo "⚠️  requirements.txt not found"
fi
echo ""

# Verify installations
echo "✅ Verifying installations..."
echo ""

echo "Checking system components:"
echo -n "  CMake:  "
cmake --version | head -n1

echo -n "  GCC:    "
gcc --version | head -n1

echo -n "  Python: "
python3 --version

echo -n "  Qt6:    "
if pkg-config --exists Qt6Core; then
    echo "✓ Qt6 found"
else
    echo "✗ Qt6 NOT found (installation may have failed)"
fi

echo -n "  FFmpeg: "
which ffmpeg > /dev/null && echo "✓ Found" || echo "✗ NOT found"

echo -n "  mpv:    "
which mpv > /dev/null && echo "✓ Found" || echo "✗ NOT found"

echo ""
echo "Python packages:"
python3 -m pip list | grep -E "fastapi|uvicorn|ffmpeg-python" || echo "  (Run in activated venv)"

echo ""
echo "╔════════════════════════════════════════════════════╗"
echo "║             Installation Complete! ✓              ║"
echo "╚════════════════════════════════════════════════════╝"
echo ""

echo "📖 Next steps:"
echo "  1. Build Qt C++ frontend:"
echo "     cd qt-frontend && ./build.sh"
echo ""
echo "  2. Start Python backend:"
echo "     source .venv/bin/activate"
echo "     python backend/main.py"
echo ""
echo "  3. Start Qt C++ frontend:"
echo "     ./qt-frontend/build/CINEBASE"
echo ""
echo "For detailed information, see QUICKSTART-QT.md"
echo ""
