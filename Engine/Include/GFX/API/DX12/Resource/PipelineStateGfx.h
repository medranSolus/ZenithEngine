#pragma once
#include "GFX/Material/Schema.h"
#include "D3D12.h"

namespace ZE::GFX::API::DX12::Resource
{
	class PipelineStateGfx final
	{
		D3D_PRIMITIVE_TOPOLOGY topology;
		DX::ComPtr<ID3D12PipelineState> state;

	public:
		PipelineStateGfx(GFX::Device& dev, const GFX::Resource::PipelineStateDesc& desc, const GFX::Material::Schema& binding);
		ZE_CLASS_MOVE(PipelineStateGfx);
		~PipelineStateGfx() = default;

		// Gfx API Internal

		constexpr D3D_PRIMITIVE_TOPOLOGY GetTopology() const noexcept { return topology; }
		ID3D12PipelineState* GetState() const noexcept { return state.Get(); }
	};
}