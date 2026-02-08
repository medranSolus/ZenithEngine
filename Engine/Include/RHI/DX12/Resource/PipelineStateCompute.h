#pragma once
#include "GFX/Binding/Schema.h"

namespace ZE::RHI::DX12::Resource
{
	class PipelineStateCompute final
	{
		DX::ComPtr<IPipelineState> state;

	public:
		PipelineStateCompute() = default;
		ZE_CLASS_MOVE(PipelineStateCompute);
		~PipelineStateCompute() = default;

		static Expected<PipelineStateCompute> Create(GFX::Device& dev, GFX::Resource::Shader& shader, const GFX::Binding::Schema& binding) noexcept;

		void Bind(GFX::CommandList& cl) const noexcept { ZE_DX_CHECK_FAILED(cl.Get().dx12.GetList()->SetPipelineState(GetState()), "Setting compute PSO resulted in debug layer messages!"); }

		// Gfx API Internal

		IPipelineState* GetState() const noexcept { return state.Get(); }
	};
}