#!/bin/bash
# KernelSU Configuration for Samsung Galaxy F22 (SM-E225F)
# Shark Recovery

# Device Configuration
export DEVICE_NAME="f22"
export DEVICE_CODENAME="SM-E225F"
export DEVICE_ARCH="arm64"
export DEVICE_KERNEL="4.19"

# KernelSU Configuration
export KSU_ARCH="arm64"
export KSU_KERNEL_VERSION="4.19"
export KSU_DEFCONFIG="mt6769t_defconfig"

# Toolchain Configuration
export CROSS_COMPILE="aarch64-linux-gnu-"
export CROSS_COMPILE_ARM32="arm-linux-gnueabi-"
export ARCH="arm64"

# Build Configuration
export KERNEL_SRC_PATH="kernel/"
export KSU_SRC_PATH="src/"
export OUTPUT_PATH="out/"
export MODULE_NAME="kernelsu.ko"

# MTK Configuration
export MTK_ARCH="arm64"
export MTK_PLATFORM="mt6769t"
export MTK_VENDOR="samsung"

# KernelSU Options
export CONFIG_KSU=y
export CONFIG_KSU_SHOW_BROKEN=y
export CONFIG_KSU_DEBUG=n
export CONFIG_KSU_KPM=y

# Logging
export VERBOSE_BUILD=1

# CI Configuration
if [ "$CI" = true ]; then
    export KSU_BUILD_DEBUG=n
    export KSU_BUILD_OPTIMIZE=y
fi

echo "=== KernelSU Config for $DEVICE_CODENAME ==="
echo "Arch: $DEVICE_ARCH"
echo "Kernel: $DEVICE_KERNEL"
echo "Platform: $MTK_PLATFORM"
echo "========================================"
