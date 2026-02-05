#pragma once
#include <atomic>
#include <expected>
#include <system_error>
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

#pragma region Error types
	// Generic handler for status codes
	// (wrapper was needed since attributes cannot be applied to simple
	// using or typedef directives in MSVC - std compliance bug)
	class [[nodiscard]] Status final
	{
		std::error_code ec = {};

	public:
		Status() noexcept = default;
		Status(const Status&) noexcept = default;
		Status(Status&&) noexcept = default;
		Status& operator=(const Status&) noexcept = default;
		Status& operator=(Status&&) noexcept = default;
		~Status() = default;

		constexpr Status(const std::error_code& e) noexcept : ec(e) {}
		constexpr Status(std::error_code&& e) noexcept : ec(std::move(e)) {}
		Status(int code, const std::error_category& cat) noexcept : ec(code, cat) {}

		constexpr operator const std::error_code& () const noexcept { return ec; }
		constexpr operator std::error_code& () noexcept { return ec; }
		explicit operator bool() const noexcept { return static_cast<bool>(ec); }

		int value() const noexcept { return ec.value(); }
		const std::error_category& category() const noexcept { return ec.category(); }
		std::string message() const noexcept { return ec.message(); }

		friend bool operator==(const Status& a, const Status& b) noexcept { return a.ec == b.ec; }
		friend bool operator!=(const Status& a, const Status& b) noexcept { return a.ec != b.ec; }
	};

	// Wrapper for needed return value or error code in case of failure
	template<typename T>
	using Expected = std::expected<T, Status>;
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