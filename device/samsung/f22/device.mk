# Device specific packages
PRODUCT_PACKAGES += \
    shark-logo \
    toolbox

# Copy logo
PRODUCT_COPY_FILES += \
    $(DEVICE_PATH)/prebuilt/logos/shark_logo.png:$(TARGET_COPY_OUT_RECOVERY)/res/images/shark.png \
    $(DEVICE_PATH)/recovery/root/system/etc/twrp.fstab:$(TARGET_COPY_OUT_RAMDISK)/recovery/root/system/etc/recovery.fstab

# Shark Recovery branding
PRODUCT_PROPERTY_OVERRIDES += \
    ro.shark.recovery.version=1.0 \
    ro.shark.device=f22 \
    ro.shark.recovery.branding=Shark Recovery

# KernelSU support
ifeq ($(ENABLE_KSU),true)
    PRODUCT_COPY_FILES += \
        device/samsung/f22/ksu.cfg:$(TARGET_COPY_OUT_RECOVERY)/root/system/etc/ksu.cfg
endif
