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
	void CPUID(U32& eax, U32& ebx, U32& ecx, U32& edx, U32 function) noexcept;
	void CPUIDEX(U32& eax, U32& ebx, U32& ecx, U32& edx, U32 function, U32 subFunction) noexcept;
	bool IsCPUIDFunctionSupported(U32 function) noexcept;

	U64 Rdtsc() noexcept;

	// Raises breakpoint on any given compiler, regardless of the current build type
	void DebugBreak() noexcept;

	void FenceStore() noexcept;
	void FenceLoad() noexcept;
	void FenceMemory() noexcept;

	// Returns number of bits set to 1 in 64 bit integer
	U8 CountBitsSet(U64 val) noexcept;
	// Returns number of bits set to 1 in 32 bit integer
	U8 CountBitsSet(U32 val) noexcept;

	// Scans U64 for index of first nonzero value from the Least Significant Bit (LSB).
	// If mask is 0 then returns UINT8_MAX
	U8 BitScanLSB(U64 mask) noexcept;
	// Scans U32 for index of first nonzero value from the Least Significant Bit (LSB).
	// If mask is 0 then returns UINT8_MAX
	U8 BitScanLSB(U32 mask) noexcept;

	// Scans U64 for index of first nonzero value from the Most Significant Bit (MSB).
	// If mask is 0 then returns UINT8_MAX
	U8 BitScanMSB(U64 mask) noexcept;
	// Scans U32 for index of first nonzero value from the Most Significant Bit (MSB).
	// If mask is 0 then returns UINT8_MAX
	U8 BitScanMSB(U32 mask) noexcept;
}