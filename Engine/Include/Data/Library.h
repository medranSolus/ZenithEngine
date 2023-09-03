#pragma once
#include "Entity.h"

namespace ZE::Data
{
	// Generic library for bookkeeping unique elements
	template<typename Key, typename T>
	class Library final
	{
		entt::dense_map<Key, T> data;

	public:
		Library() = default;
		ZE_CLASS_MOVE(Library);
		~Library() = default;

		void Transform(std::function<void(T&)> func) noexcept { for (T& item : data) func(item); }
		void Clear() noexcept { data.clear(); }
		bool Contains(const Key& name) const noexcept { return data.contains(name); }
		const T& Get(const Key& name) const noexcept { ZE_ASSERT(Contains(name), "Element not present!"); return data.at(name); }
		void Add(const Key& name, T&& element) noexcept { ZE_ASSERT(!Contains(name), "Element already present!"); data.emplace(name, std::forward<T>(element)); }
		void Add(const Key& name, const T& element) noexcept { ZE_ASSERT(!Contains(name), "Element already present!"); data.emplace(name, element); }
	};
}