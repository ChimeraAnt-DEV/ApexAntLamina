// Out-of-line destructors for auto-generated BDS headers that declare
// `virtual ~X() = default;` while owning `std::unique_ptr<Pointee>` members.
//
// MSVC tolerates destroying a unique_ptr of an incomplete type in a defaulted
// destructor, but libc++ instantiates `~unique_ptr<T>` (and its
// `static_assert(sizeof(T) >= 0)`) immediately, so the pointee must be complete
// at the point where the destructor is instantiated. Where including the
// pointee header in the owning header would create an include cycle (for
// example ShadowBanList.h includes RakPeer.h), we instead declare the
// destructor in the header and define it here, in the only translation unit of
// the Android build where all pointees are complete.
//
// This file is only reachable from the Android (`src-pl-android`) sources.

#include "mc/deps/raknet/RakPeer.h"
#include "mc/deps/raknet/ShadowBanList.h"

namespace RakNet {
RakPeer::~RakPeer() = default;
} // namespace RakNet