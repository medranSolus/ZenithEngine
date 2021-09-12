#pragma once
#include "Table.h"
#include "D3D12.h"
#include "ResourceInfo.h"

namespace ZE::GFX::API::DX12
{
	class Device;

	// Base allocator for managing buffers and textures on GPU
	class Allocator
	{
	protected:
		enum HeapFlags : U8 { Dynamic = 1, AllowBuffers = 2, AllowTextures = 4 };
		struct MemInfo
		{
			U32 Index;
			U32 Size;
		};
		class BufferInfo final
		{
		public:
			TableInfo<U32> FreeInfo;
			MemInfo* FreeList;
			TableInfo<U16> HeapsInfo;
			DX::ComPtr<ID3D12Heap>* Heaps;

			BufferInfo(U32 heapSize) noexcept;
			BufferInfo(BufferInfo&&) = delete;
			BufferInfo(const BufferInfo&) = delete;
			BufferInfo& operator=(BufferInfo&&) = delete;
			BufferInfo& operator=(const BufferInfo&) = delete;
			~BufferInfo();
		};

	private:
		static constexpr U16 HEAP_CHUNKS_GROW = 4;
		static constexpr U32 FREE_LIST_CHUNKS_GROW = 100;

#pragma region Helper functions
		// Get unique index of resource at given position. When given size of buffer returns number of chunks that it will occupy
		template<U64 MIN_CHUNK>
		static constexpr U32 GetGlobalIndex(U64 offset) { return static_cast<U32>(offset / MIN_CHUNK + static_cast<bool>(offset % MIN_CHUNK)); }
		// Get global address spanning across all heaps
		template<U64 MIN_CHUNK>
		static constexpr U64 GetGlobalOffset(U32 index) { return index * MIN_CHUNK; }
		// Get index of first chunk located inside heap
		template<U64 MIN_CHUNK, U64 HEAP_SIZE>
		static constexpr U32 GetGlobalHeapIndex(U16 heap) { return heap * HEAP_SIZE / MIN_CHUNK; }
		// Get index of heap where located is resource
		template<U64 MIN_CHUNK, U64 HEAP_SIZE>
		static constexpr U16 GetHeapFromIndex(U32 globalIndex) { return static_cast<U16>(GetGlobalOffset<MIN_CHUNK>(globalIndex) / HEAP_SIZE); }
		// Get resource offset within single heap
		template<U64 MIN_CHUNK, U64 HEAP_SIZE>
		static constexpr U64 GetLocalOffset(U32 index) { return GetGlobalOffset<MIN_CHUNK>(index) - HEAP_SIZE * GetHeapFromIndex<MIN_CHUNK, HEAP_SIZE>(index); }
#pragma endregion

		// Creates new heap and adds new free region for it
		template<U64 MIN_CHUNK, U64 HEAP_SIZE>
		static void AddHeap(Device& dev, BufferInfo& bufferInfo, HeapFlags flags);

		// Sort list of free regions
		static void Sort(BufferInfo& bufferInfo, U32 element);
		// Create resource inside heap at given local offset
		static DX::ComPtr<ID3D12Resource> Alloc(Device& dev, const D3D12_RESOURCE_DESC& desc,
			ID3D12Heap* heap, U64 offset, HeapFlags flags);

	protected:
		static constexpr U64 SMALL_CHUNK = D3D12_SMALL_RESOURCE_PLACEMENT_ALIGNMENT; // 4 KB
		static constexpr U64 NORMAL_CHUNK = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT; // 64 KB
		static constexpr U64 HUGE_CHUNK = D3D12_DEFAULT_MSAA_RESOURCE_PLACEMENT_ALIGNMENT; // 4 MB

		// Find and allocate smallest memory region in heap at given boundary (assuming boundary bigger than smallest chunk)
		template<U64 BOUNDARY, U64 HEAP_SIZE>
		static ResourceInfo AllocAlignBigChunks(Device& dev, BufferInfo& bufferInfo,
			U32 bytes, const D3D12_RESOURCE_DESC& desc, HeapFlags flags);
		// Find and allocate smallest memory region in heap at minimal boundary (given chunk size is smallest possible)
		template<U64 MIN_CHUNK, U64 HEAP_SIZE>
		static ResourceInfo AllocAlignMinimalChunks(Device& dev, BufferInfo& bufferInfo,
			U32 bytes, const D3D12_RESOURCE_DESC& desc, HeapFlags flags);
		// Remove allocated memory and return it to free pool merging whenever possible with nerby free regions (given chunk size is smallest possible)
		template<U64 MIN_CHUNK, U64 HEAP_SIZE>
		static void Remove(BufferInfo& bufferInfo, U32 id, U32 size);

		static void CreateHeap(Device& dev, ID3D12Heap** heap, HeapFlags flags, U64 heapSize);

		Allocator() = default;

	public:
		Allocator(Allocator&&) = delete;
		Allocator(const Allocator&) = delete;
		Allocator& operator=(Allocator&&) = delete;
		Allocator& operator=(const Allocator&) = delete;
		virtual ~Allocator() = default;
	};

#pragma region Functions
	template<U64 MIN_CHUNK, U64 HEAP_SIZE>
	void Allocator::AddHeap(Device& dev, BufferInfo& bufferInfo, HeapFlags flags)
	{
		Table::Append<FREE_LIST_CHUNKS_GROW>(bufferInfo.FreeInfo, bufferInfo.FreeList,
			MemInfo(GetGlobalHeapIndex<MIN_CHUNK, HEAP_SIZE>(bufferInfo.HeapsInfo.Size), HEAP_SIZE));
		Table::Append<HEAP_CHUNKS_GROW>(bufferInfo.HeapsInfo, bufferInfo.Heaps);

		CreateHeap(dev, &bufferInfo.Heaps[bufferInfo.HeapsInfo.Size - 1], flags, HEAP_SIZE);
	}

	template<U64 BOUNDARY, U64 HEAP_SIZE>
	ResourceInfo Allocator::AllocAlignBigChunks(Device& dev, BufferInfo& bufferInfo,
		U32 bytes, const D3D12_RESOURCE_DESC& desc, HeapFlags flags)
	{
		assert(bytes < HEAP_SIZE);
		constexpr U32 SMALL_CHUNKS_IN_BOUNDARY = BOUNDARY / SMALL_CHUNK;

		U32 index = 0, chunks = GetGlobalIndex<SMALL_CHUNK>(bytes);
		if (bufferInfo.FreeInfo.Size == 0)
		{
			// No more free space so allocate within new heap
			AddHeap<SMALL_CHUNK, HEAP_SIZE>(dev, bufferInfo, flags);
			index = bufferInfo.FreeList[0].Index;
			bufferInfo.FreeList[0].Index += chunks;
			bufferInfo.FreeList[0].Size -= chunks * SMALL_CHUNK;
			return { Alloc(dev, desc, bufferInfo.Heaps[bufferInfo.HeapsInfo.Size - 1].Get(), 0, flags), index };
		}
		U32 i = 0;
		for (; i < bufferInfo.FreeInfo.Size; ++i)
		{
			// Free chunks in region but before nearest selected boundary
			U32 unalignedChunks = (SMALL_CHUNKS_IN_BOUNDARY - bufferInfo.FreeList[i].Index % SMALL_CHUNKS_IN_BOUNDARY) % SMALL_CHUNKS_IN_BOUNDARY;
			// Rest of the chunks in region
			U32 remainingChunks = GetGlobalIndex<SMALL_CHUNK>(bufferInfo.FreeList[i].Size);
			if (remainingChunks > unalignedChunks)
			{
				remainingChunks -= unalignedChunks;
				if (remainingChunks > chunks)
				{
					index = bufferInfo.FreeList[i].Index + unalignedChunks;
					if (unalignedChunks == 0)
					{
						// Consume part of the free region
						bufferInfo.FreeList[i].Index += chunks;
						bufferInfo.FreeList[i].Size -= chunks * SMALL_CHUNK;
						Sort(bufferInfo, i);
					}
					else
					{
						// Region split in 2 so another entry in free list required
						bufferInfo.FreeList[i].Size = unalignedChunks * SMALL_CHUNK;
						Sort(bufferInfo, i);
						Table::Insert<FREE_LIST_CHUNKS_GROW>(bufferInfo.FreeInfo, bufferInfo.FreeList,
							i, MemInfo(index + chunks, remainingChunks * SMALL_CHUNK));
						Sort(bufferInfo, i);
					}
					break;
				}
				else if (remainingChunks == chunks)
				{
					index = bufferInfo.FreeList[i].Index + unalignedChunks;
					// Ideal fit, whole region consumed
					if (unalignedChunks == 0)
						Table::Remove<FREE_LIST_CHUNKS_GROW>(bufferInfo.FreeInfo, bufferInfo.FreeList, i);
					else
					{
						// Allocate end of region only
						bufferInfo.FreeList[i].Size -= chunks * SMALL_CHUNK;
						Sort(bufferInfo, i);
					}
					break;
				}
			}
		}
		if (i == bufferInfo.FreeInfo.Size)
		{
			// Not found any small enough place so new heap is needed anyway
			AddHeap<SMALL_CHUNK, HEAP_SIZE>(dev, bufferInfo, flags);
			index = bufferInfo.FreeList[bufferInfo.FreeInfo.Size - 1].Index;
			bufferInfo.FreeList[bufferInfo.FreeInfo.Size - 1].Index += chunks;
			bufferInfo.FreeList[bufferInfo.FreeInfo.Size - 1].Size -= chunks * SMALL_CHUNK;
			return { Alloc(dev, desc, bufferInfo.Heaps[bufferInfo.HeapsInfo.Size - 1].Get(), 0, flags), index };
		}
		return { Alloc(dev, desc, bufferInfo.Heaps[GetHeapFromIndex<SMALL_CHUNK, HEAP_SIZE>(index)].Get(),
			GetLocalOffset<SMALL_CHUNK, HEAP_SIZE>(index), flags), index };
	}

	template<U64 MIN_CHUNK, U64 HEAP_SIZE>
	ResourceInfo Allocator::AllocAlignMinimalChunks(Device& dev, BufferInfo& bufferInfo,
		U32 bytes, const D3D12_RESOURCE_DESC& desc, HeapFlags flags)
	{
		assert(bytes < HEAP_SIZE);

		U32 index = 0, chunks = GetGlobalIndex<MIN_CHUNK>(bytes);
		if (bufferInfo.FreeInfo.Size == 0)
		{
			// No more free space so allocate within new heap
			AddHeap<MIN_CHUNK, HEAP_SIZE>(dev, bufferInfo, flags);
			index = bufferInfo.FreeList[0].Index;
			bufferInfo.FreeList[0].Index += chunks;
			bufferInfo.FreeList[0].Size -= chunks * MIN_CHUNK;
			return { Alloc(dev, desc, bufferInfo.Heaps[bufferInfo.HeapsInfo.Size - 1].Get(), 0, flags), index };
		}
		for (U32 i = 0; i < bufferInfo.FreeInfo.Size; ++i)
		{
			// Check size of free region
			index = GetGlobalIndex<MIN_CHUNK>(bufferInfo.FreeList[i].Size);
			if (index > chunks)
			{
				// Shrunk region
				index = bufferInfo.FreeList[i].Index;
				bufferInfo.FreeList[i].Index += chunks;
				bufferInfo.FreeList[i].Size -= chunks * MIN_CHUNK;
				Sort(bufferInfo, i);
				break;
			}
			else if (index == chunks)
			{
				// Ideal fit, whole region occupied
				index = bufferInfo.FreeList[i].Index;
				Table::Remove<FREE_LIST_CHUNKS_GROW>(bufferInfo.FreeInfo, bufferInfo.FreeList, i);
				break;
			}
			// Because we iterate over smalles possible alignmet we shoud never reach end of this list. Sanity check for peace of mind
			assert(bufferInfo.FreeInfo.Size - 1 != i && "Should always find lowest possible chunk if there are free elements!");
		}
		return { Alloc(dev, desc, bufferInfo.Heaps[GetHeapFromIndex<MIN_CHUNK, HEAP_SIZE>(index)].Get(),
			GetLocalOffset<MIN_CHUNK, HEAP_SIZE>(index), flags), index };
	}

	template<U64 MIN_CHUNK, U64 HEAP_SIZE>
	void Allocator::Remove(BufferInfo& bufferInfo, U32 id, U32 size)
	{
		U16 heap = GetHeapFromIndex<MIN_CHUNK, HEAP_SIZE>(id);
		U32 chunks = GetGlobalIndex<MIN_CHUNK>(size);
		bool leftJoin = false, rightJoin = false;

		// Find nearby regions to expand them by newly freed memory
		for (U32 i = 0; i < bufferInfo.FreeInfo.Size; ++i)
		{
			// Only regions within single heap must count
			if (heap == GetHeapFromIndex<MIN_CHUNK, HEAP_SIZE>(bufferInfo.FreeList[i].Index))
			{
				// Check if region on the left (first) or right (second) can be connected
				if (bufferInfo.FreeList[i].Index + GetGlobalIndex<MIN_CHUNK>(bufferInfo.FreeList[i].Size) == id)
				{
					bufferInfo.FreeList[i].Size += size;
					chunks = GetGlobalIndex<MIN_CHUNK>(bufferInfo.FreeList[i].Size);
					size = i;
					leftJoin = true;
					break;
				}
				else if (id + chunks == bufferInfo.FreeList[i].Index)
				{
					bufferInfo.FreeList[i].Size += size;
					chunks = GetGlobalIndex<MIN_CHUNK>(bufferInfo.FreeList[i].Size);
					bufferInfo.FreeList[i].Index = id;
					size = i;
					rightJoin = true;
					break;
				}
			}
		}
		if (leftJoin)
		{
			U32 i = 0, limit = size;
			// Outer loop to avoid checking if not currently selected region is checked
			for (U8 part = 0; part < 2; ++part)
			{
				// When joined left side of region check if no more neighbours on the right side
				for (; i < limit; ++i)
				{
					if (heap == GetHeapFromIndex<MIN_CHUNK, HEAP_SIZE>(bufferInfo.FreeList[i].Index))
					{
						if (id + chunks == bufferInfo.FreeList[i].Index)
						{
							// Expand free region and remove absorbed one
							bufferInfo.FreeList[size].Size += bufferInfo.FreeList[i].Size;
							Table::Remove<FREE_LIST_CHUNKS_GROW>(bufferInfo.FreeInfo, bufferInfo.FreeList, i);
							return Sort(bufferInfo, size);
						}
					}
				}
				++i;
				limit = bufferInfo.FreeInfo.Size;
			}
		}
		else if (rightJoin)
		{
			U32 i = 0, limit = size;
			// Outer loop to avoid checking if not currently selected region is checked
			for (U8 part = 0; part < 2; ++part)
			{
				// When joined right side of region check if no more neighbours on the left side
				for (; i < limit; ++i)
				{
					if (heap == GetHeapFromIndex<MIN_CHUNK, HEAP_SIZE>(bufferInfo.FreeList[i].Index))
					{
						if (bufferInfo.FreeList[i].Index + GetGlobalIndex<MIN_CHUNK>(bufferInfo.FreeList[i].Size) == id)
						{
							// Expand free region and remove absorbed one
							bufferInfo.FreeList[i].Size += bufferInfo.FreeList[size].Size;
							Table::Remove<FREE_LIST_CHUNKS_GROW>(bufferInfo.FreeInfo, bufferInfo.FreeList, size);
							return Sort(bufferInfo, i);
						}
					}
				}
				++i;
				limit = bufferInfo.FreeInfo.Size;
			}
		}
		else
		{
			// No nearby region found so create new one
			Table::Append<FREE_LIST_CHUNKS_GROW>(bufferInfo.FreeInfo, bufferInfo.FreeList, MemInfo(id, size));
			Sort(bufferInfo, bufferInfo.FreeInfo.Size - 1);
		}
	}
#pragma endregion
}