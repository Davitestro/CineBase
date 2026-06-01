#!/bin/bash

# Run this script to start both backend and frontend
# Usage: ./start-all.sh

set -e

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
QT_BUILD_DIR="$PROJECT_DIR/qt-frontend/build"

echo "╔════════════════════════════════════════════════════╗"
echo "║         CINEBASE Complete Startup Script          ║"
echo "║      (Backend + Qt Frontend in parallel)         ║"
echo "╚════════════════════════════════════════════════════╝"
echo ""

# Colors
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m'
BACKEND_PID=""
FRONTEND_PID=""

cleanup() {
    echo ""
    echo -e "${YELLOW}Stopping CINEBASE processes...${NC}"

    if [ -n "$FRONTEND_PID" ] && kill -0 "$FRONTEND_PID" 2>/dev/null; then
        kill "$FRONTEND_PID" 2>/dev/null || true
        wait "$FRONTEND_PID" 2>/dev/null || true
    fi

    pkill -TERM -f "mpv" 2>/dev/null || true
    sleep 2
    pkill -KILL -f "mpv" 2>/dev/null || true

    if [ -n "$BACKEND_PID" ] && kill -0 "$BACKEND_PID" 2>/dev/null; then
        kill "$BACKEND_PID" 2>/dev/null || true
        wait "$BACKEND_PID" 2>/dev/null || true
    fi
}

trap cleanup EXIT INT TERM

# Check if Qt frontend is built
if [ ! -f "$QT_BUILD_DIR/CINEBASE" ]; then
    echo -e "${YELLOW}⚠️  Qt frontend not built yet.${NC}"
    echo "Building Qt frontend..."
    cd "$PROJECT_DIR/qt-frontend"
    ./build.sh
    echo ""
fi

# Check if venv exists
if [ ! -d ".venv" ]; then
    echo -e "${YELLOW}⚠️  Virtual environment not found.${NC}"
    echo "Creating virtual environment..."
    python3 -m venv .venv
    source .venv/bin/activate
    pip install -r requirements.txt
    echo ""
fi

# Start backend
echo -e "${BLUE}🔵 Starting Python Backend...${NC}"
cd "$PROJECT_DIR"
source .venv/bin/activate

python backend/main.py &
BACKEND_PID=$!
echo -e "${GREEN}✓ Backend started (PID: $BACKEND_PID)${NC}"

# Wait for backend to start
sleep 2

# Start frontend
echo ""
echo -e "${BLUE}🟣 Starting Qt C++ Frontend...${NC}"
"$PROJECT_DIR/cinebase.sh" &
FRONTEND_PID=$!
echo -e "${GREEN}✓ Frontend started (PID: $FRONTEND_PID)${NC}"

echo ""
echo -e "${GREEN}════════════════════════════════════════${NC}"
echo -e "${GREEN}🎬 CINEBASE is Running!${NC}"
echo -e "${GREEN}════════════════════════════════════════${NC}"
echo ""
echo "Backend PID:  $BACKEND_PID"
echo "Frontend PID: $FRONTEND_PID"
echo ""
echo "To stop:"
echo "  kill $BACKEND_PID  (or press Ctrl+C)"
echo "  kill $FRONTEND_PID (or close the window)"
echo ""
echo "Logs:"
echo "  Backend:  Shown above"
echo "  Frontend: CINEBASE window"
echo ""

# When the UI closes, stop the backend too.
wait "$FRONTEND_PID" || true
cleanup
trap - EXIT INT TERM
