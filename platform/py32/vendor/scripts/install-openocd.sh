#!/usr/bin/env sh
# Keep this script POSIX-compatible; Linux /bin/sh is commonly dash.
set -eu

OPENOCD_REPOSITORY="https://gitee.com/puya-semiconductor/openocd.git"
OPENOCD_COMMIT="0b648421f7c539024319ea7d04b4aa8abe4830da"

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
COMMON_ROOT=$(dirname "$SCRIPT_DIR")
TOOLS_ROOT="$COMMON_ROOT/tools"
OPENOCD_ROOT="$TOOLS_ROOT/openocd"
OPENOCD="$OPENOCD_ROOT/bin/openocd"
BUILD_ROOT="$TOOLS_ROOT/.openocd-install"

if [ -x "$OPENOCD" ]; then
    printf 'OpenOCD is ready: %s\n' "$OPENOCD"
    exit 0
fi

for command in git make pkg-config cc; do
    if ! command -v "$command" >/dev/null 2>&1; then
        printf 'Missing build dependency: %s\n' "$command" >&2
        printf 'Install the OpenOCD build dependencies and run make openocd again.\n' >&2
        exit 1
    fi
done

rm -rf "$BUILD_ROOT"
mkdir -p "$BUILD_ROOT"
trap 'rm -rf "$BUILD_ROOT"' EXIT HUP INT TERM

printf 'Building OpenOCD from Puya official source (%s)...\n' "$OPENOCD_COMMIT"
git -C "$BUILD_ROOT" init source
git -C "$BUILD_ROOT/source" remote add origin "$OPENOCD_REPOSITORY"
git -C "$BUILD_ROOT/source" fetch --depth 1 origin "$OPENOCD_COMMIT"
git -C "$BUILD_ROOT/source" checkout --detach FETCH_HEAD
git -C "$BUILD_ROOT/source" submodule update --init --recursive --depth 1

(
    cd "$BUILD_ROOT/source"
    ./configure --prefix="$OPENOCD_ROOT"
    make -j"$(getconf _NPROCESSORS_ONLN 2>/dev/null || printf '1')"
    rm -rf "$OPENOCD_ROOT"
    make install
)

printf 'OpenOCD installed: %s\n' "$OPENOCD"
