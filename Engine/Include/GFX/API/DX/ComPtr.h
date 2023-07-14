#pragma once
ZE_WARNING_PUSH
#include <d3dcommon.h>
#include <wrl.h>
ZE_WARNING_POP
#include <concepts>

namespace ZE::GFX::API::DX
{
	// Smart pointer managing COM interface counting
	template<std::derived_from<IUnknown> T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;
	//class ComPtr final
	//{
	//	T* ptr = nullptr;

	//	constexpr void AddRef() const noexcept { if (ptr) ptr->AddRef(); }
	//	constexpr void DeleteRef() const noexcept { if (ptr) ptr->Release(); }

	//public:
	//	ComPtr() = default;
	//	ComPtr(std::nullptr_t) noexcept {}
	//	constexpr ~ComPtr() { DeleteRef(); }

	//	constexpr ComPtr(T* other) noexcept : ptr(other) { AddRef(); }
	//	template<std::convertible_to<T> U>
	//	constexpr ComPtr(U* other) noexcept : ptr(other) { AddRef(); }

	//	constexpr ComPtr(const ComPtr& other) noexcept : ptr(other.ptr) { AddRef(); }
	//	template<std::convertible_to<T> U>
	//	constexpr ComPtr(const ComPtr<U>& other) noexcept : ptr(other.ptr) { AddRef(); }

	//	constexpr ComPtr(ComPtr&& other) noexcept : ptr(std::exchange(other.ptr, nullptr)) {}
	//	template<std::convertible_to<T> U>
	//	constexpr ComPtr(ComPtr<U>&& other) noexcept : ptr(std::exchange(other.ptr, nullptr)) {}

	//	constexpr ComPtr& operator=(T* other) noexcept { ptr = other; AddRef(); return *this; }
	//	template<std::convertible_to<T> U>
	//	constexpr ComPtr& operator=(U* other) noexcept { ptr = other; AddRef(); return *this; }

	//	constexpr ComPtr& operator=(const ComPtr& other) noexcept { ptr = other.ptr; AddRef(); return *this; }
	//	template<std::convertible_to<T> U>
	//	constexpr ComPtr& operator=(const ComPtr<U>& other) noexcept { ptr = other.ptr; AddRef(); return *this; }

	//	constexpr ComPtr& operator=(ComPtr&& other) noexcept { ptr = std::exchange(other.ptr, nullptr); return *this; }
	//	template<std::convertible_to<T> U>
	//	constexpr ComPtr& operator=(ComPtr<U>&& other) noexcept { ptr = std::exchange(other.ptr, nullptr); return *this; }

	//	constexpr T* operator->() const noexcept { return ptr; }
	//	constexpr T** operator&() noexcept { DeleteRef(); return &ptr; }

	//	constexpr bool operator==(void* other) const noexcept { return ptr == other; }
	//	constexpr bool operator!=(void* other) const noexcept { return ptr != other; }

	//	constexpr T* Get() const noexcept { return ptr; }
	//	constexpr T** GetAddressOf() noexcept { return &ptr; }
	//	constexpr T* const* GetAddressOf() const noexcept { return &ptr; }

	//	constexpr void Attach(T* other) noexcept { ZE_ASSERT(other != ptr, "Cannot attach same object!"); DeleteRef(); ptr = other; }
	//	constexpr T* Detach() noexcept { T* p = ptr; ptr = nullptr; return p; }
	//	constexpr void Reset() noexcept { DeleteRef(); ptr = nullptr; }

	//	template<std::derived_from<IUnknown> U>
	//	constexpr HRESULT As(U** other) const noexcept { return ptr->QueryInterface(IID_PPV_ARGS(other)); }
	//};
}