#pragma once
#include "Pool.h"
#include "Intrinsics.h"
#include <bitset>

// Helper template header for ChunkedTLSF methods (Warning! Causes problems with auto formatters)
#define ZE_CHUNKED_TLSF_TEMPLATE template<typename Memory, U8 SECOND_LEVEL_INDEX, U8 MEMORY_CLASS_SHIFT>
// Helper template type for ChunkedTLSF methods and inner types (Warning! Causes problems with auto formatters)
#define ZE_CHUNKED_TLSF_TYPE ChunkedTLSF<Memory, SECOND_LEVEL_INDEX, MEMORY_CLASS_SHIFT>

namespace ZE::Allocator
{
	// Flags to be passed for memory chunk created with ChunkedTLSF algorithm
	typedef U8 TLSFMemoryChunkFlags;

	// Wrapper for memory allocated by ChunkedTLSF algorithm
	// with definitions of required static methods handling creation/destruction of said memory
	template<typename Memory>
	struct TLSFMemoryChunk
	{
		Memory Memory;

		// Required static method for creation of memory: void Init(Memory&, TLSFMemoryChunkFlags, U64, void*)
		static void InitMemory(TLSFMemoryChunk* chunk, TLSFMemoryChunkFlags flags, U64 size, void* userData) { typename Memory::Init(chunk->Memory, flags, size, userData); }
		// Required static method for freeing memory: void Destroy(Memory&, void*)
		static void DestroyMemory(TLSFMemoryChunk* chunk, void* userData) noexcept { typename Memory::Destroy(chunk->Memory, userData); }
	};

	// TLSF algorithm with distinction between regions of memory (chunk) that aren't continuous in adress space.
	// According to original paper, SECOND_LEVEL_INDEX should be preferable 4 or 5.
	// To avoid over-division of adress space, MEMORY_CLASS_SHIFT controls the max size that segregates memory region to first "memory class" (bucket for memory blocks)
	template<typename Memory, U8 SECOND_LEVEL_INDEX = 5, U8 MEMORY_CLASS_SHIFT = 7>
	class ChunkedTLSF final
	{
		// Algorithm source: M. Masmano, I. Ripoll, A. Crespo, and J. Real "TLSF: a New Dynamic Memory Allocator for Real-Time Systems"
		// http://www.gii.upv.es/tlsf/files/ecrts04_tlsf.pdf
		struct Block
		{
			U64 Offset = 0;
			U64 Size = 0;
			TLSFMemoryChunk<Memory>* ChunkHandle = nullptr;
			Block* PrevPhysical = nullptr;
			Block* NextPhysical = nullptr;
			// Address of the same block here indicates that block is taken
			Block* PrevFree = nullptr;
			Block* NextFree = nullptr;

			constexpr void MarkFree() noexcept { PrevFree = nullptr; }
			constexpr void MarkTaken() noexcept { PrevFree = this; }
			constexpr bool IsFree() const noexcept { return PrevFree != this; }
		};

	public:
		typedef Pool<Block> BlockAllocator;
		typedef Pool<TLSFMemoryChunk<Memory>> ChunkAllocator;

	private:
		static constexpr U16 SMALL_BUFFER_SIZE = 1 << (MEMORY_CLASS_SHIFT + 1);
		static constexpr U8 MAX_MEMORY_CLASSES = 65 - MEMORY_CLASS_SHIFT;

		BlockAllocator& blockAllocator;
		ChunkAllocator& chunkAllocator;

		U64 chunkSize = 0;
		U32 chunkSizeDivisor = 0;
		TLSFMemoryChunkFlags chunkFlags = 0;

		std::bitset<1> flags = 0;
		// Add possibility to change number of buckets in first segregated list
		U8 firstListSize = 0;
		U8 memoryClasses = 0;
		U64 freeMemory = 0;
		U32 freeBlocks = 0;
		U32 isFreeBitmap = 0;
		Ptr<U32> innerIsFreeBitmap;
		U32 listsCount = 0;
		// 0: 0-(firstListSize) lists for small blocks
		// 1+: 0-(2^SLI-1) lists for normal blocks
		Ptr<Block*> freeList;
		Ptr<Block> nullBlock;

		static U8 SizeToMemoryClass(U64 size) noexcept { return size > SMALL_BUFFER_SIZE ? Intrin::BitScanMSB(size) - MEMORY_CLASS_SHIFT : 0; }
		static constexpr bool CheckBlock(Block& block, U64 allocSize, U64 alignment) noexcept;

		// Leave one chunk at most free to avoid constantly creating and destroying their memory
		constexpr bool IsFreeChunk() const noexcept { return flags[0]; }
		constexpr void SetFreeChunk(bool val) noexcept { flags[0] = val; }
		U32 GetListIndex(U64 size) const noexcept { const U8 memoryClass = SizeToMemoryClass(size); return GetListIndex(memoryClass, SizeToSecondIndex(size, memoryClass)); }

		constexpr U32 GetListIndex(U8 memoryClass, U16 secondIndex) const noexcept;
		constexpr U16 SizeToSecondIndex(U64 size, U8 memoryClass) const noexcept;

		Block* AllocWithChunk(U64 size, void* memoryUserData);
		Block* CreateAlloc(Block* currentBlock, U64 size, U64 alignment) noexcept;
		Block* FindFreeBlock(U64 size, U32& listIndex) const noexcept;
		void RemoveFreeBlock(Block* block) noexcept;
		void InsertFreeBlock(Block* block) noexcept;
		void MergeBlock(Block* block, Block* prev) noexcept;

	public:
		constexpr ChunkedTLSF(BlockAllocator& blockAllocator, ChunkAllocator& chunkAllocator) noexcept
			: blockAllocator(blockAllocator), chunkAllocator(chunkAllocator) {}
		ZE_CLASS_MOVE(ChunkedTLSF);
		constexpr ~ChunkedTLSF();

		constexpr U64 GetOffset(AllocHandle alloc) const noexcept { ZE_ASSERT(alloc, "Invalid allocation!"); return alloc.Cast<Block>()->Offset * chunkSizeDivisor; }
		constexpr U64 GetSize(AllocHandle alloc) const noexcept { ZE_ASSERT(alloc, "Invalid allocation!"); return alloc.Cast<Block>()->Size * chunkSizeDivisor; }
		constexpr Memory GetMemory(AllocHandle alloc) const noexcept { ZE_ASSERT(alloc, "Invalid allocation!"); return alloc.Cast<Block>()->ChunkHandle->Memory; }

		constexpr U64 GetChunkSize() const noexcept { return chunkSize * GetChunkSizeGranularity(); }
		constexpr U32 GetChunkSizeGranularity() const noexcept { return chunkSizeDivisor; }
		constexpr TLSFMemoryChunkFlags GetChunkCreationFlags() const noexcept { return chunkFlags; }
		constexpr U64 GetSumFreeMemory() const noexcept { return freeMemory + nullBlock->Size; }

		// When all allocations must be multiple of some size, use chunkSizeGranularity to speed up whole algorithm.
		// When first segregated lists holding smallest region can be divided more efficiently, use firstListSizePower to compute size of first memory class (2^firstListSizePower)
		void Init(TLSFMemoryChunkFlags memoryFlags, U64 initialChunkSize, U32 chunkSizeGranularity = 1, U8 firstListSizePower = 0);
		// To pass custom data used in creation of memory chunk, pass it as memoryUserData
		AllocHandle Alloc(U64 allocSize, U64 alignment, void* memoryUserData);
		// To pass custom data used in destruction of memory chunk, pass it as memoryUserData
		void Free(AllocHandle allocation, void* memoryUserData) noexcept;
		// Delete allocated chunks not used by any allocation (only by null block), required to call before destruction of allocator
		void DestroyFreeChunks(void* memoryUserData) noexcept;
	};

#pragma region Functions
	ZE_CHUNKED_TLSF_TEMPLATE
	constexpr bool ZE_CHUNKED_TLSF_TYPE::CheckBlock(Block& block, U64 allocSize, U64 alignment) noexcept
	{
		ZE_ASSERT(block.IsFree(), "Block is already taken!");

		const U64 alignedOffset = Math::AlignUp(block.Offset, alignment);
		return block.Size >= allocSize + alignedOffset - block.Offset;
	}

	ZE_CHUNKED_TLSF_TEMPLATE
	constexpr U32 ZE_CHUNKED_TLSF_TYPE::GetListIndex(U8 memoryClass, U16 secondIndex) const noexcept
	{
		if (memoryClass == 0)
			return secondIndex;

		return static_cast<U32>(memoryClass - 1) * (1 << SECOND_LEVEL_INDEX) + secondIndex + firstListSize;
	}

	ZE_CHUNKED_TLSF_TEMPLATE
	constexpr U16 ZE_CHUNKED_TLSF_TYPE::SizeToSecondIndex(U64 size, U8 memoryClass) const noexcept
	{
		if (memoryClass == 0)
			return static_cast<U16>(((size - 1) * firstListSize) / SMALL_BUFFER_SIZE);

		return static_cast<U16>((size >> (memoryClass + MEMORY_CLASS_SHIFT - SECOND_LEVEL_INDEX)) ^ (1U << SECOND_LEVEL_INDEX));
	}

	ZE_CHUNKED_TLSF_TEMPLATE
	typename ZE_CHUNKED_TLSF_TYPE::Block* ZE_CHUNKED_TLSF_TYPE::AllocWithChunk(U64 size, void* memoryUserData)
	{
		TLSFMemoryChunk<Memory>* newChunk = chunkAllocator.Alloc();
		TLSFMemoryChunk<Memory>::InitMemory(newChunk, chunkFlags, chunkSize * chunkSizeDivisor, memoryUserData);

		// Create new taken block
		Block* newBlock = blockAllocator.Alloc();
		newBlock->Size = size;
		newBlock->ChunkHandle = newChunk;
		newBlock->MarkTaken();

		if (nullBlock->Size > 0)
		{
			// Create new null block and switch to it
			newBlock->PrevPhysical = nullBlock;
			nullBlock->NextPhysical = newBlock;

			Block* oldNullBlock = nullBlock;
			oldNullBlock->MarkTaken();

			nullBlock = blockAllocator.Alloc();
			nullBlock->MarkFree();
			InsertFreeBlock(oldNullBlock);
		}
		else
		{
			nullBlock->PrevPhysical->NextPhysical = newBlock;
			newBlock->PrevPhysical = nullBlock->PrevPhysical;
		}

		nullBlock->Offset = size;
		nullBlock->Size = chunkSize - size;
		nullBlock->ChunkHandle = newChunk;
		nullBlock->PrevPhysical = newBlock;
		newBlock->NextPhysical = nullBlock;

		SetFreeChunk(false);
		return newBlock;
	}

	ZE_CHUNKED_TLSF_TEMPLATE
	typename ZE_CHUNKED_TLSF_TYPE::Block* ZE_CHUNKED_TLSF_TYPE::CreateAlloc(Block* currentBlock, U64 size, U64 alignment) noexcept
	{
		// Get block and pop it from the free list
		if (currentBlock != nullBlock)
			RemoveFreeBlock(currentBlock);

		// Indicate that there is no more fully free memory block
		if (currentBlock->Size == chunkSize)
			SetFreeChunk(false);

		// Append missing alignment to prev block or create new one
		const U64 misssingAlignment = Math::AlignUp(currentBlock->Offset, alignment) - currentBlock->Offset;
		if (misssingAlignment)
		{
			Block* prevBlock = currentBlock->PrevPhysical;
			ZE_ASSERT(prevBlock != nullptr, "There should be no missing alignment at offset 0!");

			if (prevBlock->IsFree() && prevBlock->ChunkHandle == currentBlock->ChunkHandle)
			{
				U32 oldList = GetListIndex(prevBlock->Size);
				prevBlock->Size += misssingAlignment;
				// Check if new size crosses list bucket
				if (oldList != GetListIndex(prevBlock->Size))
				{
					prevBlock->Size -= misssingAlignment;
					RemoveFreeBlock(prevBlock);
					prevBlock->Size += misssingAlignment;
					InsertFreeBlock(prevBlock);
				}
				else
					freeMemory += misssingAlignment;
			}
			else
			{
				Block* newBlock = blockAllocator.Alloc();
				currentBlock->PrevPhysical = newBlock;
				prevBlock->NextPhysical = newBlock;
				newBlock->PrevPhysical = prevBlock;
				newBlock->NextPhysical = currentBlock;
				newBlock->Size = misssingAlignment;
				newBlock->Offset = currentBlock->Offset;
				newBlock->ChunkHandle = currentBlock->ChunkHandle;
				newBlock->MarkTaken();

				InsertFreeBlock(newBlock);
			}

			currentBlock->Size -= misssingAlignment;
			currentBlock->Offset += misssingAlignment;
		}

		if (currentBlock->Size == size)
		{
			if (currentBlock == nullBlock)
			{
				// Setup new null block
				nullBlock = blockAllocator.Alloc();
				nullBlock->Size = 0;
				nullBlock->Offset = currentBlock->Offset + size;
				nullBlock->ChunkHandle = currentBlock->ChunkHandle;
				nullBlock->PrevPhysical = currentBlock;
				nullBlock->MarkFree();
				currentBlock->NextPhysical = nullBlock;
				currentBlock->MarkTaken();
			}
		}
		else
		{
			ZE_ASSERT(currentBlock->Size > size, "Proper block already found, shouldn't find smaller one!");
			ZE_ASSERT((currentBlock->Size + currentBlock->Offset) <= chunkSize, "Resource too big for given range!");

			// Create new free block
			Block* newBlock = blockAllocator.Alloc();
			newBlock->Size = currentBlock->Size - size;
			newBlock->Offset = currentBlock->Offset + size;
			newBlock->ChunkHandle = currentBlock->ChunkHandle;
			newBlock->PrevPhysical = currentBlock;
			newBlock->NextPhysical = currentBlock->NextPhysical;
			currentBlock->NextPhysical = newBlock;
			currentBlock->Size = size;

			if (currentBlock == nullBlock)
			{
				nullBlock = newBlock;
				nullBlock->MarkFree();
				currentBlock->MarkTaken();
			}
			else
			{
				newBlock->NextPhysical->PrevPhysical = newBlock;
				newBlock->MarkTaken();
				InsertFreeBlock(newBlock);
			}
		}
		return currentBlock;
	}

	ZE_CHUNKED_TLSF_TEMPLATE
	typename ZE_CHUNKED_TLSF_TYPE::Block* ZE_CHUNKED_TLSF_TYPE::FindFreeBlock(U64 size, U32& listIndex) const noexcept
	{
		U8 memoryClass = SizeToMemoryClass(size);
		U32 innerFreeMap = innerIsFreeBitmap[memoryClass] & (~0U << SizeToSecondIndex(size, memoryClass));
		if (!innerFreeMap)
		{
			// Check higher levels for available blocks
			U32 freeMap = isFreeBitmap & (~0UL << (memoryClass + 1));
			if (!freeMap)
				return nullptr; // No more memory available

			// Find lowest free region
			memoryClass = Intrin::BitScanLSB(freeMap);
			innerFreeMap = innerIsFreeBitmap[memoryClass];
			ZE_ASSERT(innerFreeMap != 0, "Empty free range!");
		}

		// Find lowest free subregion
		listIndex = GetListIndex(memoryClass, Intrin::BitScanLSB(innerFreeMap));
		ZE_ASSERT(freeList[listIndex], "Empty head of the list!");
		return freeList[listIndex];
	}

	ZE_CHUNKED_TLSF_TEMPLATE
	void ZE_CHUNKED_TLSF_TYPE::RemoveFreeBlock(Block* block) noexcept
	{
		ZE_ASSERT(block != nullBlock, "Cannot remove null block!");
		ZE_ASSERT(block->IsFree(), "Block is not free!");

		if (block->NextFree != nullptr)
			block->NextFree->PrevFree = block->PrevFree;
		if (block->PrevFree != nullptr)
			block->PrevFree->NextFree = block->NextFree;
		else
		{
			const U8 memClass = SizeToMemoryClass(block->Size);
			const U16 secondIndex = SizeToSecondIndex(block->Size, memClass);
			const U32 index = GetListIndex(memClass, secondIndex);

			ZE_ASSERT(freeList[index] == block, "Head of the free list is not current blok!");
			freeList[index] = block->NextFree;
			if (block->NextFree == nullptr)
			{
				innerIsFreeBitmap[memClass] &= ~(1U << secondIndex);
				if (innerIsFreeBitmap[memClass] == 0)
					isFreeBitmap &= ~(1UL << memClass);
			}
		}
		block->MarkTaken();
		--freeBlocks;
		freeMemory -= block->Size;
	}

	ZE_CHUNKED_TLSF_TEMPLATE
	void ZE_CHUNKED_TLSF_TYPE::InsertFreeBlock(Block* block) noexcept
	{
		ZE_ASSERT(block != nullBlock, "Cannot remove null block!");
		ZE_ASSERT(!block->IsFree(), "Cannot insert block twice!");

		const U8 memClass = SizeToMemoryClass(block->Size);
		const U16 secondIndex = SizeToSecondIndex(block->Size, memClass);
		const U32 index = GetListIndex(memClass, secondIndex);

		ZE_ASSERT(index < listsCount, "Index out of range!");
		block->PrevFree = nullptr;
		block->NextFree = freeList[index];
		freeList[index] = block;
		if (block->NextFree != nullptr)
			block->NextFree->PrevFree = block;
		else
		{
			innerIsFreeBitmap[memClass] |= 1U << secondIndex;
			isFreeBitmap |= 1UL << memClass;
		}
		++freeBlocks;
		freeMemory += block->Size;
	}

	ZE_CHUNKED_TLSF_TEMPLATE
	void ZE_CHUNKED_TLSF_TYPE::MergeBlock(Block* block, Block* prev) noexcept
	{
		ZE_ASSERT(block->PrevPhysical == prev, "Cannot merge seperate physical regions!");
		ZE_ASSERT(!prev->IsFree(), "Cannot merge block that belongs to free list!");

		block->Offset = prev->Offset;
		block->Size += prev->Size;
		block->PrevPhysical = prev->PrevPhysical;
		if (block->PrevPhysical)
			block->PrevPhysical->NextPhysical = block;
		blockAllocator.Free(prev);
	}

	ZE_CHUNKED_TLSF_TEMPLATE
	constexpr ZE_CHUNKED_TLSF_TYPE::~ChunkedTLSF()
	{
		if (chunkSizeDivisor != 0)
		{
			ZE_ASSERT(!nullBlock->ChunkHandle, "Memory used by free chunk not destroyed (forgot to call DestroyFreeChunks()), memory leak!");
			ZE_ASSERT(nullBlock->Offset == 0 && nullBlock->Size == chunkSize, "Not all allocations have been freed before destroying allocator, memory leak!");

			freeList.DeleteArray();
			innerIsFreeBitmap.DeleteArray();
		}
	}

	ZE_CHUNKED_TLSF_TEMPLATE
	void ZE_CHUNKED_TLSF_TYPE::Init(TLSFMemoryChunkFlags memoryFlags, U64 initialChunkSize, U32 chunkSizeGranularity, U8 firstListSizePower)
	{
		ZE_ASSERT(chunkSizeGranularity != 0, "Chunk granularity cannot be 0!");
		ZE_ASSERT(initialChunkSize % chunkSizeGranularity == 0, "Chunk size should be multiple of chunk granularity!");
		chunkSize = initialChunkSize / chunkSizeGranularity;
		chunkSizeDivisor = chunkSizeGranularity;
		chunkFlags = memoryFlags;

		firstListSize = 1U << (firstListSizePower != 0 ? firstListSizePower : SECOND_LEVEL_INDEX);
		ZE_ASSERT(firstListSize <= SMALL_BUFFER_SIZE, "First segregated list should be big enough to create span across all bytes of small buffers!");

		nullBlock = blockAllocator.Alloc();
		nullBlock->Size = chunkSize;
		nullBlock->ChunkHandle = nullptr; // Lazy alloc: create first chunk only when requested (helpful in multiple pool scenario)
		nullBlock->MarkFree();

		const U8 memoryClass = SizeToMemoryClass(nullBlock->Size);
		const U16 sli = SizeToSecondIndex(nullBlock->Size, memoryClass);
		listsCount = (memoryClass != 0) * ((memoryClass - 1) * (1UL << SECOND_LEVEL_INDEX) + sli) + firstListSize + 1UL;

		memoryClasses = memoryClass + 2;
		innerIsFreeBitmap = new U32[memoryClasses];
		memset(innerIsFreeBitmap, 0, memoryClasses * sizeof(U32));

		freeList = new Block * [listsCount];
		memset(freeList, 0, listsCount * sizeof(Block*));
	}

	ZE_CHUNKED_TLSF_TEMPLATE
	AllocHandle ZE_CHUNKED_TLSF_TYPE::Alloc(U64 allocSize, U64 alignment, void* memoryUserData)
	{
		allocSize = Math::DivideRoundUp(allocSize, static_cast<U64>(chunkSizeDivisor));
		ZE_ASSERT(allocSize > 0, "Cannot allocate empty block!");
		ZE_ASSERT(allocSize <= chunkSize, "Requested allocation too big for current chunk size!");

		// Quick check for too small pool
		if (allocSize > GetSumFreeMemory())
			return AllocWithChunk(allocSize, memoryUserData);

		// If no free blocks in pool then check only null block
		if (freeBlocks == 0 && CheckBlock(*nullBlock, allocSize, alignment))
		{
			// Lazy allocation, create memory chunk on first request
			if (nullBlock->ChunkHandle == nullptr)
			{
				TLSFMemoryChunk<Memory>* firstChunk = chunkAllocator.Alloc();
				TLSFMemoryChunk<Memory>::InitMemory(firstChunk, chunkFlags, chunkSize * chunkSizeDivisor, memoryUserData);
				nullBlock->ChunkHandle = firstChunk;
			}
			return CreateAlloc(nullBlock, allocSize, alignment);
		}

		// Round up to the next block
		U64 sizeForNextList = allocSize;
		U64 smallSizeStep = SMALL_BUFFER_SIZE / firstListSize;
		if (allocSize > SMALL_BUFFER_SIZE)
		{
			sizeForNextList += (1ULL << (Intrin::BitScanMSB(allocSize) - SECOND_LEVEL_INDEX));
		}
		else if (allocSize > SMALL_BUFFER_SIZE - smallSizeStep)
			sizeForNextList = SMALL_BUFFER_SIZE + 1;
		else
			sizeForNextList += smallSizeStep;

		// Check larger bucket
		U32 nextListIndex = 0;
		Block* nextListBlock = FindFreeBlock(sizeForNextList, nextListIndex);
		while (nextListBlock)
		{
			if (CheckBlock(*nextListBlock, allocSize, alignment))
				return CreateAlloc(nextListBlock, allocSize, alignment);
			nextListBlock = nextListBlock->NextFree;
		}

		// If failed check null block
		if (CheckBlock(*nullBlock, allocSize, alignment))
			return CreateAlloc(nullBlock, allocSize, alignment);

		// Check best fit bucket
		U32 prevListIndex = 0;
		Block* prevListBlock = FindFreeBlock(allocSize, prevListIndex);
		while (prevListBlock)
		{
			if (CheckBlock(*prevListBlock, allocSize, alignment))
				return CreateAlloc(prevListBlock, allocSize, alignment);
			prevListBlock = prevListBlock->NextFree;
		}

		// Worst case, full search has to be done
		while (++nextListIndex < listsCount)
		{
			nextListBlock = freeList[nextListIndex];
			while (nextListBlock)
			{
				if (CheckBlock(*nextListBlock, allocSize, alignment))
					return CreateAlloc(nextListBlock, allocSize, alignment);
				nextListBlock = nextListBlock->NextFree;
			}
		}

		// Nothing fits this allocation, create new chunk
		return AllocWithChunk(allocSize, memoryUserData);
	}

	ZE_CHUNKED_TLSF_TEMPLATE
	void ZE_CHUNKED_TLSF_TYPE::Free(AllocHandle allocation, void* memoryUserData) noexcept
	{
		ZE_ASSERT(allocation, "Invalid allocation!");
		Ptr<Block> block = allocation.Cast<Block>();
		ZE_ASSERT(!block->IsFree(), "Block is already free!");

		// Try merging
		Ptr<Block> next = block->NextPhysical;
		Ptr<Block> prev = block->PrevPhysical;
		if (prev != nullptr && prev->IsFree() && prev->ChunkHandle == block->ChunkHandle)
		{
			RemoveFreeBlock(prev);
			MergeBlock(block, prev);
		}

		if (!next->IsFree())
			InsertFreeBlock(block);
		else if (next->ChunkHandle == block->ChunkHandle)
		{
			if (next == nullBlock)
				MergeBlock(nullBlock, block);
			else
			{
				RemoveFreeBlock(next);
				MergeBlock(next, block);
				InsertFreeBlock(next);
			}
			// Check if chunk can be destroyed
			if (next->Size == chunkSize)
			{
				if (IsFreeChunk())
				{
					TLSFMemoryChunk<Memory>::DestroyMemory(next->ChunkHandle, memoryUserData);
					chunkAllocator.Free(next->ChunkHandle);
					next->ChunkHandle = nullptr;

					// Delete whole block
					if (prev)
						prev->NextPhysical = next->NextPhysical;
					if (next->NextPhysical)
						next->NextPhysical->PrevPhysical = prev;

					// Setup new null block
					if (next == nullBlock)
					{
						ZE_ASSERT(prev, "Trying to remove chunk when there is no more left!");

						if (prev->IsFree())
						{
							RemoveFreeBlock(prev);
							nullBlock = prev;
						}
						else
						{
							nullBlock->Size = 0;
							nullBlock->ChunkHandle = prev->ChunkHandle;
						}
					}
				}
				else
					SetFreeChunk(true);
			}
		}
	}

	ZE_CHUNKED_TLSF_TEMPLATE
	void ZE_CHUNKED_TLSF_TYPE::DestroyFreeChunks(void* memoryUserData) noexcept
	{
		if (nullBlock->ChunkHandle)
		{
			TLSFMemoryChunk<Memory>::DestroyMemory(nullBlock->ChunkHandle, memoryUserData);
			chunkAllocator.Free(nullBlock->ChunkHandle);
			nullBlock->ChunkHandle = nullptr;
		}
	}
#pragma endregion
}

#undef ZE_CHUNKED_TLSF_TEMPLATE
#undef ZE_CHUNKED_TLSF_TYPE