#pragma once
#include "Intrinsics.h"
#include "Logger.h"

// Helper macro to create a FourCC code from four characters
#define ZE_MAKE_FOURCC(c0, c1, c2, c3) (static_cast<U32>(c0) | (static_cast<U32>(c1) << 8) | (static_cast<U32>(c2) << 16) | (static_cast<U32>(c3) << 24))
// Helper macro to check if a FourCC code matches the given four characters
#define ZE_IS_FOURCC(fourCC, c0, c1, c2, c3) (static_cast<U32>(c0) | (static_cast<U32>(c1) << 8) | (static_cast<U32>(c2) << 16) | (static_cast<U32>(c3) << 24)) == fourCC

#pragma region Asserts
#if _ZE_MODE_DEBUG || _ZE_MODE_DEV
// Debug assert with ability to specify level of log entry
#	define ZE_ASSERT_LVL(lvl, condition, message) do { if (!(condition)) { ZE::Logger::##lvl##(message, true); ZE::Intrin::DebugBreak(); } } while (false)
#elif _ZE_MODE_PROFILE
// Debug assert with ability to specify level of log entry
#	define ZE_ASSERT_LVL(lvl, condition, message) do { if (!(condition)) ZE::Logger::##lvl##(message, true); } while (false)
#elif _ZE_MODE_RELEASE
// Debug assert with ability to specify level of log entry
#	define ZE_ASSERT_LVL(lvl, condition, message) ((void)0)
#endif

#if !_ZE_MODE_RELEASE
// Assert allowing debug break
#	define ZE_BREAK() ZE::Intrin::DebugBreak()
#else
// Assert allowing debug break
#	define ZE_BREAK() ((void)0)
#endif

// Regular debug assert
#define ZE_ASSERT(condition, message) ZE_ASSERT_LVL(Error, condition, message)
// Regular debug assert for warnings
#define ZE_ASSERT_WARN(condition, message) ZE_ASSERT_LVL(Warning, condition, message)
// Assert for checking whether resource was freed before deletion
#define ZE_ASSERT_FREED(condition) ZE_ASSERT_LVL(Error, condition, "Resource not freed before deletion!")
// Assert for checking whether selected code path has been initialized before
#define ZE_ASSERT_INIT(condition) ZE_ASSERT_LVL(Error, condition, "Accessing not properly initialized code path!")
// Always failing debug assert, indicating wrong code path that shouldn't occur
#define ZE_FAIL(message) ZE_ASSERT_LVL(Error, false, message)
// Always failing debug assert, indicating warning condition that shouldn't occur but allows for ruther program execution
#define ZE_WARNING(message) ZE_ASSERT_LVL(Warning, false, message)
// Failing assert indicating not correctly handled enum value in switch-case
#define ZE_ENUM_UNHANDLED() ZE_ASSERT_LVL(Warning, false, "Unhandled enum value!"); [[fallthrough]]
// Check if quaternion vector is unit length
#define ZE_ASSERT_Q_UNIT_V(rotor) ZE_ASSERT_LVL(Warning, ZE::Math::IsUnitQuaternion(rotor), "Quaternion is not unit quaternion!")
// Check if stored quaternion is unit length
#define ZE_ASSERT_Q_UNIT(rotor) ZE_ASSERT_Q_UNIT_V(ZE::Math::XMLoadFloat4(&rotor))
#pragma endregion

#pragma region Error codes
#ifndef __FILENAME__
namespace ZE
{
	// Extract filename from filepath to hide full path in error logs
	constexpr const char* GetFilename(const char* path) noexcept
	{
		const char* file = path;
		const char* prevFile = path;
		while (*path)
		{
			++path;
			if (*path == '/' || *path == '\\')
			{
				if (file != prevFile)
					prevFile = file;
				file = path;
			}
		}
		return prevFile + 1;
	}
}
// Current file filename
#define __FILENAME__ ZE::GetFilename(__FILE__)
#endif

// Log informational message base on given code
#define ZE_CODE_INFO(code, msg) ZE::Logger::CodeInfo(code, msg, __LINE__, __FILENAME__)
// Log warning message base on given code
#define ZE_CODE_WARNING(code, msg) do { ZE_BREAK(); ZE::Logger::CodeWarning(code, msg, __LINE__, __FILENAME__); } while (false)
// Log error message base on given code
#define ZE_CODE_ERROR(code, msg) do { ZE_BREAK(); ZE::Logger::CodeError(code, msg, __LINE__, __FILENAME__); } while (false)
// Log critical error message base on given code
#define ZE_CODE_CRITICAL(code, msg) do { ZE_BREAK(); ZE::Logger::CodeCritical(code, msg, __LINE__, __FILENAME__); } while (false)

// Instantly return if received error code
#define ZE_CODE_RET_FAILED(call) do { if (Status __code = (call)) return __code; } while (false)
// Instantly return if received error code (wrapped in std::unexpected)
#define ZE_CODE_RET_FAILED_EXPECT(call) do { if (Status __code = (call)) return std::unexpected(__code); } while (false)
// Instantly return if received error code and log aprioriate error
#define ZE_LOG_RET_FAILED(call, info) do { if (Status __code = (call)) { ZE_CODE_ERROR(__code, info); return __code; } } while (false)
// Instantly return if received error code (wrapped in std::unexpected) and log aprioriate error
#define ZE_LOG_RET_FAILED_EXPECT(call, info) do { if (Status __code = (call)) { ZE_CODE_ERROR(__code, info); return std::unexpected(__code); } } while (false)
// Instantly return if received error code instead of expected object
#define ZE_EXPECT_RET_FAILED(expectedVar, call) do { auto __exp = (call); if (!__exp) return std::unexpected(__exp.error()); expectedVar = std::move(*__exp); } while (false)
// Instantly return if received error code instead of expected object (return error code only)
#define ZE_EXPECT_RET_FAILED_CODE(expectedVar, call) do { auto __exp = (call); if (!__exp) return __exp.error(); expectedVar = std::move(*__exp); } while (false)

#if !_ZE_MODE_RELEASE
// Log informational message base on given code (silenced in release build)
#	define ZE_CODE_INFO_DEBUG(code, msg) ZE::Logger::CodeInfo(code, msg, __LINE__, __FILENAME__)
// Log warning message base on given code (silenced in release build)
#	define ZE_CODE_WARNING_DEBUG(code, msg) do { ZE_BREAK(); ZE::Logger::CodeWarning(code, msg, __LINE__, __FILENAME__); } while (false)
// Log error message base on given code (silenced in release build)
#	define ZE_CODE_ERROR_DEBUG(code, msg) do { ZE_BREAK(); ZE::Logger::CodeError(code, msg, __LINE__, __FILENAME__); } while (false)
#else
// Log informational message base on given code (silenced in release build)
#	define ZE_CODE_INFO_DEBUG(code, msg) ((void)0)
// Log warning message base on given code (silenced in release build)
#	define ZE_CODE_WARNING_DEBUG(code, msg) ZE_BREAK(); ((void)0)
// Log error message base on given code (silenced in release build)
#	define ZE_CODE_ERROR_DEBUG(code, msg) ZE_BREAK(); ((void)0)
#endif

#pragma endregion

#pragma region Class members
// Adds defaulted copy/move constructors/assign operators
#define ZE_CLASS_DEFAULT(className) \
	className(className&&) = default; \
	className(const className&) = default; \
	className& operator=(className&&) = default; \
	className& operator=(const className&) = default

// Deletes all copy and move constructors/assign operators
#define ZE_CLASS_DELETE(className) \
	className(className&&) = delete; \
	className(const className&) = delete; \
	className& operator=(className&&) = delete; \
	className& operator=(const className&) = delete

// Adds defaulted copy constructors/assign operators and deletes move constructors/assign operators
#define ZE_CLASS_COPY(className) \
	className(className&&) = delete; \
	className(const className&) = default; \
	className& operator=(className&&) = delete; \
	className& operator=(const className&) = default

// Adds defaulted copy constructors/assign operators
#define ZE_CLASS_COPY_ONLY(className) \
	className(const className&) = default; \
	className& operator=(const className&) = default

// Adds deleted copy constructors/assign operators
#define ZE_CLASS_NO_COPY(className) \
	className(const className&) = delete; \
	className& operator=(const className&) = delete

// Adds defaulted move constructors/assign operators and deletes copy constructors/assign operators
#define ZE_CLASS_MOVE(className) \
	className(className&&) = default; \
	className(const className&) = delete; \
	className& operator=(className&&) = default; \
	className& operator=(const className&) = delete

// Adds deleted move constructors/assign operators
#define ZE_CLASS_MOVE_ONLY(className) \
	className(className&&) = default; \
	className& operator=(className&&) = default;

// Adds deleted move constructors/assign operators
#define ZE_CLASS_NO_MOVE(className) \
	className(className&&) = delete; \
	className& operator=(className&&) = delete;
#pragma endregion

#pragma region String utility
// Macro for removing parentheses around macro values
#define ZE_DEPAREN(x) ZE_VANISH x
// Part of ZE_DEPAREN() macro procedure
#define ZE_VANISH(...) __VA_ARGS__

// Macro for turning literal values into strings
#define ZE_XSTRINGIFY(x) #x
// Macro for turning values into strings
#define ZE_STRINGIFY(x) ZE_XSTRINGIFY(x)
// Macro for turning versions into strings
#define ZE_STRINGIFY_VERSION(major, minor, patch) ZE_STRINGIFY(major)"."ZE_STRINGIFY(minor)"."ZE_STRINGIFY(patch)
#pragma endregion

#pragma region Compiler warnings
// Compiler agnostic way of handling pragma directive
#define ZE_PRAGMA(X) _Pragma(#X)

// Compiler agnostic macros for handling warnings
#if _ZE_COMPILER_MSVC
// Compiler agnostic way of pushing warnings on stack
#	define ZE_WARNING_PUSH ZE_PRAGMA(warning(push, 0))
// Compiler agnostic way of popping warnings from stack
#	define ZE_WARNING_POP ZE_PRAGMA(warning(pop))

// Disable MSVC specific warning
#	define ZE_WARNING_DISABLE_MSVC(number) ZE_PRAGMA(warning(disable : number))
// Disable GCC and Clang specific warning
#	define ZE_WARNING_DISABLE_GCC_CLANG(warningName)
#elif _ZE_COMPILER_GCC || _ZE_COMPILER_CLANG
// Compiler agnostic way of pushing warnings on stack
#	define ZE_WARNING_PUSH ZE_PRAGMA(GCC diagnostic push)
// Compiler agnostic way of popping warnings from stack
#	define ZE_WARNING_POP ZE_PRAGMA(GCC diagnostic pop)

// Disable MSVC specific warning
#	define ZE_WARNING_DISABLE_MSVC(number)
// Disable GCC and Clang specific warning
#	define ZE_WARNING_DISABLE_GCC_CLANG(warningName) ZE_PRAGMA(GCC diagnostic ignored #warningName)
#else
#	error Compiler not supported!
#endif
#pragma endregion

// Allow for classic bit operations on enum class type of enumeration
#define ZE_ENUM_OPERATORS(Type, BaseType) \
	inline constexpr BaseType Base(Type e) noexcept { return static_cast<BaseType>(e); } \
	inline constexpr BaseType operator&(Type e1, Type e2) noexcept { return static_cast<BaseType>(e1) & static_cast<BaseType>(e2); } \
	inline constexpr BaseType operator|(Type e1, Type e2) noexcept { return static_cast<BaseType>(e1) | static_cast<BaseType>(e2); } \
	inline constexpr BaseType operator^(Type e1, Type e2) noexcept { return static_cast<BaseType>(e1) ^ static_cast<BaseType>(e2); } \
	inline constexpr BaseType operator&(Type e1, BaseType e2) noexcept { return static_cast<BaseType>(e1) & e2; } \
	inline constexpr BaseType operator|(Type e1, BaseType e2) noexcept { return static_cast<BaseType>(e1) | e2; } \
	inline constexpr BaseType operator^(Type e1, BaseType e2) noexcept { return static_cast<BaseType>(e1) ^ e2; } \
	inline constexpr BaseType operator&(BaseType e1, Type e2) noexcept { return e1 & static_cast<BaseType>(e2); } \
	inline constexpr BaseType operator|(BaseType e1, Type e2) noexcept { return e1 | static_cast<BaseType>(e2); } \
	inline constexpr BaseType operator^(BaseType e1, Type e2) noexcept { return e1 ^ static_cast<BaseType>(e2); } \
	inline constexpr BaseType operator~(Type e) noexcept { return ~static_cast<BaseType>(e); } \
	inline constexpr BaseType& operator&=(BaseType& e1, Type e2) noexcept { e1 = e1 & e2; return e1; } \
	inline constexpr BaseType& operator|=(BaseType& e1, Type e2) noexcept { e1 = e1 | e2; return e1; } \
	inline constexpr BaseType& operator^=(BaseType& e1, Type e2) noexcept { e1 = e1 ^ e2; return e1; } \
	inline constexpr bool operator==(BaseType e1, Type e2) noexcept { return e1 == static_cast<BaseType>(e2); } \
	inline constexpr bool operator!=(BaseType e1, Type e2) noexcept { return e1 != static_cast<BaseType>(e2); }