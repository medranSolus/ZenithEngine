#pragma once
#include "GFX/CommandList.h"
#include "GFX/IndexData.h"

namespace ZE::GFX::API::VK::Resource
{
	class IndexBuffer final
	{
		VkBuffer buffer = VK_NULL_HANDLE;
		U32 count = 0;
		VkIndexType indexType;
		Allocation alloc;

	public:
		IndexBuffer() = default;
		IndexBuffer(GFX::Device& dev, const IndexData& data);
		ZE_CLASS_MOVE(IndexBuffer);
		~IndexBuffer() { ZE_ASSERT(buffer == VK_NULL_HANDLE && alloc.IsFree(), "Resource not freed before deletion!"); }

		constexpr U32 GetCount() const noexcept { return count; }

		void Free(GFX::Device& dev) noexcept { dev.Get().vk.GetMemory().Remove(dev.Get().vk, alloc); }
		void Bind(GFX::CommandList& cl) const noexcept { vkCmdBindIndexBuffer(cl.Get().vk.GetBuffer(), buffer, 0, indexType); }

		IndexData GetData(GFX::Device& dev, GFX::CommandList& cl) const;
	};
}