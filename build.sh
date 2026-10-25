#!/bin/bash
# Shark Recovery Build Script
# Builds TWRP + KernelSU for SM-E225F

set -e

# Configuration
WORKSPACE="${HOME}/shark-build"
DEVICE_PATH="device/samsung/f22"
REPO_URL="https://github.com/minimal-manifest-twrp/platform_manifest_twrp_aosp.git"
REPO_BRANCH="twrp-12"

echo "=== Shark Recovery Build Script ==="
echo "Device: SM-E225F (f22)"
echo "Build workspace: $WORKSPACE"
echo ""

# Check dependencies
check_deps() {
    echo "Checking dependencies..."

    deps=("git" "curl" "wget" "repo")

    for dep in "${deps[@]}"; do
        if ! command -v $dep &> /dev/null; then
            echo "Installing $dep..."
            if [ "$dep" = "repo" ]; then
                mkdir -p ~/bin
                curl https://storage.googleapis.com/git-repo-downloads/repo > ~/bin/repo
                chmod a+x ~/bin/repo
                export PATH="$HOME/bin:$PATH"
            else
                sudo apt-get update && sudo apt-get install -y $dep
            fi
        fi
    done

    echo "Dependencies OK"
}

# Setup workspace
setup_workspace() {
    echo "Setting up workspace..."

    if [ -d "$WORKSPACE" ]; then
        echo "Workspace exists, cleaning..."
        rm -rf "$WORKSPACE"
    fi

    mkdir -p "$WORKSPACE"
    cd "$WORKSPACE"

    echo "Initializing repo..."
    repo init -u "$REPO_URL" -b "$REPO_BRANCH" --depth=1

    echo "Syncing source..."
    repo sync -c -j$(nproc) --force-sync --no-clone-bundle --no-tags

    echo "Workspace ready"
}

# Copy device tree
copy_device_tree() {
    echo "Copying device tree..."

    LOCAL_REPO="/Users/prabhas/SM-E225F"

    # Remove existing device tree
    rm -rf "$WORKSPACE/device/samsung/f22"
    mkdir -p "$WORKSPACE/device/samsung"

    # Copy device tree
    cp -r "$LOCAL_REPO/device" "$WORKSPACE/device/samsung/f22"

    # Copy prebuilt assets
    if [ -d "$LOCAL_REPO/prebuilt" ]; then
        mkdir -p "$WORKSPACE/device/samsung/f22/prebuilt"
        cp -r "$LOCAL_REPO/prebuilt"/* "$WORKSPACE/device/samsung/f22/prebuilt/" 2>/dev/null || true
    fi

    # Copy KernelSU
    if [ -d "$LOCAL_REPO/KernelSU" ]; then
        mkdir -p "$WORKSPACE/device/samsung/f22/KernelSU"
        cp -r "$LOCAL_REPO/KernelSU"/* "$WORKSPACE/device/samsung/f22/KernelSU/" 2>/dev/null || true
    fi

    # Create logo directory
    mkdir -p "$WORKSPACE/device/samsung/f22/prebuilt/logos"
    if [ -f "$LOCAL_REPO/prebuilt/logos/shark_logo.png" ]; then
        cp "$LOCAL_REPO/prebuilt/logos/shark_logo.png" "$WORKSPACE/device/samsung/f22/prebuilt/logos/"
    fi

    echo "Device tree copied"
}

# Configure build
configure_build() {
    echo "Configuring build..."

    cd "$WORKSPACE"

    # Setup environment
    source build/envsetup.sh

    # Lunch device
    lunch twrp_f22-eng

    echo "Build configured"
}

# Build recovery
build_recovery() {
    echo "Building Shark Recovery..."
    echo "This may take 1-3 hours..."
    echo ""

    cd "$WORKSPACE"

    # Clean build
    make clean

    # Build with progress
    make -j$(nproc) recoveryimage 2>&1 | tee /tmp/build.log

    echo ""
    echo "=== Build Complete ==="
    echo ""

    if [ -f "out/target/product/f22/recovery.img" ]; then
        echo "SUCCESS: recovery.img built"
        echo "Location: $WORKSPACE/out/target/product/f22/recovery.img"
        echo ""
        ls -lh "$WORKSPACE/out/target/product/f22/recovery.img"
        return 0
    else
        echo "ERROR: Build failed"
        echo "Check /tmp/build.log"
        return 1
    fi
}

# Main
main() {
    check_deps
    setup_workspace
    copy_device_tree
    configure_build
    build_recovery
}

main "$@"
