#pragma once
#include "Perf.h"

namespace ZE
{
	// Clone of std::lock_guard for performance measurements
	template<bool SHORT>
	class PerfGuard final
	{
	public:
		PerfGuard(const std::string& sectionTag) noexcept;
		ZE_CLASS_DELETE(PerfGuard);
		~PerfGuard();
	};

	// PerfGuard for short measurements
	typedef PerfGuard<true> PerfGuardShort;
	// Normal PerfGuard measurements
	typedef PerfGuard<false> PerfGuardNormal;

#pragma region Functions
	template<bool SHORT>
	PerfGuard<SHORT>::PerfGuard(const std::string& sectionTag) noexcept
	{
		if constexpr (SHORT)
			Perf::Get().StartShort(sectionTag);
		else
			Perf::Get().Start(sectionTag);
	}

	template<bool SHORT>
	PerfGuard<SHORT>::~PerfGuard()
	{
		if constexpr (SHORT)
			Perf::Get().StopShort();
		else
			Perf::Get().Stop();
	}
#pragma endregion
}

#if _ZE_MODE_PROFILE
// Start measurements section
#	define ZE_PERF_GUARD(sectionTag) ZE::PerfGuardNormal __perf(sectionTag)
// Start short measurements section
#	define ZE_PERF_GUARD_SHORT(sectionTag) ZE::PerfGuardShort __perfShort(sectionTag)
#else
// Start measurements section
#	define ZE_PERF_GUARD(sectionTag)
// Start short measurements section
#	define ZE_PERF_GUARD_SHORT(sectionTag)
#endif