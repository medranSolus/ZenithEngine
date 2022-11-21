#pragma once
#include "Data/Entity.h"
#include "D3D12.h"
#include "ResourceInfo.h"
#include "Table.h"

namespace ZE::GFX::API::DX12
{
	class Device;

	// Base allocator for managing buffers and textures on GPU based on TLSF algorithm
	class Allocator
	{
	protected:
		typedef U32 Chunk;

		typedef U8 HeapFlags;
		enum HeapFlag : HeapFlags { Dynamic = 1, AllowBuffers = 2, AllowTextures = 4, NoMSAA = 8 };

		struct Block
		{
			Chunk Offset = 0;
			Chunk Size = 0;
			EID HeapNumber = INVALID_EID;
			Block* PrevPhysical = nullptr;
			Block* NextPhysical = nullptr;
			// Address of the same block here indicates that block is taken
			Block* PrevFree = nullptr;
			Block* NextFree = nullptr;

			constexpr void MarkFree() noexcept { PrevFree = nullptr; }
			constexpr void MarkTaken() noexcept { PrevFree = this; }
			constexpr bool IsFree() const noexcept { return PrevFree != this; }
		};
		class BufferInfo
		{
			// According to original paper it should be preferable 4 or 5:
			// M. Masmano, I. Ripoll, A. Crespo, and J. Real "TLSF: a New Dynamic Memory Allocator for Real-Time Systems"
			// http://www.gii.upv.es/tlsf/files/ecrts04_tlsf.pdf
			static constexpr U8 SECOND_LEVEL_INDEX = 4;
			static constexpr U8 MEMORY_CLASS_SHIFT = 2;
			static constexpr U16 SMALL_BUFFER_SIZE = 1 << (MEMORY_CLASS_SHIFT + 1);
			static constexpr U8 MAX_MEMORY_CLASSES = 65 - MEMORY_CLASS_SHIFT;
			static constexpr U64 BLOCK_ALLOCATOR_CAPACITY = 100;

			// Leave one heap at most free to avoid constatnly creating and destroying heaps
			bool freeHeap = false;
			Chunk freeChunks = 0;
			U32 freeBlocks = 0;
			U32 isFreeBitmap = 0;
			U8 memoryClasses = 0;
			U32* innerIsFreeBitmap = nullptr;
			U32 listsCount = 0;
			// 0: 0-15 lists for small blocks
			// 1+: 0-(2^SLI-1) lists for normal blocks
			Block** freeList = nullptr;
			Block* nullBlock = nullptr;

			ZE::Allocator::Pool<Block> blockAllocator;
			Data::Storage* heapStorage;

			static U8 SizeToMemoryClass(Chunk size) noexcept { return size > SMALL_BUFFER_SIZE ? Intrin::BitScanMSB(size) - MEMORY_CLASS_SHIFT : 0; }

			static constexpr bool CheckBlock(Block& block, Chunk allocSize, Chunk alignment) noexcept;
			static constexpr U16 SizeToSecondIndex(Chunk size, U8 memoryClass) noexcept;
			static constexpr U32 GetListIndex(U8 memoryClass, U16 secondIndex) noexcept;
			static U32 GetListIndex(Chunk size) noexcept;

			static void CreateHeap(Device& dev, ID3D12Heap** heap, HeapFlags flags, U64 heapSize);

			constexpr Chunk GetSumFreeChunks() const noexcept { return freeChunks + nullBlock->Size; }

			Block* AllocWithHeap(Device& dev, HeapFlags flags,
				U64 heapSize, U64 smallChunkSize, Chunk size);
			Block* CreateAlloc(Block* currentBlock, Chunk size,
				Chunk alignment, U64 heapSize, U64 smallChunkSize) noexcept;
			Block* FindFreeBlock(Chunk size, U32& listIndex) const noexcept;
			void RemoveFreeBlock(Block* block) noexcept;
			void InsertFreeBlock(Block* block) noexcept;
			void MergeBlock(Block* block, Block* prev) noexcept;

		public:
			BufferInfo() = default;
			BufferInfo(Device& dev, Data::Storage& heapStorage,
				HeapFlags flags, U64 heapSize, U64 chunkSize);
			ZE_CLASS_DELETE(BufferInfo);
			~BufferInfo();

			Block* Alloc(Device& dev, HeapFlags flags, U64 heapSize,
				U64 smallChunkSize, Chunk size, Chunk alignment);
			void Free(Block* block, U64 heapSize, U64 smallChunkSize) noexcept;
		};

		static constexpr U64 SMALL_CHUNK = D3D12_SMALL_RESOURCE_PLACEMENT_ALIGNMENT; // 4 KB
		static constexpr U64 NORMAL_CHUNK = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT; // 64 KB
		static constexpr U64 HUGE_CHUNK = D3D12_DEFAULT_MSAA_RESOURCE_PLACEMENT_ALIGNMENT; // 4 MB

		Data::Storage heaps;

		static constexpr U64 GetHeapAlignment(HeapFlags flags) noexcept;
		static constexpr D3D12_HEAP_TYPE GetHeapType(HeapFlags flags) noexcept;
		static constexpr D3D12_HEAP_FLAGS GetHeapFlags(HeapFlags flags) noexcept;
		static constexpr D3D12_RESOURCE_STATES GetResourceState(HeapFlags flags) noexcept;

		DX::ComPtr<ID3D12Resource> CreateCommittedResource(Device& dev,
			const D3D12_RESOURCE_DESC& desc, HeapFlags flags);
		DX::ComPtr<ID3D12Resource> CreateResource(Device& dev,
			const D3D12_RESOURCE_DESC& desc, U64 chunkSize, Block* block, HeapFlags flags);

		// Find and allocate smallest memory region in heap at given boundary (assuming boundary bigger than smallest chunk)
		ResourceInfo AllocBigChunks(Device& dev, BufferInfo& bufferInfo, U64 bytes,
			const D3D12_RESOURCE_DESC& desc, HeapFlags flags, U64 heapSize, U64 chunkSize, U64 smallChunkSize);

		// Find and allocate smallest memory region in heap at minimal boundary (given chunk size is smallest possible)
		ResourceInfo AllocMinimalChunks(Device& dev, BufferInfo& bufferInfo, U64 bytes,
			const D3D12_RESOURCE_DESC& desc, HeapFlags flags, U64 heapSize, U64 chunkSize);

		// Remove allocated memory and return it to free pool merging whenever possible with nerby free regions
		void Remove(BufferInfo& bufferInfo, ResourceInfo& resInfo, U64 heapSize, U64 smallChunkSize) noexcept;

		Allocator() = default;

	public:
		ZE_CLASS_DELETE(Allocator);
		virtual ~Allocator() = default;
	};
}