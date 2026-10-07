#!/usr/bin/env bash

set -e

cd "$(
    dirname "$0"
)/build"

exec java \
    -cp \
    "runtime/LibrePluginHost.jar:runtime/LibrePluginAPI.jar" \
    librege.host.Main \
    client
