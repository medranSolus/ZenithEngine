#pragma once
#include "GFX/Binding/Schema.h"

namespace ZE::RHI::VK::Resource
{
	class PipelineStateCompute final
	{
		VkPipeline state = VK_NULL_HANDLE;

	public:
		PipelineStateCompute() = default;
		PipelineStateCompute(GFX::Device& dev, GFX::Resource::Shader& shader, const GFX::Binding::Schema& binding);
		ZE_CLASS_MOVE(PipelineStateCompute);
		~PipelineStateCompute() { ZE_ASSERT(state == VK_NULL_HANDLE, "Pipeline not freed before deletion!"); }

		void Bind(GFX::CommandList& cl) const noexcept { vkCmdBindPipeline(cl.Get().vk.GetBuffer(), VK_PIPELINE_BIND_POINT_COMPUTE, state); }
		void Free(GFX::Device& dev) noexcept;

		// Gfx API Internal

		constexpr VkPipeline GetState() const noexcept { return state; }
	};
}