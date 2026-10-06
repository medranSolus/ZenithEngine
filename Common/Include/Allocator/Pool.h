#pragma once
#include "BasicTypes.h"
#include <vector>

namespace ZE::Allocator
{
	// Allocator for objects of type T using a list of arrays to speed up allocation.
	// Number of elements that can be allocated is not bounded because allocator can create multiple blocks
	template<typename T, bool USE_MUTEX = false>
	class Pool final
	{
		using LockType = std::conditional_t<USE_MUTEX, std::shared_mutex, std::monostate>;

		union Item
		{
			// UINT64_MAX means end of list
			U64 NextFreeIndex;
			alignas(T) U8 Data[sizeof(T)];
		};
		struct ItemBlock
		{
			std::unique_ptr<Item[]> Items;
			U64 Capacity = 0;
			U64 FirstFreeIndex = 0;
			U64 Allocated = 0;
		};

		U64 firstBlockCapacity;
		bool freeBlock = false;
		std::vector<ItemBlock> itemBlocks;
		LockType mutex;

		ItemBlock& CreateNewBlock() noexcept;
		void MoveFrom(Pool&& pool) noexcept;

	public:
		constexpr Pool(U64 firstBlockCapacity) noexcept : firstBlockCapacity(firstBlockCapacity) {}
		ZE_CLASS_NO_COPY(Pool);
		Pool(Pool&& pool) noexcept { MoveFrom(std::move(pool)); }
		Pool& operator=(Pool&& pool) noexcept { MoveFrom(std::move(pool)); return *this; }
		~Pool() { Clear(); }

		template<typename... Types>
		T* Alloc(Types&&... args) noexcept;
		void Free(T* ptr) noexcept;
		// Use fast clears with POD that don't need invocation of destructor
		void Clear(bool fastClear = false) noexcept;
	};

#pragma region Functions
	template<typename T, bool USE_MUTEX>
	typename Pool<T, USE_MUTEX>::ItemBlock& Pool<T, USE_MUTEX>::CreateNewBlock() noexcept
	{
		U64 newBlockCapacity = itemBlocks.size() ? itemBlocks.back().Capacity * 3 / 2 : firstBlockCapacity;
		ItemBlock& newBlock = itemBlocks.emplace_back(std::make_unique<Item[]>(newBlockCapacity), newBlockCapacity, 0, 0);
		--newBlockCapacity;

		// Setup singly-linked list of all free items in this block
		for (U64 i = 0; i < newBlockCapacity; ++i)
			newBlock.Items[i].NextFreeIndex = i + 1;

		newBlock.Items[newBlockCapacity].NextFreeIndex = UINT64_MAX;
		freeBlock = false;
		return newBlock;
	}

	template<typename T, bool USE_MUTEX>
	void Pool<T, USE_MUTEX>::MoveFrom(Pool&& pool) noexcept
	{
		firstBlockCapacity = pool.firstBlockCapacity;
		freeBlock = pool.freeBlock;
		itemBlocks = std::move(pool.itemBlocks);
	}

	template<typename T, bool USE_MUTEX> template<typename... Types>
	T* Pool<T, USE_MUTEX>::Alloc(Types&&... args) noexcept
	{
		Item* item = nullptr;
		{
			LockGuardRW lock(USE_MUTEX ? reinterpret_cast<std::shared_mutex*>(&mutex) : nullptr, USE_MUTEX);
			for (U64 i = itemBlocks.size(); i;)
			{
				ItemBlock& block = itemBlocks.at(--i);

				// This block has some free items, use first one
				if (block.FirstFreeIndex != UINT64_MAX)
				{
					ZE_ASSERT(block.FirstFreeIndex < block.Capacity, "Incorrect index!");
					ZE_ASSERT(block.Allocated < block.Capacity, "Block is already full!");
					if (block.Allocated++ == 0)
						freeBlock = false;

					item = &block.Items[block.FirstFreeIndex];
					block.FirstFreeIndex = item->NextFreeIndex;
				}
			}

			if (item == nullptr)
			{
				// No block has free item, create new one
				ItemBlock& newBlock = CreateNewBlock();
				item = &newBlock.Items[0];
				newBlock.FirstFreeIndex = item->NextFreeIndex;
				++newBlock.Allocated;
			}
		}

		T* result = reinterpret_cast<T*>(&item->Data);
		new(result) T(std::forward<Types>(args)...);
		return result;
	}

	template<typename T, bool USE_MUTEX>
	void Pool<T, USE_MUTEX>::Free(T* ptr) noexcept
	{
		ZE_ASSERT(ptr, "Invalid pointer!");
		ptr->~T();

		LockGuardRW lock(USE_MUTEX ? reinterpret_cast<std::shared_mutex*>(&mutex) : nullptr, USE_MUTEX);

		Item* item = reinterpret_cast<Item*>(ptr);
		// Search all memory blocks to find ptr
		for (U64 i = itemBlocks.size(); i;)
		{
			ItemBlock& block = itemBlocks.at(--i);

			// Check if item is in address range of this block
			if (item >= block.Items.get() && item < block.Items.get() + block.Capacity)
			{
				item->NextFreeIndex = block.FirstFreeIndex;
				block.FirstFreeIndex = static_cast<U64>(item - block.Items.get());

				ZE_ASSERT(block.Allocated > 0, "Trying to dealocate on empty list!");
				if (--block.Allocated == 0)
				{
					if (freeBlock)
						itemBlocks.erase(itemBlocks.begin() + i, itemBlocks.begin() + i + 1);
					else
						freeBlock = true;
				}
				return;
			}
		}
		ZE_FAIL("Pointer doesn't belong to this memory pool!");
	}

	template<typename T, bool USE_MUTEX>
	void Pool<T, USE_MUTEX>::Clear(bool fastClear) noexcept
	{
		LockGuardRW lock(USE_MUTEX ? reinterpret_cast<std::shared_mutex*>(&mutex) : nullptr, USE_MUTEX);

		if (!fastClear)
		{
			for (auto& block : itemBlocks)
			{
				// No need to gather free elements
				if (block.FirstFreeIndex == UINT64_MAX)
				{
					for (U64 i = 0; i < block.Capacity; ++i)
						reinterpret_cast<T*>(block.Items[i].Data)->~T();
				}
				else
				{
					// Traverse free list to know which element to delete
					std::vector<bool> isInUse(block.Capacity, true);
					do
					{
						ZE_ASSERT(block.FirstFreeIndex < block.Capacity, "Incorrect index!");

						isInUse.at(block.FirstFreeIndex) = false;
						block.FirstFreeIndex = block.Items[block.FirstFreeIndex].NextFreeIndex;
					} while (block.FirstFreeIndex != UINT64_MAX);

					// Delete remaining elements
					for (U64 i = 0; i < block.Capacity; ++i)
						if (isInUse.at(i))
							reinterpret_cast<T*>(block.Items[i].Data)->~T();
				}
			}
		}
		itemBlocks.clear();
	}
#pragma endregion
}