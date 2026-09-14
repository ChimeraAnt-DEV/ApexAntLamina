<div align="center">

# ApexAntLamina

**LeviLamina for Android** — a hardened, Android-native hard fork of the LeviLamina mod loader, built entirely by the **Chimera Team** for Minecraft Bedrock Edition on Android.

</div>

---

ApexAntLamina is a full port of the LeviLamina mod loader that runs natively on Android (**arm64-v8a**). It brings LeviLamina's powerful modding API — its comprehensive mod API, utility interfaces, powerful event system, and rich set of base interfaces — to Minecraft Bedrock Edition on Android devices.

The entire Android adaptation was **made fully by the Chimera Team**. We forked the official [LeviLamina](https://github.com/LiteLDev/LeviLamina) project and worked day and night to port, adapt, and harden the Android-specific core: the preload-native mod bootstrap (`libpreload-levilamina.so`), the Android runtime shim (`src-pl-android/`), the preloader JNI bridge, Termux and LeviLauncher-compatible install layouts, and the accompanying Android launcher app.

> **Copyright © 2026 Chimera Team (ChimeraAnt-DEV). All rights reserved.**
>
> This project is a hard fork of [LeviLamina](https://github.com/LiteLDev/LeviLamina), which is Copyright © 2024 LeviMC. ApexAntLamina is distributed under an **All Rights Reserved** license for all Android-specific work, adaptations, and modifications made by the Chimera Team. Use, modification, and redistribution of the Chimera Team's work is prohibited without explicit permission.
>
> **License notice:** This repository was forked from the official LeviLamina project. Its original LGPL-3.0 text (`COPYING` and `COPYING.LESSER`) was deleted from this repository by a contributor; the LICENSE files are intentionally **not** restored by the Chimera Team, and this project does **not** claim to be licensed under the LGPL-3.0. The modified work in this repository is protected and **All Rights Reserved** by the Chimera Team.

---

## What is it?

ApexAntLamina is a lightweight, modular, and versatile mod loader — but unlike the desktop-oriented upstream, it is purpose-built for **Android**. It loads native preload mods on Minecraft Bedrock Edition running on Android devices, through a preloader runtime (`libpreloader.so`) that provides the memory, hook, and symbol APIs the mod loader forwards to.

| | Upstream LeviLamina | ApexAntLamina |
|---|---|---|
| Platform | Windows / Linux | **Android (arm64-v8a)** |
| Runtime | BDS / client desktop | Bedrock Android + preloader runtime |
| Mod format | Native preload mods | Native preload mods (`preload-native`) |
| Loader | BDS mod manager | Android launcher / Termux `dlopen` |

## Features

- **Native Android port of LeviLamina** — the full LeviLamina modding API, ported to run on Android.
- **Preload-native mod support** — the loader ships as a `preload-native` mod (`libpreload-levilamina.so`) driven by the preloader's JNI bridge.
- **Android runtime shim** — the Android-specific core lives in `src-pl-android/`: memory-operator overrides for AArch64, Android config/logging/paths, built-in command registration, and a Termux-aware layout (logs route to the launcher's files dir, or to Termux pipes when installed there).
- **Android launcher app** — a minimal LeviLauncher-compatible Android host (`android/`) that bundles the preload mod, sets up the mods root, and drives the mod lifecycle through the preloader's Java JNI bridge.
- **Cross-built with xmake + Android NDK** — the native mod cross-compiles for Android with the NDK toolchain (`android|arm64-v8a`, `android|armeabi-v7a`), with desktop-only pieces (mimalloc, cpr/libcurl, Windows prelink) cleanly excluded.
- **Comprehensive API surface** — the full LeviLamina API for mods: events, utilities, reflection, i18n, plugins, and more.

## Repository layout

```
├── android/            # Android launcher app (LeviLaunchroid-compatible host)
├── src/                # Core LeviLamina source (mod loader, plugin framework)
├── src-server/         # Dedicated-server-specific implementations
├── src-client/         # Client-specific implementations
├── src-pl-android/     # Android-specific runtime shim (the heart of the port)
├── src-pl-export/      # Mod registration / export glue
├── src-test/           # Common, server, and client tests
├── docs/               # API + main documentation
├── xmake.lua           # xmake build script (Android/Windows/Linux)
└── tooth.json          # Tooth / lip package manifest
```

## Building

ApexAntLamina cross-compiles for Android using [xmake](https://xmake.io/) with the Android NDK toolchain.

```bash
# For the Android preload mod (arm64-v8a)
xmake f -p android -a arm64-v8a -m release -y --target_type=server
xmake -v -y -P .

# For Android launcher APK
cd android && gradle --no-daemon assembleDebug
```

> Note: the launcher project itself currently lives in the separate [LeviLaunchroid-investigation](https://github.com/ChimeraAnt-DEV/LeviLaunchroid-investigation) fork; this repository ships the native mod `.so` and the launcher host used to boot it.

## License

**Copyright © 2026 Chimera Team (ChimeraAnt-DEV). All rights reserved.**

ApexAntLamina is a hard fork of [LeviLamina](https://github.com/LiteLDev/LeviLamina) (Copyright © 2024 LeviMC).

All Android-specific work, the Android port, adaptations, and modifications made by the Chimera Team are proprietary and **All Rights Reserved**. The Chimera Team worked day and night to ensure it works on Android; please do not over-credit the upstream team for the Android work, which they did not do.

The official LeviLamina project is licensed under the LGPL-3.0 License for its non-closed-source parts. **The LGPL-3.0 license files (`COPYING` / `COPYING.LESSER`) were deleted from this repository by a third party** and have not been restored; ApexAntLamina's own work is **not** released under the LGPL-3.0.
