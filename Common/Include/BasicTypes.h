#pragma once
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
	typedef std::atomic_bool BoolAtom;

	typedef std::atomic_uint8_t UA8;
	typedef std::atomic_uint16_t UA16;
	typedef std::atomic_uint32_t UA32;
	typedef std::atomic_uint64_t UA64;

	typedef std::atomic_int8_t SA8;
	typedef std::atomic_int16_t SA16;
	typedef std::atomic_int32_t SA32;
	typedef std::atomic_int64_t SA64;
#pragma endregion

#pragma region Vector types
	struct UInt2 { U32 X, Y; };
	struct UInt3 : public UInt2 { U32 Z; };
	struct UInt4 : public UInt3 { U32 W; };
	struct SInt2 { S32 X, Y; };
	struct SInt3 : public SInt2 { S32 Z; };
	struct SInt4 : public SInt3 { S32 W; };
#pragma endregion

#pragma region Vector operators
constexpr bool operator==(const UInt2& i1, const UInt2& i2) noexcept { return i1.X == i2.X && i1.Y == i2.Y; }
constexpr bool operator==(const UInt3& i1, const UInt3& i2) noexcept { return i1.Z == i2.Z && static_cast<const UInt2&>(i1) == static_cast<const UInt2&>(i2); }
constexpr bool operator==(const UInt4& i1, const UInt4& i2) noexcept { return i1.W == i2.W && static_cast<const UInt3&>(i1) == static_cast<const UInt3&>(i2); }
constexpr bool operator==(const SInt2& i1, const SInt2& i2) noexcept { return i1.X == i2.X && i1.Y == i2.Y; }
constexpr bool operator==(const SInt3& i1, const SInt3& i2) noexcept { return i1.Z == i2.Z && static_cast<const SInt2&>(i1) == static_cast<const SInt2&>(i2); }
constexpr bool operator==(const SInt4& i1, const SInt4& i2) noexcept { return i1.W == i2.W && static_cast<const SInt3&>(i1) == static_cast<const SInt3&>(i2); }

constexpr bool operator!=(const UInt2& i1, const UInt2& i2) noexcept { return i1.X != i2.X || i1.Y != i2.Y; }
constexpr bool operator!=(const UInt3& i1, const UInt3& i2) noexcept { return i1.Z != i2.Z || static_cast<const UInt2&>(i1) != static_cast<const UInt2&>(i2); }
constexpr bool operator!=(const UInt4& i1, const UInt4& i2) noexcept { return i1.W != i2.W || static_cast<const UInt3&>(i1) != static_cast<const UInt3&>(i2); }
constexpr bool operator!=(const SInt2& i1, const SInt2& i2) noexcept { return i1.X != i2.X || i1.Y != i2.Y; }
constexpr bool operator!=(const SInt3& i1, const SInt3& i2) noexcept { return i1.Z != i2.Z || static_cast<const SInt2&>(i1) != static_cast<const SInt2&>(i2); }
constexpr bool operator!=(const SInt4& i1, const SInt4& i2) noexcept { return i1.W != i2.W || static_cast<const SInt3&>(i1) != static_cast<const SInt3&>(i2); }
#pragma endregion