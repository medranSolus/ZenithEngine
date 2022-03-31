#pragma once
#include "Ptr.h"
#include <atomic>
#include <cstdint>

#pragma region Base types
	typedef uint8_t U8;
	typedef uint16_t U16;
	typedef uint32_t U32;
	typedef uint64_t U64;

	typedef int8_t S8;
	typedef int16_t S16;
	typedef int32_t S32;
	typedef int64_t S64;
#pragma endregion

#pragma region Atomic types
	typedef std::atomic_uint8_t UA8;
	typedef std::atomic_uint16_t UA16;
	typedef std::atomic_uint32_t UA32;
	typedef std::atomic_uint64_t UA64;

	typedef std::atomic_int8_t SA8;
	typedef std::atomic_int16_t SA16;
	typedef std::atomic_int32_t SA32;
	typedef std::atomic_int64_t SA64;
#pragma endregion