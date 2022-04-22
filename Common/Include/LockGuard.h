#pragma once
#include "Macros.h"
#include <mutex>

namespace ZE
{
	// Clone of std::lock_guard allowing deciding if mutex lock have to be aquired
	class LockGuard
	{
		std::mutex* lock = nullptr;

	public:
		constexpr LockGuard(std::mutex& mutex, bool aquireLock) noexcept { if (aquireLock) { lock = &mutex; mutex.lock(); } }
		ZE_CLASS_DELETE(LockGuard);
		~LockGuard() { if (lock) lock->unlock(); }
	};
}