#!/bin/bash
# CINEBASE - Library path wrapper script
# Fixes library conflicts by prioritizing system libraries over snap libraries

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
EXEC_PATH="$SCRIPT_DIR/qt-frontend/build/CINEBASE"

# Strip Snap/VS Code runtime variables. Launching from the Snap build of VS Code
# can otherwise make the system Qt app load /snap/core* glibc libraries.
for var_name in $(env | sed -n 's/^\(SNAP[^=]*\)=.*/\1/p'); do
    unset "$var_name"
done

unset GTK_EXE_PREFIX
unset GTK_PATH
unset GTK_MODULES
unset GTK_IM_MODULE_FILE
unset GIO_MODULE_DIR
unset LOCPATH
unset LD_PRELOAD

if [ -n "${XDG_DATA_DIRS_VSCODE_SNAP_ORIG:-}" ]; then
    export XDG_DATA_DIRS="$XDG_DATA_DIRS_VSCODE_SNAP_ORIG"
else
    export XDG_DATA_DIRS="/usr/local/share:/usr/share:/var/lib/snapd/desktop"
fi

if [ -n "${XDG_CONFIG_DIRS_VSCODE_SNAP_ORIG:-}" ]; then
    export XDG_CONFIG_DIRS="$XDG_CONFIG_DIRS_VSCODE_SNAP_ORIG"
else
    export XDG_CONFIG_DIRS="/etc/xdg"
fi

# Use only system library paths for the Qt executable.
export LD_LIBRARY_PATH="/usr/lib/x86_64-linux-gnu:/usr/lib:/lib/x86_64-linux-gnu:/lib"

# Run the application
if [ -f "$EXEC_PATH" ]; then
    exec "$EXEC_PATH" "$@"
else
    echo "❌ CINEBASE executable not found at: $EXEC_PATH"
    echo "Please build first: make build"
    exit 1
fi
