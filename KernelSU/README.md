# KernelSU Integration for Shark Recovery

KernelSU (LKMSU) source and integration for Samsung Galaxy F22 (SM-E225F).

## Overview

This directory contains the complete KernelSU source tree for building KernelSU as a Loadable Kernel Module (LKM) for Shark Recovery on Samsung Galaxy F22.

## Source

Full KernelSU source tree cloned from:
https://github.com/tiann/KernelSU

Version: Active development branch

## Directory Structure

```
KernelSU/
├── src/              # Full KernelSU source code
│   ├── kernel/       # Kernel module source
│   ├── userspace/    # Userspace daemon
│   ├── manager/      # Android manager app
│   └── ...
├── config.sh         # Device build configuration
├── KSUCONFIG         # KernelSU config
├── README.md         # This file
└── .gitignore        # Build outputs
```

## Building KernelSU

### Prerequisites
- aarch64-linux-gnu toolchain
- Rust (for userspace builds)
- Kernel source for mt6769t

### Build Commands

```bash
cd KernelSU/src

# Build kernel module
make ARCH=arm64 \
     CROSS_COMPILE=aarch64-linux-gnu- \
     KERNEL_DIR=../kernel \
     -j$(nproc)

# Build userspace daemon
cd userspace
cargo build --release
```

### Integration with TWRP

KernelSU is integrated into Shark Recovery via:
- `device/samsung/f22/ksu_integration.mk`
- BoardConfig.mk: `TW_INCLUDE_KSU := true`
- Automatic LKM compilation during recovery build

## Configuration

Device-specific config in `config.sh`:
- Architecture: arm64
- Kernel version: 4.19
- Platform: mt6769t
- Toolchain: aarch64-linux-gnu-

## License

KernelSU is licensed under GPL-2.0

Original work by tiann
https://github.com/tiann/KernelSU

## Credits

- KernelSU Team (tiann)
- Shark Recovery Team
- MediaTek kernel sources
