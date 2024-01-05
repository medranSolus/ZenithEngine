#pragma once
#include "GFX/Resource/MeshData.h"
#include "GFX/CommandList.h"
#include "IO/File.h"

namespace ZE::RHI::VK::Resource
{
	class Mesh final
	{
		VkBuffer buffer = VK_NULL_HANDLE;
		U32 vertexCount = 0;
		U32 indexCount = 0;
		U16 vertexSize = 0;
		VkIndexType indexType = VK_INDEX_TYPE_NONE_KHR;
		Allocation alloc;

		constexpr bool IsIndexBufferPresent() const noexcept { return indexCount != 0; }
		constexpr U32 GetIndexSize() const noexcept;

	public:
		Mesh() = default;
		Mesh(GFX::Device& dev, IO::DiskManager& disk, const GFX::Resource::MeshData& data);
		Mesh(GFX::Device& dev, IO::DiskManager& disk, const GFX::Resource::MeshFileData& data, IO::File& file);
		ZE_CLASS_MOVE(Mesh);
		~Mesh() { ZE_ASSERT_FREED(alloc.IsFree() && buffer == VK_NULL_HANDLE); }

		constexpr U32 GetSize() const noexcept { return Math::AlignUp(indexCount * GetIndexSize(), GFX::Resource::MeshData::VERTEX_BUFFER_ALIGNMENT) + vertexCount * vertexSize; }
		constexpr U32 GetVertexCount() const noexcept { return vertexCount; }
		constexpr U32 GetIndexCount() const noexcept { return indexCount; }
		constexpr U16 GetVertexSize() const noexcept { return vertexSize; }
		constexpr PixelFormat GetIndexFormat() const noexcept;
		void Free(GFX::Device& dev) noexcept { dev.Get().vk.GetMemory().Remove(dev.Get().vk, alloc); }

		void Draw(GFX::Device& dev, GFX::CommandList& cl) const noexcept(!_ZE_DEBUG_GFX_API);
		GFX::Resource::MeshData GetData(GFX::Device& dev, GFX::CommandList& cl) const;
	};

#pragma region Functions
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

	constexpr PixelFormat Mesh::GetIndexFormat() const noexcept
	{
		switch (indexType)
		{
		default:
			ZE_FAIL("Unknown type of index format!");
		case VK_INDEX_TYPE_UINT32:
			return PixelFormat::R32_UInt;
		case VK_INDEX_TYPE_UINT16:
			return PixelFormat::R16_UInt;
		case VK_INDEX_TYPE_UINT8_EXT:
			return PixelFormat::R8_UInt;
		}
	}
#pragma endregion
}