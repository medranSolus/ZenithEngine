#pragma once
#include "Entity.h"

namespace ZE::Data
{
	// Generic library for bookkeeping unique elements
	template<typename Key, typename Type>
	class Library final
	{
		entt::dense_map<Key, Type> data;

	public:
		Library() = default;
		ZE_CLASS_MOVE(Library);
		~Library() = default;

		void Clear() noexcept { data.clear(); }
		bool Contains(const Key& name) const noexcept { return data.contains(name); }
		const Type& Get(const Key& name) const noexcept { ZE_ASSERT(Contains(name), "Element not present!"); return data.at(name); }
		void Add(const Key& name, Type&& element) noexcept { ZE_ASSERT(!Contains(name), "Element already present!"); data.emplace(name, std::forward<Type>(element)); }
		void Add(const Key& name, const Type& element) noexcept { ZE_ASSERT(!Contains(name), "Element already present!"); data.emplace(name, element); }
	};
}