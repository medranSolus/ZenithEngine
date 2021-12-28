#pragma once
#include "GFX/Material/Schema.h"
#include "D3D12.h"

namespace ZE::GFX::API::DX12::Resource
{
	class PipelineStateCompute final
	{
		DX::ComPtr<ID3D12PipelineState> state;

	public:
		PipelineStateCompute(GFX::Device& dev, GFX::Resource::Shader& shader, const GFX::Material::Schema& binding);
		ZE_CLASS_MOVE(PipelineStateCompute);
		~PipelineStateCompute() = default;

		// Gfx API Internal

		ID3D12PipelineState* GetState() const noexcept { return state.Get(); }
	};
}