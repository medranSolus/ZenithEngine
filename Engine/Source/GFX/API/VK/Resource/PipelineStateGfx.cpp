#include "GFX/API/VK/Resource/PipelineStateGfx.h"

namespace ZE::GFX::API::VK::Resource
{
	PipelineStateGfx::PipelineStateGfx(GFX::Device& dev, const GFX::Resource::PipelineStateDesc& desc, const GFX::Binding::Schema& binding)
	{
	}

	void PipelineStateGfx::Free(GFX::Device& dev) noexcept
	{
		if (state)
		{
			vkDestroyPipeline(dev.Get().vk.GetDevice(), state, nullptr);
			state = VK_NULL_HANDLE;
		}
	}
}