#pragma once
#include "Entity.h"

namespace ZE::Data
{
	// Generic library for bookkeeping unique elements
	template<typename KEY, typename T>
	class Library final
	{
		entt::dense_map<KEY, T> data;

	public:
		Library() = default;
		ZE_CLASS_MOVE(Library);
		~Library() = default;

		void Transform(std::function<void(T&)> func) noexcept { for (T& item : data) func(item); }
		void Clear() noexcept { data.clear(); }
		bool Contains(const KEY& name) const noexcept { return data.contains(name); }
		const T& Get(const KEY& name) const noexcept { ZE_ASSERT(Contains(name), "Element not present!"); return data.at(name); }
		void Add(const KEY& name, T&& element) noexcept { ZE_ASSERT(!Contains(name), "Element already present!"); data.emplace(name, std::forward<T>(element)); }
		void Add(const KEY& name, const T& element) noexcept { ZE_ASSERT(!Contains(name), "Element already present!"); data.emplace(name, element); }
	};
}