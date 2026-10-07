#!/usr/bin/env bash

set -e

cd "$(
    dirname "$0"
)"

if [ ! -f build/runtime/SERVER.jar ]; then
    echo "SERVER.jar is not built."
    echo
    echo "Run:"
    echo "  lbbe build"
    exit 1
fi

cd build

exec java \
    -cp \
    "runtime/SERVER.jar:runtime/LibrePluginAPI.jar" \
    librege.server.Main
