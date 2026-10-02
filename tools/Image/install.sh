#!/usr/bin/env bash

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"

echo "=================================="
echo " LibreGE Image Tool Installer"
echo "=================================="

if ! command -v cmake >/dev/null 2>&1; then
    echo "ERROR: CMake is not installed."
    exit 1
fi

if ! command -v ninja >/dev/null 2>&1; then
    echo "ERROR: Ninja is not installed."
    exit 1
fi

if [ ! -f "$ROOT/vendor/stb/stb_image.h" ]; then
    echo "[Image] Initializing dependencies..."

    cd "$ROOT"

    git submodule update --init --recursive
fi

rm -rf "$BUILD_DIR"

cmake \
    -S "$SCRIPT_DIR" \
    -B "$BUILD_DIR" \
    -G Ninja \
    -DCMAKE_BUILD_TYPE=Release

cmake --build "$BUILD_DIR"

mkdir -p "$HOME/.local/bin"

cp \
    "$BUILD_DIR/libre-image" \
    "$HOME/.local/bin/libre-image"

chmod +x "$HOME/.local/bin/libre-image"

echo
echo "Installed successfully:"
echo "$HOME/.local/bin/libre-image"
echo

echo "Try:"
echo "  libre-image help"
