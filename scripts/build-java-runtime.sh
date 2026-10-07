#!/usr/bin/env bash

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

cd "$ROOT"

JAVA_BUILD="$ROOT/build/java"

rm -rf "$JAVA_BUILD"

mkdir -p \
    "$JAVA_BUILD/api" \
    "$JAVA_BUILD/host" \
    "$JAVA_BUILD/server" \
    "$JAVA_BUILD/sample"

echo "=========================================="
echo " Building LibreGE Java Runtime"
echo "=========================================="

if ! command -v javac >/dev/null 2>&1; then
    echo "ERROR: javac is missing."
    exit 1
fi

if ! command -v jar >/dev/null 2>&1; then
    echo "ERROR: jar is missing."
    exit 1
fi

# ------------------------------------------------------------
# Plugin API
# ------------------------------------------------------------

find java/plugin-api/src \
    -name '*.java' \
    -print0 \
    | xargs -0 javac \
        -d "$JAVA_BUILD/api"

jar \
    --create \
    --file "$JAVA_BUILD/LibrePluginAPI.jar" \
    -C "$JAVA_BUILD/api" .

# ------------------------------------------------------------
# Plugin Host
# ------------------------------------------------------------

find java/plugin-host/src \
    -name '*.java' \
    -print0 \
    | xargs -0 javac \
        -cp "$JAVA_BUILD/LibrePluginAPI.jar" \
        -d "$JAVA_BUILD/host"

cat > "$JAVA_BUILD/host-manifest.mf" <<'MANIFEST'
Manifest-Version: 1.0
Main-Class: librege.host.Main
Class-Path: LibrePluginAPI.jar

MANIFEST

jar \
    --create \
    --file "$JAVA_BUILD/LibrePluginHost.jar" \
    --manifest "$JAVA_BUILD/host-manifest.mf" \
    -C "$JAVA_BUILD/host" .

# ------------------------------------------------------------
# Sample plugin
# ------------------------------------------------------------

find java/sample-plugin/src \
    -name '*.java' \
    -print0 \
    | xargs -0 javac \
        -cp "$JAVA_BUILD/LibrePluginAPI.jar" \
        -d "$JAVA_BUILD/sample"

jar \
    --create \
    --file "$JAVA_BUILD/SamplePlugin.jar" \
    --manifest \
    java/sample-plugin/resources/META-INF/MANIFEST.MF \
    -C "$JAVA_BUILD/sample" .

# ------------------------------------------------------------
# SERVER.jar
#
# Java standard library only for the test server.
# ------------------------------------------------------------

find java/server/src \
    -name '*.java' \
    -print0 \
    | xargs -0 javac \
        -d "$JAVA_BUILD/server"

cat > "$JAVA_BUILD/server-manifest.mf" <<'MANIFEST'
Manifest-Version: 1.0
Main-Class: librege.server.Main

MANIFEST

jar \
    --create \
    --file "$JAVA_BUILD/SERVER.jar" \
    --manifest "$JAVA_BUILD/server-manifest.mf" \
    -C "$JAVA_BUILD/server" .

# ------------------------------------------------------------
# Runtime
# ------------------------------------------------------------

mkdir -p \
    build/runtime \
    build/plugins

cp \
    "$JAVA_BUILD/LibrePluginAPI.jar" \
    build/runtime/

cp \
    "$JAVA_BUILD/LibrePluginHost.jar" \
    build/runtime/

cp \
    "$JAVA_BUILD/SERVER.jar" \
    build/runtime/

cp \
    "$JAVA_BUILD/SamplePlugin.jar" \
    build/plugins/

echo
echo "Created:"
echo "  build/runtime/SERVER.jar"
echo "  build/runtime/LibrePluginAPI.jar"
echo "  build/runtime/LibrePluginHost.jar"
echo "  build/plugins/SamplePlugin.jar"
