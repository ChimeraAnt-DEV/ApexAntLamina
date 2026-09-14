// pl::hook implementation for Android, backed by the hook engine inside
// libpreloader.so (the GlossHook-based pl::memory::hook exported by the
// preloader-android runtime that LeviLauncher already loads with
// RTLD_GLOBAL). On AArch64 the trampoline installed by pl_hook preserves
// the general purpose registers by default, which is why RegisterSaveOptions
// are accepted but ignored.

#include "pl/Hook.h"

#include <dlfcn.h>

#include <cstddef>

namespace {

using HookFn   = int (*)(void*, void*, void**, int);
using UnhookFn = bool (*)(void*, void*);

// pl::memory::hook / pl::memory::unhook, exported by libpreloader.so.
constexpr const char* kHookSymbol   = "_ZN2pl6memory4hookEPvS2_PS2_NS0_12HookPriorityE";
constexpr const char* kUnhookSymbol = "_ZN2pl6memory6unhookEPvS2_";

struct HookBackend {
    void*    handle{};
    HookFn   hook{};
    UnhookFn unhook{};

    bool init() {
        if (handle) {
            return hook != nullptr;
        }
        handle = dlopen("libpreloader.so", RTLD_NOW | RTLD_GLOBAL);
        if (!handle) {
            return false;
        }
        hook   = reinterpret_cast<HookFn>(dlsym(handle, kHookSymbol));
        unhook = reinterpret_cast<UnhookFn>(dlsym(handle, kUnhookSymbol));
        return hook != nullptr && unhook != nullptr;
    }
};

HookBackend& backend() {
    static HookBackend instance;
    return instance;
}
} // namespace

static_assert(sizeof(pl::hook::Priority) == sizeof(int), "pl::hook::Priority must match C ABI enum size");

extern "C" {

PLAPI int pl_hook(void* target, void* detour, void** originalFunc, pl::hook::Priority priority) {
    if (!target || !detour || !originalFunc || !backend().init()) {
        return -1;
    }
    return backend().hook(target, detour, originalFunc, static_cast<int>(priority));
}

PLAPI int pl_hook_ex(
    void*                          target,
    void*                          detour,
    void**                         originalFunc,
    pl::hook::Priority             priority,
    pl::hook::RegisterSaveOptions  options
) {
    (void)options;
    // The Android GlossHook engine always saves the general-purpose
    // registers; the format-specific register set from the x64 preloader's
    // pl::hook::pl_hook_ex does not apply on AArch64.
    return pl_hook(target, detour, originalFunc, priority);
}

PLAPI bool pl_unhook(void* target, void* detour) {
    if (!backend().init()) {
        return false;
    }
    return backend().unhook(target, detour);
}

PLAPI bool pl_hook_options(pl::hook::FuncPtr target, pl::hook::RegisterSaveOptions options, bool replace) {
    // No-op on Android; register-save options do not apply to the AArch64
    // GlossHook engine. Return true so caller expectations are preserved.
    (void)target;
    (void)options;
    (void)replace;
    return true;
}
} // extern "C"