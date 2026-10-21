#pragma once
#include "Macros.h"

namespace ZE
{
    // Fast test-and-set spin lock for small code sections
    class SpinLock final
    {
        BoolAtom locked = false;

    public:
        SpinLock() = default;
        ZE_CLASS_DEFAULT(SpinLock);
        ~SpinLock() = default;

        bool IsLocked() const noexcept { return locked.load(std::memory_order_relaxed); }
        void Unlock() noexcept { locked.store(false, std::memory_order_release); }

        void Lock() noexcept;
    };
}