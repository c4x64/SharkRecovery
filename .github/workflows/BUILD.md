# GitHub Actions Build Workflows

## Available Workflows

### 1. Build TWRP (Full Build)
**File**: `.github/workflows/build_twrp.yml`

Full TWRP build using repo sync and mka.

**Trigger**: Manual workflow dispatch

**Inputs**:
- `manifest_url`: TWRP manifest URL
- `manifest_branch`: TWRP branch (twrp-12, twrp-13, etc.)
- `device_tree_url`: Your device tree repo URL
- `device_tree_branch`: Branch to use
- `device_codename`: Device codename (f22)
- `lunch_target`: Lunch target (twrp_f22-eng)

**Process**:
1. Maximize disk space
2. Setup swap (4GB)
3. Install dependencies
4. Cache ccache
5. repo init
6. repo sync (100GB+ download)
7. Fetch device tree
8. lunch target
9. mka recoveryimage
10. Upload artifacts

**Time**: 30-60 minutes
**Disk**: 150-200 GB

**Use when**: You want full TWRP build

---

### 2. Build Prebuilt (Fast)
**File**: `.github/workflows/build_prebuilt.yml`

Fast build using prebuilt TWRP + patching.

**Trigger**: Push/PR/Auto

**Process**:
1. Install minimal dependencies
2. Download prebuilt TWRP
3. Extract recovery
4. Apply Shark branding
5. Integrate KernelSU
6. Repack
7. Generate visualization

**Time**: 5-10 minutes
**Disk**: < 5 GB

**Use when**: You want fast builds with actual results

---

### 3. Emulate
**File**: `.github/workflows/emulate.yml`

Post-build emulation and visualization.

**Trigger**: After build completes

**Process**:
1. Download artifacts
2. Run emulation scripts
3. Generate screenshots
4. Create reports

---

## How to Use Build TWRP Workflow

### Step 1: Go to Actions Tab
https://github.com/c4x64/SharkRecovery/actions

### Step 2: Select Workflow
Choose "Build TWRP with Device Tree"

### Step 3: Run Workflow
Click "Run workflow" dropdown

### Step 4: Fill Parameters

```
Manifest URL: https://github.com/minimal-manifest-twrp/platform_manifest_twrp_aosp.git
Manifest Branch: twrp-12
Device Tree URL: https://github.com/c4x64/SharkRecovery.git
Device Tree Branch: main
Device Codename: f22
Lunch Target: twrp_f22-eng
```

### Step 5: Monitor Build
- Click on running workflow
- Watch logs
- Wait 30-60 minutes

### Step 6: Download
- Go to workflow run
- Download artifacts
- Find recovery.img

---

## Workflow Outputs

### Artifacts
- `recovery.img` - Compiled recovery image
- `build.log` - Full build log
- `build_info.txt` - Build metadata
- `recovery_preview.txt` - Visual preview
- `build_preview.png` - Screenshot

### Releases
After successful build:
- Tagged release created
- ZIP package with recovery
- README with instructions

---

## Prerequisites

### For Full Build
- Device tree properly configured
- Prebuilt kernel in device tree
- BoardConfig.mk correct
- Lunch target exists

### For Prebuilt Build
- Prebuilt TWRP available
- Shark logo present
- KernelSU source ready

---

## Troubleshooting

### Build Fails: No space left
**Solution**: Workflow maximizes disk space automatically

### Build Fails: Repo sync timeout
**Solution**: Uses optimized fetch and prune

### Build Fails: Device not found
**Solution**: Check device codename and lunch target

### Build Fails: Out of memory
**Solution**: Uses swap file (4GB)

---

## Optimization

The workflow includes:
- ✅ Disk space maximization
- ✅ Ccache for faster rebuilds
- ✅ Swap file creation
- ✅ Optimized repo sync
- ✅ Cleanup after install
- ✅ Artifact uploads

---

## Current Configuration

For Shark Recovery SM-E225F:

```yaml
manifest_url: https://github.com/minimal-manifest-twrp/platform_manifest_twrp_aosp.git
manifest_branch: twrp-12
device_tree_url: https://github.com/c4x64/SharkRecovery.git
device_tree_branch: main
device_codename: f22
lunch_target: twrp_f22-eng
```

---

## Notes

- Full build takes 30-60 minutes
- First build is slowest (no cache)
- Subsequent builds faster with ccache
- Artifacts kept for 30 days
- Releases auto-created on manual trigger

The workflow now actually builds TWRP from source with your device tree!
