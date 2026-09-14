#pragma once

#ifdef __cplusplus
#define PRELOADER_MAYBE_UNUSED [[maybe_unused]]
#else
#define PRELOADER_MAYBE_UNUSED
#endif

#if defined(_WIN32)
#define PLAPI PRELOADER_MAYBE_UNUSED __declspec(dllexport)
#else
// The Android ApexAntLamina .so is loaded into the launcher process together
// with libpreloader.so. All pl-* symbols live in the ApexAntLamina binary
// itself (the compat shim), so everything is an export with default
// visibility on non-Windows platforms.
#define PLAPI PRELOADER_MAYBE_UNUSED __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
#define PLCAPI extern "C" PLAPI
#else
#define PLCAPI extern PLAPI
#endif