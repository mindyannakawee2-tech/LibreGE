#!/usr/bin/env bash

set -e

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

cd "$ROOT/build"

if [ ! -x ./LibreGE ]; then
    echo "LibreGE isn't built."
    echo
    echo "Run:"
    echo "  lbbe build"
    exit 1
fi

LIBREGE_PLAYER_NAME="Alice" \
    ./LibreGE &

CLIENT1=$!

sleep 0.5

LIBREGE_PLAYER_NAME="Bob" \
    ./LibreGE &

CLIENT2=$!

echo
echo "Started:"
echo "  Alice PID: $CLIENT1"
echo "  Bob   PID: $CLIENT2"
echo
echo "Use WASD independently in each window."

wait
