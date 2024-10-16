#pragma once
#include "WinAPI.h"
#include "Utils.h"

namespace ZE::WinAPI
{
	class Perf final
	{
		// Counts per second
		long double frequency;

	public:
		Perf() noexcept { LARGE_INTEGER freq; QueryPerformanceFrequency(&freq); frequency = Utils::SafeCast<long double>(freq.QuadPart); }

		constexpr long double GetFrequency() const noexcept { return frequency; }
		U64 GetCurrentTimestamp() const noexcept { LARGE_INTEGER stamp; QueryPerformanceCounter(&stamp); return stamp.QuadPart; }
	};
}