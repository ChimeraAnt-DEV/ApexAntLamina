#pragma once

#ifdef __cplusplus

#include <cstddef>
#include <memory_resource>

#include "pl/Macro.h"

namespace pl {

// A memory resource that can provide executable memory. On Android the
// compat shim maps RW then flips to RX when the executable() view is
// requested (W^X enforcement is not available before Android 11, mirroring
// the behaviour required by the Android preloader's hook engine).
class DualMappingMemoryResource final : public std::pmr::memory_resource {
    struct Impl;
    Impl* impl;

    [[nodiscard]] bool do_is_equal(memory_resource const& other) const noexcept override;
    void*              do_allocate(size_t bytes, size_t align) override;
    void               do_deallocate(void* ptr, size_t bytes, size_t align) override;

public:
    DualMappingMemoryResource();
    ~DualMappingMemoryResource() override;

    PLAPI static DualMappingMemoryResource& getInstance();

    PLAPI void* executable(void* ptr, size_t bytes, size_t align = alignof(std::max_align_t)) const;
};

class RWXMemoryResource final : public std::pmr::memory_resource {
    struct Impl;
    Impl* impl;

    [[nodiscard]] bool do_is_equal(memory_resource const& other) const noexcept override;
    void*              do_allocate(size_t bytes, size_t align) override;
    void               do_deallocate(void* ptr, size_t bytes, size_t align) override;

public:
    RWXMemoryResource();
    ~RWXMemoryResource() override;

    PLAPI static std::pmr::memory_resource& getInstance();
};

} // namespace pl

#endif