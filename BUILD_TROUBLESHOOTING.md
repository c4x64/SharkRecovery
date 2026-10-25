# Build Troubleshooting Guide

## Current Build Status

**Device Tree**: ✅ Correct structure
**Prebuilts**: ✅ Image.gz, dtb, dtbo.img present
**Product Name**: ✅ twrp_f22
**Lunch Target**: ✅ twrp_f22-eng

## Potential Issues

### Issue 1: GitHub Actions Disk Space

**Problem**: GitHub Actions runner has only 14GB, TWRP needs 150GB+

**Symptoms**:
```
No space left on device
repo sync fails
make fails with disk full
```

**Solutions**:
1. Use `jlumbroso/free-disk-space@v3` action (already in workflow)
2. Add swap file (already in workflow)
3. Use ccache to reduce rebuilds (already in workflow)
4. Build with `-j2` not `-j$(nproc)` (fixed)

**Current workflow**: Uses all optimizations

### Issue 2: Java Version

**Problem**: TWRP needs Java 8, not Java 17

**Symptoms**:
```
javac version 17, expected 1.8
error: source release 8 not supported
```

**Solution**: Use Ubuntu 20.04 with Java 8 (fixed in build_twrp_fixed.yml)

### Issue 3: Device Tree Copy

**Problem**: Device tree copied to wrong location

**Current workflow**:
```bash
cp -r /tmp/device_tree_temp/device/* device/samsung/${DEVICE_CODENAME}/
```

**Problem**: If device tree is hosted at `https://github.com/c4x64/SharkRecovery`, the structure is:
```
SharkRecovery/
├── device/samsung/f22/
├── KernelSU/
└── ...
```

When we clone it, we get:
```
/tmp/device_tree_temp/device/samsung/f22/
```

But we're copying to:
```
device/samsung/${DEVICE_CODENAME}/
```

**Need to copy from**: `/tmp/device_tree_temp/device/samsung/f22/*`
**Not**: `/tmp/device_tree_temp/device/*`

### Issue 4: Repo Sync Time

**Problem**: repo sync takes too long, times out

**Symptoms**:
```
timeout after 6 hours
repo sync incomplete
```

**Solutions**:
- Use `--depth=1` (already used)
- Use `--no-clone-bundle` (already used)
- Use `--optimized-fetch` (already used)
- Use `-j4` not `-j$(nproc)` (fixed)

### Issue 5: Kernel Module Compilation

**Problem**: KernelSU needs kernel source

**Current**: KernelSU source is in repo, but kernel source is missing

**Solution**: 
- KernelSU LKM needs actual kernel source for mt6769t
- Currently Building TWRP without kernel compile
- KernelSU integration may fail if no kernel source

## Fix the Device Tree Copy

The workflow is copying device tree incorrectly. Let me fix it.

Current:
```bash
cp -r /tmp/device_tree_temp/device/* device/samsung/f22/
```

Should be:
```bash
cp -r /tmp/device_tree_temp/device/samsung/f22/* device/samsung/f22/
```

Or better, clone directly to correct location.

## Recommended Fix

Let's update the workflow to:
1. Clone device tree directly to `device/samsung/f22`
2. Don't try to copy structure
3. Verify files exist before lunch
4. Add better error messages

Want me to update the workflow with the correct device tree copy logic?
