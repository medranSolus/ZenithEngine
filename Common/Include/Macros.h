#pragma once
#include <cassert>

// Regular debug assert
#define ZE_ASSERT(condition, message) assert((condition) && message)

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