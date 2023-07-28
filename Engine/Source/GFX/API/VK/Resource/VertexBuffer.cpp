#include "GFX/API/VK/Resource/VertexBuffer.h"
#include "GFX/API/VK/VulkanException.h"

namespace ZE::GFX::API::VK::Resource
{
	VertexBuffer::VertexBuffer(GFX::Device& dev, const VertexData& data)
	{
		ZE_VK_ENABLE_ID();

		UploadInfoBuffer uploadInfo = {};
		uploadInfo.InitData = data.Vertices;
		uploadInfo.CreateInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, nullptr };
		uploadInfo.CreateInfo.flags = 0;
		uploadInfo.CreateInfo.size = Utils::SafeCast<VkDeviceSize>(data.Count) * data.VertexSize;
		uploadInfo.CreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		uploadInfo.CreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		uploadInfo.CreateInfo.queueFamilyIndexCount = 0;
		uploadInfo.CreateInfo.pQueueFamilyIndices = nullptr;
		ZE_VK_THROW_NOSUCC(vkCreateBuffer(dev.Get().vk.GetDevice(), &uploadInfo.CreateInfo, nullptr, &buffer));
		ZE_VK_SET_ID(dev.Get().vk.GetDevice(), buffer, VK_OBJECT_TYPE_BUFFER,
			"Vertex buffer [count:" + std::to_string(data.Count) + "] [vertex_size:" + std::to_string(data.VertexSize) + "]");

		alloc = dev.Get().vk.GetMemory().AllocBuffer(dev.Get().vk, buffer, Allocation::Usage::GPU);

		const U32 deviceIndex = 0;
		VkBindBufferMemoryDeviceGroupInfo deviceGroupInfo = { VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_DEVICE_GROUP_INFO, nullptr };
		deviceGroupInfo.deviceIndexCount = 1;
		deviceGroupInfo.pDeviceIndices = &deviceIndex;

		U8* mappedMemory = nullptr;
		uploadInfo.Dest = { VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_INFO, &deviceGroupInfo };
		uploadInfo.Dest.buffer = buffer;
		dev.Get().vk.GetMemory().GetAllocInfo(alloc, uploadInfo.Dest.memoryOffset, uploadInfo.Dest.memory, &mappedMemory);

		// Memory possible to write directly
		if (mappedMemory != nullptr)
		{
			ZE_VK_THROW_NOSUCC(vkBindBufferMemory2(dev.Get().vk.GetDevice(), 1, &uploadInfo.Dest));
			memcpy(mappedMemory, data.Vertices, uploadInfo.CreateInfo.size);
		}
		else
		{
			uploadInfo.DestStage = VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT;
			uploadInfo.DestAccess = VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT;
			dev.Get().vk.UploadBindBuffer(uploadInfo);
		}
	}

	void VertexBuffer::Bind(GFX::CommandList& cl) const noexcept
	{
		const VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers(cl.Get().vk.GetBuffer(), 0, 1, &buffer, &offset);
	}

	VertexData VertexBuffer::GetData(GFX::Device& dev, GFX::CommandList& cl) const
	{
		return {};
	}
}