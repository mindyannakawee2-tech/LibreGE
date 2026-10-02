#!/usr/bin/env bash

set -e

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

BUILD_DIR="$ROOT/tools/LibreBuilder/build"

echo "=================================="
echo " LibreBuilder installer"
echo "=================================="
echo

if ! command -v cmake >/dev/null 2>&1; then
    echo "ERROR: cmake is not installed."
    echo
    echo "Install it with:"
    echo "sudo apt install cmake"
    exit 1
fi

if ! command -v ninja >/dev/null 2>&1; then
    echo "ERROR: ninja is not installed."
    echo
    echo "Install it with:"
    echo "sudo apt install ninja-build"
    exit 1
fi

mkdir -p "$BUILD_DIR"

cmake \
    -S "$ROOT/tools/LibreBuilder" \
    -B "$BUILD_DIR" \
    -G Ninja \
    -DCMAKE_BUILD_TYPE=Release

cmake --build "$BUILD_DIR"

mkdir -p "$HOME/.local/bin"

cp \
    "$BUILD_DIR/lbbe" \
    "$HOME/.local/bin/lbbe"

chmod +x "$HOME/.local/bin/lbbe"

echo
echo "LibreBuilder installed:"
echo "$HOME/.local/bin/lbbe"
echo

case ":$PATH:" in
    *":$HOME/.local/bin:"*)
        ;;
    *)
        echo "WARNING:"
        echo "~/.local/bin is not currently in PATH."
        echo
        echo "Add this to ~/.bashrc:"
        echo
        echo 'export PATH="$HOME/.local/bin:$PATH"'
        echo
        ;;
esac

echo
echo "Try:"
echo "lbbe parse"
echo "lbbe build"
echo "lbbe run"
