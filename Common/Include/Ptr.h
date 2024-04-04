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

		constexpr T** operator&() noexcept { return &ptr; }
		constexpr T& operator*() noexcept { ZE_ASSERT(ptr != nullptr, "Invalid pointer!"); return *ptr; }
		constexpr const T& operator*() const noexcept { ZE_ASSERT(ptr != nullptr, "Invalid pointer!"); return *ptr; }
		constexpr T* operator->() noexcept { ZE_ASSERT(ptr != nullptr, "Invalid pointer!"); return ptr; }
		constexpr const T* operator->() const noexcept { ZE_ASSERT(ptr != nullptr, "Invalid pointer!"); return ptr; }

		// Reinterpret cast wrapper
		template<typename S>
		constexpr S* Cast() noexcept { return reinterpret_cast<S*>(ptr); }
		// Reinterpret cast const wrapper
		template<typename S>
		constexpr const S* CastConst() const noexcept { return reinterpret_cast<const S*>(ptr); }
		// Reinterpret cast wrapper directly on pointer
		template<typename S>
		constexpr S CastPtr() noexcept { return reinterpret_cast<S>(ptr); }
		// Reinterpret cast const wrapper directly on pointer
		template<typename S>
		constexpr const S CastPtrConst() const noexcept { return reinterpret_cast<const S>(ptr); }

		constexpr operator T* () const noexcept { return ptr; }
	};

	// Raw pointer wrapper specialization for Ptr<void> with reduced funcionality
	class PtrVoid final
	{
		void* ptr;

	public:
		constexpr PtrVoid() noexcept : ptr(nullptr) {}
		constexpr PtrVoid(void* p) noexcept : ptr(p) {}
		constexpr PtrVoid(PtrVoid&& p) noexcept : ptr(p.ptr) { p.ptr = nullptr; }
		constexpr PtrVoid(const PtrVoid& p) noexcept : ptr(p.ptr) {}

		constexpr PtrVoid& operator=(PtrVoid&& p) noexcept { ptr = p.ptr; p.ptr = nullptr; return *this; }
		constexpr PtrVoid& operator=(const PtrVoid& p) noexcept { ptr = p.ptr; return *this; }

		~PtrVoid() = default;

		// For conveniece use this instead of calling normal 'free()'. It's not prohibited but this way pointer is safely set to null or other pointer
		void Free(void* p = nullptr) noexcept { ZE_ASSERT(ptr != nullptr, "Invalid pointer!"); free(ptr); ptr = p; }

		// Reinterpret cast wrapper
		template<typename S>
		constexpr S* Cast() noexcept { return reinterpret_cast<S*>(ptr); }
		// Reinterpret cast const wrapper
		template<typename S>
		constexpr const S* CastConst() const noexcept { return reinterpret_cast<const S*>(ptr); }
		// Reinterpret cast wrapper directly on pointer
		template<typename S>
		constexpr S CastPtr() noexcept { return reinterpret_cast<S>(ptr); }
		// Reinterpret cast const wrapper directly on pointer
		template<typename S>
		constexpr const S CastPtrConst() const noexcept { return reinterpret_cast<const S>(ptr); }

		constexpr operator void*& () noexcept { return ptr; }
		constexpr operator void* () const noexcept { return ptr; }
	};
}