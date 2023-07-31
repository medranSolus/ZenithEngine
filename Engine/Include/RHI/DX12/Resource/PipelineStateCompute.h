#pragma once
#include "GFX/Binding/Schema.h"

namespace ZE::RHI::DX12::Resource
{
	class PipelineStateCompute final
	{
		DX::ComPtr<IPipelineState> state;

	public:
		PipelineStateCompute() = default;
		PipelineStateCompute(GFX::Device& dev, GFX::Resource::Shader& shader, const GFX::Binding::Schema& binding);
		ZE_CLASS_MOVE(PipelineStateCompute);
		~PipelineStateCompute() { ZE_ASSERT(state == nullptr, "Pipeline not freed before deletion!"); }

		void Bind(GFX::CommandList& cl) const noexcept { cl.Get().dx12.GetList()->SetPipelineState(GetState()); }
		void Free(GFX::Device& dev) noexcept { state = nullptr; }

		// Gfx API Internal

		IPipelineState* GetState() const noexcept { return state.Get(); }
	};
}