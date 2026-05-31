#!/bin/bash

# CINEBASE Qt C++ Build and Run Script

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"

echo "╔════════════════════════════════════╗"
echo "║         CINEBASE Build Script      ║"
echo "╚════════════════════════════════════╝"
echo ""

# Colors
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check if cmake is installed
if ! command -v cmake &> /dev/null; then
    echo -e "${YELLOW}CMake not found. Please install CMake 3.16 or later.${NC}"
    echo "Ubuntu/Debian: sudo apt install cmake"
    echo "Fedora: sudo dnf install cmake"
    echo "Arch: sudo pacman -S cmake"
    exit 1
fi

# Check if Qt6 is installed
if ! pkg-config --exists Qt6Core; then
    echo -e "${YELLOW}Qt6 not found. Please install Qt6 development libraries.${NC}"
    echo "Ubuntu/Debian: sudo apt install qt6-base-dev"
    echo "Fedora: sudo dnf install qt6-qtbase-devel"
    echo "Arch: sudo pacman -S qt6-base"
    exit 1
fi

# Create build directory
if [ ! -d "$BUILD_DIR" ]; then
    echo -e "${BLUE}Creating build directory...${NC}"
    mkdir -p "$BUILD_DIR"
fi

# Configure CMake
echo -e "${BLUE}Configuring CMake...${NC}"
cd "$BUILD_DIR"
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
echo -e "${BLUE}Building CINEBASE...${NC}"
cmake --build . --config Release --parallel $(nproc)

echo -e "${GREEN}✓ Build successful!${NC}"
echo ""
echo -e "${BLUE}Build artifacts:${NC}"
echo "  Executable: $BUILD_DIR/CINEBASE"
echo "  Data dir:   $SCRIPT_DIR"
echo ""

# Ask to run when launched from an interactive terminal.
if [ -t 0 ]; then
    read -p "Would you like to run CINEBASE now? (y/n) " -n 1 -r
    echo ""
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        echo -e "${BLUE}Starting CINEBASE...${NC}"
        "$BUILD_DIR/CINEBASE"
    fi
fi
