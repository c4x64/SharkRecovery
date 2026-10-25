# KernelSU for SM-E225F

KernelSU source and integration for Shark Recovery on Samsung Galaxy F22.

## Overview

This directory contains KernelSU (LKMSU) source for building KernelSU as a Loadable Kernel Module for the Samsung Galaxy F22.

KernelSU provides root access through kernel-level integration without modifying the boot image.

## Directory Structure

```
KernelSU/
├── README.md          # This file
├── KSUCONFIG          # KernelSU configuration
├── config.sh          # Device specific configuration
├── kernel/           # Kernel source (mt6769t)
├── src/              # KernelSU source (clone from tiann/KernelSU)
└── patches/          # Device specific patches
```

## Setup

### 1. Clone KernelSU Source

```bash
cd /path/to/shark-recovery-f22/KernelSU
git clone https://github.com/tiann/KernelSU.git src
```

### 2. Configure Device

The `config.sh` file contains device-specific configuration:
- Architecture: arm64
- Kernel version: 4.19
- SoC: mt6769t
- Module output paths

### 3. Build KernelSU

```bash
cd src
make -j$(nproc) ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu-
```

This builds the LKM `.ko` module.

## Integration with TWRP Build

KernelSU is automatically integrated via the device tree:

1. Copy `KernelSU` to TWRP source: `device/samsung/f22/KernelSU/`
2. Enable in BoardConfig.mk: `TW_INCLUDE_KSU := true`
3. Build recovery: `make recoveryimage`

The build system will:
- Compile KernelSU LKM
- Bundle it into recovery ramdisk
- Create installer zip

## Configuration Files

### KSUCONFIG
KernelSU build configuration for device:
```
CONFIG_KSU=y
CONFIG_KSU_SHOW_BROKEN=y
CONFIG_KSU_DEBUG=y
CONFIG_KSU_KSU_SUPPORT=y
```

### config.sh
Device build configuration:
```bash
export ARCH=arm64
export CROSS_COMPILE=aarch64-linux-gnu-
export KERNEL_DEFCONFIG=mt6769t_defconfig
```

## Building for CI

CI builds automatically:
1. Clone KernelSU from tiann/KernelSU
2. Apply device patches from `patches/`
3. Build LKM module
4. Package into recovery image
5. Create installer zip

## Testing

After flashing Shark Recovery:
1. Boot into recovery
2. Go to Advanced → KernelSU
3. Install KernelSU app
4. Check root access

## Troubleshooting

**Build fails?**
- Ensure correct toolchain: aarch64-linux-gnu-
- Check kernel config includes modules
- Verify defconfig matches device

**Module loads but doesn't work?**
- Check kernel version matches (4.19.x)
- Verify module signature
- Check dmesg for errors

## Credits

- KernelSU: tiann @ github.com/tiann/KernelSU
- Samsung MediaTek kernel sources
- Shark Recovery Team

## License

GPL-2.0
