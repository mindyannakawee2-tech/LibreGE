#!/usr/bin/env bash

set -e

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

cd "$ROOT"

if [ ! -f build/runtime/SERVER.jar ]; then
    echo "SERVER.jar isn't built."
    echo
    echo "Run:"
    echo "  lbbe build"
    exit 1
fi

cd build

exec java \
    -jar runtime/SERVER.jar
