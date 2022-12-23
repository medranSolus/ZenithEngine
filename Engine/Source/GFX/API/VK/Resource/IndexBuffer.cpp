#include "GFX/API/VK/Resource/IndexBuffer.h"
#include "GFX/API/VK/VulkanException.h"

namespace ZE::GFX::API::VK::Resource
{
	IndexBuffer::IndexBuffer(GFX::Device& dev, const IndexData& data)
	{
		ZE_VK_ENABLE_ID();

		count = data.Count;
		switch (data.IndexSize)
		{
		case sizeof(U8):
		{
			indexType = VK_INDEX_TYPE_UINT8_EXT;
			break;
		}
		case sizeof(U16):
		{
			indexType = VK_INDEX_TYPE_UINT16;
			break;
		}
		case sizeof(U32):
		{
			indexType = VK_INDEX_TYPE_UINT32;
			break;
		}
		default:
		{
			ZE_FAIL("Only 8, 16 and 32 bit indices are supported for Vulkan!");
			break;
		}
		}

		UploadInfoBuffer uploadInfo;
		uploadInfo.InitData = data.Indices;
		uploadInfo.CreateInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, nullptr };
		uploadInfo.CreateInfo.flags = 0;
		uploadInfo.CreateInfo.size = static_cast<VkDeviceSize>(data.Count) * data.IndexSize;
		uploadInfo.CreateInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		uploadInfo.CreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		uploadInfo.CreateInfo.queueFamilyIndexCount = 0;
		uploadInfo.CreateInfo.pQueueFamilyIndices = nullptr;
		ZE_VK_THROW_NOSUCC(vkCreateBuffer(dev.Get().vk.GetDevice(), &uploadInfo.CreateInfo, nullptr, &buffer));
		ZE_VK_SET_ID(dev.Get().vk.GetDevice(), buffer, VK_OBJECT_TYPE_BUFFER,
			"Index buffer [count:" + std::to_string(data.Count) + "] [index_size:" + std::to_string(data.IndexSize) + "]");

		alloc = dev.Get().vk.GetMemory().AllocBuffer(dev.Get().vk, buffer, Allocation::Usage::GPU);

		U8* mappedMemory = nullptr;
		uploadInfo.Dest = { VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_INFO, nullptr };
		uploadInfo.Dest.buffer = buffer;
		dev.Get().vk.GetMemory().GetAllocInfo(alloc, uploadInfo.Dest.memoryOffset, uploadInfo.Dest.memory, &mappedMemory);

		// Memory possible to write directly
		if (mappedMemory != nullptr)
		{
			ZE_VK_THROW_NOSUCC(vkBindBufferMemory2(dev.Get().vk.GetDevice(), 1, &uploadInfo.Dest));
			memcpy(mappedMemory, data.Indices, uploadInfo.CreateInfo.size);
		}
		else
		{
			uploadInfo.DestStage = VK_PIPELINE_STAGE_2_INDEX_INPUT_BIT;
			uploadInfo.DestAccess = VK_ACCESS_2_INDEX_READ_BIT;
			dev.Get().vk.UploadBindBuffer(uploadInfo);
		}
	}

	IndexData IndexBuffer::GetData(GFX::Device& dev, GFX::CommandList& cl) const
	{
		return {};
	}
}