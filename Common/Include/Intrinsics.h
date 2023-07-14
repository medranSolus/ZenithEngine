#pragma once
#include "BasicTypes.h"
#if _ZE_COMPILER_MSVC
#   include <intrin.h>
#elif _ZE_COMPILER_CLANG || _ZE_COMPILER_GCC
#   include <x86intrin.h>
#	include <cpuid.h>
#else
#   error Unsupported compiler!
#endif

namespace ZE::Intrin
{
	inline void CPUID(U32& eax, U32& ebx, U32& ecx, U32& edx, U32 function) noexcept;
	inline void CPUIDEX(U32& eax, U32& ebx, U32& ecx, U32& edx, U32 function, U32 subFunction) noexcept;
	inline bool IsCPUIDFunctionSupported(U32 function) noexcept;

	inline U64 Rdtsc() noexcept;

	inline void FenceStore() noexcept;
	inline void FenceLoad() noexcept;
	inline void FenceMemory() noexcept;

	// Returns number of bits set to 1 in 64 bit integer
	inline U8 CountBitsSet(U64 val) noexcept;
	// Returns number of bits set to 1 in 32 bit integer
	inline U8 CountBitsSet(U32 val) noexcept;

	// Scans U64 for index of first nonzero value from the Least Significant Bit (LSB).
	// If mask is 0 then returns UINT8_MAX
	inline U8 BitScanLSB(U64 mask) noexcept;
	// Scans U32 for index of first nonzero value from the Least Significant Bit (LSB).
	// If mask is 0 then returns UINT8_MAX
	inline U8 BitScanLSB(U32 mask) noexcept;

	// Scans U64 for index of first nonzero value from the Most Significant Bit (MSB).
	// If mask is 0 then returns UINT8_MAX
	inline U8 BitScanMSB(U64 mask) noexcept;
	// Scans U32 for index of first nonzero value from the Most Significant Bit (MSB).
	// If mask is 0 then returns UINT8_MAX
	inline U8 BitScanMSB(U32 mask) noexcept;

#pragma region Functions
	inline void CPUID(U32& eax, U32& ebx, U32& ecx, U32& edx, U32 function) noexcept
	{
		ZE_ASSERT(IsCPUIDFunctionSupported(function), "Unsupported CPUID function!");

#if _ZE_COMPILER_MSVC || _ZE_COMPILER_CLANG || _ZE_COMPILER_GCC
		int cpuInfo[4];
		__cpuid(cpuInfo, function);

		eax = static_cast<U32>(cpuInfo[0]);
		ebx = static_cast<U32>(cpuInfo[1]);
		ecx = static_cast<U32>(cpuInfo[2]);
		edx = static_cast<U32>(cpuInfo[3]);
#else
#   error Unsupported compiler!
#endif
	}

	inline void CPUIDEX(U32& eax, U32& ebx, U32& ecx, U32& edx, U32 function, U32 subFunction) noexcept
	{
		ZE_ASSERT(IsCPUIDFunctionSupported(function), "Unsupported CPUID function!");
		ZE_ASSERT(IsCPUIDFunctionSupported(subFunction), "Unsupported CPUID extended function!");

#if _ZE_COMPILER_MSVC
		int cpuInfo[4];
		__cpuidex(cpuInfo, static_cast<int>(function), static_cast<int>(subFunction));

		eax = static_cast<U32>(cpuInfo[0]);
		ebx = static_cast<U32>(cpuInfo[1]);
		ecx = static_cast<U32>(cpuInfo[2]);
		edx = static_cast<U32>(cpuInfo[3]);
#elif  _ZE_COMPILER_CLANG || _ZE_COMPILER_GCC
		__cpuid_count(function, subFunction, eax, ebx, ecx, edx);
#else
#   error Unsupported compiler!
#endif
	}

	inline bool IsCPUIDFunctionSupported(U32 function) noexcept
	{
#if _ZE_COMPILER_MSVC
		int cpuInfo[4];
		__cpuid(cpuInfo, static_cast<int>(function & 0x80000000));

		if (cpuInfo[0] == 0 || static_cast<U32>(cpuInfo[0]) < function)
			return false;
#elif  _ZE_COMPILER_CLANG || _ZE_COMPILER_GCC
		const U32 maxFunction = static_cast<U32>(__get_cpuid_max(function & 0x80000000, nullptr));

		if (cpuInfo[0] == 0 || cpuInfo[0] < function)
			return false;
#else
#   error Unsupported compiler!
#endif
		return true;
	}

	inline U64 Rdtsc() noexcept
	{
#if _ZE_COMPILER_MSVC || _ZE_COMPILER_CLANG || _ZE_COMPILER_GCC
		return __rdtsc();
#else
#   error Unsupported compiler!
#endif
	}

	inline void FenceStore() noexcept
	{
#if _ZE_COMPILER_MSVC || _ZE_COMPILER_CLANG || _ZE_COMPILER_GCC
		_mm_sfence();
#else
#   error Unsupported compiler!
#endif
	}

	inline void FenceLoad() noexcept
	{
#if _ZE_COMPILER_MSVC || _ZE_COMPILER_CLANG || _ZE_COMPILER_GCC
		_mm_lfence();
#else
#   error Unsupported compiler!
#endif
	}

	inline void FenceMemory() noexcept
	{
#if _ZE_COMPILER_MSVC
		__faststorefence();
#elif _ZE_COMPILER_CLANG || _ZE_COMPILER_GCC
		_mm_mfence();
#else
#   error Unsupported compiler!
#endif
	}

	inline U8 CountBitsSet(U64 val) noexcept
	{
#if __cplusplus >= 202002L
		return static_cast<U8>(std::popcount(val));
#elif _ZE_COMPILER_MSVC
		return static_cast<U8>(__popcnt64(val));
#elif _ZE_COMPILER_CLANG || _ZE_COMPILER_GCC
		return static_cast<U8>(__builtin_popcountll(val));
#else
		val -= (val >> 1) & 0x5555555555555555;
		val = (val & 0x3333333333333333) + ((val >> 2) & 0x3333333333333333);
		return static_cast<U8>((((val + (val >> 4)) & 0x0F0F0F0F0F0F0F0F) * 0x0101010101010101) >> 56);
#endif
	}

	inline U8 CountBitsSet(U32 val) noexcept
	{
#if __cplusplus >= 202002L
		return static_cast<U8>(std::popcount(val));
#elif _ZE_COMPILER_MSVC
		return static_cast<U8>(__popcnt(val));
#elif _ZE_COMPILER_CLANG || _ZE_COMPILER_GCC
		return static_cast<U8>(__builtin_popcount(val));
#else
		U32 c = val - ((val >> 1) & 0x55555555);
		c = ((c >> 2) & 0x33333333) + (c & 0x33333333);
		c = ((c >> 4) + c) & 0x0F0F0F0F;
		c = ((c >> 8) + c) & 0x00FF00FF;
		return static_cast<U8>(((c >> 16) + c) & 0x0000FFFF);
#endif
	}

	inline U8 BitScanLSB(U64 mask) noexcept
	{
#if _ZE_COMPILER_MSVC
		unsigned long pos;
		if (_BitScanForward64(&pos, mask))
			return static_cast<U8>(pos);
		return UINT8_MAX;
#elif _ZE_COMPILER_CLANG || _ZE_COMPILER_GCC
		return static_cast<U8>(__builtin_ffsll(mask)) - 1U;
#else
		U8 pos = 0;
		U64 bit = 1;
		do
		{
			if (mask & bit)
				return pos;
			bit <<= 1;
		} while (pos++ < 63);
		return UINT8_MAX;
#endif
	}

	inline U8 BitScanLSB(U32 mask) noexcept
	{
#if _ZE_COMPILER_MSVC
		unsigned long pos;
		if (_BitScanForward(&pos, mask))
			return static_cast<U8>(pos);
		return UINT8_MAX;
#elif _ZE_COMPILER_CLANG || _ZE_COMPILER_GCC
		return static_cast<U8>(__builtin_ffs(mask)) - 1U;
#else
		U8 pos = 0;
		U32 bit = 1;
		do
		{
			if (mask & bit)
				return pos;
			bit <<= 1;
		} while (pos++ < 31);
		return UINT8_MAX;
#endif
	}

	inline U8 BitScanMSB(U64 mask) noexcept
	{
#if _ZE_COMPILER_MSVC
		unsigned long pos;
		if (_BitScanReverse64(&pos, mask))
			return static_cast<U8>(pos);
#elif _ZE_COMPILER_CLANG || _ZE_COMPILER_GCC
		if (mask)
			return 63 - static_cast<U8>(__builtin_clzll(mask));
#else
		U8 pos = 63;
		U64 bit = 1ULL << 63;
		do
		{
			if (mask & bit)
				return pos;
			bit >>= 1;
		} while (pos-- > 0);
#endif
		return UINT8_MAX;
	}

	inline U8 BitScanMSB(U32 mask) noexcept
	{
#if _ZE_COMPILER_MSVC
		unsigned long pos;
		if (_BitScanReverse(&pos, mask))
			return static_cast<U8>(pos);
#elif _ZE_COMPILER_CLANG || _ZE_COMPILER_GCC
		if (mask)
			return 31 - static_cast<U8>(__builtin_clz(mask));
#else
		U8 pos = 31;
		U32 bit = 1UL << 31;
		do
		{
			if (mask & bit)
				return pos;
			bit >>= 1;
		} while (pos-- > 0);
#endif
		return UINT8_MAX;
	}
#pragma endregion
}