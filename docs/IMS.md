# VoLTE, IMS and SMS on a Samsung GSI

This document is about the **userspace** side. The KPM in this repository does
not implement any of it.

## Short answer

- **VoLTE and VoWiFi require an IMS stack.** Samsung's stock IMS stack is
  proprietary, tied to the One UI telephony framework, and is not present in an
  AOSP GSI. A kernel module cannot add it.
- **The KPM only removes kernel enforcement** (DEFEX, FIVE/PROCA, targeted
  SELinux) that can stop an IMS process from starting or reading its files.
- **Your realistic options** are a Floss IMS stack (works on some
  device/carrier combinations), a ported Samsung IMS stack (firmware-specific
  and fragile), or a One UI-based ROM if VoLTE is mandatory.
- **SMS may work without VoLTE.** Many networks deliver SMS over NAS/SGs while
  the device is attached to LTE. If your carrier has retired SGs and uses
  SMS-over-IMS (SMSoIP), SMS needs a registered IMS stack.

If a carrier is LTE-only and no IMS stack works, there is no kernel-side trick
that will restore calls or SMS. That is the honest bottom line.

---

## 1. Why it breaks on a GSI

### On stock Samsung firmware

Samsung does not use the AOSP IMS implementation as-is. The stock firmware ships:

- `com.sec.imsservice` (`imsservice.apk`), a privileged app that extends
  Samsung's own IMS interfaces and registers with the Samsung telephony
  framework. Its manifest uses `android:sharedUserId="android.uid.system"`.
- Samsung framework classes (`com.sec.internal.ims.*`, `com.samsung.android.ims.*`)
  inside the One UI `framework.jar` / `telephony-common.jar`.
- Carrier customization (CSC) that defines the IMS APN, P-CSCF discovery,
  VoLTE/VoWiFi switches and carrier-specific SIP behavior.
- Vendor RIL and IMS libraries (`libsec-ril`, IMS daemons) that create the IMS
  PDN and expose P-CSCF addresses to the framework.

### On an AOSP GSI

The GSI has the AOSP IMS *framework* (`android.telephony.ims`, `ImsResolver`,
`MmTelFeature`), but:

- No Samsung `imsservice.apk` and no Samsung framework classes.
- No Samsung CSC integration in the One UI sense.
- The vendor partition may still contain Samsung RIL blobs, but nothing binds
  them to an IMS service.
- `ImsResolver` finds no installed `ImsService`, so IMS never registers.

The result:

| Service | Without IMS |
| --- | --- |
| Voice calls | Fall back to CS (2G/3G) if the network still has it; fail on LTE-only carriers. |
| SMS | Often still works over NAS/SGs; SMSoIP needs IMS. |
| VoWiFi | Does not work. |
| Emergency calls | Should use CS fallback; do not rely on a GSI for emergency calling. |

Kernel enforcement is a **separate** layer: DEFEX can kill an unsigned ported
IMS binary, FIVE can log/deny integrity, and SELinux can deny file or binder
access. Removing those does not create an IMS stack.

---

## 2. Diagnose before changing anything

Collect the current state:

```sh
# Is any IMS service installed?
adb shell pm list packages | grep -iE 'ims|rcs'
adb shell dumpsys package me.phh.ims 2>/dev/null | head -40
adb shell dumpsys package com.sec.imsservice 2>/dev/null | head -40

# What does telephony think?
adb shell dumpsys telephony.registry | grep -iE 'ims|volte|registration'
adb shell dumpsys carrier_config | grep -iE 'ims|volte|vt_|wfc' | head -60

# Properties (varies by GSI)
adb shell getprop | grep -iE 'ims|volte|vt|wfc|carrier'

# Radio log: the useful one
adb shell logcat -b radio -v time -d | grep -iE 'ims|volte|sip|register|pcscf' | tail -100
```

On a working VoLTE device, the radio log contains a successful SIP `REGISTER`
and the telephony registry reports IMS as registered. On a GSI with no IMS
service, you typically see repeated IMS registration failures, `ImsService` not
found, or no IMS PDN at all.

The AOSP testing menu (`*#*#4636#*#*` → Phone information) often shows
`IMS Registration: Not registered`. That is a symptom, not a fixable setting.

---

## 3. Option A: Floss IMS (phhusson/ims)

Repository: <https://github.com/phhusson/ims>

This is an open-source IMS client that implements `MmTelFeature`, VoLTE, VoWiFi
and SMS over IMS. It is used by some phh-based GSIs and is the most practical
starting point.

### Check whether your GSI already includes it

```sh
adb shell pm list packages | grep me.phh.ims
adb shell dumpsys package me.phh.ims | grep -E 'versionName|userId|codePath'
```

If it is present, skip to **Configure the APN** below. Do not install a second
copy.

### What it needs

- Android 12 or newer (`minSdk 32` in the current source).
- Installation as a **privileged system app**. The manifest declares
  `android:sharedUserId="android.uid.system"` and the service requires
  `android.permission.BIND_IMS_SERVICE`, which is a privileged permission.
- An **IMS APN** and a network that lets the modem establish the IMS PDN.
- P-CSCF discovery through the vendor RIL. If Samsung's RIL does not provide a
  P-CSCF address, Floss IMS cannot register even though it starts.
- The ROM's platform signature if you keep `android.uid.system`. You cannot
  install a self-built APK as a normal user app and expect it to bind as the
  system IMS service.

This is why "just install the APK" usually fails. Integration is a system-image
or overlay-module job, not a sideload.

### If your GSI ships it

1. Confirm the IMS APN exists (next section).
2. Enable VoLTE/VoWiFi in Settings → Network & internet → SIMs (the exact path
   varies by GSI).
3. Reboot and check the radio log for SIP registration:

   ```sh
   adb shell logcat -b radio -v time | grep -iE 'phh|ims|sip|register'
   ```

4. Test a call while the preferred network is set to LTE-only. If the call works
   with no CS fallback, VoLTE is up.
5. Test SMS with mobile data off. If SMS works only with data on, you are
   probably using SMSoIP, not CS/NAS SMS.

### If your GSI does not ship it

You need to build it into the system image or mount it as a privileged app with
the correct signature and permissions. The upstream repository has the source
and a minimal build note; it does not ship a universal flashable zip. Some
third-party GSIs and modules package it, but they are tied to a specific ROM
signing key and Android version. Treat any prebuilt as ROM-specific.

Practical alternatives:

- Use a GSI variant that already includes Floss IMS (often the "floss" builds).
- Ask the GSI maintainer to include `me.phh.ims` as a privileged app.
- Use a One UI-based ROM if you cannot modify the GSI.

### Limitations

- Works on some carriers/devices, not all. Samsung Exynos RIL behavior and
  carrier SIP profiles are the usual failure points.
- No RCS, no video calling, no Samsung-specific features.
- VoWiFi depends on ePDG reachability and carrier policy.
- Emergency calling over IMS is not a solved problem on GSIs. Keep a working
  stock ROM or a phone available for emergencies.

---

## 4. Option B: port the Samsung IMS stack

Reference attempt: <https://github.com/jameskdev/android_samsung_imsservice>

That project patches Samsung's `imsservice.apk` from an A21s (SM-A217N, Korean
firmware, Android 11) so it extends the AOSP IMS compat interfaces, and includes
the proprietary vendor IMS binaries and libraries. Its own README states that
VoLTE was confirmed on LineageOS 18.1 for that device, and that permissive
SELinux was required at the time.

**Scope warning:** this is a device/firmware-specific proof of concept, not a
general Samsung GSI solution. Do not expect an A21s Android 11 APK to work on an
S23 running Android 14.

If you still want to try:

1. Match the firmware. Use the IMS APK, framework jars, vendor libs and CSC
   files from the **same** stock firmware version as your device.
2. Deodex/patch the APK as the reference project describes.
3. Sign it with your ROM's platform key (the manifest uses
   `android.uid.system`).
4. Install it as a privileged app, add the `privapp-permissions` whitelist, and
   include the vendor IMS binaries/libraries in the right partition.
5. Provide the carrier CSC/APN configuration.
6. Address SELinux. Global permissive is the usual shortcut; this KPM's
   `selinux targeted on` with UID `1000` is a narrower alternative, but it is
   still a security reduction and may not cover every domain the port needs.

Typical failures at this stage are `ClassNotFoundException`,
`NoSuchMethodError`, missing `com.sec.*` classes, and binder transactions the
GSI framework does not implement. **The KPM cannot fix any of those.**

---

## 5. Carrier and APN checks

These apply to every IMS option.

### IMS APN

The device needs an APN with type `ims` (and on some carriers `ia` for the
initial attach). It is separate from the default data APN.

Check in Settings → Network & internet → SIMs → Access Point Names. If the list
has no `ims` entry, copy the APN values from the stock firmware's
`apns-conf.xml` or from your carrier's support page. Do not guess the APN name;
a wrong `ims` APN prevents the IMS PDN from coming up.

Some GSIs hide the IMS APN type in the UI. You can inspect the database read-only:

```sh
adb shell content query --uri content://telephony/carriers \
  --projection name:apn:type:mcc:mnc:protocol |
  grep -i ims
```

Editing the telephony database directly is not recommended; add the APN through
the Settings UI so the framework reloads it correctly.

### Provisioning

- Confirm the SIM is provisioned for VoLTE by the carrier. A SIM that works in a
  stock Samsung phone may still be blocked in an unrecognized device.
- Some carriers whitelist by IMEI/TAC. **Do not spoof the IMEI.** It is illegal in
  many countries and can get the device blocked.
- Check that the account is not on a plan without VoLTE.
- Test the same SIM in the stock ROM. If VoLTE fails there too, the problem is
  carrier-side, not the GSI.

### CSC and carrier config

Samsung's carrier customization is not part of AOSP. A GSI may keep some vendor
properties but does not run One UI's CSC framework. If your carrier requires
specific IMS timers, SIP headers or APN types from CSC, a generic GSI config may
not provide them.

Check what the framework sees:

```sh
adb shell dumpsys carrier_config | grep -iE 'ims|volte|wfc|vt' | head -80
```

Missing or `false` VoLTE flags are a strong hint that the carrier config is
generic, not that the kernel is blocking anything.

---

## 6. Verify a working state

Use all of these, not just the Settings toggle:

```sh
# 1. IMS service is bound
adb shell dumpsys activity services | grep -iE 'ims|MmTel'

# 2. Telephony registry reports IMS registration
adb shell dumpsys telephony.registry | grep -iE 'ims|volte'

# 3. Radio log shows a successful SIP REGISTER and a P-CSCF
adb shell logcat -b radio -v time -d |
  grep -iE 'REGISTER|200 OK|pcscf|ims pdn|ims apn' | tail -50

# 4. LTE-only call test
#    Settings → SIMs → preferred network type → LTE only
#    Place a call. If it succeeds, VoLTE is active.

# 5. SMS test with mobile data disabled
#    If SMS fails, try with data enabled and IMS registered.
```

A Settings toggle that says "VoLTE" is not proof. Some GSIs show the toggle even
when no IMS service is registered.

---

## 7. When to stop

Stop pursuing a GSI if any of these is true:

- Your carrier is LTE-only and no IMS stack registers.
- The stock ROM also fails VoLTE with the same SIM (carrier provisioning issue).
- The ported IMS APK crashes with missing Samsung framework classes and you
  cannot supply the matching framework.
- You need reliable emergency calling. A GSI without working IMS is not a safe
  daily driver for emergency calls.

In those cases, use a One UI-based custom ROM, restore the stock ROM, or use a
different device. That is a better outcome than a phone that looks like it has
VoLTE but cannot place a call.

---

## 8. What the KPM changes for IMS, concretely

| Symptom | Does the KPM help? |
| --- | --- |
| `avc: denied` for `system_server` / `radio` while starting IMS | Yes, with `selinux targeted on` (default) or a specific UID added. |
| DEFEX kills a ported IMS executable | Yes, DEFEX hooks skip enforcement. |
| FIVE logs integrity failures for IMS files | Partly; synchronous failures are neutralized, asynchronous state is not faked. |
| No `ImsService` installed | **No.** Install an IMS stack. |
| `ClassNotFoundException` / `NoSuchMethodError` | **No.** Framework mismatch. |
| IMS APN missing or carrier blocks VoLTE | **No.** Userspace/carrier fix. |
| SIP registration rejected by the carrier | **No.** Carrier/SIP configuration. |

If `status` shows every hook as `ok` and IMS still does not register, the kernel
side is no longer the blocker. Continue with the userspace checklist above.
