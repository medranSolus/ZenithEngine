#pragma once
#include "Types.h"
#include <vector>

namespace ZE
{
	// Allocator for objects of type T using a list of arrays to speed up allocation.
	// Number of elements that can be allocated is not bounded because allocator can create multiple blocks.
	template<typename T>
	class PoolAllocator
	{
		union Item
		{
			// UINT64_MAX means end of list
			U64 NextFreeIndex;
			alignas(T) U8 Data[sizeof(T)];
		};
		struct ItemBlock
		{
			Item* Items;
			U64 Capacity;
			U64 FirstFreeIndex;
		};

		const U64 firstBlockCapacity;
		bool freeBlock = false;
		std::vector<ItemBlock> itemBlocks;

		ItemBlock& CreateNewBlock() noexcept;

	public:
		PoolAllocator(U64 firstBlockCapacity) noexcept : firstBlockCapacity(firstBlockCapacity) {}
		ZE_CLASS_DEFAULT(PoolAllocator);
		~PoolAllocator() { Clear(); }

		template<typename... Types>
		T* Alloc(Types... args) noexcept;
		void Free(T* ptr) noexcept;
		void Clear() noexcept;
	};

#pragma region Functions
	template<typename T>
	typename PoolAllocator<T>::ItemBlock& PoolAllocator<T>::CreateNewBlock() noexcept
	{
		U64 newBlockCapacity = itemBlocks.size() ? itemBlocks.back().Capacity * 3 / 2 : firstBlockCapacity;
		ItemBlock& newBlock = itemBlocks.emplace_back(new Item[newBlockCapacity], newBlockCapacity, 0);
		--newBlockCapacity;

		// Setup singly-linked list of all free items in this block.
		for (U64 i = 0; i < newBlockCapacity; ++i)
			newBlock.Items[i].NextFreeIndex = i + 1;

		newBlock.Items[newBlockCapacity].NextFreeIndex = UINT64_MAX;
		freeBlock = false;
		return newBlock;
	}

	template<typename T> template<typename... Types>
	T* PoolAllocator<T>::Alloc(Types... args) noexcept
	{
		for (U64 i = itemBlocks.size(); i;)
		{
			ItemBlock& block = itemBlocks.at(--i);

			// This block has some free items, use first one
			if (block.FirstFreeIndex != UINT64_MAX)
			{
				if (block.FirstFreeIndex == 0)
					freeBlock = false;

				Item* item = &block.Items[block.FirstFreeIndex];
				block.FirstFreeIndex = item->NextFreeIndex;

				T* result = reinterpret_cast<T*>(&item->Data);
				new(result) T(std::forward<Types>(args)...);
				return result;
			}
		}

		// No block has free item, create new one
		ItemBlock& newBlock = CreateNewBlock();
		Item* item = &newBlock.Items[0];
		newBlock.FirstFreeIndex = item->NextFreeIndex;

		T* result = reinterpret_cast<T*>(&item->Data);
		new(result) T(std::forward<Types>(args)...);
		return result;
	}

	template<typename T>
	void PoolAllocator<T>::Free(T* ptr) noexcept
	{
		// Search all memory blocks to find ptr
		for (U64 i = itemBlocks.size(); i;)
		{
			ItemBlock& block = itemBlocks.at(--i);
			Item* item = reinterpret_cast<Item*>(ptr);

			// Check if item is in address range of this block
			if (item >= block.Items && item < block.Items + block.Capacity)
			{
				ptr->~T();

				item->NextFreeIndex = block.FirstFreeIndex;
				block.FirstFreeIndex = static_cast<U64>(item - block.Items);

				if (block.FirstFreeIndex == 0)
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

	template<typename T>
	void PoolAllocator<T>::Clear() noexcept
	{
		for (auto& block : itemBlocks)
			delete[] block.Items;
		itemBlocks.clear();
	}
#pragma endregion
}