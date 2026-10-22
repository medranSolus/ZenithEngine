#include "SpinLock.h"

namespace ZE
{
    void SpinLock::Lock() noexcept
    {
        U8 backoff = 1;
        while (true)
        {
            // Read-only spin (prevents cache line invalidation storms)
            if (!IsLocked())
            {
                // Try to acquire exclusive access
                if (!locked.exchange(true, std::memory_order_acquire))
                    return;
            }

            // Exponential backoff to reduce bus contention
            for (U8 i = 0; i < backoff; ++i)
                Intrin::Pause();

            if (backoff < 64)
                backoff <<= 1;
        }
    }
}