# Shark Recovery for Samsung Galaxy F22 (SM-E225F)

Custom recovery based on TWRP with KernelSU (LKMSU) integration for Samsung Galaxy F22 (SM-E225F). **Shark Recovery** - a TWRP fork with custom branding and KernelSU support.

## 🦈 Device Information
- **Device**: Samsung Galaxy F22 4G
- **Model Numbers**: SM-E225F, SM-E225F/DS
- **Codename**: f22
- **SoC**: MediaTek Helio G80 (mt6769t)
- **Android Version**: Android 11/12
- **Kernel**: Linux 4.19
- **Recovery**: Shark Recovery (TWRP based)

## ✨ Features
- 🦈 **Shark Recovery Branding** - Custom shark logo and UI theme
- 🔓 **KernelSU (LKMSU)** - Next-gen KernelSU via LKM module
- 🔐 FBE (File-Based Encryption) support
- 💾 USB OTG support
- 📁 NTFS support
- ⚡ Fast boot / recovery switching
- 🎨 Custom theming with portrait_hdpi

## 🏗️ Building

### Prerequisites
```bash
# Install repo tool
curl https://storage.googleapis.com/git-repo-downloads/repo > ~/bin/repo
chmod a+x ~/bin/repo

# Initialize TWRP source
mkdir -p ~/twrp && cd ~/twrp
repo init -u https://github.com/minimal-manifest-twrp/platform_manifest_twrp_aosp.git -b twrp-12
repo sync -c
```

### Setup Device Tree
```bash
# Copy device tree to source
mkdir -p ~/twrp/device/samsung
cp -r /path/to/shark-recovery-f22/device/samsung/f22 ~/twrp/device/samsung/
cp -r /path/to/shark-recovery-f22/device/samsung/f22/prebuilt/* ~/twrp/device/samsung/f22/prebuilt/

# Copy KernelSU source
git clone https://github.com/tiann/KernelSU ~/twrp/kernel/kernel-su
```

### Build Commands
```bash
cd ~/twrp
. build/envsetup.sh
lunch twrp_f22-eng
make -j$(nproc) recoveryimage
```

The build includes:
- Shark logo integration
- KernelSU (LKMSU) LKM module
- Custom recovery UI
- Device-specific patches

## 🚀 CI/CD

Automatic builds are available via GitHub Actions:
- **Main branch**: Builds on every push
- **Nightly builds**: 2 AM UTC daily
- **Manual triggers**: Via workflow dispatch
- **KernelSU variants**: Available in build matrix

Build artifacts:
- `recovery.img` - Shark Recovery image
- `KernelSU.zip` - KSU installer for device
- Release packages with changelog

## 📂 Repository Structure

```
.
├── device/samsung/f22/       # Device tree
│   ├── BoardConfig.mk        # Build configuration
│   ├── device.mk             # Device packages
│   ├── twrp_f22.mk          # Product definition
│   ├── prebuilt/            # Custom logos
│   ├── recovery/            # Recovery files
│   └── KernelSU/            # KSU integration
├── KernelSU/                # KernelSU source
├── prebuilt/logos/          # Shark logo assets
├── .github/workflows/       # CI/CD pipelines
└── README.md
```

## 🦈 Customization

### Logo Replacement
The shark logo is in `prebuilt/logos/shark_logo.png`
Size: 1024x1024 PNG (transparency supported)

### Branding Changes
Edit `BoardConfig.mk`:
```makefile
TW_DEVICE_VERSION := shark-recovery-f22
TW_DEVICE_CUSTOM_NAME := "Galaxy F22 (SM-E225F)"
```

## 🔧 KernelSU Integration

KernelSU is built as an LKM (Loadable Kernel Module) for this device:

**Source in repo**:
- `KernelSU/src/` - KernelSU source tree
- `KernelSU/config.sh` - Device config
- `device/samsung/f22/kernel/` - Kernel source

Build process automatically:
1. Compiles KernelSU as LKM
2. Bundles with recovery
3. Generates installer zip

## 📋 Requirements for Building

```bash
# Ubuntu/Debian
sudo apt update
sudo apt install git curl repo openjdk-17-jdk build-essential \
    bc bison flex gcc-multilib g++-multilib \
    lib32z1-dev lib32ncurses5-dev lib32readline-dev \
    lib32stdc++6 lib32z1 liblz4-tool libncurses5-dev \
    libssl-dev lzop m4 texinfo unzip zlib1g-dev
```

## 🤝 Credits
- **Shark Recovery Team**
- Base TWRP: akhil1999
- TeamWin Recovery Project
- KernelSU Team (tiann)
- Samsung MediaTek team

## 📄 License
Apache 2.0 for device tree
GPL v2 for KernelSU

---

**Shark Recovery** 🦈 - Built with ❤️ for Samsung Galaxy F22

![Shark Logo](prebuilt/logos/shark_logo.png)
