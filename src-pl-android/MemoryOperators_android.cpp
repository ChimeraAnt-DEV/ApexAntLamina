// Android (AArch64) memory-operator overrides for LeviLamina.
//
// On Windows the dedicated server hook flows through IMemoryAllocator from
// Bedrock's memory block (`MemoryOperators_win.cpp`). On Android the mod is
// a plain .so loaded into the launcher process: operator new/delete are
// simply routed to the platform allocator, which is already capable of
// over-aligned allocations via C++17 aligned-new. The `ll_memory_operator_overrided`
// symbol is what NativeModManager checks before it will dlopen a sub-mod
// (see NativeModManager.cpp), so this translation unit must be present in
// the ApexAntLamina .so.

#define LL_MEMORY_OPERATORS
#include "ll/api/memory/MemoryOperators.h"

#include <cstdlib>
#include <cstring>

#include <malloc.h> // malloc_usable_size (bionic)

namespace ll::memory {

namespace {

class FallbackAllocator : public ::Bedrock::Memory::IMemoryAllocator {
public:
    void* allocate(uint64 size) override { return std::malloc(size != 0 ? size : 1); }

    void release(void* ptr) override { std::free(ptr); }

    void* alignedAllocate(uint64 size, uint64 alignment) override {
        if (alignment < sizeof(void*)) {
            return std::malloc(size != 0 ? size : 1);
        }
        void* p = nullptr;
        if (posix_memalign(&p, alignment, size != 0 ? size : 1) != 0) {
            return nullptr;
        }
        return p;
    }

    void alignedRelease(void* ptr) override { std::free(ptr); }

    uint64 getUsableSize(void* ptr, bool) override {
        if (!ptr) {
            return 0;
        }
        return malloc_usable_size(ptr);
    }

    void* _realloc(gsl::not_null<void*> ptr, uint64 newSize) override { return std::realloc(ptr, newSize); }

    void* _alignedRealloc(gsl::not_null<void*> ptr, uint64 newSize, uint64 alignment) override {
        void* p = alignedAllocate(newSize, alignment);
        if (!p) {
            return nullptr;
        }
        std::memcpy(p, ptr, malloc_usable_size(ptr.get()));
        alignedRelease(ptr.get());
        return p;
    }
};

} // namespace

IMemoryAllocator& getDefaultAllocator() {
    static FallbackAllocator ins;
    return ins;
}

} // namespace ll::memory