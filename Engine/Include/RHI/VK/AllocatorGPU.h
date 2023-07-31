#pragma once
#include "Allocator/ChunkedTLSF.h"
#include "VK.h"
#include "Allocation.h"

namespace ZE::RHI::VK
{
	class Device;

	// Allocator for managing buffers and textures on GPU based on TLSF algorithm
	class AllocatorGPU final
	{
		typedef Allocator::TLSFMemoryChunkFlags MemoryFlags;
		enum MemoryFlag : MemoryFlags { None = 0, HostVisible = 1 };

		struct AllocParams
		{
			Device& Dev;
			U32 MemIndex = UINT32_MAX;
			void* pNext = nullptr;
			bool NewAllocation = false;
		};
		struct Memory
		{
			VkDeviceMemory DeviceMemory = VK_NULL_HANDLE;
			U8* MappedMemory = nullptr;

			static void Init(Memory& chunk, MemoryFlags flags, U64 size, void* userData);
			static void Destroy(Memory& chunk, void* userData) noexcept;
		};
		struct HeapInfo
		{
			U64 Size = 0;
			U64 Usage = 0;
			U64 Budget = 0;
		};
		typedef Allocator::ChunkedTLSF<Memory, 5, 7> MemoryTypeAllocator;

		static constexpr U64 BLOCK_ALLOC_CAPACITY = 200;
		static constexpr U64 CHUNK_ALLOC_CAPACITY = 30;

		MemoryTypeAllocator::BlockAllocator blockAllocator;
		MemoryTypeAllocator::ChunkAllocator chunkAllocator;
		std::vector<MemoryTypeAllocator> allocators;
		Ptr<VkMemoryType> typeInfo;
		Ptr<HeapInfo> heapInfo;
		U32 deviceMemoryCount = 0;
		U16 minimalAlignment = 1;
		U8 texturesStartIndex = 0;
		// Single image buffer heap | ReBAR
		std::bitset<2> flags = 0;

		static constexpr void UpdateBudget(HeapInfo& heapInfo, VkDeviceSize usage, VkDeviceSize budget) noexcept;

		void SetSingleBufferImageHeap(bool present) noexcept { flags[0] = present; }
		void SetReBAR(bool enabled) noexcept { flags[1] = enabled; }

		constexpr void FindMemoryPreferences(Allocation::Usage usage, bool isIntegratedGPU,
			VkMemoryPropertyFlags& required, VkMemoryPropertyFlags& preferred, VkMemoryPropertyFlags& notPreferred) noexcept;
		Allocation Alloc(Device& dev, Allocation::Usage usage, VkBuffer buffer, VkImage image,
			const VkMemoryRequirements& memoryReq, const VkMemoryDedicatedRequirements& dedicatedMemoryReq);

	public:
		AllocatorGPU() : blockAllocator(BLOCK_ALLOC_CAPACITY), chunkAllocator(CHUNK_ALLOC_CAPACITY) {}
		ZE_CLASS_MOVE(AllocatorGPU);
		~AllocatorGPU();

		constexpr bool IsSingleBufferImageHeap() const noexcept { return flags[0]; }
		constexpr bool IsReBAREnabled() const noexcept { return flags[1]; }

		void Init(Device& dev);
		void Destroy(Device& dev) noexcept;
		Allocation AllocBuffer(Device& dev, VkBuffer buffer, Allocation::Usage usage);
		// Disjoint images need plane requirements
		Allocation AllocImage(Device& dev, VkImage image, Allocation::Usage usage, VkImagePlaneMemoryRequirementsInfo* planeInfo = nullptr);
		void Remove(Device& dev, Allocation& alloc) noexcept;
		void HandleBudget(Device& dev) noexcept;
		void GetAllocInfo(const Allocation& alloc, VkDeviceSize& offset, VkDeviceMemory& memory, U8** mappedMemory = nullptr) const noexcept;
		U8* GetMappedMemory(const Allocation& alloc) const noexcept;
	};
}