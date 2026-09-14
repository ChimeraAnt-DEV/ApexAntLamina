// Android implementation of the sys_utils functions that need a real
// module handle on ARM64. Replaces SystemUtils_linux.cpp on Android:
//
//  * getCurrentModuleHandle: /proc/self/maps on Android starts with the main
//    executable (the game), NOT the module containing this code. NativeMod
//    identity checks compare against getCurrentModuleHandle(), so it has to
//    be the base address of ApexAntLamina itself, which we resolve with
//    dladdr() on an anchor symbol inside this .so.
//
//  * DynamicLibrary: NativeModManager loads sub-mods with dlopen/dlclose
//    (there is no LoadLibrary on Android). The implementation lives in this
//    Android platform file so the native-mod loading path is complete.

#include "ll/api/utils/SystemUtils.h"

#include <dlfcn.h>

#include <filesystem>
#include <optional>
#include <system_error>

namespace ll::internal {

LLAPI void* getCurrentModuleHandle() noexcept {
    static void* handle = [] {
        Dl_info info{};
        // Anchor: a symbol that is always resolved in this translation unit.
        if (dladdr(reinterpret_cast<void*>(&getCurrentModuleHandle), &info) && info.dli_fbase) {
            return info.dli_fbase;
        }
        return (void*)nullptr;
    }();
    return handle;
}

} // namespace ll::internal

namespace ll::utils::sys_utils {

std::optional<std::system_error> DynamicLibrary::load(std::filesystem::path const& path) noexcept {
    if (lib) {
        return std::system_error{std::make_error_code(std::errc::already_connected)};
    }
    void* handle = dlopen(path.c_str(), RTLD_NOW | RTLD_GLOBAL);
    if (!handle) {
        return std::system_error{std::make_error_code(std::errc::invalid_argument)};
    }
    lib = handle;
    return std::nullopt;
}

std::optional<std::system_error> DynamicLibrary::free() noexcept {
    if (!lib) {
        return std::nullopt;
    }
    if (dlclose(lib) != 0) {
        return std::system_error{std::make_error_code(std::errc::invalid_argument)};
    }
    lib = nullptr;
    return std::nullopt;
}

void* DynamicLibrary::getAddress(char const* name) noexcept {
    if (!lib) {
        return nullptr;
    }
    return dlsym(lib, name);
}

} // namespace ll::utils::sys_utils