#pragma once
#include "GFX/Binding/Schema.h"

namespace ZE::GFX::API::VK::Resource
{
	class PipelineStateGfx final
	{
	public:
		PipelineStateGfx() = default;
		PipelineStateGfx(GFX::Device& dev, const GFX::Resource::PipelineStateDesc& desc, const GFX::Binding::Schema& binding);
		ZE_CLASS_MOVE(PipelineStateGfx);
		~PipelineStateGfx() = default;

		void SetStencilRef(GFX::CommandList& cl, U32 refValue) const noexcept {}
		void Bind(GFX::CommandList& cl) const noexcept {}
	};
}