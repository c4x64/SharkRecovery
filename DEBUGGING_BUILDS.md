# Why Builds Fail and How to Fix Them

## Common Build Failures

### 1. "No such product"

**Error**: `No such product: twrp_f22-eng`

**Causes**:
- Device tree not copied correctly
- Lunch target name is wrong
- Device directory structure incorrect

**Fix**:
```bash
# Check device tree location
ls device/samsung/f22/
ls device/samsung/f22/AndroidProducts.mk

# Verify lunch target
grep -r "PRODUCT_NAME" device/samsung/f22/
```

Expected: `PRODUCT_NAME := twrp_f22`

---

### 2. "Out of disk space"

**Error**: `No space left on device`

**Causes**:
- GitHub Actions runner only has 14GB base
- TWRP needs 150GB+

**Fix**:
- Use `jlumbroso/free-disk-space@v3` action
- Setup swap file
- Use ccache
- Build with `-j2` not `-j$(nproc)`

---

### 3. "Java version mismatch"

**Error**: `javac version 17, expected 1.8`

**Fix**:
```bash
# Use Java 8 for TWRP
sudo apt-get install openjdk-8-jdk
export JAVA_HOME=/usr/lib/jvm/java-8-openjdk-amd64
```

---

### 4. "Device not found"

**Error**: Device tree not detected

**Causes**:
- Wrong directory structure
- Missing AndroidProducts.mk
- Wrong codename

**Fix**:
Device tree should be at:
```
device/samsung/f22/
├── AndroidProducts.mk
├── BoardConfig.mk
├── device.mk
├── twrp_f22.mk
└── Android.mk
```

---

### 5. "Kernel not found"

**Error**: `TARGET_PREBUILT_KERNEL not found`

**Fix**:
Prebuilt kernel must exist at:
```
device/samsung/f22/prebuilt/Image.gz
```

If missing, build will fail.

---

### 6. "ccache errors"

**Error**: `ccache: error`

**Fix**:
```bash
export USE_CCACHE=1
export CCACHE_DIR=$HOME/.ccache
ccache -M 50G
```

---

## Current Build Status

### What's Working:
✅ Repo structure
✅ Device tree
✅ GitHub Actions configured
✅ Disk space optimization
✅ Swap setup
✅ Ccache setup

### What's Failing:
❌ Possibly missing prebuilt kernel
❌ Device tree path incorrect
❌ Lunch target mismatch
❌ Disk space still insufficient

## Debug Steps

### 1. Check device tree
```bash
ls -la device/samsung/f22/
```

Should see:
- BoardConfig.mk
- device.mk
- twrp_f22.mk
- AndroidProducts.mk

### 2. Check lunch target
```bash
grep "PRODUCT_NAME" device/samsung/f22/twrp_f22.mk
```

Should output: `PRODUCT_NAME := twrp_f22`

### 3. Check prebuilts
```bash
ls -lh device/samsung/f22/prebuilt/
```

Should have: Image.gz, dtb, dtbo.img

### 4. Check BoardConfig
```bash
grep "TW_DEVICE_VERSION" device/samsung/f22/BoardConfig.mk
```

---

## Quick Test

Run this locally first:
```bash
. build/envsetup.sh
lunch twrp_f22-eng
make recoveryimage -j2
```

If it works locally, it should work on CI.

---

## The Real Issue

GitHub Actions Ubuntu runners have:
- 14GB disk (after free space action: ~50GB)
- 7GB RAM
- 6 hour timeout

TWRP build needs:
- 150GB disk
- 8GB+ RAM
- 3-7 hours

**We need self-hosted runner** or prebuilt approach.

---

## Alternative: Use Prebuilt

If full build keeps failing, use prebuilt approach:
1. Download existing TWRP
2. Patch with device tree
3. Add Shark branding
4. Add KernelSU
5. Repack

This works on GitHub Actions.

Want me to switch to prebuilt approach?
