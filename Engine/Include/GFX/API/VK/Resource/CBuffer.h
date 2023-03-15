#pragma once
#include "GFX/Binding/Context.h"
#include "GFX/CommandList.h"

namespace ZE::GFX::API::VK::Resource
{
	class CBuffer final
	{
		static constexpr VkPipelineStageFlags2 USAGE_STAGE = VK_PIPELINE_STAGE_2_PRE_RASTERIZATION_SHADERS_BIT | VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
		static constexpr VkAccessFlags2 USAGE_ACCESS = VK_ACCESS_2_UNIFORM_READ_BIT;

		VkBuffer buffer = VK_NULL_HANDLE;
		Allocation alloc;
		mutable U32 lastUsedQueue;

	public:
		CBuffer() = default;
		CBuffer(GFX::Device& dev, const void* values, U32 bytes);
		ZE_CLASS_MOVE(CBuffer);
		~CBuffer() { ZE_ASSERT(buffer == VK_NULL_HANDLE && alloc.IsFree(), "Resource not freed before deletion!"); }

		void Free(GFX::Device& dev) noexcept { dev.Get().vk.GetMemory().Remove(dev.Get().vk, alloc); }

		void Update(GFX::Device& dev, const void* values, U32 bytes) const;
		void Bind(GFX::CommandList& cl, GFX::Binding::Context& bindCtx) const noexcept;
		void GetData(GFX::Device& dev, void* values, U32 bytes) const;
	};
}