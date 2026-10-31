# SPDX-License-Identifier: GPL-2.0-or-later
#
# Build a KernelPatch Module (.kpm) for APatch / KPM-capable KernelSU.
#
#   make TARGET_COMPILE=aarch64-linux-gnu- KP_DIR=/path/to/KernelPatch
#
# The output is samsung-gsi-fix.kpm, a relocatable aarch64 ELF.

ifeq ($(filter clean,$(MAKECMDGOALS)),)
ifndef TARGET_COMPILE
$(error TARGET_COMPILE is not set, e.g. TARGET_COMPILE=aarch64-linux-gnu-)
endif
endif

KP_DIR ?= $(CURDIR)/.build/KernelPatch
KPM := samsung-gsi-fix.kpm
LDS := gsi_fix.lds

CC := $(TARGET_COMPILE)gcc
READELF ?= $(TARGET_COMPILE)readelf

CFLAGS += -nostdinc
CFLAGS += -fno-pic -fno-PIC -fno-pie -fno-stack-protector
CFLAGS += -fno-asynchronous-unwind-tables -fno-unwind-tables -fno-exceptions
CFLAGS += -fno-common
CFLAGS += -O2
CFLAGS += -Wall -Wextra -Wno-unused-parameter -Wno-sign-compare
CFLAGS += -I$(KP_DIR)/kernel/include
CFLAGS += -I$(KP_DIR)/kernel/patch/include
CFLAGS += -I$(KP_DIR)/kernel/linux/include
CFLAGS += -I$(KP_DIR)/kernel/linux/arch/arm64/include
CFLAGS += -I$(KP_DIR)/kernel/linux/tools/arch/arm64/include

OBJS := src/main.o src/hooks.o src/ctl.o src/config.o src/util.o

all: $(KPM)

$(KPM): $(OBJS)
	$(CC) -r -o $@ $(OBJS)

src/%.o: src/%.c
	$(CC) $(CFLAGS) -T$(LDS) -c -o $@ $<

$(OBJS): src/config.h src/hooks.h src/util.h

check: $(KPM)
	@$(READELF) -S $(KPM) | grep -q '\.kpm\.info' && echo "KPM metadata section: ok"
	@$(READELF) -S $(KPM) | grep -q '\.kpm\.init' && echo "KPM init section: ok"
	@$(READELF) -h $(KPM) | grep -q 'REL (Relocatable file)' && echo "ELF type: relocatable ok"

clean:
	rm -f $(KPM) $(OBJS)

.PHONY: all check clean
