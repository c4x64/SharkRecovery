# Installation

This guide assumes an arm64 Samsung device that already boots an AOSP GSI and has
a working KPM-capable root solution.

If you have not built the module yet, see the [README](../README.md#build).

---

## 1. Pre-flight checks

Run these on the device:

```sh
uname -m
# aarch64

uname -r
# e.g. 5.10.198-android13-8-... or 5.15.104-android14-...

zcat /proc/config.gz 2>/dev/null | grep -E 'CONFIG_KALLSYMS|CONFIG_SECURITY_SELINUX'
# or: adb shell "grep -E 'KALLSYMS|SELINUX' /boot/config-$(uname -r)" (rarely present)
```

You need:

- `CONFIG_KALLSYMS=y`. Without it the module cannot resolve any kernel symbol.
- `CONFIG_SECURITY_SELINUX=y` for the SELinux hooks. If SELinux is disabled, the
  module still loads but those three hooks report `missing`.
- KernelPatch support for your kernel. If your root solution boots and exposes a
  KPM screen or a `kpatch` binary, you are probably fine.

Back up your current boot image **before** changing anything:

```sh
adb shell "su -c 'dd if=/dev/block/by-name/boot of=/sdcard/boot-backup.img'"
adb pull /sdcard/boot-backup.img
```

The exact partition name differs on some devices (`boot_a`, `init_boot`, etc.).
Use the same image your root solution patched.

---

## 2. Get the KPM

Either build it:

```sh
./build.sh
```

or download `samsung-gsi-fix.kpm` from a CI artifact / release.

Sanity-check the file on a computer:

```sh
file samsung-gsi-fix.kpm
# ELF 64-bit LSB relocatable, ARM aarch64

aarch64-linux-gnu-readelf -S samsung-gsi-fix.kpm | grep kpm
# .kpm.info, .kpm.init, .kpm.ctl0, .kpm.exit must be present
```

Push it to the device:

```sh
adb push samsung-gsi-fix.kpm /data/local/tmp/
```

---

## 3. Install with your root solution

### APatch

APatch loads KPMs natively.

**Boot-time install (persistent):**

1. Open APatch Manager.
2. Go to **KPM**.
3. Tap install and select `samsung-gsi-fix.kpm`.
4. Reboot. APatch's boot loader loads installed KPMs from
   `/data/adb/ap/kpm/`.

**One-shot load (current boot only):**

Use the **Load** action in the same screen and select the file. This calls the
KernelPatch loader immediately and is lost on reboot.

**Control:** tap the installed module and use the control dialog. Enter
`status`, `selinux targeted off`, etc.

**Disable without uninstalling:** use the module's enable/disable toggle in the
APatch UI. APatch represents a disabled KPM with a `disable` marker in the
module's directory.

### SukiSU-Ultra

SukiSU-Ultra includes KPM support in the kernel.

1. Open the SukiSU manager.
2. Find the **KPM** section.
3. Install or load `samsung-gsi-fix.kpm` the same way as any other KPM.

If your build also ships the KernelPatch superkey interface, the `truncate`
commands in the next section work and give you `ctl0` access even when the
manager UI does not expose a control dialog.

### KernelSU-Next + KPatch-Next

Stock KernelSU cannot load KPMs. KernelSU-Next gains KPM support through the
[KPatch-Next module](https://github.com/KernelSU-Next/KPatch-Next-Module).

1. Install the KPatch-Next module in KernelSU-Next and reboot.
2. Verify that `kpatch` exists:

   ```sh
   adb shell "su -c 'ls /data/adb/modules/KPatch-Next/bin'"
   ```

3. Load the KPM:

   ```sh
   adb shell "su -c '/data/adb/modules/KPatch-Next/bin/kpatch kpm load /data/local/tmp/samsung-gsi-fix.kpm'"
   ```

4. For a persistent install, use the KPatch-Next WebUI's KPM page. It stores
   KPMs under `/data/adb/kp-next/kpm/` and loads them at boot.

**Control:**

```sh
adb shell "su -c '/data/adb/modules/KPatch-Next/bin/kpatch kpm ctl0 samsung-gsi-fix \"status\"'"
```

### Standalone KernelPatch (superkey)

If your device runs KernelPatch directly (APatch exposes the same interface),
authenticate with the superkey and use the `module` commands:

```sh
adb shell "truncate <superkey> module load /data/local/tmp/samsung-gsi-fix.kpm"
adb shell "truncate <superkey> module list"
adb shell "truncate <superkey> module info samsung-gsi-fix"
adb shell "truncate <superkey> module ctl0 samsung-gsi-fix 'status'"
```

To load with init arguments:

```sh
adb shell "truncate <superkey> module load /data/local/tmp/samsung-gsi-fix.kpm 'selinux=off uid=1000 uid=1001'"
```

To unload:

```sh
adb shell "truncate <superkey> module unload samsung-gsi-fix"
```

---

## 4. Verify

Ask the module for its state:

```sh
# APatch control dialog, or one of the CLI forms above
truncate <superkey> module ctl0 samsung-gsi-fix "status"
```

Expected output on a Samsung Exynos kernel with DEFEX and FIVE:

```
samsung-gsi-fix 1.0.0
defex: on, hook=ok
five: on, bprm=ok mmap=ok verify=ok
selinux: targeted=on permissive=off, avc_has_perm=ok flags=ok xperms=ok
uids: 1000 1001
note: VoLTE/SMS still need a userspace IMS stack; see docs/IMS.md
```

On a MediaTek Samsung kernel without DEFEX/FIVE, expect:

```
defex: on, hook=missing
five: on, bprm=missing mmap=missing verify=missing
selinux: targeted=on permissive=off, avc_has_perm=ok flags=ok xperms=ok
```

`missing` is not an error: the symbol is absent from that kernel. `fail` means
the symbol was found but `hook_wrap()` refused it (function too small, already
claimed in an incompatible way, or not a valid text address).

Check the kernel log for installation details:

```sh
adb shell dmesg | grep gsi-fix
```

Typical lines:

```
[gsi-fix] init: event=load args=(none)
[gsi-fix] task_defex_enforce: hooked at ffffffc0...
[gsi-fix] five_bprm_check: hooked at ffffffc0...
[gsi-fix] avc_has_perm: hooked at ffffffc0...
[gsi-fix] loaded; use ctl0 'status' for hook state
```

If a hook reports `fail`, include the full `dmesg` output in a bug report.

---

## 5. Configure

Defaults are intentionally conservative for a fix module:

| Setting | Default | Why |
| --- | --- | --- |
| `defex` | on | Samsung DEFEX can kill ported executables. |
| `five` | on | Neutralizes FIVE measurement failures. |
| `selinux targeted` | on, UIDs `1000 1001` | Covers `system_server` and the phone process. |
| `selinux permissive` | off | Global permissive is a debugging tool, not a default. |

Common changes:

```sh
# If the device boots and telephony works without SELinux help:
truncate <superkey> module ctl0 samsung-gsi-fix "selinux targeted off"

# If a ported IMS app runs under its own UID:
adb shell "dumpsys package <package.name> | grep userId="
truncate <superkey> module ctl0 samsung-gsi-fix "uid add <uid>"

# Temporary debugging only:
truncate <superkey> module ctl0 samsung-gsi-fix "selinux permissive on"
# ... collect logs ...
truncate <superkey> module ctl0 samsung-gsi-fix "selinux permissive off"
```

Settings are in-memory. A reboot restores the compiled defaults or the init
arguments used at load time.

---

## 6. Persistence and upgrades

| Root solution | Persistent KPM location | How to update |
| --- | --- | --- |
| APatch | `/data/adb/ap/kpm/<module-id>/` | Install the new `.kpm` in the app, reboot. |
| KPatch-Next | `/data/adb/kp-next/kpm/` | Replace the file in the WebUI, reboot. |
| SukiSU-Ultra | Manager-managed | Use the manager's KPM screen. |
| Standalone KernelPatch | Not persisted by KernelPatch | Re-run `module load`, or add it to your own boot script. |

Because the module has no persistent state of its own, replacing the `.kpm` is
enough; there is no config file to migrate.

---

## 7. Uninstall and recovery

### Normal uninstall

```sh
# KernelPatch / APatch superkey
truncate <superkey> module unload samsung-gsi-fix

# KPatch-Next
kpatch kpm unload samsung-gsi-fix
```

Then remove the installed file in your root manager so it does not load on the
next boot.

### If the device bootloops

The module only hooks functions and does not modify the boot image, so recovery
is usually a file removal:

1. Boot into a recovery that can mount `/data` (TWRP with FBE support, or your
   device's equivalent).
2. Remove the KPM:

   ```sh
   # APatch
   rm -rf /data/adb/ap/kpm/<module-id>

   # KPatch-Next
   rm -f /data/adb/kp-next/kpm/samsung-gsi-fix.kpm
   ```

   The APatch module id is derived from the KPM name; it is normally
   `samsung-gsi-fix`, but list the directory if unsure.

3. Reboot.

If you cannot mount `/data`, restore the boot image you backed up in step 1.
That removes the root solution and therefore the KPM loader as well.

### If the device boots but telephony breaks

Unload the module first, then reboot and test. If the problem disappears, report
it with:

```sh
adb shell dmesg | grep -E 'gsi-fix|avc|defex|five'
adb shell logcat -b all -d > logcat.txt
adb shell getprop > props.txt
```

---

## 8. What this module will not fix

- A missing or incompatible IMS stack (see [IMS.md](IMS.md)).
- Carrier provisioning, a missing `ims` APN, or a VoLTE block on the IMEI.
- Framework `ClassNotFoundException` / `NoSuchMethodError` from porting Samsung
  APKs to a GSI.
- Kernel drivers or vendor blobs that are absent from the GSI's vendor partition.

If `status` shows all hooks `ok` and IMS still fails, the remaining problem is
almost certainly userspace or carrier-side.
