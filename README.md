# 🦈 Shark Recovery for Samsung Galaxy F22 (SM-E225F)

Custom recovery based on TWRP with KernelSU integration. Built with GitHub Actions CI.

## Quick Start

```bash
# Local build (5 minutes)
./build_prebuilt.sh
```

## GitHub Actions

- **Fast build**: `.github/workflows/build_prebuilt.yml` (5 min)
- **Verification**: `.github/workflows/NO_LARGE_DOWNLOAD.yml` (1 min)
- **Full build**: `.github/workflows/full_build_ksu_twrp.yml` (30-60 min)

## Device Support
- **Model**: SM-E225F / SM-E225F/DS
- **SoC**: MediaTek Helio G80 (mt6769t)
- **Kernel**: 4.19
- **Android**: 11/12

## Features
- 🦈 Custom shark logo and branding
- 🔓 KernelSU LKM integration
- 📱 USB OTG support
- 💾 NTFS support
- 🔐 FBE/Encryption support

## Build Artifacts
- `recovery.img` - Shark Recovery image
- `build_report.txt` - Build information
- `preview.png` - Screenshot

## Repository
```
SharkRecovery/
├── device/samsung/f22/       # Device tree
├── KernelSU/                 # KernelSU source
├── prebuilt/logos/           # Shark logo
├── .github/workflows/        # CI pipelines
│   ├── build_prebuilt.yml    # Fast build
│   ├── NO_LARGE_DOWNLOAD.yml # Verification
│   └── full_build_ksu_twrp.yml # Full build
└── build_prebuilt.sh         # Fast builder
```

## License
Apache 2.0 for device tree, GPL-2.0 for KernelSU
