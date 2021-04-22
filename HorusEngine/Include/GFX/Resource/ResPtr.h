#pragma once
#include "IBindable.h"

namespace GFX::Resource
{
	template<typename R>
	class ResPtr final
	{
		static_assert(std::is_base_of_v<IBindable, R>, "ResPtr target type must be a IBindable type!");
		static constexpr std::align_val_t ALIGNMENT = static_cast<std::align_val_t>(16);

		U64* count = nullptr;
		R* ptr = nullptr;

	public:
		ResPtr() = default;
		// This should be private, but no idea how to perform cast and access this ctor... BIG TODO: Find a way
		constexpr ResPtr(U64* count, R* ptr) noexcept : count(count), ptr(ptr) { ++(*count); }
		constexpr ResPtr(ResPtr&& rp) noexcept { *this = std::forward<ResPtr>(rp); }
		constexpr ResPtr(const ResPtr& rp) noexcept { *this = rp; }
		template<typename ...Params>
		ResPtr(Graphics& gfx, Params&& ...p);
		~ResPtr();

		// TODO: Change all static casts into implicit ones
		template<typename T>
		constexpr ResPtr<T> CastStatic() const noexcept { return { count, static_cast<T*>(ptr) }; }
		template<typename T>
		constexpr operator ResPtr<T>() const noexcept { return CastStatic<T>(); }
		template<typename T>
		constexpr ResPtr<T> CastDynamic() const noexcept { return { count, dynamic_cast<T*>(ptr) }; }

		constexpr R& operator*() { return *ptr; }
		constexpr const R& operator*() const { return *ptr; }
		constexpr R* operator->() { return ptr; }
		constexpr const R* operator->() const { return ptr; }

		constexpr bool operator==(const void* p) const noexcept { return ptr == p; }
		constexpr bool operator!=(const void* p) const noexcept { return ptr != p; }
		constexpr bool operator==(std::nullptr_t p) const noexcept { return ptr == p; }
		constexpr bool operator!=(std::nullptr_t p) const noexcept { return ptr != p; }

		constexpr ResPtr& operator=(ResPtr&& rp) noexcept;
		constexpr ResPtr& operator=(const ResPtr& rp) noexcept;
	};
}