#pragma once
#include "GFX/Binding/Schema.h"

namespace ZE::GFX::API::VK::Resource
{
	class PipelineStateGfx final
	{
		VkPipeline state = VK_NULL_HANDLE;

	public:
		PipelineStateGfx() = default;
		PipelineStateGfx(GFX::Device& dev, const GFX::Resource::PipelineStateDesc& desc, const GFX::Binding::Schema& binding);
		ZE_CLASS_MOVE(PipelineStateGfx);
		~PipelineStateGfx() { ZE_ASSERT(state == VK_NULL_HANDLE, "Pipeline not freed before deletion!"); }

		void SetStencilRef(GFX::CommandList& cl, U32 refValue) const noexcept {}
		void Bind(GFX::CommandList& cl) const noexcept { vkCmdBindPipeline(cl.Get().vk.GetBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, state); }

		void Free(GFX::Device& dev) noexcept;

		// Gfx API Internal

		constexpr VkPipeline GetState() const noexcept { return state; }
	};
}