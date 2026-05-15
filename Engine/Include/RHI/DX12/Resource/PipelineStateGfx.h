#pragma once
#include "GFX/Binding/Schema.h"

namespace ZE::RHI::DX12::Resource
{
	class PipelineStateGfx final
	{
		DX::ComPtr<IPipelineState> state;
		D3D_PRIMITIVE_TOPOLOGY topology = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;

	public:
		PipelineStateGfx() = default;
		ZE_CLASS_MOVE(PipelineStateGfx);
		~PipelineStateGfx() { if (state) GarbageCollector::Get().Register(state); }

		static Expected<PipelineStateGfx> Create(GFX::Device& dev, const GFX::Resource::PipelineStateDesc& desc, const GFX::Binding::Schema& binding) noexcept;

		void SetStencilRef(GFX::CommandList& cl, U32 refValue) const noexcept { ZE_DX_CHECK_FAILED(cl.Get().dx12.GetList()->OMSetStencilRef(refValue), "Setting stencil ref resulted in debug layer messages!"); }

		void Bind(GFX::CommandList& cl) const noexcept;

		// Gfx API Internal

		IPipelineState* GetState() const noexcept { return state.Get(); }
		D3D_PRIMITIVE_TOPOLOGY GetTopology() const noexcept { return topology; }
	};
}