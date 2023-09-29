#pragma once
#include "GFX/Binding/Schema.h"

namespace ZE::RHI::DX12::Resource
{
	class PipelineStateGfx final
	{
		DX::ComPtr<IPipelineState> state;
		D3D_PRIMITIVE_TOPOLOGY topology;

	public:
		PipelineStateGfx() = default;
		PipelineStateGfx(GFX::Device& dev, const GFX::Resource::PipelineStateDesc& desc, const GFX::Binding::Schema& binding);
		ZE_CLASS_MOVE(PipelineStateGfx);
		~PipelineStateGfx() { ZE_ASSERT_FREED(state == nullptr); }

		void SetStencilRef(GFX::CommandList& cl, U32 refValue) const noexcept { cl.Get().dx12.GetList()->OMSetStencilRef(refValue); }
		void Free(GFX::Device& dev) noexcept { state = nullptr; }

		void Bind(GFX::CommandList& cl) const noexcept;

		// Gfx API Internal

		IPipelineState* GetState() const noexcept { return state.Get(); }
		D3D_PRIMITIVE_TOPOLOGY GetTopology() const noexcept { return topology; }
	};
}