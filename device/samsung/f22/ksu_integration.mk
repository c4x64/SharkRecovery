# KernelSU Integration for Shark Recovery
# LKMSU Build System

LOCAL_PATH := $(call my-dir)

# KernelSU Configuration
KSU_PATH := $(LOCAL_PATH)/KernelSU
KSU_SRC_PATH := $(KSU_PATH)/src
KSU_KERNEL_PATH := $(KSU_PATH)/kernel

# Build flags
ifeq ($(TW_INCLUDE_KSU),true)
    # Include KernelSU in recovery
    PRODUCT_PACKAGES += \
        kernelsu

    # KernelSU module
    PRODUCT_COPY_FILES += \
        $(KSU_PATH)/ksu.zygote:$(TARGET_COPY_OUT_RECOVERY)/root/system/etc/ksu/zygote \
        $(KSU_PATH)/ksu.cfg:$(TARGET_COPY_OUT_RECOVERY)/root/system/etc/ksu.cfg

    # KernelSU kernel module
    PRODUCT_COPY_FILES += \
        $(KSU_SRC_PATH)/kernelsu.ko:$(TARGET_COPY_OUT_RECOVERY)/root/system/lib/modules/kernelsu.ko

    # KernelSU config
    PRODUCT_PROPERTY_OVERRIDES += \
        ro.ksu.enabled=1 \
        ro.ksu.version=1.0 \
        ro.ksu.lkm=1

    # Enable KSU in recovery
    TW_INCLUDE_KERNELOVERLAY := true
    TW_EXCLUDE_KERNELSU := false

    # KernelSU hooks
    $(call inherit-product, $(LOCAL_PATH)/KernelSU/ksu.mk)
endif

# KernelSU Build Rules
ifeq ($(BUILD_KERNELSU),true)
    KSU_MODULE := kernelsu
    KSU_MODULE_SRC := $(KSU_SRC_PATH)/KernelSU
    KSU_MODULE_OUT := $(OUT_DIR)/target/product/f22/kernel/

    # Build KernelSU as LKM
    $(KSU_MODULE_OUT)/kernelsu.ko: $(KSU_MODULE_SRC)/Makefile
        $(MAKE) -C $(KSU_KERNEL_PATH) \
            M=$(KSU_MODULE_SRC) \
            ARCH=arm64 \
            CROSS_COMPILE=$(TARGET_CROSS) \
            modules
endif
