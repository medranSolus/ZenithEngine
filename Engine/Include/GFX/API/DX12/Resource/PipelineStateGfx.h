#pragma once
#include "GFX/Resource/DataBinding.h"
#include "D3D12.h"

namespace ZE::GFX::API::DX12::Resource
{
	class PipelineStateGfx final
	{
		DX::ComPtr<ID3D12PipelineState> state;

	public:
		PipelineStateGfx(GFX::Device& dev, const GFX::Resource::PipelineStateDesc& desc, const GFX::Resource::DataBinding& binding);
		ZE_CLASS_MOVE(PipelineStateGfx);
		~PipelineStateGfx() = default;

		// Gfx API Internal

		ID3D12PipelineState* GetState() const noexcept { return state.Get(); }
	};
}