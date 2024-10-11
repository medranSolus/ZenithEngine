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

		U64 Size() const noexcept { return data.size(); }
		void Iter(std::function<void(const T&)> func) const noexcept { for (const auto& item : data) func(item.second); }
		void Transform(std::function<void(T&)> func) noexcept { for (auto item : data) func(item.second); }
		void Transform(std::function<void(const Key&, T&)> func) noexcept { for (auto item : data) func(item.first, item.second); }
		void TransformCheck(std::function<bool(const Key&, T&)> func) noexcept { for (auto item : data) if (func(item.first, item.second)) break; }
		void Clear() noexcept { data.clear(); }
		bool Contains(const Key& name) const noexcept { return data.contains(name); }
		const T& Get(const Key& name) const noexcept { ZE_ASSERT(Contains(name), "Element not present!"); return data.at(name); }
		T& Get(const Key& name) noexcept { ZE_ASSERT(Contains(name), "Element not present!"); return data.at(name); }
		void Remove(const Key& name) noexcept { ZE_ASSERT(Contains(name), "Element not present!"); data.erase(name); }

		template<typename ...P>
		void Add(const Key& name, P&&... params) noexcept { ZE_ASSERT(!Contains(name), "Element already present!"); data.emplace(name, T{ std::forward<P>(params)... }); }
		void Add(const Key& name, T&& element) noexcept { ZE_ASSERT(!Contains(name), "Element already present!"); data.emplace(name, std::forward<T>(element)); }
		void Add(const Key& name, const T& element) noexcept { ZE_ASSERT(!Contains(name), "Element already present!"); data.emplace(name, element); }
	};
}