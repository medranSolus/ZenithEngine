#pragma once
#include "GFX/Binding/Schema.h"

namespace ZE::GFX::API::VK::Resource
{
	class PipelineStateGfx final
	{
		VkPipeline state = VK_NULL_HANDLE;
		VkPrimitiveTopology topology;
		VkStencilFaceFlags stencilFace;

	public:
		PipelineStateGfx() = default;
		PipelineStateGfx(GFX::Device& dev, const GFX::Resource::PipelineStateDesc& desc, const GFX::Binding::Schema& binding);
		ZE_CLASS_MOVE(PipelineStateGfx);
		~PipelineStateGfx() { ZE_ASSERT(state == VK_NULL_HANDLE, "Pipeline not freed before deletion!"); }

		void SetStencilRef(GFX::CommandList& cl, U32 refValue) const noexcept { vkCmdSetStencilReference(cl.Get().vk.GetBuffer(), stencilFace, refValue); }

		void Bind(GFX::CommandList& cl) const noexcept;
		void Free(GFX::Device& dev) noexcept;

		// Gfx API Internal

		constexpr VkPipeline GetState() const noexcept { return state; }
		constexpr VkPrimitiveTopology GetTopology() const noexcept { return topology; }
	};
}