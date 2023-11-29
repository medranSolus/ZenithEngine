#include "RHI/VK/Resource/CBuffer.h"
#include "RHI/VK/VulkanException.h"

namespace ZE::RHI::VK::Resource
{
	CBuffer::CBuffer(GFX::Device& dev, IO::DiskManager& disk, const GFX::Resource::CBufferData& data)
	{
		ZE_VK_ENABLE_ID();
		lastUsedQueue = dev.Get().vk.GetGfxQueueIndex();

		UploadInfoBuffer uploadInfo = {};
		uploadInfo.InitData = data.Data;
		uploadInfo.CreateInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, nullptr };
		uploadInfo.CreateInfo.flags = 0;
		uploadInfo.CreateInfo.size = data.Bytes;
		uploadInfo.CreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		uploadInfo.CreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		uploadInfo.CreateInfo.queueFamilyIndexCount = 0;
		uploadInfo.CreateInfo.pQueueFamilyIndices = nullptr;
		ZE_VK_THROW_NOSUCC(vkCreateBuffer(dev.Get().vk.GetDevice(), &uploadInfo.CreateInfo, nullptr, &buffer));
		ZE_VK_SET_ID(dev.Get().vk.GetDevice(), buffer, VK_OBJECT_TYPE_BUFFER,
			"CBuffer [size:" + std::to_string(data.Bytes) + "]");

		alloc = dev.Get().vk.GetMemory().AllocBuffer(dev.Get().vk, buffer, Allocation::Usage::GPU);

		const U32 deviceIndex = 0;
		VkBindBufferMemoryDeviceGroupInfo deviceGroupInfo = { VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_DEVICE_GROUP_INFO, nullptr };
		deviceGroupInfo.deviceIndexCount = 1;
		deviceGroupInfo.pDeviceIndices = &deviceIndex;

		uploadInfo.Dest = { VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_INFO, &deviceGroupInfo };
		uploadInfo.Dest.buffer = buffer;

		U8* mappedMemory = nullptr;
		dev.Get().vk.GetMemory().GetAllocInfo(alloc, uploadInfo.Dest.memoryOffset, uploadInfo.Dest.memory, &mappedMemory);

		// Memory possible to write directly
		if (mappedMemory != nullptr)
		{
			ZE_VK_THROW_NOSUCC(vkBindBufferMemory2(dev.Get().vk.GetDevice(), 1, &uploadInfo.Dest));
			std::memcpy(mappedMemory, data.Data, data.Bytes);
		}
		else
		{
			// Sync with all supported shader stages since can be used anywhere (but mostly VS, PS or CS)
			uploadInfo.DestStage = USAGE_STAGE;
			uploadInfo.DestAccess = USAGE_ACCESS;
			dev.Get().vk.UploadBindBuffer(uploadInfo);
		}
	}

	CBuffer::CBuffer(GFX::Device& dev, IO::DiskManager& disk, const GFX::Resource::CBufferFileData& data, IO::File& file)
	{
	}

	void CBuffer::Update(GFX::Device& dev, IO::DiskManager& disk, const GFX::Resource::CBufferData& data) const
	{
		U8* memory = dev.Get().vk.GetMemory().GetMappedMemory(alloc);
		if (memory)
			memcpy(memory, data.Data, data.Bytes);
		else
		{
			UploadInfoBufferUpdate updateInfo = {};
			updateInfo.Buffer = buffer;
			updateInfo.LastUsedQueue = lastUsedQueue;
			updateInfo.Data = data.Data;
			updateInfo.Bytes = data.Bytes;
			updateInfo.DestStage = USAGE_STAGE;
			updateInfo.DestAccess = USAGE_ACCESS;
			dev.Get().vk.UpdateBuffer(updateInfo);
		}
	}

	void CBuffer::Bind(GFX::CommandList& cl, GFX::Binding::Context& bindCtx) const noexcept
	{
		// Transfer ownership to new queue
		if (lastUsedQueue != cl.Get().vk.GetFamily())
		{
			VkBufferMemoryBarrier2 barrier = { VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2, nullptr };
			barrier.srcStageMask = USAGE_STAGE;
			barrier.srcAccessMask = USAGE_ACCESS;
			barrier.srcQueueFamilyIndex = lastUsedQueue;
			barrier.buffer = buffer;
			cl.Get().vk.TransferOwnership(barrier);
			lastUsedQueue = cl.Get().vk.GetFamily();
		}
	}

	void CBuffer::GetData(GFX::Device& dev, void* values, U32 bytes) const
	{
	}
}