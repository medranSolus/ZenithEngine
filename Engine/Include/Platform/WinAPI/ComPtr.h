#pragma once
ZE_WARNING_PUSH
#include <unknwn.h>
ZE_WARNING_POP
#include <concepts>

namespace ZE::Platform::WinAPI
{
	// Smart pointer managing COM interface counting
	template<std::derived_from<IUnknown> T>
	class ComPtr final
	{
		T* ptr = nullptr;

		constexpr void AddRef() const noexcept { if (ptr) ptr->AddRef(); }
		constexpr U32 ReleaseRef() const noexcept { if (ptr) return ptr->Release(); return 0; }

	public:
		constexpr ComPtr() = default;
		constexpr ComPtr(std::nullptr_t) noexcept : ptr(nullptr) {}
		~ComPtr() { ReleaseRef(); }

		// Construct from raw pointer
		explicit constexpr ComPtr(T* p) noexcept : ptr(p) { AddRef(); }
		template<std::derived_from<IUnknown> U> requires std::convertible_to<U*, T*>
		explicit constexpr ComPtr(U* p) noexcept : ptr(p) { AddRef(); }

		// Copy
		constexpr ComPtr(const ComPtr& other) noexcept : ptr(other.Get()) { AddRef(); }
		template<std::derived_from<IUnknown> U> requires std::convertible_to<U*, T*>
		constexpr ComPtr(const ComPtr<U>& other) noexcept : ptr(other.Get()) { AddRef(); }

		// Move
		constexpr ComPtr(ComPtr&& other) noexcept : ptr(other.Detach()) {}
		template<std::derived_from<IUnknown> U> requires std::convertible_to<U*, T*>
		constexpr ComPtr(ComPtr<U>&& other) noexcept : ptr(other.Detach()) {}

		// Assignment from raw pointer
		constexpr ComPtr& operator=(std::nullptr_t) noexcept { ReleaseRef(); ptr = nullptr; return *this; }
		constexpr ComPtr& operator=(T* p) noexcept { if (ptr != p) { ReleaseRef(); ptr = p; AddRef(); } return *this; }
		template<std::derived_from<IUnknown> U> requires std::convertible_to<U*, T*>
		constexpr ComPtr& operator=(U* p) noexcept { if (ptr != p) { ReleaseRef(); ptr = p; AddRef(); } return *this; }

		// Copy assignment
		constexpr ComPtr& operator=(const ComPtr& other) noexcept { if (ptr != other.Get()) { ReleaseRef(); ptr = other.Get(); AddRef(); } return *this; }
		template<std::derived_from<IUnknown> U> requires std::convertible_to<U*, T*>
		constexpr ComPtr& operator=(const ComPtr<U>& other) noexcept { if (ptr != other.Get()) { ReleaseRef(); ptr = other.Get(); AddRef(); } return *this; }

		// Move assignment
		constexpr ComPtr& operator=(ComPtr&& other) noexcept { std::swap(ptr, other.ptr); return *this; }
		template<std::derived_from<IUnknown> U> requires std::convertible_to<U*, T*>
		constexpr ComPtr& operator=(ComPtr<U>&& other) noexcept { std::swap(ptr, other.ptr); return *this; }

		constexpr operator bool() const noexcept { return ptr != nullptr; }
		constexpr T* operator->() const noexcept { return ptr; }
		// For receiving out parameters - release current pointer first
		constexpr T** operator&() noexcept { ReleaseRef(); return &ptr; }

		constexpr bool operator==(const void* other) const noexcept { return ptr == other; }
		constexpr bool operator!=(const void* other) const noexcept { return ptr != other; }

		constexpr T* Get() const noexcept { return ptr; }
		constexpr T** GetAddressOf() noexcept { return &ptr; }
		constexpr T* const* GetAddressOf() const noexcept { return &ptr; }

		constexpr void Attach(T* other) noexcept { ZE_ASSERT(other != ptr, "Cannot attach same object!"); ReleaseRef(); ptr = other; }
		constexpr T* Detach() noexcept { T* p = ptr; ptr = nullptr; return p; }
		constexpr U32 Reset() noexcept { U32 refCount = ReleaseRef(); ptr = nullptr; return refCount; }

		template<std::derived_from<IUnknown> U>
		constexpr HRESULT As(U** other) const noexcept { return ptr ? ptr->QueryInterface(IID_PPV_ARGS(other)) : E_POINTER; }
	};
}