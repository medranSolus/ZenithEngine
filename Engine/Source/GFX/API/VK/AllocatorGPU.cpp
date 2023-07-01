#include "GFX/API/VK/Device.h"
#include "GFX/API/VK/VulkanException.h"

namespace ZE::GFX::API::VK
{
	void AllocatorGPU::Memory::Init(Memory& chunk, MemoryFlags flags, U64 size, void* userData)
	{
		ZE_VK_ENABLE_ID();
		AllocParams& params = *reinterpret_cast<AllocParams*>(userData);

		VkMemoryPriorityAllocateInfoEXT priorityInfo = { VK_STRUCTURE_TYPE_MEMORY_PRIORITY_ALLOCATE_INFO_EXT, params.pNext };
		priorityInfo.priority = 0.5f;

		VkMemoryAllocateFlagsInfo allocFlags = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO, params.Dev.IsExtensionSupported(VK_EXT_MEMORY_PRIORITY_EXTENSION_NAME) ? &priorityInfo : params.pNext };
		allocFlags.flags = VK_MEMORY_ALLOCATE_DEVICE_MASK_BIT;
		allocFlags.deviceMask = 1;

		VkMemoryAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, &allocFlags };
		allocInfo.allocationSize = size;
		allocInfo.memoryTypeIndex = params.MemIndex;
		ZE_VK_THROW_NOSUCC(vkAllocateMemory(params.Dev.GetDevice(), &allocInfo, nullptr, &chunk.DeviceMemory));

		params.NewAllocation = true;
		if (flags & MemoryFlag::HostVisible)
		{
			ZE_VK_THROW_NOSUCC(vkMapMemory(params.Dev.GetDevice(), chunk.DeviceMemory, 0, size, 0, reinterpret_cast<void**>(&chunk.MappedMemory)));
		}
		ZE_VK_SET_ID(params.Dev.GetDevice(), chunk.DeviceMemory, VK_OBJECT_TYPE_DEVICE_MEMORY,
			"Memory chunk [size:" + std::to_string(size) + "]" +
			(flags & MemoryFlag::HostVisible ? " mapped at " + std::to_string((U64)chunk.MappedMemory) : ""));
	}

	void AllocatorGPU::Memory::Destroy(Memory& chunk, void* userData) noexcept
	{
		VkDevice device = reinterpret_cast<Device*>(userData)->GetDevice();
		if (chunk.MappedMemory)
		{
			vkUnmapMemory(device, chunk.DeviceMemory);
			chunk.MappedMemory = nullptr;
		}
		if (chunk.DeviceMemory)
		{
			vkFreeMemory(device, chunk.DeviceMemory, nullptr);
			chunk.DeviceMemory = VK_NULL_HANDLE;
		}
	}

	constexpr void AllocatorGPU::UpdateBudget(HeapInfo& heapInfo, VkDeviceSize usage, VkDeviceSize budget) noexcept
	{
		heapInfo.Usage = usage;
		heapInfo.Budget = budget;

		// Some bugged drivers return the budget incorrectly, e.g. 0 or much bigger than heap size
		if (heapInfo.Budget > heapInfo.Size)
			heapInfo.Budget = heapInfo.Size;
		if (heapInfo.Budget == 0)
			heapInfo.Budget = heapInfo.Size * 8 / 10; // 80% heuristics
	}

	constexpr void AllocatorGPU::FindMemoryPreferences(Allocation::Usage usage, bool isIntegratedGPU,
		VkMemoryPropertyFlags& required, VkMemoryPropertyFlags& preferred, VkMemoryPropertyFlags& notPreferred) noexcept
	{
		notPreferred = VK_MEMORY_PROPERTY_PROTECTED_BIT | VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD | VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD;

		switch (usage)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case Allocation::Usage::CPU:
		{
			required = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
			preferred = 0;
			notPreferred |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
			break;
		}
		case Allocation::Usage::DebugCPU:
		{
			required = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
			preferred = VK_MEMORY_PROPERTY_HOST_CACHED_BIT | VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD | VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD;
			notPreferred = 0;
			break;
		}
		case Allocation::Usage::StagingToCPU:
		{
			required = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
			preferred = isIntegratedGPU ? 0 : VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
			break;
		}
		case Allocation::Usage::GPU:
		{
			required = isIntegratedGPU ? 0 : VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
			preferred = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
			if (IsReBAREnabled())
				required |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
			break;
		}
		case Allocation::Usage::StagingToGPU:
		{
			required = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
			if (IsReBAREnabled())
				required |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
			preferred = isIntegratedGPU ? 0 : VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
			notPreferred |= VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
			break;
		}
		case Allocation::Usage::OnlyTransferGPU:
		{
			required = 0;
			preferred = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
			break;
		}
		}
	}

	Allocation AllocatorGPU::Alloc(Device& dev, Allocation::Usage usage, VkBuffer buffer, VkImage image,
		const VkMemoryRequirements& memoryReq, const VkMemoryDedicatedRequirements& dedicatedMemoryReq)
	{
		VkMemoryPropertyFlags requiredFlags, preferredFlags, notPreferredFlags;
		FindMemoryPreferences(usage, dev.IsIntegratedGPU(), requiredFlags, preferredFlags, notPreferredFlags);

		// Go over memory types to find best suited one
		AllocParams params = { dev };
		U32 memoryCost = UINT32_MAX;
		for (U32 memTypeCount = allocators.size() / (1 + IsSingleBufferImageHeap()), memoryIndex = 0, memTypeBit = 1; memoryIndex < memTypeCount; ++memoryIndex, memTypeBit <<= 1)
		{
			if (memoryReq.memoryTypeBits & memTypeBit)
			{
				const VkMemoryPropertyFlags memoryFlags = typeInfo[memoryIndex].propertyFlags;
				if ((requiredFlags & ~memoryFlags) == 0)
				{
					// Calculate cost as number of bits from preferred flags not present and number of not preferred flags present
					const U32 cost = Intrin::CountBitsSet(preferredFlags & ~memoryFlags) + Intrin::CountBitsSet(memoryFlags & notPreferredFlags);
					if (cost < memoryCost)
					{
						params.MemIndex = memoryIndex;
						if (cost == 0)
							break;
						memoryCost = cost;
					}
				}
			}
		}

		// When required use dedicated alloc and for some heuristics if not too many allocations done already
		VkMemoryDedicatedAllocateInfo dedicatedAlloc = { VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO, nullptr };
		if (dedicatedMemoryReq.requiresDedicatedAllocation
			|| deviceMemoryCount <= dev.GetLimits().maxMemoryAllocationCount * 3 / 4
			&& (dedicatedMemoryReq.prefersDedicatedAllocation || memoryReq.size >= allocators.at(params.MemIndex).GetChunkSize()))
		{
			dedicatedAlloc.image = image;
			dedicatedAlloc.buffer = buffer;
			params.pNext = &dedicatedAlloc;
		}

		AllocHandle alloc = allocators.at(params.MemIndex).Alloc(memoryReq.size, memoryReq.alignment, &params);
		if (params.NewAllocation)
			++deviceMemoryCount;
		// Manual counting when budget disabled
		if (!dev.IsExtensionSupported(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME))
			heapInfo[typeInfo[params.MemIndex].heapIndex].Usage += allocators.at(params.MemIndex).GetSize(alloc);
		return { alloc, params.MemIndex };
	}

	AllocatorGPU::~AllocatorGPU()
	{
		if (typeInfo)
			typeInfo.DeleteArray();
		if (heapInfo)
			heapInfo.DeleteArray();
	}

	void AllocatorGPU::Init(Device& dev)
	{
		VkPhysicalDeviceMemoryBudgetPropertiesEXT memoryBudget = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_BUDGET_PROPERTIES_EXT, nullptr };
		VkPhysicalDeviceMemoryProperties2 memoryProps = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2, dev.IsExtensionSupported(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME) ? &memoryBudget : nullptr };
		vkGetPhysicalDeviceMemoryProperties2(dev.GetPhysicalDevice(), &memoryProps);

		typeInfo = new VkMemoryType[memoryProps.memoryProperties.memoryTypeCount];
		for (U32 i = 0; i < memoryProps.memoryProperties.memoryTypeCount; ++i)
			typeInfo[i] = memoryProps.memoryProperties.memoryTypes[i];

		// Chek for Resizeable BAR enabled in the system (but not in integrated GPU)
		// (system shouldn't report 2 DEVICE_LOCAL heaps and there should be DEVICE_LOCAL | HOST_VISIBLE memory available)
		if (!dev.IsIntegratedGPU())
			SetReBAR(true);
		heapInfo = new HeapInfo[memoryProps.memoryProperties.memoryHeapCount];
		for (U32 i = 0, deviceHeapIdx = UINT32_MAX; i < memoryProps.memoryProperties.memoryHeapCount; ++i)
		{
			if (memoryProps.memoryProperties.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
			{
				if (deviceHeapIdx == UINT32_MAX)
					deviceHeapIdx = i;
				else
					SetReBAR(false);
			}
			heapInfo[i].Size = memoryProps.memoryProperties.memoryHeaps[i].size;
			if (!dev.IsExtensionSupported(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME))
			{
				memoryBudget.heapUsage[i] = 0;
				memoryBudget.heapBudget[i] = 0;
			}
			UpdateBudget(heapInfo[i], memoryBudget.heapUsage[i], memoryBudget.heapBudget[i]);
		}
		if (IsReBAREnabled())
		{
			SetReBAR(false);
			for (U32 i = 0; i < memoryProps.memoryProperties.memoryTypeCount; ++i)
			{
				const VkMemoryPropertyFlags memoryFlags = memoryProps.memoryProperties.memoryTypes[i].propertyFlags;
				if ((~memoryFlags & (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) == 0)
				{
					SetReBAR(true);
					break;
				}
			}
		}

		U32 allocatorCount = memoryProps.memoryProperties.memoryTypeCount;
		U64 deviceHeapSize = Settings::GetHeapSizeBuffers();
		U64 hostHeapSize = Settings::GetHeapSizeHost();
		U64 stagingHeapSize = Settings::GetHeapSizeStaging();
		// Chek alignment rules between images and buffers
		if (dev.GetLimits().bufferImageGranularity != 1)
		{
			// On small sizes just round up allocation size, otherwise separate pools for buffers and images
			if (dev.GetLimits().bufferImageGranularity <= 256)
			{
				minimalAlignment = dev.GetLimits().bufferImageGranularity;
				deviceHeapSize += Settings::GetHeapSizeTextures();
				SetSingleBufferImageHeap(true);
			}
			else
			{
				// Offset all textures further into allocators
				texturesStartIndex = allocatorCount;
				allocatorCount *= 2;
				hostHeapSize /= 2;
				stagingHeapSize /= 2;
			}
		}

		// Allocate for all memory types (use vector due to contructor constraint)
		allocators.reserve(allocatorCount);
		bool localHeapFound = false, localTexturesHeapFound = false;
		for (U32 i = 0; i < allocatorCount; ++i)
		{
			const VkMemoryPropertyFlags flags = memoryProps.memoryProperties.memoryTypes[i % memoryProps.memoryProperties.memoryTypeCount].propertyFlags;
			U64 heapSize = hostHeapSize;
			if (dev.IsIntegratedGPU())
			{
				if (i < memoryProps.memoryProperties.memoryTypeCount)
					heapSize += deviceHeapSize;
				else
					heapSize += Settings::GetHeapSizeTextures();
			}
			else if (flags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
			{
				if (i < memoryProps.memoryProperties.memoryTypeCount)
				{
					heapSize = localHeapFound ? stagingHeapSize : deviceHeapSize;
					localHeapFound = true;
				}
				else
				{
					heapSize = localTexturesHeapFound ? stagingHeapSize : Settings::GetHeapSizeTextures();
					localTexturesHeapFound = true;
				}
			}
			allocators.emplace_back(blockAllocator, chunkAllocator).Init(IsReBAREnabled() || (flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) ? MemoryFlag::HostVisible : MemoryFlag::None, heapSize, 1, 2);
		}
	}

	void AllocatorGPU::Destroy(Device& dev) noexcept
	{
		for (MemoryTypeAllocator& alloc : allocators)
			alloc.DestroyFreeChunks(&dev);
	}

	Allocation AllocatorGPU::AllocBuffer(Device& dev, VkBuffer buffer, Allocation::Usage usage)
	{
		const VkBufferMemoryRequirementsInfo2 bufferMemoryReq = { VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2, nullptr, buffer };

		VkMemoryDedicatedRequirements dedicatedMemoryReq = { VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS, nullptr };
		VkMemoryRequirements2 memoryReq = { VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2, &dedicatedMemoryReq };
		vkGetBufferMemoryRequirements2(dev.GetDevice(), &bufferMemoryReq, &memoryReq);

		return Alloc(dev, usage, buffer, VK_NULL_HANDLE, memoryReq.memoryRequirements, dedicatedMemoryReq);
	}

	Allocation AllocatorGPU::AllocImage(Device& dev, VkImage image, Allocation::Usage usage, VkImagePlaneMemoryRequirementsInfo* planeInfo)
	{
		const VkImageMemoryRequirementsInfo2 imageMemoryReq = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2, planeInfo, image };

		VkMemoryDedicatedRequirements dedicatedMemoryReq = { VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS, nullptr };
		VkMemoryRequirements2 memoryReq = { VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2, &dedicatedMemoryReq };
		vkGetImageMemoryRequirements2(dev.GetDevice(), &imageMemoryReq, &memoryReq);

		return Alloc(dev, usage, VK_NULL_HANDLE, image, memoryReq.memoryRequirements, dedicatedMemoryReq);
	}

	void AllocatorGPU::Remove(Device& dev, Allocation& alloc) noexcept
	{
		ZE_ASSERT(!alloc.IsFree(), "Invalid allocation!");
		// Manual counting when budget disabled
		if (!dev.IsExtensionSupported(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME))
			heapInfo[typeInfo[alloc.MemoryIndex - (alloc.MemoryIndex >= texturesStartIndex ? texturesStartIndex : 0)].heapIndex].Usage -= allocators.at(alloc.MemoryIndex).GetSize(alloc.Handle);

		allocators.at(alloc.MemoryIndex).Free(alloc.Handle, &dev);
		alloc.Free();
	}

	void AllocatorGPU::HandleBudget(Device& dev) noexcept
	{
		// Update to current budget
		if (dev.IsExtensionSupported(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME))
		{
			VkPhysicalDeviceMemoryBudgetPropertiesEXT memoryBudget = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_BUDGET_PROPERTIES_EXT, nullptr };
			VkPhysicalDeviceMemoryProperties2 memoryProps = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2, &memoryBudget };
			vkGetPhysicalDeviceMemoryProperties2(dev.GetPhysicalDevice(), &memoryProps);

			for (U32 i = 0; i < memoryProps.memoryProperties.memoryHeapCount; ++i)
				UpdateBudget(heapInfo[i], memoryBudget.heapUsage[i], memoryBudget.heapBudget[i]);
		}
	}

	void AllocatorGPU::GetAllocInfo(const Allocation& alloc, VkDeviceSize& offset, VkDeviceMemory& memory, U8** mappedMemory) const noexcept
	{
		ZE_ASSERT(!alloc.IsFree(), "Invalid allocation!");

		offset = allocators.at(alloc.MemoryIndex).GetOffset(alloc.Handle);
		Memory mem = allocators.at(alloc.MemoryIndex).GetMemory(alloc.Handle);
		memory = mem.DeviceMemory;
		if (mappedMemory)
			*mappedMemory = mem.MappedMemory + (mem.MappedMemory ? offset : 0);
	}

	U8* AllocatorGPU::GetMappedMemory(const Allocation& alloc) const noexcept
	{
		ZE_ASSERT(!alloc.IsFree(), "Invalid allocation!");

		U8* memory = allocators.at(alloc.MemoryIndex).GetMemory(alloc.Handle).MappedMemory;
		if (memory)
			return memory + allocators.at(alloc.MemoryIndex).GetOffset(alloc.Handle);
		return nullptr;
	}
}