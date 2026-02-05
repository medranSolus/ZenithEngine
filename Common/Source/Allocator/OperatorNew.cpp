#include "Allocator/OperatorNew.h"
#include <new>

namespace ZE::Allocator
{
	void CheckNewReplacement(bool& status) noexcept
	{
		status = true;
	}
}

#if _ZE_COMPILER_MSVC
#	define ZE_OPERATOR_NEW_API __CRTDECL
#else
#	define ZE_OPERATOR_NEW_API __attribute__((__cdecl__))
#endif

void* ZE_OPERATOR_NEW_API operator new(size_t size)
{
	if (size == 0)
		size = 1;
	while (true)
	{
		void* ptr = std::malloc(size);
		if (ptr) [[likely]]
			return ptr;
		std::new_handler handler = std::get_new_handler();
		ZE_BREAK();
		handler ? (*handler)() : std::abort();
	}
}

void* ZE_OPERATOR_NEW_API operator new[](size_t size)
{
	if (size == 0)
		size = 1;
	while (true)
	{
		void* ptr = std::malloc(size);
		if (ptr) [[likely]]
			return ptr;
		std::new_handler handler = std::get_new_handler();
		ZE_BREAK();
		handler ? (*handler)() : std::abort();
	}
}

void* ZE_OPERATOR_NEW_API operator new(size_t size, std::align_val_t alignment)
{
	if (size == 0)
		size = 1;
	while (true)
	{
#if _ZE_COMPILER_MSVC
		void* ptr = _aligned_malloc(size, static_cast<size_t>(alignment));
#else
		void* ptr = std::aligned_alloc(static_cast<size_t>(alignment), size);
#endif
		if (ptr) [[likely]]
			return ptr;
		std::new_handler handler = std::get_new_handler();
		ZE_BREAK();
		handler ? (*handler)() : std::abort();
	}
}

void* ZE_OPERATOR_NEW_API operator new[](size_t size, std::align_val_t alignment)
{
	if (size == 0)
		size = 1;
	while (true)
	{
#if _ZE_COMPILER_MSVC
		void* ptr = _aligned_malloc(size, static_cast<size_t>(alignment));
#else
		void* ptr = std::aligned_alloc(static_cast<size_t>(alignment), size);
#endif
		if (ptr) [[likely]]
			return ptr;
		std::new_handler handler = std::get_new_handler();
		ZE_BREAK();
		handler ? (*handler)() : std::abort();
	}
}

void ZE_OPERATOR_NEW_API operator delete(void* ptr) noexcept
{
	std::free(ptr);
}

void ZE_OPERATOR_NEW_API operator delete[](void* ptr) noexcept
{
	std::free(ptr);
}

void ZE_OPERATOR_NEW_API operator delete(void* ptr, size_t size) noexcept
{
	std::free(ptr);
}

void ZE_OPERATOR_NEW_API operator delete[](void* ptr, size_t size) noexcept
{
	std::free(ptr);
}

void ZE_OPERATOR_NEW_API operator delete(void* ptr, std::align_val_t alignment) noexcept
{
#if _ZE_COMPILER_MSVC
	_aligned_free(ptr);
#else
	std::free(ptr);
#endif
}

void ZE_OPERATOR_NEW_API operator delete[](void* ptr, std::align_val_t alignment) noexcept
{
#if _ZE_COMPILER_MSVC
	_aligned_free(ptr);
#else
	std::free(ptr);
#endif
}

void ZE_OPERATOR_NEW_API operator delete(void* ptr, size_t size, std::align_val_t alignment) noexcept
{
#if _ZE_COMPILER_MSVC
	_aligned_free(ptr);
#else
	std::free(ptr);
#endif
}

void ZE_OPERATOR_NEW_API operator delete[](void* ptr, std::size_t size, std::align_val_t alignment) noexcept
{
#if _ZE_COMPILER_MSVC
	_aligned_free(ptr);
#else
	std::free(ptr);
#endif
}