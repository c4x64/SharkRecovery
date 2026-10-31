#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-2.0-or-later
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
KP_DIR="${KP_DIR:-$ROOT/.build/KernelPatch}"
# Pinned for reproducible builds.  Override KP_COMMIT to test a newer tree.
KP_COMMIT="${KP_COMMIT:-d4c88d6672687456b299e4b5153c24dc979d789b}"

if [[ -z "${TARGET_COMPILE:-}" ]]; then
    for prefix in aarch64-elf- aarch64-linux-gnu- aarch64-none-elf- aarch64-unknown-linux-gnu-; do
        if command -v "${prefix}gcc" >/dev/null 2>&1; then
            TARGET_COMPILE="$prefix"
            break
        fi
    done
fi

if [[ -z "${TARGET_COMPILE:-}" ]]; then
    echo "error: no aarch64 GCC found." >&2
    echo "  Debian/Ubuntu: sudo apt install gcc-aarch64-linux-gnu" >&2
    echo "  macOS:         brew install messense/macos-cross-toolchains/aarch64-unknown-linux-gnu" >&2
    echo "  Or set TARGET_COMPILE to a cross prefix (with trailing dash)." >&2
    exit 1
fi

if [[ ! -d "$KP_DIR/kernel" ]]; then
    echo "==> Cloning KernelPatch into $KP_DIR"
    mkdir -p "$(dirname "$KP_DIR")"
    git clone --depth 1 https://github.com/bmax121/KernelPatch.git "$KP_DIR"
fi

echo "==> Checking out KernelPatch $KP_COMMIT"
git -C "$KP_DIR" fetch --depth 1 origin "$KP_COMMIT"
git -C "$KP_DIR" checkout --detach "$KP_COMMIT"

echo "==> Building with ${TARGET_COMPILE}gcc"
make -C "$ROOT" clean
make -C "$ROOT" TARGET_COMPILE="$TARGET_COMPILE" KP_DIR="$KP_DIR"
make -C "$ROOT" check TARGET_COMPILE="$TARGET_COMPILE" KP_DIR="$KP_DIR"

echo
echo "Built: $ROOT/samsung-gsi-fix.kpm"
