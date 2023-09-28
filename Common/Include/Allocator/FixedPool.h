#pragma once
#include "BasicTypes.h"
#include <type_traits>
#include <vector>

namespace ZE::Allocator
{
	// Allocator for objects of type T using a fixed-size array.
	// Number of elements that can be allocated is bounded
	template<typename T>
	class FixedPool final
	{
		struct Item
		{
			union
			{
				// UINT64_MAX means end of list
				U64 NextFreeIndex;
				alignas(T) U8 Data[sizeof(T)];
			};
			bool IsFree;
		};

		U64 maxCount = 0;
		U64 firstFreeIndex = 0;
		Ptr<Item> items;

		constexpr void RestoreFullFreeList() noexcept;
		constexpr void DeleteAllElements(bool forceFastClear) noexcept;

	public:
		FixedPool() = default;
		ZE_CLASS_DEFAULT(FixedPool);
		~FixedPool();

		constexpr U64 GetMaxElements() const noexcept { return maxCount; }

		constexpr void Init(U64 maxItems) noexcept;

		constexpr T* Get(U64 index) noexcept;

		// Returns `nullptr` when no more objects in pool are available
		template<typename... Types>
		constexpr T* Alloc(Types&&... args) noexcept;
		constexpr void Free(T* ptr) noexcept;
		// Fast clear will be used with POD that don't need destructor invocation but you can force this behavior
		constexpr void Clear(bool forceFastClear = false) noexcept;
	};

#pragma region Functions
	template<typename T>
	constexpr void FixedPool<T>::RestoreFullFreeList() noexcept
	{
		firstFreeIndex = 0;
		for (U64 i = 1; i < maxCount; ++i)
		{
			items[i - 1].NextFreeIndex = i;
			items[i - 1].IsFree = true;
		}
		items[maxCount - 1].NextFreeIndex = UINT64_MAX;
		items[maxCount - 1].IsFree = true;
	}

	template<typename T>
	constexpr void FixedPool<T>::DeleteAllElements(bool forceFastClear) noexcept
	{
		if (!std::is_trivial_v<T> || forceFastClear)
		{
			// No need to gather free elements
			if (firstFreeIndex == UINT64_MAX)
			{
				for (U64 i = 0; i < maxCount; ++i)
					reinterpret_cast<T*>(items[i].Data)->~T();
			}
			else
			{
				// Traverse free list to know which element to delete
				std::vector<bool> isFree(maxCount, false);
				do
				{
					isFree.at(firstFreeIndex) = true;
					firstFreeIndex = items[firstFreeIndex].NextFreeIndex;
				} while (firstFreeIndex != UINT64_MAX);

				// Delete remaining elements
				for (U64 i = 0; i < maxCount; ++i)
					if (!isFree.at(i))
						reinterpret_cast<T*>(items[i].Data)->~T();
			}
		}
	}

	template<typename T>
	FixedPool<T>::~FixedPool()
	{
		if (maxCount)
		{
			DeleteAllElements(false);
			items.DeleteArray();
		}
	}

	template<typename T>
	constexpr void FixedPool<T>::Init(U64 maxItems) noexcept
	{
		maxCount = maxItems;
		items = new Item[maxCount];
		RestoreFullFreeList();
	}

	template<typename T>
	constexpr T* FixedPool<T>::Get(U64 index) noexcept
	{
		ZE_ASSERT(index < maxCount, "Accessing pool out of range!");
		if (index > maxCount)
			return nullptr;

		ZE_ASSERT(!items[index].IsFree, "Accessing empty element!");
		if (items[index].IsFree)
			return nullptr;
		return reinterpret_cast<T*>(items[index].Data);
	}

	template<typename T> template<typename... Types>
	constexpr T* FixedPool<T>::Alloc(Types&&... args) noexcept
	{
		if (firstFreeIndex == UINT64_MAX)
			return nullptr;

		T* obj = reinterpret_cast<T*>(items[firstFreeIndex].Data);
		firstFreeIndex = items[firstFreeIndex].NextFreeIndex;
		items[firstFreeIndex].IsFree = false;

		new(obj) T(std::forward<Types>(args)...);
		return obj;
	}

	template<typename T>
	constexpr void FixedPool<T>::Free(T* ptr) noexcept
	{
		ZE_ASSERT(ptr, "Invalid pointer!");
		ZE_ASSERT(ptr >= items.data() && ptr < items + maxCount,
			"Pointer doesn't belong to this memory pool!");

		Item* item = reinterpret_cast<Item*>(ptr);
		ptr->~T();

		item->NextFreeIndex = firstFreeIndex;
		item->IsFree = true;
		firstFreeIndex = static_cast<U64>(item - items);
	}

	template<typename T>
	constexpr void FixedPool<T>::Clear(bool forceFastClear) noexcept
	{
		DeleteAllElements(forceFastClear);
		RestoreFullFreeList();
	}
#pragma endregion
}