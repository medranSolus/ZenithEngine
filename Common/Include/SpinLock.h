#pragma once
#include "Macros.h"

namespace ZE
{
    // Cache line size boundary for spin lock memory safety
#define ZE_SPIN_LOCK_CACHE_LINE_SIZE 64

    // Fast test-and-test-and-set spin lock for small code sections
    class alignas(ZE_SPIN_LOCK_CACHE_LINE_SIZE) SpinLock final
    {
        BoolAtom locked = false;
        U8 cachePadding[ZE_SPIN_LOCK_CACHE_LINE_SIZE - sizeof(BoolAtom)]; // Padding to not invalidate cache line with writes to adjacent locations

    public:
        SpinLock() = default;
        ZE_CLASS_DEFAULT(SpinLock);
        ~SpinLock() = default;

        bool IsLocked() const noexcept { return locked.load(std::memory_order_relaxed); }
        void Unlock() noexcept { locked.store(false, std::memory_order_release); }

        void Lock() noexcept;
    };
    static_assert(sizeof(SpinLock) == ZE_SPIN_LOCK_CACHE_LINE_SIZE, "Invalid padding of the SpinLock!");
}