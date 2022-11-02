#pragma once
#include "BasicTypes.h"
#if _ZE_COMPILER_MSVC
#   include <intrin.h>
#   pragma intrinsic(__rdtsc, __faststorefence)
#elif _ZE_COMPILER_CLANG || _ZE_COMPILER_GCC
#   include <x86intrin.h>
#else
#   error Unsupported compiler!
#endif

namespace ZE::Intrin
{
	inline U64 Rdtsc() noexcept;
	inline void FenceStore() noexcept;
	// Returns number of bits set to 1
	inline U32 CountBitsSet(U32 val) noexcept;
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
	inline U64 Rdtsc() noexcept
	{
		return __rdtsc();
	}

	inline void FenceStore() noexcept
	{
#if _ZE_COMPILER_MSVC
		__faststorefence();
#elif _ZE_COMPILER_CLANG || _ZE_COMPILER_GCC
		_mm_sfence();
#endif
	}

	inline U32 CountBitsSet(U32 val) noexcept
	{
#if __cplusplus >= 202002L
		return std::popcount(val);
#elif _ZE_COMPILER_MSVC
		return __popcnt(val);
#elif _ZE_COMPILER_CLANG || _ZE_COMPILER_GCC
		return static_cast<U32>(__builtin_popcount(val));
#else
		U32 c = val - ((val >> 1) & 0x55555555);
		c = ((c >> 2) & 0x33333333) + (c & 0x33333333);
		c = ((c >> 4) + c) & 0x0F0F0F0F;
		c = ((c >> 8) + c) & 0x00FF00FF;
		c = ((c >> 16) + c) & 0x0000FFFF;
		return c;
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