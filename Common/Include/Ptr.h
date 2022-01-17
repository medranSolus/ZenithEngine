#pragma once
#include "Macros.h"
#include <memory>

namespace ZE
{
	// Basic raw pointer wrapper for performing checks during access
	// and setting itself to nullptr when used in move operation.
	// Recomended to use on member pointers to avoid neccessity of defining
	// custom move functions that only nulls the incoming pointer
	template<typename T>
	class Ptr final
	{
		T* ptr;

	public:
		constexpr Ptr() noexcept : ptr(nullptr) {}
		constexpr Ptr(T* p) noexcept : ptr(p) {}
		constexpr Ptr(Ptr&& p) noexcept : ptr(p.ptr) { p.ptr = nullptr; }
		constexpr Ptr(const Ptr& p) noexcept : ptr(p.ptr) {}

		constexpr Ptr& operator=(Ptr&& p) noexcept { ptr = p.ptr; p.ptr = nullptr; return *this; }
		constexpr Ptr& operator=(const Ptr& p) noexcept { ptr = p.ptr; return *this; }

		~Ptr() = default;

		// Never call 'delete' on this wrapper, use this method to delete object and set pointer to null or other pointer
		constexpr void Delete(T* p = nullptr) noexcept { ZE_ASSERT(ptr != nullptr, "Invalid pointer!"); delete ptr; ptr = p; }
		// Never call 'delete[]' on this wrapper, use this method to delete array and set pointer to null or other pointer
		constexpr void DeleteArray(T* p = nullptr) noexcept { ZE_ASSERT(ptr != nullptr, "Invalid pointer!"); delete[] ptr; ptr = p; }
		// For conveniece use this instead of calling normal 'free()'. It's not prohibited but this way pointer is safely set to null or other pointer
		constexpr void Free(T* p = nullptr) noexcept { ZE_ASSERT(ptr != nullptr, "Invalid pointer!"); free(ptr); ptr = p; }

		constexpr T& operator*() { ZE_ASSERT(ptr != nullptr, "Invalid pointer!"); return *ptr; }
		constexpr const T& operator*() const { ZE_ASSERT(ptr != nullptr, "Invalid pointer!"); return *ptr; }
		constexpr T* operator->() { ZE_ASSERT(ptr != nullptr, "Invalid pointer!"); return ptr; }
		constexpr const T* operator->() const { ZE_ASSERT(ptr != nullptr, "Invalid pointer!"); return ptr; }

		// Reinterpret cast wrapper
		template<typename S>
		constexpr S* Cast() noexcept { return reinterpret_cast<S*>(ptr); }
		// Reinterpret cast const wrapper
		template<typename S>
		constexpr const S* CastConst() const noexcept { return reinterpret_cast<const S*>(ptr); }

		constexpr operator T* () noexcept { return ptr; }
		constexpr operator const T* () const noexcept { return ptr; }

		constexpr bool operator==(const void* p) const noexcept { return ptr == p; }
		constexpr bool operator!=(const void* p) const noexcept { return ptr != p; }
		constexpr bool operator==(std::nullptr_t p) const noexcept { return ptr == p; }
		constexpr bool operator!=(std::nullptr_t p) const noexcept { return ptr != p; }
	};
}