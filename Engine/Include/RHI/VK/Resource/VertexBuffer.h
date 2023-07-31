#pragma once
#include "GFX/CommandList.h"
#include "GFX/VertexData.h"

namespace ZE::RHI::VK::Resource
{
	class VertexBuffer final
	{
		VkBuffer buffer = VK_NULL_HANDLE;
		Allocation alloc;

	public:
		VertexBuffer() = default;
		VertexBuffer(GFX::Device& dev, const GFX::VertexData& data);
		ZE_CLASS_MOVE(VertexBuffer);
		~VertexBuffer() { ZE_ASSERT(buffer == VK_NULL_HANDLE && alloc.IsFree(), "Resource not freed before deletion!"); }

		void Free(GFX::Device& dev) noexcept { dev.Get().vk.GetMemory().Remove(dev.Get().vk, alloc); }

		void Bind(GFX::CommandList& cl) const noexcept;
		GFX::VertexData GetData(GFX::Device& dev, GFX::CommandList& cl) const;
	};
}