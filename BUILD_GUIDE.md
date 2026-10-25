# Shark Recovery Build Guide

## Quick Start

### Local Build

```bash
# 1. Prerequisites
sudo apt update
sudo apt install repo git openjdk-17-jdk build-essential

# 2. Setup environment
mkdir -p ~/shark-build && cd ~/shark-build

# 3. Init TWRP source
repo init -u https://github.com/minimal-manifest-twrp/platform_manifest_twrp_aosp.git -b twrp-12
repo sync -c -j$(nproc)

# 4. Copy device tree
cp -r /Users/prabhas/SM-E225F/device/samsung/f22 device/samsung/

# 5. Configure build
. build/envsetup.sh
lunch twrp_f22-eng

# 6. Build
make -j$(nproc) recoveryimage
```

Build output: `~/shark-build/out/target/product/f22/recovery.img`

### VM/Lightweight Build

For lightweight VM builds, use Docker:

```bash
docker run --rm -v $(pwd):/work -w /work \
  ubuntu:22.04 bash build.sh
```

## KernelSU Integration

### LKMSU Build Steps

1. Clone KernelSU
```bash
cd device/samsung/f22/KernelSU
git clone https://github.com/tiann/KernelSU.git src
```

2. Configure
```bash
source config.sh
```

3. Build LKM
```bash
cd src
make KERNEL_DIR=../kernel ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu-
```

4. The module will be at: `kernelsu.ko`

## CI/CD Build

GitHub Actions builds automatically on:
- Push to main
- Pull requests
- Nightly schedule (2 AM UTC)

Artifacts available in Actions tab.

## Troubleshooting

### Build Errors

**Out of memory:**
```bash
make -j2 recoveryimage  # Reduce threads
```

**Missing dependencies:**
Check `build.sh` or install via apt

**KernelSU build fails:**
Ensure kernel source matches device (4.19 for mt6769t)

## Custom Logo

Replace logo at:
`prebuilt/logos/shark_logo.png`

Format: PNG, 1024x1024, transparency OK

## Device Specifics

- **Board**: mt6769t (Helio G80)
- **Android**: 11/12
- **Kernel**: 4.19
- **Recovery partition**: 32MB
- **Encryption**: FBE support
