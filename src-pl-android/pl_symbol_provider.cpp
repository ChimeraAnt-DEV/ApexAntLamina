// pl::symbol_provider implementation for Android.
//
// The PC preloader's symbol provider answers queries from an export map
// generated from the server/client binary. On Android the game exports its
// symbols in libminecraftpe.so; we resolve them lazily with dlvsym/dlsym so
// a mod compiled against ApexAntLamina can look symbols up by name.

#include "pl/SymbolProvider.h"

#include <dlfcn.h>

#include <cstdio>
#include <cstring>
#include <mutex>
#include <string>

namespace {

constexpr const char* kGameModule = "libminecraftpe.so";

void* resolveGameSymbol(const char* name, size_t size, bool silent) {
    if (!name || size == 0) {
        return nullptr;
    }
    std::string symbol{name, size};

    void* handle = dlopen(kGameModule, RTLD_NOW | RTLD_NOLOAD);
    if (!handle) {
        if (!silent) {
            std::fprintf(stderr, "[ApexAntLamina] game module %s is not loaded\n", kGameModule);
        }
        return nullptr;
    }

    dlerror();
    void* result = dlsym(handle, symbol.c_str());
    if (!result && !silent) {
        if (auto* error = dlerror()) {
            std::fprintf(stderr, "[ApexAntLamina] failed to resolve %s: %s\n", symbol.c_str(), error);
        } else {
            std::fprintf(stderr, "[ApexAntLamina] symbol %s not found\n", symbol.c_str());
        }
    }
    return result;
}
} // namespace

extern "C" {

PLAPI void* pl_resolve_symbol(const char* symbolName) { return resolveGameSymbol(symbolName, std::strlen(symbolName), false); }

PLAPI void* pl_resolve_symbol_n(const char* symbolName, size_t size) { return resolveGameSymbol(symbolName, size, false); }

PLAPI void* pl_resolve_symbol_silent(const char* symbolName) {
    return resolveGameSymbol(symbolName, std::strlen(symbolName), true);
}

PLAPI void* pl_resolve_symbol_silent_n(const char* symbolName, size_t size) {
    return resolveGameSymbol(symbolName, size, true);
}

PLAPI const char* const* pl_lookup_symbol(void* func, size_t* resultLength) {
    // Deprecated API. The Android port cannot reverse-map an address to a
    // symbol name with the same fidelity, so this always reports no result.
    (void)func;
    if (resultLength) {
        *resultLength = 0;
    }
    return nullptr;
}

PLAPI const char* const* pl_lookup_symbol_disp(void* func, size_t* resultLength, unsigned int* displacement) {
    (void)func;
    if (resultLength) {
        *resultLength = 0;
    }
    if (displacement) {
        *displacement = 0;
    }
    return nullptr;
}

PLAPI void pl_free_lookup_result(const char* const* result) {
    // No-op: no heap-backed result is ever returned by the Android port.
    (void)result;
}
} // extern "C"