#pragma once
#include "Intrinsics.h"
#include "Logger.h"

#if _ZE_MODE_DEBUG || _ZE_MODE_DEV
// Regular debug assert
#	define ZE_ASSERT(condition, message) do { if (!(condition)) { ZE::Logger::Error(message, true); ZE::Intrin::DebugBreak(); } } while (false)
#elif _ZE_MODE_PROFILE
// Regular debug assert
#	define ZE_ASSERT(condition, message) do { if (!(condition)) ZE::Logger::Error(message, true); } while (false)
#elif _ZE_MODE_RELEASE
// Regular debug assert
#	define ZE_ASSERT(condition, message) ((void)0)
#endif

// Always failing debug assert, indicating wrong code path that shouldn't occur
#define ZE_FAIL(message) ZE_ASSERT(false, message)
// Assert allowing debug break
#define ZE_BREAK() ZE_FAIL("Debug break")
// Failing assert indicating not correctly handled enum value in switch-case
#define ZE_ENUM_UNHANDLED() ZE_FAIL("Unhandled enum value!"); [[fallthrough]]
// Check if quaternion vector is unit length
#define ZE_ASSERT_Q_UNIT_V(rotor) ZE_ASSERT(ZE::Math::Internal::XMQuaternionIsUnit(rotor), "Quaternion is not unit quaternion!")
// Check if stored quaternion is unit length
#define ZE_ASSERT_Q_UNIT(rotor) ZE_ASSERT_Q_UNIT_V(ZE::Math::XMLoadFloat4(&rotor))

// Adds defaulted copy/move constructors/assign operators
#define ZE_CLASS_DEFAULT(className) \
	className(className&&) = default; \
	className(const className&) = default; \
	className& operator=(className&&) = default; \
	className& operator=(const className&) = default

// Adds defaulted copy constructors/assign operators and deletes move constructors/assign operators
#define ZE_CLASS_COPY(className) \
	className(className&&) = delete; \
	className(const className&) = default; \
	className& operator=(className&&) = delete; \
	className& operator=(const className&) = default

// Adds defaulted move constructors/assign operators and deletes copy constructors/assign operators
#define ZE_CLASS_MOVE(className) \
	className(className&&) = default; \
	className(const className&) = delete; \
	className& operator=(className&&) = default; \
	className& operator=(const className&) = delete

// Deletes all copy and move constructors/assign operators
#define ZE_CLASS_DELETE(className) \
	className(className&&) = delete; \
	className(const className&) = delete; \
	className& operator=(className&&) = delete; \
	className& operator=(const className&) = delete

// Adds deleted copy constructors/assign operators
#define ZE_CLASS_NO_COPY(className) \
	className(const className&) = delete; \
	className& operator=(const className&) = delete

// Adds deleted move constructors/assign operators
#define ZE_CLASS_NO_MOVE(className) \
	className(className&&) = delete; \
	className& operator=(className&&) = delete;

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