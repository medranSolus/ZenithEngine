#include "GFX/API/VK/Resource/DynamicCBuffer.h"
#include "GFX/API/VK/VulkanException.h"

namespace ZE::GFX::API::VK::Resource
{
	void DynamicCBuffer::AllocBlock(GFX::Device& dev)
	{
		ZE_VK_ENABLE_ID();

		VkBufferCreateInfo bufferInfo;
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.pNext = nullptr;
		bufferInfo.flags = 0;
		bufferInfo.size = BLOCK_SIZE;
		bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		bufferInfo.queueFamilyIndexCount = 0;
		bufferInfo.pQueueFamilyIndices = nullptr;
		VkBuffer block = VK_NULL_HANDLE;
		ZE_VK_THROW_NOSUCC(vkCreateBuffer(dev.Get().vk.GetDevice(), &bufferInfo, nullptr, &block));
		ZE_VK_SET_ID(dev.Get().vk.GetDevice(), block, VK_OBJECT_TYPE_BUFFER,
			"DynamicCBuffer_" + std::to_string(resInfo.size()) + " [size:" + std::to_string(BLOCK_SIZE) + "]");

		Allocation alloc = dev.Get().vk.GetMemory().AllocBuffer(dev.Get().vk, block, Allocation::Usage::StagingToGPU);

		const U32 deviceIndex = 0;
		VkBindBufferMemoryDeviceGroupInfo deviceGroupInfo;
		deviceGroupInfo.sType = VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_DEVICE_GROUP_INFO;
		deviceGroupInfo.pNext = nullptr;
		deviceGroupInfo.deviceIndexCount = 1;
		deviceGroupInfo.pDeviceIndices = &deviceIndex;

		VkBindBufferMemoryInfo bindInfo;
		bindInfo.sType = VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_INFO;
		bindInfo.pNext = &deviceGroupInfo;
		bindInfo.buffer = block;
		dev.Get().vk.GetMemory().GetAllocInfo(alloc, bindInfo.memoryOffset, bindInfo.memory, &buffer);
		ZE_ASSERT(buffer != nullptr, "Dynamic buffer always should be accessible from CPU!");

		ZE_VK_THROW_NOSUCC(vkBindBufferMemory2(dev.Get().vk.GetDevice(), 1, &bindInfo));
		resInfo.emplace_back(alloc);
	}

	void DynamicCBuffer::MapBlock(GFX::Device& dev, U64 block)
	{
		ZE_ASSERT(block < resInfo.size(), "Trying to map block outside of range!");

		buffer = dev.Get().vk.GetMemory().GetMappedMemory(resInfo.at(block));
		ZE_ASSERT(buffer != nullptr, "Dynamic buffer always should be accessible from CPU!");
	}

	DynamicCBuffer::~DynamicCBuffer()
	{
		for (auto& res : resInfo)
		{
			ZE_ASSERT(res.IsFree(), "Resource not freed before deletion!");
		}
	}

	GFX::Resource::DynamicBufferAlloc DynamicCBuffer::Alloc(GFX::Device& dev, const void* values, U32 bytes)
	{
		ZE_ASSERT(buffer, "Dynamic buffer has been freed already!");
		ZE_ASSERT(bytes <= BLOCK_SIZE, "Structure too large for dynamic buffer!");

		const U32 newBlock = Math::AlignUp(bytes, static_cast<U32>(dev.Get().vk.GetLimits().minUniformBufferOffsetAlignment));
#ifndef _ZE_RENDER_GRAPH_SINGLE_THREAD
		const std::lock_guard<std::mutex> lock(allocLock);
#endif
		if (nextOffset + newBlock > BLOCK_SIZE)
		{
			nextOffset = 0;
			if (++currentBlock >= resInfo.size())
				AllocBlock(dev);
			else
				MapBlock(dev, currentBlock);
		}
		memcpy(buffer + nextOffset, values, bytes);

		GFX::Resource::DynamicBufferAlloc info
		{
			nextOffset,
				currentBlock
		};
		nextOffset += newBlock;
		return info;
	}

	void DynamicCBuffer::Bind(GFX::CommandList& cl, GFX::Binding::Context& bindCtx, const GFX::Resource::DynamicBufferAlloc& allocInfo) const noexcept
	{
		ZE_ASSERT(buffer, "Dynamic buffer has been freed already!");
		ZE_ASSERT(allocInfo.Block <= currentBlock, "Block out of range!");
		ZE_ASSERT(allocInfo.Offset < BLOCK_SIZE, "Offset out of range!");

		//const auto& schema = bindCtx.BindingSchema.Get().dx12;
		//ZE_ASSERT(schema.GetCurrentType(bindCtx.Count) == Binding::Schema::BindType::CBV,
		//	"Bind slot is not a constant buffer! Wrong root signature or order of bindings!");

		//const D3D12_GPU_VIRTUAL_ADDRESS address = resInfo.at(allocInfo.Block).second + allocInfo.Offset;
		//auto* list = cl.Get().dx12.GetList();
		//if (schema.IsCompute())
		//	list->SetComputeRootConstantBufferView(bindCtx.Count++, address);
		//else
		//	list->SetGraphicsRootConstantBufferView(bindCtx.Count++, address);
	}

	void DynamicCBuffer::StartFrame(GFX::Device& dev)
	{
		ZE_ASSERT(buffer, "Dynamic buffer has been freed already!");

		nextOffset = 0;
		const U64 blockCount = resInfo.size();
		if (blockCount > 1)
		{
			MapBlock(dev, 0);

			if (currentBlock + BLOCK_SHRINK_STEP < blockCount)
			{
				for (U64 i = currentBlock + 1; i < blockCount; ++i)
					dev.Get().vk.GetMemory().Remove(dev.Get().vk, resInfo.at(i));
				resInfo.resize(currentBlock + 1);
			}
			currentBlock = 0;
		}
	}

	void DynamicCBuffer::Free(GFX::Device& dev) noexcept
	{
		ZE_ASSERT(buffer, "Dynamic buffer has been freed already!");

		buffer = nullptr;
		for (auto& alloc : resInfo)
			dev.Get().vk.GetMemory().Remove(dev.Get().vk, alloc);
	}
}