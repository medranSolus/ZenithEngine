#pragma once
#include "BasicTypes.h"
#include "WinAPI.h"

namespace ZE::WinAPI
{
	class Perf final
	{
		// Counts per second
		long double frequency;

	public:
		Perf() noexcept { LARGE_INTEGER freq; QueryPerformanceFrequency(&freq); frequency = static_cast<long double>(freq.QuadPart); }

		constexpr long double GetFrequency() const noexcept { return frequency; }
		U64 GetCurrentTimestamp() const noexcept { LARGE_INTEGER stamp; QueryPerformanceCounter(&stamp); return stamp.QuadPart; }
	};
}