#include "GFX/API/DX12/Allocator.h"
#include "GFX/API/DX12/Device.h"

namespace ZE::GFX::API::DX12
{
	constexpr bool Allocator::BufferInfo::CheckBlock(Block& block, Chunk allocSize, Chunk alignment) noexcept
	{
		ZE_ASSERT(block.IsFree(), "Block is already taken!");

		const Chunk alignedOffset = Math::AlignUp(block.Offset, alignment);
		return block.Size >= allocSize + alignedOffset - block.Offset;
	}

	constexpr U16 Allocator::BufferInfo::SizeToSecondIndex(Chunk size, U8 memoryClass) noexcept
	{
		if (memoryClass == 0)
			return static_cast<U16>((size - 1) / 8);

		return static_cast<U16>((size >> (memoryClass + MEMORY_CLASS_SHIFT - SECOND_LEVEL_INDEX)) ^ (1U << SECOND_LEVEL_INDEX));
	}

	constexpr U32 Allocator::BufferInfo::GetListIndex(U8 memoryClass, U16 secondIndex) noexcept
	{
		if (memoryClass == 0)
			return secondIndex;

		return static_cast<U32>(memoryClass - 1) * (1 << SECOND_LEVEL_INDEX) + secondIndex + (1 << SECOND_LEVEL_INDEX);
	}

	U32 Allocator::BufferInfo::GetListIndex(Chunk size) noexcept
	{
		U8 memoryClass = SizeToMemoryClass(size);
		return GetListIndex(memoryClass, SizeToSecondIndex(size, memoryClass));
	}

	void Allocator::BufferInfo::CreateHeap(Device& dev, ID3D12Heap** heap, HeapFlags flags, U64 heapSize)
	{
		ZE_GFX_ENABLE(dev);

		D3D12_HEAP_DESC desc;
		desc.SizeInBytes = heapSize;
		desc.Properties.Type = GetHeapType(flags);
		desc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		desc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		desc.Properties.CreationNodeMask = 0;
		desc.Properties.VisibleNodeMask = 0;
		desc.Alignment = GetHeapAlignment(flags);
		desc.Flags = GetHeapFlags(flags);

		ZE_GFX_THROW_FAILED(dev.GetDevice()->CreateHeap(&desc, IID_PPV_ARGS(heap)));
	}

	Allocator::Block* Allocator::BufferInfo::AllocWithHeap(Device& dev,
		HeapFlags flags, U64 heapSize, U64 smallChunkSize, Chunk size)
	{
		EID newHeap = heapStorage->create();
		CreateHeap(dev, &heapStorage->emplace<DX::ComPtr<ID3D12Heap>>(newHeap), flags, heapSize);

		// Create new taken block
		Block* newBlock = blockAllocator.Alloc();
		newBlock->Size = size;
		newBlock->HeapNumber = newHeap;
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
		nullBlock->Size = heapSize / smallChunkSize - size;
		nullBlock->HeapNumber = newHeap;
		nullBlock->PrevPhysical = newBlock;
		newBlock->NextPhysical = nullBlock;

		freeHeap = false;
		return newBlock;
	}

	Allocator::Block* Allocator::BufferInfo::CreateAlloc(Block* currentBlock,
		Chunk size, Chunk alignment, U64 heapSize, U64 smallChunkSize) noexcept
	{
		// Get block and pop it from the free list
		if (currentBlock != nullBlock)
			RemoveFreeBlock(currentBlock);

		// Indicate that there is no more fully free block
		if (currentBlock->Size == heapSize / smallChunkSize)
			freeHeap = false;

		// Append missing alignment to prev block or create new one
		const Chunk misssingAlignment = Math::AlignUp(currentBlock->Offset, alignment) - currentBlock->Offset;
		if (misssingAlignment)
		{
			Block* prevBlock = currentBlock->PrevPhysical;
			ZE_ASSERT(prevBlock != nullptr, "There should be no missing alignment at offset 0!");

			if (prevBlock->IsFree() && prevBlock->HeapNumber == currentBlock->HeapNumber)
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
					freeChunks += misssingAlignment;
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
				newBlock->HeapNumber = currentBlock->HeapNumber;
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
				nullBlock->HeapNumber = currentBlock->HeapNumber;
				nullBlock->PrevPhysical = currentBlock;
				nullBlock->MarkFree();
				currentBlock->NextPhysical = nullBlock;
				currentBlock->MarkTaken();
			}
		}
		else
		{
			ZE_ASSERT(currentBlock->Size > size, "Proper block already found, shouldn't find smaller one!");
			ZE_ASSERT((currentBlock->Size + currentBlock->Offset) * smallChunkSize <= heapSize, "Resource too big for given range!");

			// Create new free block
			Block* newBlock = blockAllocator.Alloc();
			newBlock->Size = currentBlock->Size - size;
			newBlock->Offset = currentBlock->Offset + size;
			newBlock->HeapNumber = currentBlock->HeapNumber;
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

	Allocator::Block* Allocator::BufferInfo::FindFreeBlock(Chunk size, U32& listIndex) const noexcept
	{
		U8 memoryClass = SizeToMemoryClass(size);
		U32 innerFreeMap = innerIsFreeBitmap[memoryClass] & (~0U << SizeToSecondIndex(size, memoryClass));
		if (!innerFreeMap)
		{
			// Check higher levels for avaiable blocks
			U32 freeMap = isFreeBitmap & (~0UL << (memoryClass + 1));
			if (!freeMap)
				return nullptr; // No more memory avaible

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

	Allocator::BufferInfo::BufferInfo(Device& dev, Data::Storage& heapStorage,
		HeapFlags flags, U64 heapSize, U64 chunkSize)
		: blockAllocator(BLOCK_ALLOCATOR_CAPACITY), heapStorage(&heapStorage)
	{
		ZE_ASSERT(heapSize % chunkSize == 0, "Heap size should be multiple of single chunk size!");
		EID firstHeap = heapStorage.create();
		CreateHeap(dev, &heapStorage.emplace<DX::ComPtr<ID3D12Heap>>(firstHeap), flags, heapSize);

		nullBlock = blockAllocator.Alloc();
		nullBlock->Size = heapSize / chunkSize;
		nullBlock->HeapNumber = firstHeap;
		nullBlock->MarkFree();

		const U8 memoryClass = SizeToMemoryClass(nullBlock->Size);
		const U16 sli = SizeToSecondIndex(nullBlock->Size, memoryClass);
		listsCount = (memoryClass == 0 ? 0 : (memoryClass - 1) * (1UL << SECOND_LEVEL_INDEX) + sli) + 1 + (1UL << SECOND_LEVEL_INDEX);

		memoryClasses = memoryClass + 2;
		innerIsFreeBitmap = new U32[memoryClasses];
		memset(innerIsFreeBitmap, 0, memoryClasses * sizeof(U32));

		freeList = new Block * [listsCount];
		memset(freeList, 0, listsCount * sizeof(Block*));
	}

	Allocator::BufferInfo::~BufferInfo()
	{
		delete[] freeList;
		delete[] innerIsFreeBitmap;
	}

	void Allocator::BufferInfo::RemoveFreeBlock(Block* block) noexcept
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
		freeChunks -= block->Size;
	}

	void Allocator::BufferInfo::InsertFreeBlock(Block* block) noexcept
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
		freeChunks += block->Size;
	}

	void Allocator::BufferInfo::MergeBlock(Block* block, Block* prev) noexcept
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

	Allocator::Block* Allocator::BufferInfo::Alloc(Device& dev, HeapFlags flags,
		U64 heapSize, U64 smallChunkSize, Chunk allocSize, Chunk alignment)
	{
		ZE_ASSERT(allocSize > 0, "Cannot allocate empty block!");

		// Quick check for too small pool
		if (allocSize > GetSumFreeChunks())
			return AllocWithHeap(dev, flags, heapSize, smallChunkSize, allocSize);

		// If no free blocks in pool then check only null block
		if (freeBlocks == 0 && CheckBlock(*nullBlock, allocSize, alignment))
			return CreateAlloc(nullBlock, allocSize, alignment, heapSize, smallChunkSize);

		// Round up to the next block
		Chunk sizeForNextList = allocSize;
		Chunk smallSizeStep = SMALL_BUFFER_SIZE / (1 << SECOND_LEVEL_INDEX);
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
				return CreateAlloc(nextListBlock, allocSize, alignment, heapSize, smallChunkSize);
			nextListBlock = nextListBlock->NextFree;
		}

		// If failed check null block
		if (CheckBlock(*nullBlock, allocSize, alignment))
			return CreateAlloc(nullBlock, allocSize, alignment, heapSize, smallChunkSize);

		// Check best fit bucket
		U32 prevListIndex = 0;
		Block* prevListBlock = FindFreeBlock(allocSize, prevListIndex);
		while (prevListBlock)
		{
			if (CheckBlock(*prevListBlock, allocSize, alignment))
				return CreateAlloc(prevListBlock, allocSize, alignment, heapSize, smallChunkSize);
			prevListBlock = prevListBlock->NextFree;
		}

		// Worst case, full search has to be done
		while (++nextListIndex < listsCount)
		{
			nextListBlock = freeList[nextListIndex];
			while (nextListBlock)
			{
				if (CheckBlock(*nextListBlock, allocSize, alignment))
					return CreateAlloc(nextListBlock, allocSize, alignment, heapSize, smallChunkSize);
				nextListBlock = nextListBlock->NextFree;
			}
		}

		// Nothing fits this allocation, create new heap
		return AllocWithHeap(dev, flags, heapSize, smallChunkSize, allocSize);
	}

	void Allocator::BufferInfo::Free(Block* block, U64 heapSize, U64 smallChunkSize) noexcept
	{
		Block* next = block->NextPhysical;
		ZE_ASSERT(!block->IsFree(), "Block is already free!");

		// Try merging
		Block* prev = block->PrevPhysical;
		if (prev != nullptr && prev->IsFree() && prev->HeapNumber == block->HeapNumber)
		{
			RemoveFreeBlock(prev);
			MergeBlock(block, prev);
		}

		if (!next->IsFree())
			InsertFreeBlock(block);
		else if (next->HeapNumber == block->HeapNumber)
		{
			if (next == nullBlock)
				MergeBlock(nullBlock, block);
			else
			{
				RemoveFreeBlock(next);
				MergeBlock(next, block);
				InsertFreeBlock(next);
			}
			// Check if heap can be destroyed
			if (next->Size == heapSize / smallChunkSize)
			{
				if (freeHeap)
				{
					heapStorage->destroy(next->HeapNumber);

					// Delete whole block
					if (prev)
						prev->NextPhysical = next->NextPhysical;
					if (next->NextPhysical)
						next->NextPhysical->PrevPhysical = prev;

					// Setup new null block
					if (next == nullBlock)
					{
						ZE_ASSERT(prev, "Trying to remove heap when there is no more left!");

						if (prev->IsFree())
						{
							RemoveFreeBlock(prev);
							nullBlock = prev;
						}
						else
						{
							nullBlock->Size = 0;
							nullBlock->HeapNumber = prev->HeapNumber;
						}
					}
				}
				else
					freeHeap = true;
			}
		}
	}

	constexpr U64 Allocator::GetHeapAlignment(HeapFlags flags) noexcept
	{
		return flags & HeapFlag::NoMSAA ? D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT : D3D12_DEFAULT_MSAA_RESOURCE_PLACEMENT_ALIGNMENT;
	}

	constexpr D3D12_HEAP_TYPE Allocator::GetHeapType(HeapFlags flags) noexcept
	{
		return flags & HeapFlag::Dynamic ? D3D12_HEAP_TYPE_UPLOAD : D3D12_HEAP_TYPE_DEFAULT;
	}

	constexpr D3D12_HEAP_FLAGS Allocator::GetHeapFlags(HeapFlags flags) noexcept
	{
		return static_cast<D3D12_HEAP_FLAGS>(D3D12_HEAP_FLAG_CREATE_NOT_ZEROED | D3D12_HEAP_FLAG_DENY_RT_DS_TEXTURES
			| (flags & HeapFlag::AllowBuffers ? 0 : D3D12_HEAP_FLAG_DENY_BUFFERS)
			| (flags & HeapFlag::AllowTextures ? 0 : D3D12_HEAP_FLAG_DENY_NON_RT_DS_TEXTURES));
	}

	constexpr D3D12_RESOURCE_STATES Allocator::GetResourceState(HeapFlags flags) noexcept
	{
		return flags & HeapFlag::Dynamic ? D3D12_RESOURCE_STATE_GENERIC_READ : D3D12_RESOURCE_STATE_COPY_DEST;
	}

	DX::ComPtr<ID3D12Resource> Allocator::CreateCommittedResource(Device& dev,
		const D3D12_RESOURCE_DESC& desc, HeapFlags flags)
	{
		ZE_GFX_ENABLE(dev);

		D3D12_HEAP_PROPERTIES heapProp;
		heapProp.Type = GetHeapType(flags);
		heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProp.CreationNodeMask = 0;
		heapProp.VisibleNodeMask = 0;

		DX::ComPtr<ID3D12Resource> res;
		ZE_GFX_THROW_FAILED(dev.GetDevice()->CreateCommittedResource(&heapProp,
			GetHeapFlags(flags), &desc, GetResourceState(flags), nullptr, IID_PPV_ARGS(&res)));
		return res;
	}

	DX::ComPtr<ID3D12Resource> Allocator::CreateResource(Device& dev,
		const D3D12_RESOURCE_DESC& desc, U64 chunkSize, Block* block, HeapFlags flags)
	{
		ZE_VALID_EID(block->HeapNumber);
		ZE_GFX_ENABLE(dev);

		DX::ComPtr<ID3D12Resource> res;
		ZE_GFX_THROW_FAILED(dev.GetDevice()->CreatePlacedResource(heaps.get<DX::ComPtr<ID3D12Heap>>(block->HeapNumber).Get(),
			block->Offset * chunkSize, &desc, GetResourceState(flags), nullptr, IID_PPV_ARGS(&res)));
		return res;
	}

	ResourceInfo Allocator::AllocBigChunks(Device& dev, BufferInfo& bufferInfo, U64 bytes,
		const D3D12_RESOURCE_DESC& desc, HeapFlags flags, U64 heapSize, U64 chunkSize, U64 smallChunkSize)
	{
		ZE_ASSERT(smallChunkSize >= SMALL_CHUNK, "Minimal chunk should at least 4 KB!");
		ZE_ASSERT(smallChunkSize % SMALL_CHUNK == 0, "Minimal chunk should be multiple of 4 KB!");
		ZE_ASSERT(chunkSize % smallChunkSize == 0, "Bigger chunk should be multiply of smaller one!");

		if (bytes >= heapSize)
			return { CreateCommittedResource(dev, desc, flags), 0 };

		Block* block = bufferInfo.Alloc(dev, flags, heapSize, smallChunkSize, Math::DivideRoundUp(bytes, smallChunkSize), chunkSize / smallChunkSize);
		ZE_ASSERT(block != nullptr, "Should always find some blocks or at least create new heap!");

		return { CreateResource(dev, desc, smallChunkSize, block, flags), block };
	}

	ResourceInfo Allocator::AllocMinimalChunks(Device& dev, BufferInfo& bufferInfo, U64 bytes,
		const D3D12_RESOURCE_DESC& desc, HeapFlags flags, U64 heapSize, U64 chunkSize)
	{
		ZE_ASSERT(chunkSize >= SMALL_CHUNK, "Minimal chunk should at least 4 KB!");
		ZE_ASSERT(chunkSize % SMALL_CHUNK == 0, "Minimal chunk should be multiple of 4 KB!");

		if (bytes >= heapSize)
			return { CreateCommittedResource(dev, desc, flags), 0 };

		Block* block = bufferInfo.Alloc(dev, flags, heapSize, chunkSize, Math::DivideRoundUp(bytes, chunkSize), 1);
		ZE_ASSERT(block != nullptr, "Should always find some blocks or at least create new heap!");

		return { CreateResource(dev, desc, chunkSize, block, flags), block };
	}

	void Allocator::Remove(BufferInfo& bufferInfo, ResourceInfo& resInfo, U64 heapSize, U64 smallChunkSize) noexcept
	{
		resInfo.Resource = nullptr;

		// Committed resource, nothing inside algorithm
		if (resInfo.Handle == 0)
			return;
		bufferInfo.Free(reinterpret_cast<Block*>(resInfo.Handle), heapSize, smallChunkSize);
		resInfo.Handle = 0;
	}
}