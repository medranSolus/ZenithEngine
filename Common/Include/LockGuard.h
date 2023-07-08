#pragma once
#include "Macros.h"
#include <shared_mutex>

namespace ZE
{
	// Clone of std::lock_guard allowing deciding if mutex lock have to be aquired
	template<bool SHARED>
	class LockGuard
	{
		std::shared_mutex* lock = nullptr;

	public:
		constexpr LockGuard(std::shared_mutex& mutex, bool aquireLock) noexcept;
		ZE_CLASS_DELETE(LockGuard);
		~LockGuard();
	};

	// Read-only lock guard version
	typedef LockGuard<true> LockGuardRO;
	// Read-write lock guard version
	typedef LockGuard<false> LockGuardRW;

#pragma region Functions
	template<bool SHARED>
	constexpr LockGuard<SHARED>::LockGuard(std::shared_mutex& mutex, bool aquireLock) noexcept
	{
		if (aquireLock)
		{
			lock = &mutex;
			if constexpr (SHARED)
				mutex.lock_shared();
			else
				mutex.lock();
		}
	}

	template<bool SHARED>
	LockGuard<SHARED>::~LockGuard()
	{
		if (lock)
		{
			if constexpr (SHARED)
				lock->unlock_shared();
			else
				lock->unlock();
		}
	}
#pragma endregion
}