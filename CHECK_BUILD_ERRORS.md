# Build Error Checklist

## Check Device Tree Structure

Your device tree must be at the correct location and have correct files.

### Expected Structure
```
device/samsung/f22/
├── AndroidProducts.mk
├── BoardConfig.mk
├── bootimg.mk
├── device.mk
├── twrp_f22.mk
├── Android.mk
├── prebuilt/
│   ├── Image.gz
│   ├── dtb
│   └── dtbo.img
└── recovery/
    └── root/
```

### Verify Files Exist
