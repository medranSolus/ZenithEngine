#pragma once
#include "GFX/Device.h"
#include "D3D12.h"

namespace ZE::GFX::API::DX12::Resource
{
	class PipelineStateGfx final
	{
		DX::ComPtr<ID3D12PipelineState> state;

	public:
		PipelineStateGfx(GFX::Device& dev, const GFX::Resource::PipelineStateDesc& desc);
		PipelineStateGfx(PipelineStateGfx&&) = default;
		PipelineStateGfx(const PipelineStateGfx&) = delete;
		PipelineStateGfx& operator=(PipelineStateGfx&&) = default;
		PipelineStateGfx& operator=(const PipelineStateGfx&) = delete;
		~PipelineStateGfx() = default;

		// Gfx API Internal

		ID3D12PipelineState* GetState() const noexcept { return state.Get(); }
	};
}