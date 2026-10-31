# Troubleshooting

## Build problems

### `TARGET_COMPILE is not set`

Pass a cross-compiler prefix with a trailing dash:

```sh
make TARGET_COMPILE=aarch64-linux-gnu- KP_DIR=/path/to/KernelPatch
```

or let the script detect one:

```sh
./build.sh
```

### `aarch64-linux-gnu-gcc: command not found`

- Debian/Ubuntu: `sudo apt install gcc-aarch64-linux-gnu`
- Fedora: `sudo dnf install gcc-aarch64-linux-gnu`
- macOS: `brew install messense/macos-cross-toolchains/aarch64-unknown-linux-gnu`
- Or use any working `aarch64-elf-gcc`, `aarch64-none-elf-gcc` or
  `aarch64-unknown-linux-gnu-gcc`:

  ```sh
  make TARGET_COMPILE=aarch64-elf- KP_DIR=...
  ```

### KernelPatch checkout fails

`build.sh` clones with `--depth 1` and then fetches the pinned commit. On a slow
or restricted network, clone manually:

```sh
git clone https://github.com/bmax121/KernelPatch.git .build/KernelPatch
git -C .build/KernelPatch checkout d4c88d6672687456b299e4b5153c24dc979d789b
make TARGET_COMPILE=aarch64-linux-gnu- KP_DIR="$PWD/.build/KernelPatch"
```

If you intentionally build against a newer KernelPatch, set
`KP_COMMIT=<sha> ./build.sh`. A KPM built against a newer ABI may fail to load on
an older KernelPatch; keep the pin unless you know the device's KP version.

### `readelf` missing in `make check`

Install `binutils-aarch64-linux-gnu`, or skip `make check`. The `.kpm` artifact
is still produced.

---

## Loading problems

### `module load` returns an error

Check, in order:

1. **Is KPM loading supported?** Stock KernelSU does not load KPMs. Use APatch,
   SukiSU-Ultra, or KernelSU-Next + KPatch-Next.
2. **Does the kernel have `CONFIG_KALLSYMS=y`?** KernelPatch requires it.
3. **Is the KPM ABI compatible?** Build against the KernelPatch commit your
   device runs, or close to it.
4. **Is the file intact?** `file samsung-gsi-fix.kpm` must say
   `ELF 64-bit LSB relocatable, ARM aarch64`.
5. **Is the module already loaded?** `module list` / `kpatch kpm list`.

### `module ctl0` says `-ENOENT` or "module not found"

The name passed to `ctl0` must match `KPM_NAME`, not the filename. This module's
name is:

```
samsung-gsi-fix
```

Confirm with:

```sh
truncate <superkey> module list
# or
kpatch kpm list
```

### `status` shows `missing`

The symbol was not found in kallsyms. Common causes:

- **MediaTek Samsung kernels** often have no DEFEX/FIVE. `defex` and `five`
  reporting `missing` is expected; the SELinux hooks still work.
- **`CONFIG_KALLSYMS_ALL=n`** hides non-exported symbols. Samsung security
  functions are usually non-exported. Rebuild the kernel with
  `CONFIG_KALLSYMS_ALL=y` if you control it; otherwise accept the missing hooks.
- **CFI kernels** expose `<name>.cfi_jt`. The module tries that automatically.
  If the function is fully inlined and has no symbol at all, it cannot be hooked.
- **Vendor symbol renaming.** Some kernels rename or LTO-clone security
  functions. Check `dmesg` and the vendor source.

`missing` is informational. The module does not fail to load because of it.

### `status` shows `fail`

The symbol exists, but `hook_wrap()` returned an error. Typical reasons:

- The function is too small to patch safely (fewer than the required
  instructions for a branch/trampoline).
- The address is not in executable kernel text (a data alias, CFI jump table
  with unusual layout, or an invalid kallsyms entry).
- Another hook owns the function in a way that cannot be chained.

Collect:

```sh
adb shell dmesg | grep -E 'gsi-fix|hook'
adb shell "truncate <superkey> module ctl0 samsung-gsi-fix 'status'"
```

Include the kernel version, device model, and full log in a report. Do not try to
force a hook on a function that refuses; a bad inline patch can crash the device.

### The module loads but the device becomes unstable

Unload it first:

```sh
truncate <superkey> module unload samsung-gsi-fix
# or
kpatch kpm unload samsung-gsi-fix
```

If the instability stops, the module is involved. Test with individual features
disabled to isolate:

```sh
truncate <superkey> module ctl0 samsung-gsi-fix "defex off"
truncate <superkey> module ctl0 samsung-gsi-fix "five off"
truncate <superkey> module ctl0 samsung-gsi-fix "selinux targeted off"
```

Remember that toggles are in-memory; reloading or rebooting restores defaults.

### Bootloop

The module does not modify the boot image. Remove its file from `/data` in a
recovery that can mount `/data`:

```sh
# APatch
rm -rf /data/adb/ap/kpm/<module-id>

# KPatch-Next
rm -f /data/adb/kp-next/kpm/samsung-gsi-fix.kpm
```

If `/data` is not accessible, restore the boot image your root solution patched.
See [INSTALL.md](INSTALL.md#7-uninstall-and-recovery).

---

## SELinux problems

### Denials continue for UID 1000 or 1001

First confirm the hooks are `ok`, not `missing`/`fail`:

```sh
truncate <superkey> module ctl0 samsung-gsi-fix "status"
```

Then check the denial's context. The hook only skips checks performed **while
the current task runs as one of the listed UIDs**. A denial in a kernel worker
(`kworker`, UID 0) or in a different process is not covered.

```sh
adb shell dmesg | grep -E 'avc: denied' | tail -30
```

Look at `scontext`/`tcontext` and the `comm` field. If the denying process is not
system/radio, add its UID:

```sh
adb shell "dumpsys package <package> | grep userId="
truncate <superkey> module ctl0 samsung-gsi-fix "uid add <uid>"
```

Some checks do not go through `avc_has_perm*` (for example, checks performed in
vendor kernel code or through a different LSM). Those cannot be intercepted by
this module. For diagnosis only, enable global permissive and see whether the
denial disappears:

```sh
truncate <superkey> module ctl0 samsung-gsi-fix "selinux permissive on"
# test
truncate <superkey> module ctl0 samsung-gsi-fix "selinux permissive off"
```

If global permissive fixes it but targeted does not, the check is running in a
context outside the allowlist, or it bypasses the hooked functions.

### Adding a UID for an IMS app

```sh
adb shell "dumpsys package me.phh.ims | grep userId="
# userId=1000 or userId=10xxx
truncate <superkey> module ctl0 samsung-gsi-fix "uid add 10xxx"
truncate <superkey> module ctl0 samsung-gsi-fix "uid list"
```

The allowlist is capped at 16 UIDs. `uid reset` restores `1000 1001`.

### Is targeted SELinux safe?

No. It skips the SELinux AVC checks that flow through `avc_has_perm`,
`avc_has_perm_flags` and `avc_has_extended_perms` for those processes —
including `system_server`. It is narrower than global permissive and does not
cover checks that bypass those wrappers (vendor code, other LSMs), but it is
still a security reduction. Prefer turning it off if your device works without
it:

```sh
truncate <superkey> module ctl0 samsung-gsi-fix "selinux targeted off"
```

---

## Conflicts with other modules

### KernelPatch's own SELinux bypass

KernelPatch hooks `avc_denied` for its su handling. This module deliberately
hooks `avc_has_perm`, `avc_has_perm_flags` and `avc_has_extended_perms` instead,
so the two do not fight over the same function. If a future KernelPatch version
changes that, check the KernelPatch release notes before loading both.

### Another KPM hooks the same functions

KernelPatch supports hook chains. Two modules can wrap the same function, but
the order of `before` callbacks and the final `ret` value depend on chain order.
If both modules try to force a return value, the last one wins. Do not stack
multiple "SELinux fix" or "DEFEX bypass" modules; disable one.

### Magisk / KernelSU modules that also modify SELinux

A module that runs `setenforce 0` at boot or injects `magiskpolicy` rules is
independent of this KPM. Global permissive from such a module makes this
module's targeted mode irrelevant. Check:

```sh
adb shell getenforce
```

`Permissive` means something already disabled enforcement globally.

---

## IMS still not working

This is expected on many GSIs. The KPM does not implement IMS. Work through
[IMS.md](IMS.md):

1. Is an `ImsService` installed?
2. Is there an `ims` APN?
3. Does the radio log show a SIP `REGISTER` attempt?
4. Does the carrier provision VoLTE for this SIM/IMEI?
5. Does the stock ROM work with the same SIM?

If the answer to (1) is no, no kernel change will help. If (3) shows a
registration attempt but the carrier rejects it, the problem is the SIP profile
or provisioning, not the kernel.

---

## Collecting a useful bug report

```sh
{
  echo "=== device ==="
  getprop ro.product.device
  getprop ro.product.model
  getprop ro.build.version.release
  uname -a
  echo "=== root solution ==="
  getprop | grep -iE 'apatch|kernelsu|sukisu|kpatch'
  echo "=== module status ==="
  # replace with your control command
  truncate <superkey> module ctl0 samsung-gsi-fix "status"
  echo "=== kernel log ==="
  dmesg | grep -E 'gsi-fix|avc|defex|five|proca' | tail -200
  echo "=== radio log ==="
  logcat -b radio -v time -d | grep -iE 'ims|volte|sip|register' | tail -200
} > /sdcard/gsi-fix-report.txt
```

Redact IMEI, IMSI, phone number and carrier account details before posting.

---

## FAQ

**Does this work on MediaTek Samsung devices?**
The module loads and applies SELinux hooks. DEFEX/FIVE symbols are often absent,
so those hooks report `missing`. It will not add an IMS stack either way.

**Does this work on Snapdragon Samsung devices?**
Usually yes for SELinux. DEFEX presence varies by model and firmware; FIVE may or
may not be present. Check `status`.

**Does it survive a ROM update?**
The KPM file lives in `/data`, but the root solution patches the boot image. An
OTA usually replaces the boot image and removes root; re-root and reinstall.

**Does unloading restore the kernel?**
Yes. `KPM_EXIT` removes this module's callbacks from the hook chains. Other
modules' callbacks on the same functions are left in place.

**Can it brick the device?**
It does not write to partitions. A bad hook can crash the kernel, but removing
the KPM file or restoring the boot image recovers the device. Keep a backup.

**Why is DEFEX bypass global but SELinux targeted?**
DEFEX's enforcement functions do not expose a UID argument that can be filtered
reliably. SELinux does, through `current_uid()` at the permission check. The
module therefore offers per-UID SELinux control but not per-UID DEFEX control.

**Can it hide root from banking apps?**
No. It is not a hiding module, and global permissive mode will make Play
Integrity and banking apps more likely to complain, not less.
