#include "RHI/VK/Resource/Mesh.h"
#include "RHI/VK/VulkanException.h"

namespace ZE::RHI::VK::Resource
{
	constexpr U32 Mesh::GetIndexSize() const noexcept
	{
		switch (indexType)
		{
		case VK_INDEX_TYPE_UINT8_EXT:
			return sizeof(U8);
		case VK_INDEX_TYPE_UINT16:
			return sizeof(U16);
		case VK_INDEX_TYPE_UINT32:
			return sizeof(U32);
		default:
			return 0;
		}
	}

	Mesh::Mesh(GFX::Device& dev, IO::DiskManager& disk, const GFX::Resource::MeshData& data)
		: vertexCount(data.VertexCount), vertexSize(data.VertexSize)
	{
		ZE_VK_ENABLE_ID();
		Device& device = dev.Get().vk;

		UploadInfoBuffer uploadInfo = {};
		uploadInfo.InitData = data.PackedMesh.get();
		uploadInfo.CreateInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, nullptr };
		uploadInfo.CreateInfo.flags = 0;
		uploadInfo.CreateInfo.size = Utils::SafeCast<VkDeviceSize>(data.IndexCount * data.IndexSize + data.VertexCount * data.VertexSize);
		uploadInfo.CreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		uploadInfo.CreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		uploadInfo.CreateInfo.queueFamilyIndexCount = 0;
		uploadInfo.CreateInfo.pQueueFamilyIndices = nullptr;

		std::unique_ptr<U8[]> dataBuffer = nullptr;
		if (data.IndexCount)
		{
			// Pack mesh data into single buffer: index + vertex data
			indexCount = data.IndexCount;
			switch (data.IndexSize)
			{
				case sizeof(U8) :
				{
					indexType = VK_INDEX_TYPE_UINT8_EXT;
					break;
				}
				case sizeof(U16) :
				{
					indexType = VK_INDEX_TYPE_UINT16;
					break;
				}
				case sizeof(U32) :
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
			uploadInfo.CreateInfo.usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		}

		ZE_VK_THROW_NOSUCC(vkCreateBuffer(device.GetDevice(), &uploadInfo.CreateInfo, nullptr, &buffer));
		ZE_VK_SET_ID(device.GetDevice(), buffer, VK_OBJECT_TYPE_BUFFER,
			"Mesh geometry buffer [size:" + std::to_string(uploadInfo.CreateInfo.size) + "]");

		alloc = device.GetMemory().AllocBuffer(device, buffer, Allocation::Usage::GPU);

		const U32 deviceIndex = 0;
		VkBindBufferMemoryDeviceGroupInfo deviceGroupInfo = { VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_DEVICE_GROUP_INFO, nullptr };
		deviceGroupInfo.deviceIndexCount = 1;
		deviceGroupInfo.pDeviceIndices = &deviceIndex;

		U8* mappedMemory = nullptr;
		uploadInfo.Dest = { VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_INFO, &deviceGroupInfo };
		uploadInfo.Dest.buffer = buffer;
		device.GetMemory().GetAllocInfo(alloc, uploadInfo.Dest.memoryOffset, uploadInfo.Dest.memory, &mappedMemory);

		// Memory possible to write directly
		if (mappedMemory != nullptr)
		{
			ZE_VK_THROW_NOSUCC(vkBindBufferMemory2(dev.Get().vk.GetDevice(), 1, &uploadInfo.Dest));
			memcpy(mappedMemory, uploadInfo.InitData, uploadInfo.CreateInfo.size);
		}
		else
		{
			uploadInfo.DestStage = VK_PIPELINE_STAGE_2_INDEX_INPUT_BIT;
			uploadInfo.DestAccess = VK_ACCESS_2_INDEX_READ_BIT;
			dev.Get().vk.UploadBindBuffer(uploadInfo);
		}
	}

	Mesh::Mesh(GFX::Device& dev, IO::DiskManager& disk, const GFX::Resource::MeshFileData& data, IO::File& file)
		: vertexCount(data.VertexCount)
	{
	}

	void Mesh::Draw(GFX::Device& dev, GFX::CommandList& cl) const noexcept(!_ZE_DEBUG_GFX_API)
	{
		VkCommandBuffer cmd = cl.Get().vk.GetBuffer();

		const VkDeviceSize offset = Utils::SafeCast<VkDeviceSize>(indexCount * GetIndexSize());
		vkCmdBindVertexBuffers(cmd, 0, 1, &buffer, &offset);
		if (IsIndexBufferPresent())
		{
			vkCmdBindIndexBuffer(cmd, buffer, 0, indexType);
			vkCmdDrawIndexed(cmd, indexCount, 1, 0, 0, 0);
		}
		else
			vkCmdDraw(cmd, vertexCount, 1, 0, 0);
	}

	GFX::Resource::MeshData Mesh::GetData(GFX::Device& dev, GFX::CommandList& cl) const
	{
		return { INVALID_EID, nullptr, 0, 0, 0, 0 };
	}
}