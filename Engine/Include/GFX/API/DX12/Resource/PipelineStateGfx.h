#pragma once
#include "GFX/Binding/Schema.h"
#include "D3D12.h"

namespace ZE::GFX::API::DX12::Resource
{
	class PipelineStateGfx final
	{
		DX::ComPtr<ID3D12PipelineState> state;
		D3D_PRIMITIVE_TOPOLOGY topology;

	public:
		PipelineStateGfx(GFX::Device& dev, const GFX::Resource::PipelineStateDesc& desc, const GFX::Binding::Schema& binding);
		ZE_CLASS_MOVE(PipelineStateGfx);
		~PipelineStateGfx() = default;

		void Bind(GFX::CommandList& cl) const noexcept;

		// Gfx API Internal

		ID3D12PipelineState* GetState() const noexcept { return state.Get(); }
		D3D_PRIMITIVE_TOPOLOGY GetTopology() const noexcept { return topology; }
	};
}