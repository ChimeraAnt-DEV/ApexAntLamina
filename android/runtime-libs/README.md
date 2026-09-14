# Prebuilt native libraries injected by the CI workflow.

This directory is filled at build time by `.github/workflows/build-apk.yml`:

- `arm64-v8a/libpreloader.so`
  Built from LiteLDev/preloader-android@0.2.3 (CMake + Android NDK), the
  LeviLaunchroid-compatible runtime. It is loaded with `RTLD_GLOBAL` by
  `org.levimc.launcher.CoreJni`.

- `arm64-v8a/libpreload-levilamina.so`
  The ApexAntLamina preload-native mod built from this repository via xmake
  (see `.github/workflows/build-mod.yml`). It declares `PLGetModRegistration`
  and is dlopen'ed by the preloader at runtime.

`.gitignore` keeps built artifacts out of the tree; the APK build script
downloads and/or builds both files before invoking Gradle.