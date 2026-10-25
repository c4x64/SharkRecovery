BOARD_CUSTOM_BOOTIMG_MK := device/samsung/f22/bootimg.mk

$(call inherit-product, $(SRC_TARGET_DIR)/product/base.mk)

# Shark Recovery specific bootimg settings
BOARD_MKBOOTIMG_ARGS += \
    --board "SRPUB24A003" \
    --dtb $(TARGET_PREBUILT_DTB) \
    --dtb_offset $(BOARD_DTB_OFFSET) \
    --recovery_dtbo $(BOARD_PREBUILT_DTBOIMAGE)
