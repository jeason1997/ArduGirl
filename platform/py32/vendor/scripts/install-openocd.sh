#!/usr/bin/env sh
# Keep this script POSIX-compatible; Linux /bin/sh is commonly dash.
set -eu

OPENOCD_REPOSITORY="https://gitee.com/puya-semiconductor/openocd.git"
OPENOCD_COMMIT="0b648421f7c539024319ea7d04b4aa8abe4830da"

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
COMMON_ROOT=$(dirname "$SCRIPT_DIR")
TOOLS_ROOT="$COMMON_ROOT/tools"
OPENOCD_ROOT="$TOOLS_ROOT/openocd-linux"
OPENOCD="$OPENOCD_ROOT/bin/openocd"
BUILD_ROOT="$TOOLS_ROOT/.openocd-linux-install"

if [ -x "$OPENOCD" ]; then
    printf 'OpenOCD is ready: %s\n' "$OPENOCD"
    exit 0
fi

for command in git make pkg-config cc libtoolize aclocal autoconf autoheader automake; do
    if ! command -v "$command" >/dev/null 2>&1; then
        printf 'Missing build dependency: %s\n' "$command" >&2
        printf 'Install the OpenOCD build dependencies and run make openocd again.\n' >&2
        exit 1
    fi
done

if ! pkg-config --exists libusb-1.0; then
    printf 'Missing build dependency: libusb-1.0 development files\n' >&2
    printf 'Install libusb-1.0-0-dev and run make openocd again.\n' >&2
    exit 1
fi

if ! pkg-config --exists hidapi-hidraw && ! pkg-config --exists hidapi-libusb; then
    printf 'Missing build dependency: hidapi development files\n' >&2
    printf 'Install libhidapi-dev and run make openocd again.\n' >&2
    exit 1
fi

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
    # Puya 快照的顶层生成文件未包含新增 PY32 驱动；只重建顶层，避免递归改写内嵌 JimTcl。
    libtoolize --copy --force
    aclocal -I m4
    autoconf
    autoheader
    automake --add-missing --copy

    # WSL 挂载盘会丢失内嵌项目生成文件的先后顺序，避免它们调用快照硬编码的旧版 aclocal。
    find src/jtag/drivers/libjaylink \( -name configure.ac -o -name Makefile.am \) -exec touch {} +
    sleep 1
    find src/jtag/drivers/libjaylink \( -name aclocal.m4 -o -name configure -o -name config.h.in -o -name Makefile.in \) -exec touch {} +
    # 固定的旧版 OpenOCD 会在新版 GCC 下产生新增诊断；保留警告，但不要把主机工具警告升级为错误。
    ./configure --prefix="$OPENOCD_ROOT" CFLAGS="-O2 -Wno-error"
    make -j"$(getconf _NPROCESSORS_ONLN 2>/dev/null || printf '1')"
    rm -rf "$OPENOCD_ROOT"
    make install
)

printf 'OpenOCD installed: %s\n' "$OPENOCD"
