#pragma once
#include "GFX/Binding/Schema.h"

namespace ZE::GFX::API::DX12::Resource
{
	class PipelineStateCompute final
	{
		DX::ComPtr<IPipelineState> state;

	public:
		PipelineStateCompute() = default;
		PipelineStateCompute(GFX::Device& dev, GFX::Resource::Shader& shader, const GFX::Binding::Schema& binding);
		ZE_CLASS_MOVE(PipelineStateCompute);
		~PipelineStateCompute() = default;

		void Bind(GFX::CommandList& cl) const noexcept { cl.Get().dx12.GetList()->SetPipelineState(GetState()); }

		// Gfx API Internal

		IPipelineState* GetState() const noexcept { return state.Get(); }
	};
}