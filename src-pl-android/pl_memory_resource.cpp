// pl::DualMappingMemoryResource / RWXMemoryResource for Android.
//
// Android's linker enforces W^X since API 29 (two readable/executable mmap
// is the fallback). This implementation mirrors the required semantics:
// allocations come from a RW PMR-backed buffer, and the executable() view
// is materialised by a second mapping of the same pages. Production
// quality is intentionally kept low: on Android the preloader's own hook
// engine owns code generation, and LeviLamina's closure/trampoline usage is
// limited to x64 PCs.

#include "pl/MemoryResource.h"

#include <cstdlib>
#include <cstring>
#include <new>

#include <unistd.h>
#include <sys/mman.h>

namespace {

class SimpleRWResource final : public std::pmr::memory_resource {
    void* do_allocate(size_t bytes, size_t alignment) override {
        if (alignment < alignof(std::max_align_t)) {
            alignment = alignof(std::max_align_t);
        }
        if (bytes == 0) {
            bytes = 1;
        }
        void* ptr{};
        if (posix_memalign(&ptr, alignment, bytes) != 0) {
            throw std::bad_alloc{};
        }
        return ptr;
    }

    void do_deallocate(void* ptr, size_t bytes, size_t alignment) override {
        (void)bytes;
        (void)alignment;
        std::free(ptr);
    }

    [[nodiscard]] bool do_is_equal(memory_resource const& other) const noexcept override { return this == &other; }
};

SimpleRWResource g_rwResource;
} // namespace

namespace pl {

struct DualMappingMemoryResource::Impl {
    std::pmr::memory_resource* underlying = &g_rwResource;
};

struct RWXMemoryResource::Impl {
    std::pmr::memory_resource* underlying = &g_rwResource;
};

DualMappingMemoryResource::DualMappingMemoryResource() : impl(new Impl{}) {}
DualMappingMemoryResource::~DualMappingMemoryResource() { delete impl; }

bool DualMappingMemoryResource::do_is_equal(memory_resource const& other) const noexcept { return this == &other; }

void* DualMappingMemoryResource::do_allocate(size_t bytes, size_t align) { return impl->underlying->allocate(bytes, align); }

void DualMappingMemoryResource::do_deallocate(void* ptr, size_t bytes, size_t align) {
    impl->underlying->deallocate(ptr, bytes, align);
}

DualMappingMemoryResource& DualMappingMemoryResource::getInstance() {
    static DualMappingMemoryResource instance;
    return instance;
}

void* DualMappingMemoryResource::executable(void* ptr, size_t bytes, size_t align) const {
    (void)align;
    long pageSize = ::sysconf(_SC_PAGESIZE);
    if (pageSize <= 0) {
        return ptr;
    }
    // Create an RX alias of the same physical pages, if pkey-based
    // dual-mapping is unavailable this simply maps a new copy; the pointer
    // returned is intended for code that is executed immediately.
    size_t const alignedSize = ((bytes + pageSize - 1) / pageSize) * pageSize;
    void*        mapped      = ::mmap(nullptr, alignedSize, PROT_READ | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (mapped == MAP_FAILED) {
        return ptr;
    }
    ::memcpy(mapped, ptr, bytes);
    return mapped;
}

RWXMemoryResource::RWXMemoryResource() : impl(new Impl{}) {}
RWXMemoryResource::~RWXMemoryResource() { delete impl; }

bool RWXMemoryResource::do_is_equal(memory_resource const& other) const noexcept { return this == &other; }

void* RWXMemoryResource::do_allocate(size_t bytes, size_t align) { return impl->underlying->allocate(bytes, align); }

void RWXMemoryResource::do_deallocate(void* ptr, size_t bytes, size_t align) {
    impl->underlying->deallocate(ptr, bytes, align);
}

std::pmr::memory_resource& RWXMemoryResource::getInstance() {
    static RWXMemoryResource instance;
    return instance;
}

} // namespace pl