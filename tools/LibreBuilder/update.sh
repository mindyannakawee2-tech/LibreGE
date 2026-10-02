#!/usr/bin/env bash

set -euo pipefail

ROOT="$(git rev-parse --show-toplevel 2>/dev/null || true)"

if [ -z "$ROOT" ]; then
    echo "[LibreBuilder] ERROR: Not inside a Git repository."
    exit 1
fi

cd "$ROOT"

echo "=========================================="
echo " LibreBuilder Engine Updater"
echo "=========================================="
echo

# ------------------------------------------------------------
# Engine-owned files/directories
#
# IMPORTANT:
# User project files must NOT be placed here.
# ------------------------------------------------------------

MANAGED_PATHS=(
    "src/graphics"
    "tools/LibreBuilder"
    "vendor"
    "CMakeLists.txt"
    "install-lbbe.sh"
)

# ------------------------------------------------------------
# Determine remote
# ------------------------------------------------------------

if ! git remote get-url origin >/dev/null 2>&1; then
    echo "[LibreBuilder] ERROR: Git remote 'origin' does not exist."
    exit 1
fi

REMOTE_URL="$(git remote get-url origin)"

echo "[LibreBuilder] Repository:"
echo "  $REMOTE_URL"
echo

# ------------------------------------------------------------
# Determine upstream branch
# ------------------------------------------------------------

UPSTREAM_BRANCH="main"

echo "[LibreBuilder] Checking for updates..."
echo

git fetch origin "$UPSTREAM_BRANCH"

REMOTE_COMMIT="$(git rev-parse "origin/$UPSTREAM_BRANCH")"

echo "[LibreBuilder] Latest engine revision:"
echo "  $REMOTE_COMMIT"
echo

# ------------------------------------------------------------
# Protect locally modified engine files
# ------------------------------------------------------------

ENGINE_DIRTY=0

for path in "${MANAGED_PATHS[@]}"; do
    if [ -e "$path" ]; then
        if ! git diff --quiet -- "$path"; then
            echo "[LibreBuilder] Modified engine file detected:"
            echo "  $path"
            ENGINE_DIRTY=1
        fi

        if ! git diff --cached --quiet -- "$path"; then
            echo "[LibreBuilder] Staged engine modification detected:"
            echo "  $path"
            ENGINE_DIRTY=1
        fi
    fi
done

if [ "$ENGINE_DIRTY" -ne 0 ]; then
    echo
    echo "=========================================="
    echo " UPDATE CANCELLED"
    echo "=========================================="
    echo
    echo "LibreBuilder found local modifications"
    echo "inside engine-managed files."
    echo
    echo "Nothing has been overwritten."
    echo
    echo "Commit or move those changes before updating."
    exit 1
fi

# ------------------------------------------------------------
# Backup current engine
# ------------------------------------------------------------

BACKUP_ROOT="$ROOT/.librege/backups"
TIMESTAMP="$(date +%Y%m%d-%H%M%S)"
BACKUP_DIR="$BACKUP_ROOT/$TIMESTAMP"

mkdir -p "$BACKUP_DIR"

echo "[LibreBuilder] Creating safety backup:"
echo "  $BACKUP_DIR"
echo

for path in "${MANAGED_PATHS[@]}"; do
    if [ -e "$path" ]; then
        mkdir -p "$BACKUP_DIR/$(dirname "$path")"

        cp -a \
            "$path" \
            "$BACKUP_DIR/$path"
    fi
done

# ------------------------------------------------------------
# Record old revision
# ------------------------------------------------------------

mkdir -p .librege

CURRENT_UPSTREAM=""

if [ -f ".librege/upstream-commit" ]; then
    CURRENT_UPSTREAM="$(cat .librege/upstream-commit)"
fi

if [ "$CURRENT_UPSTREAM" = "$REMOTE_COMMIT" ]; then
    echo "[LibreBuilder] LibreGE is already up to date."
    rm -rf "$BACKUP_DIR"
    exit 0
fi

# ------------------------------------------------------------
# Update engine-owned paths only
# ------------------------------------------------------------

echo
echo "[LibreBuilder] Updating engine files..."
echo

for path in "${MANAGED_PATHS[@]}"; do

    # Does this path exist upstream?
    if git cat-file -e "origin/$UPSTREAM_BRANCH:$path" 2>/dev/null; then

        echo "  Updating: $path"

        git restore \
            --source="origin/$UPSTREAM_BRANCH" \
            --worktree \
            --staged \
            -- "$path"

    else
        echo "  Upstream does not contain: $path"
    fi

done

# ------------------------------------------------------------
# Update Git submodules
# ------------------------------------------------------------

echo
echo "[LibreBuilder] Updating engine dependencies..."

git submodule sync --recursive
git submodule update --init --recursive

# ------------------------------------------------------------
# Store upstream engine revision
# ------------------------------------------------------------

echo "$REMOTE_COMMIT" > .librege/upstream-commit

# ------------------------------------------------------------
# Reinstall LibreBuilder if its source changed
# ------------------------------------------------------------

if [ -x "./install-lbbe.sh" ]; then

    echo
    echo "[LibreBuilder] Updating LibreBuilder..."

    ./install-lbbe.sh

fi

echo
echo "=========================================="
echo " LibreGE update complete"
echo "=========================================="
echo
echo "Engine revision:"
echo "  $REMOTE_COMMIT"
echo
echo "Backup:"
echo "  $BACKUP_DIR"
echo
echo "Project files were left untouched."
