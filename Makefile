.PHONY: help install build build-qt run run-qt run-backend clean

help:
	@echo "╔════════════════════════════════════════════════════╗"
	@echo "║           CINEBASE - Available Commands            ║"
	@echo "╚════════════════════════════════════════════════════╝"
	@echo ""
	@echo "Setup:"
	@echo "  make install          - Install all dependencies"
	@echo ""
	@echo "Build:"
	@echo "  make build            - Build Qt C++ frontend"
	@echo "  make build-qt         - Build Qt C++ frontend (same as build)"
	@echo ""
	@echo "Run:"
	@echo "  make run              - Run backend + Qt frontend (recommended)"
	@echo "  make run-backend      - Run Python backend only"
	@echo "  make run-qt           - Run Qt frontend only"
	@echo ""
	@echo "Maintenance:"
	@echo "  make clean            - Clean build artifacts"
	@echo "  make help             - Show this help message"
	@echo ""

install:
	@echo "📦 Installing dependencies..."
	@chmod +x install-dependencies.sh
	@./install-dependencies.sh

build: build-qt

build-qt:
	@echo "🔨 Building Qt C++ Frontend..."
	@echo "Cleaning previous build..."
	@rm -rf qt-frontend/build
	@cd qt-frontend && chmod +x build.sh && ./build.sh

run:
	@echo "🎬 Starting CINEBASE (Backend + Qt Frontend)..."
	@chmod +x start-all.sh
	@./start-all.sh

run-backend:
	@echo "🔵 Starting Python Backend..."
	@. .venv/bin/activate && python3 backend/main.py

run-qt:
	@echo "🟣 Starting Qt C++ Frontend..."
	@if [ ! -f qt-frontend/build/CINEBASE ]; then \
		echo "❌ Qt frontend not built. Running: make build-qt"; \
		$(MAKE) build-qt; \
	fi
	@chmod +x cinebase.sh
	@./cinebase.sh

clean:
	@echo "🧹 Cleaning build artifacts..."
	@rm -rf qt-frontend/build
	@echo "✓ Clean complete"
