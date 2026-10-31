# samsung-gsi-fix

A **KernelPatch Module (KPM)** for Samsung phones running an AOSP GSI.

It removes kernel-side enforcement that commonly stops ported telephony and IMS
userspace from running, without making the whole kernel permissive:

| Kernel blocker | Hooked functions | Effect |
| --- | --- | --- |
| Samsung **DEFEX** | `task_defex_enforce`, `task_defex_zero_creds` | Returns allow before enforcement runs. Global, because DEFEX has no per-UID policy. |
| Samsung **FIVE / PROCA** | `five_bprm_check`, `five_file_mmap`, `five_file_verify` | Neutralizes synchronous integrity failures. See the limitation below. |
| **SELinux** | `avc_has_perm`, `avc_has_perm_flags`, `avc_has_extended_perms` | Skips AVC checks made through these wrappers for selected UIDs (default `1000` system and `1001` radio), or skips them globally if explicitly enabled. Checks that bypass these wrappers are unaffected. |

The module works with **APatch** and with KPM-capable KernelSU builds
(**SukiSU-Ultra**, or **KernelSU-Next + the KPatch-Next module**). Stock KernelSU
does not load KPMs.

> **Build-verified, not device-tested.** The module compiles cleanly against the
> pinned KernelPatch ABI and its ELF sections and symbols have been inspected,
> but it has not been run on a physical Samsung device in this repository. Read
> [Safety](#safety) and test on a device you can recover.

---

## Read this first: VoLTE and SMS

**This module cannot make VoLTE work by itself, and it does not implement SMS.**

VoLTE, VoWiFi and SMS-over-IMS are implemented in **userspace** by an IMS stack.
Samsung's stack is proprietary, tied to the One UI telephony framework, and is
not shipped in an AOSP GSI. A kernel module can remove kernel enforcement that
stops an IMS process from starting, but it cannot add the missing IMS service,
register it with `TelephonyManager`, or make a carrier's SIP servers accept the
device.

What you still need:

1. A working IMS stack:
   - [phhusson/ims](https://github.com/phhusson/ims) (Floss IMS) — works on some
     device/carrier combinations.
   - A ported Samsung IMS stack — firmware-specific and fragile.
   - Or a One UI-based ROM if VoLTE is a hard requirement.
2. Correct carrier provisioning and an `ims` APN.
3. A carrier that permits VoLTE for your device/IMEI.

**SMS:** ordinary SMS can still work over CS fallback (2G/3G) without VoLTE, if
your carrier still operates a CS network. SMS over IMS (SMS over LTE) needs a
registered IMS stack. On LTE-only carriers, neither calls nor SMS-over-IMS work
until IMS is working.

See **[docs/IMS.md](docs/IMS.md)** for the full, honest walkthrough.

### FIVE limitation

The FIVE hooks suppress synchronous failures from the measurement helpers. They
do **not** fake a trusted task-integrity state. On the kernels inspected for this
project, FIVE/PROCA perform measurement, auditing and DSMS reporting, while
**DEFEX is the component that actually denies execution**. If a device kills a
process asynchronously from another Samsung security component, these hooks will
not stop that. Check `dmesg` for the component name before assuming this module
is the right fix.

---

## Requirements

- arm64 Samsung device.
- Linux 4.14–6.12. KernelPatch advertises 3.18–6.12; Samsung GKI 5.10/5.15/6.1
  are the realistic targets.
- `CONFIG_KALLSYMS=y`. `CONFIG_KALLSYMS_ALL=y` is strongly recommended; without
  it, some non-exported Samsung symbols may not be found and those hooks are
  skipped.
- A KPM-capable root solution:
  - **APatch** — native KPM support.
  - **SukiSU-Ultra** — native KPM support.
  - **KernelSU-Next** with the
    [KPatch-Next module](https://github.com/KernelSU-Next/KPatch-Next-Module).
  - Stock KernelSU — **not supported** for KPM loading.
- Unlocked bootloader. Knox will be tripped; Samsung Pay, Secure Folder and some
  banking apps may stop working.

## Build

### Using the build script (recommended)

```sh
./build.sh
```

The script finds an aarch64 GCC, clones KernelPatch into `.build/KernelPatch`,
checks out the pinned commit, builds, and verifies the KPM sections.

Requirements:

- Linux: `sudo apt install build-essential git gcc-aarch64-linux-gnu`
- macOS: `brew install messense/macos-cross-toolchains/aarch64-unknown-linux-gnu`
  (or any `aarch64-elf-gcc` / `aarch64-none-elf-gcc` / `aarch64-unknown-linux-gnu-gcc` toolchain)
- `git`, `make`

### Manual build

```sh
git clone https://github.com/bmax121/KernelPatch.git .build/KernelPatch
git -C .build/KernelPatch checkout d4c88d6672687456b299e4b5153c24dc979d789b

make TARGET_COMPILE=aarch64-linux-gnu- KP_DIR="$PWD/.build/KernelPatch"
make check TARGET_COMPILE=aarch64-linux-gnu- KP_DIR="$PWD/.build/KernelPatch"
```

Output: `samsung-gsi-fix.kpm` — a relocatable aarch64 ELF.

The KernelPatch commit is pinned for reproducible builds. Override it with
`KP_COMMIT=<sha> ./build.sh` if you need a newer ABI.

## Install

Full instructions, including recovery, are in **[docs/INSTALL.md](docs/INSTALL.md)**.

### APatch

1. Open APatch Manager → **KPM**.
2. Install `samsung-gsi-fix.kpm` for boot-time loading, or use **Load** for an
   immediate one-shot load (lost on reboot).
3. To change settings, open the module's **Control** dialog and enter a command
   such as `status`.

### KernelPatch superkey (APatch / standalone KernelPatch)

```sh
adb push samsung-gsi-fix.kpm /data/local/tmp/
adb shell "truncate <superkey> module load /data/local/tmp/samsung-gsi-fix.kpm"
adb shell "truncate <superkey> module ctl0 samsung-gsi-fix 'status'"
```

Replace `<superkey>` with your KernelPatch superkey. The module name is
`samsung-gsi-fix` (visible in `module list`).

### SukiSU-Ultra

Use the manager's KPM screen to install/load the `.kpm`. If your build exposes
the KernelPatch superkey interface, the `truncate ... module ...` commands above
also work.

### KernelSU-Next + KPatch-Next

Install the KPatch-Next module first, then either use its WebUI or the bundled
CLI:

```sh
adb push samsung-gsi-fix.kpm /data/local/tmp/
adb shell "su -c 'kpatch kpm load /data/local/tmp/samsung-gsi-fix.kpm'"
adb shell "su -c 'kpatch kpm ctl0 samsung-gsi-fix \"status\"'"
```

`kpatch` lives in `/data/adb/modules/KPatch-Next/bin`; use the full path if it is
not on `PATH`.

## Control commands

All commands go through `ctl0`:

| Command | Meaning |
| --- | --- |
| `status` | Version, hook state (`ok` / `fail` / `missing`), current toggles and UID list. |
| `help` | Command summary. |
| `defex on\|off` | Enable/disable the DEFEX allow hooks. Default: **on**. |
| `five on\|off` | Enable/disable the FIVE/PROCA allow hooks. `proca` is an alias. Default: **on**. |
| `selinux targeted on\|off` | Allow SELinux checks for the configured UIDs. Default: **on**. |
| `selinux permissive on\|off` | Allow **all** SELinux checks globally. Default: **off**. Dangerous. |
| `uid add <uid>` | Add a UID to the targeted allowlist. |
| `uid del <uid>` | Remove a UID. |
| `uid list` | List the allowlist. |
| `uid reset` | Restore `1000 1001`. |

Examples:

```sh
truncate <superkey> module ctl0 samsung-gsi-fix "status"
truncate <superkey> module ctl0 samsung-gsi-fix "selinux targeted off"
truncate <superkey> module ctl0 samsung-gsi-fix "uid add 10123"
```

### Init arguments

KernelPatch passes an argument string to `KPM_INIT`. Supported tokens:

```
defex=on|off
five=on|off
selinux=on|off          # targeted allowlist
permissive=on|off       # global SELinux permissive
uid=<uid>               # add a UID (repeatable)
```

Example: `defex=on five=on selinux=on uid=1000 uid=1001`.

APatch's boot-time KPM loader currently loads with no args, so the compiled
defaults are used. Load with explicit args only through a CLI that accepts them.

## How it works

`src/hooks.c` resolves each symbol with `kallsyms_lookup_name()` and falls back
to `<name>.cfi_jt` on CFI kernels. Missing symbols are not fatal; the module
records them as `missing` and still applies the remaining hooks.

- **DEFEX** is skipped *before* the origin runs, because
  `task_defex_enforce()` can kill the calling process as a side effect.
- **FIVE** runs first and is overridden *after*. `five_bprm_check()` returning
  `-ENOMEM` is deliberately preserved: Samsung's code allocates the new task
  integrity object there, and continuing without it would cause a NULL
  dereference later.
- **SELinux** hooks the `avc_has_perm*` wrappers rather than `avc_denied`, which
  KernelPatch itself already hooks for its own su handling. This avoids fighting
  the host for the same function.

Unloading the module removes only its own callbacks from KernelPatch's hook
chains; other modules' hooks on the same functions are left intact.

## Safety

- **Targeted SELinux bypass is still a security reduction.** It disables *all*
  permission checks for processes running as the listed UIDs, including
  `system_server` and the phone process. Turn it off with
  `selinux targeted off` if the device works without it.
- **Global permissive mode** (`selinux permissive on`) disables SELinux
  enforcement kernel-wide. Use it only for debugging, and expect Play Integrity,
  banking apps and enterprise software to object.
- **DEFEX/FIVE bypass** weakens Samsung's integrity enforcement. It is intended
  for a device you control and can reflash.
- Keep a copy of your boot image and know how to remove the KPM before installing.
- Do not stack this with another module that hooks the same functions unless you
  understand KernelPatch's chain ordering.

## Troubleshooting

See **[docs/TROUBLESHOOTING.md](docs/TROUBLESHOOTING.md)**.

Quick checks:

```sh
# Kernel log: hook installation and symbolized status
adb shell dmesg | grep gsi-fix

# Module control output
adb shell "truncate <superkey> module ctl0 samsung-gsi-fix 'status'"

# SELinux denials (if targeted mode is on, these should disappear for 1000/1001)
adb shell dmesg | grep -i avc
```

## Repository layout

```
src/main.c        KPM metadata, init args, init/exit
src/hooks.c       DEFEX / FIVE / SELinux hooks and symbol resolution
src/ctl.c         ctl0 command parser and status output
src/config.c      runtime state and UID allowlist
src/util.c        freestanding string/number helpers
gsi_fix.lds       linker script for KernelPatch section layout
build.sh          toolchain detection, pinned KernelPatch checkout, build
docs/             install, IMS and troubleshooting guides
```

## License

GPL-2.0. See [LICENSE](LICENSE).

This project is not affiliated with Samsung, KernelPatch, APatch, KernelSU or
any carrier. Samsung IMS binaries are proprietary and are **not** included.
