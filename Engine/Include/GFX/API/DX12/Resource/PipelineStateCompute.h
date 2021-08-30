#pragma once
#include "GFX/Device.h"
#include "D3D12.h"

namespace ZE::GFX::API::DX12::Resource
{
	class PipelineStateCompute final
	{
		DX::ComPtr<ID3D12PipelineState> state;

	public:
		PipelineStateCompute(GFX::Device& dev, const std::wstring& nameCS);
		PipelineStateCompute(PipelineStateCompute&&) = default;
		PipelineStateCompute(const PipelineStateCompute&) = delete;
		PipelineStateCompute& operator=(PipelineStateCompute&&) = default;
		PipelineStateCompute& operator=(const PipelineStateCompute&) = delete;
		~PipelineStateCompute() = default;

		// Gfx API Internal

		ID3D12PipelineState* GetState() const noexcept { return state.Get(); }
	};
}