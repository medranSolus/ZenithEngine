#pragma once
#include "GFX/CommandList.h"
#include "GFX/MeshData.h"

namespace ZE::RHI::VK::Resource
{
	class Mesh final
	{
		VkBuffer buffer = VK_NULL_HANDLE;
		U32 vertexCount = 0;
		U32 indexCount = 0;
		VkIndexType indexType = VK_INDEX_TYPE_NONE_KHR;
		Allocation alloc;

		constexpr bool IsIndexBufferPresent() const noexcept { return indexCount != 0; }
		constexpr U32 GetIndexSize() const noexcept;

	public:
		Mesh() = default;
		Mesh(GFX::Device& dev, const GFX::MeshData& data);
		ZE_CLASS_MOVE(Mesh);
		~Mesh() { ZE_ASSERT_FREED(alloc.IsFree() && buffer == VK_NULL_HANDLE); }

		constexpr U32 GetVertexCount() const noexcept { return vertexCount; }
		constexpr U32 GetIndexCount() const noexcept { return indexCount; }
		void Free(GFX::Device& dev) noexcept { dev.Get().vk.GetMemory().Remove(dev.Get().vk, alloc); }

		void Draw(GFX::Device& dev, GFX::CommandList& cl) const noexcept(!_ZE_DEBUG_GFX_API);
		GFX::MeshData GetData(GFX::Device& dev, GFX::CommandList& cl) const;
	};
}